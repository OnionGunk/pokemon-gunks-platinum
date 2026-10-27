#include "dexnav_search.h"

#include <nitro.h>
#include <string.h>

#include "constants/charcode.h"
#include "constants/field/window.h"
#include "constants/savedata/save_table.h"
#include "constants/species.h"

#include "field/field_system.h"
#include "overlay005/ov5_021F2D20.h"

#include "bg_window.h"
#include "font.h"
#include "heap.h"
#include "inlines.h"
#include "map_matrix.h"
#include "map_tile_behavior.h"
#include "math_util.h"
#include "message.h"
#include "game_options.h"
#include "message_util.h"
#include "overworld_anim_manager.h"
#include "player_avatar.h"
#include "render_window.h"
#include "save_player.h"
#include "string_gf.h"
#include "terrain_collision_manager.h"
#include "text.h"
#include "trainer_info.h"

#define SEARCH_MIN_DISTANCE    2
#define SEARCH_MAX_DISTANCE    4
#define SEARCH_LOST_DISTANCE   8
#define SEARCH_PLACE_ATTEMPTS  40
#define PATCH_SHAKE_SOFT       0
#define PATCH_SHAKE_SHINY      2

#define MAX_SEARCH_LEVEL       255
#define NUM_STATS              6
#define MAX_PERFECT_IVS        4
#define SHINY_ODDS             4096

#define HUD_LEFT               2
#define HUD_TOP                19
#define HUD_WIDTH              27
#define HUD_HEIGHT             4

// The bar sits exactly where the field message box goes and borrows its tiles and palettes
// (see field_message.c), so it costs no extra VRAM; it is redrawn after any message closes.
#define FIELD_MSGBOX_BASE_TILE (1024 - (18 + 12))
#define FIELD_MSGBOX_PALETTE   10
#define FIELD_MSG_TEXT_PALETTE 12
#define FIELD_MSG_BASE_TILE    (((1024 - (18 + 12) - 9 - (32 * 8)) - (18 + 12 + 24)) - (27 * 4))

typedef struct DexNavSearch {
    u16 species;
    u8 level;
    BOOL requested;
    BOOL active;
    int x;
    int z;
    int mapHeaderID;
    OverworldAnimManager *anim;
    Window *hud;
    // rolled when the search starts so the bar can show them before the battle
    u32 personality;
    u8 ivs[NUM_STATS];
    u8 perfectIVs;
    BOOL shiny;
    u8 searchLevel;
    // consecutive searches for the same species on the same map
    u16 chainSpecies;
    int chainMapHeaderID;
    u16 chain;
} DexNavSearch;

// Lives in the main binary so it survives the field unloading for the DexNav screen and battles.
static DexNavSearch sSearch;

// Test hooks: the emulator harness writes these before a new game. gTestLeadSpecies picks the
// test save's lead Pokemon; gTestSaveMode picks post-game (0) or story start (1).
u16 gTestLeadSpecies;
u16 gTestSaveMode;

int DexNavSave_SaveSize(void)
{
    return sizeof(DexNavSave);
}

void DexNavSave_Init(DexNavSave *dexNavSave)
{
    memset(dexNavSave, 0, sizeof(DexNavSave));
}

int DexNavLevels_SaveSize(void)
{
    return sizeof(DexNavLevels);
}

void DexNavLevels_Init(DexNavLevels *dexNavLevels)
{
    memset(dexNavLevels, 0, sizeof(DexNavLevels));
}

static DexNavSave *GetDexNavSave(SaveData *saveData)
{
    return SaveData_SaveTable(saveData, SAVE_TABLE_ENTRY_DEXNAV);
}

static DexNavLevels *GetDexNavLevels(SaveData *saveData)
{
    return SaveData_SaveTable(saveData, SAVE_TABLE_ENTRY_DEXNAV_LEVELS);
}

u8 DexNavSave_GetSearchLevel(SaveData *saveData, u16 species)
{
    return species < MAX_SPECIES ? GetDexNavLevels(saveData)->searchLevel[species] : 0;
}

u16 DexNavSave_GetSelectedSpecies(SaveData *saveData)
{
    return GetDexNavSave(saveData)->selectedSpecies;
}

void DexNavSave_SetSelectedSpecies(SaveData *saveData, u16 species)
{
    GetDexNavSave(saveData)->selectedSpecies = species;
}

static void DexNavSave_IncrementSearchLevel(SaveData *saveData, u16 species)
{
    DexNavLevels *levels = GetDexNavLevels(saveData);

    if (species < MAX_SPECIES && levels->searchLevel[species] < MAX_SEARCH_LEVEL) {
        levels->searchLevel[species]++;
        // the box block is only written on full saves, the same way PC box changes are handled
        SaveData_SetFullSaveRequired();
    }
}

static int Abs(int value)
{
    return value < 0 ? -value : value;
}

static u32 Random32(void)
{
    return (LCRNG_Next() << 16) | LCRNG_Next();
}

static BOOL IsSearchableTile(FieldSystem *fieldSystem, int x, int z, fx32 playerHeight)
{
    if (!TileBehavior_IsTallGrass(TerrainCollisionManager_GetTileBehavior(fieldSystem, x, z))) {
        return FALSE;
    }

    u8 heightSource;
    fx32 height = TerrainCollisionManager_GetHeight(fieldSystem, 0, FX32_ONE * 16 * x, FX32_ONE * 16 * z, &heightSource);

    if (height != playerHeight) {
        return FALSE;
    }

    return MapMatrix_GetMapHeaderIDAtCoords(fieldSystem->mapMatrix, x / 32, z / 32) == fieldSystem->location->mapHeaderID;
}

// Higher search levels and longer chains give more perfect IVs and more shiny rolls.
static void RollHiddenMon(FieldSystem *fieldSystem)
{
    u32 otID = TrainerInfo_ID(SaveData_GetTrainerInfo(fieldSystem->saveData));
    int shinyRolls = 1 + sSearch.searchLevel / 10 + sSearch.chain / 5;
    int stat;

    sSearch.perfectIVs = sSearch.searchLevel / 5 + (sSearch.chain >= 10 ? 1 : 0);

    if (sSearch.perfectIVs > MAX_PERFECT_IVS) {
        sSearch.perfectIVs = MAX_PERFECT_IVS;
    }

    sSearch.shiny = FALSE;

    for (int i = 0; i < shinyRolls; i++) {
        if (LCRNG_RandMod(SHINY_ODDS) == 0) {
            sSearch.shiny = TRUE;
            break;
        }
    }

    sSearch.personality = Random32();

    if (sSearch.shiny) {
        // Gen 4 shininess: (otID high ^ otID low ^ pid high ^ pid low) < 8
        u16 low = sSearch.personality & 0xFFFF;
        u16 high = (otID >> 16) ^ (otID & 0xFFFF) ^ low;
        sSearch.personality = ((u32)high << 16) | low;
    }

    for (stat = 0; stat < NUM_STATS; stat++) {
        sSearch.ivs[stat] = LCRNG_RandMod(32);
    }

    for (int i = 0; i < sSearch.perfectIVs; i++) {
        do {
            stat = LCRNG_RandMod(NUM_STATS);
        } while (sSearch.ivs[stat] == 31 && i < NUM_STATS);

        sSearch.ivs[stat] = 31;
    }
}

static u16 HiddenMonAbility(void)
{
    u16 ability1 = SpeciesData_GetSpeciesValue(sSearch.species, SPECIES_DATA_ABILITY_1);
    u16 ability2 = SpeciesData_GetSpeciesValue(sSearch.species, SPECIES_DATA_ABILITY_2);

    return (ability2 != 0 && (sSearch.personality & 1)) ? ability2 : ability1;
}

static void AppendAscii(String *dst, const char *src)
{
    for (; *src != '\0'; src++) {
        char c = *src;

        if (c >= 'A' && c <= 'Z') {
            String_AppendChar(dst, CHAR_A + (c - 'A'));
        } else if (c >= 'a' && c <= 'z') {
            String_AppendChar(dst, CHAR_a + (c - 'a'));
        } else if (c >= '0' && c <= '9') {
            String_AppendChar(dst, CHAR_0 + (c - '0'));
        } else if (c == '.') {
            String_AppendChar(dst, CHAR_PERIOD);
        } else if (c == '/') {
            String_AppendChar(dst, CHAR_SLASH);
        } else {
            String_AppendChar(dst, CHAR_SPACE);
        }
    }
}

static void AppendNumber(String *dst, int num, enum HeapID heapID)
{
    String *numStr = String_Init(8, heapID);

    String_FormatInt(numStr, num, 3, PADDING_MODE_NONE, CHARSET_MODE_EN);
    String_Concat(dst, numStr);
    String_Free(numStr);
}

static void AppendFromBank(String *dst, u32 bankID, u32 entryID, enum HeapID heapID)
{
    MessageLoader *loader = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, bankID, heapID);
    String *entry = MessageLoader_GetNewString(loader, entryID);

    String_Concat(dst, entry);
    String_Free(entry);
    MessageLoader_Free(loader);
}

// right-aligns against the bar's inner edge using the font's own metrics
static void PrintRight(String *string, int y)
{
    int width = Font_CalcStringWidth(FONT_SYSTEM, string, Font_GetAttribute(FONT_SYSTEM, FONTATTR_LETTER_SPACING));
    Text_AddPrinterWithParams(sSearch.hud, FONT_SYSTEM, string, HUD_WIDTH * 8 - width - 2, y, TEXT_SPEED_NO_TRANSFER, NULL);
}

static void DrawHud(FieldSystem *fieldSystem)
{
    enum HeapID heapID = HEAP_ID_FIELD1;

    if (sSearch.hud == NULL) {
        sSearch.hud = Heap_Alloc(heapID, sizeof(Window));
        Window_Add(fieldSystem->bgConfig, sSearch.hud, BG_LAYER_MAIN_3, HUD_LEFT, HUD_TOP, HUD_WIDTH, HUD_HEIGHT, FIELD_MSG_TEXT_PALETTE, FIELD_MSG_BASE_TILE);
        // same frame graphics the field loads for its message boxes, in the player's chosen style
        LoadMessageBoxGraphics(fieldSystem->bgConfig, BG_LAYER_MAIN_3, FIELD_MSGBOX_BASE_TILE, FIELD_MSGBOX_PALETTE, Options_Frame(SaveData_GetOptions(fieldSystem->saveData)), heapID);
    }

    Window_FillTilemap(sSearch.hud, Font_GetAttribute(FONT_SYSTEM, FONTATTR_BG_COLOR));

    // line 1: species, level, shiny mark, search level and chain
    String *line = String_Init(96, heapID);
    String *name = MessageUtil_SpeciesName(sSearch.species, heapID);

    String_Concat(line, name);
    String_Free(name);
    AppendAscii(line, "  Lv.");
    AppendNumber(line, sSearch.level, heapID);

    if (sSearch.shiny) {
        String_AppendChar(line, CHAR_SPACE);
        String_AppendChar(line, CHAR_STAR);
    }

    Text_AddPrinterWithParams(sSearch.hud, FONT_SYSTEM, line, 0, 0, TEXT_SPEED_NO_TRANSFER, NULL);

    String_Clear(line);
    AppendAscii(line, "Search Lv.");
    AppendNumber(line, sSearch.searchLevel, heapID);
    PrintRight(line, 0);

    // line 2: nature, ability, and potential (one star per guaranteed perfect IV)
    String_Clear(line);
    AppendFromBank(line, TEXT_BANK_NATURE_NAMES, Pokemon_GetNatureOf(sSearch.personality), heapID);
    AppendAscii(line, "  ");
    AppendFromBank(line, TEXT_BANK_ABILITY_NAMES, HiddenMonAbility(), heapID);
    Text_AddPrinterWithParams(sSearch.hud, FONT_SYSTEM, line, 0, 16, TEXT_SPEED_NO_TRANSFER, NULL);

    // chain, then potential: one star per guaranteed perfect IV
    String_Clear(line);
    AppendAscii(line, "Chain ");
    AppendNumber(line, sSearch.chain, heapID);
    String_AppendChar(line, CHAR_SPACE);
    String_AppendChar(line, CHAR_SPACE);

    for (int i = 0; i < MAX_PERFECT_IVS; i++) {
        String_AppendChar(line, i < sSearch.perfectIVs ? CHAR_STAR : CHAR_MINUS);
    }

    PrintRight(line, 16);
    String_Free(line);

    Window_DrawMessageBox(sSearch.hud, FIELD_MSGBOX_BASE_TILE, FIELD_MSGBOX_PALETTE);
    Window_ScheduleCopyToVRAM(sSearch.hud);
}

static void RemoveHud(void)
{
    if (sSearch.hud != NULL) {
        Window_EraseMessageBox(sSearch.hud, FALSE);
        Window_ClearAndCopyToVRAM(sSearch.hud);
        Window_Remove(sSearch.hud);
        Heap_Free(sSearch.hud);
        sSearch.hud = NULL;
    }
}

void DexNavSearch_Request(u16 species, u8 level)
{
    sSearch.species = species;
    sSearch.level = level;
    sSearch.requested = TRUE;
    // any earlier patch and bar belonged to the field that was just torn down
    sSearch.active = FALSE;
    sSearch.anim = NULL;
    sSearch.hud = NULL;
}

BOOL DexNavSearch_TryStart(FieldSystem *fieldSystem)
{
    if (!sSearch.requested) {
        return FALSE;
    }

    sSearch.requested = FALSE;

    int playerX = PlayerAvatar_GetXPos(fieldSystem->playerAvatar);
    int playerZ = PlayerAvatar_GetZPos(fieldSystem->playerAvatar);
    fx32 playerHeight = PlayerAvatar_GetPos(fieldSystem->playerAvatar)->y;
    int span = SEARCH_MAX_DISTANCE * 2 + 1;

    for (int attempt = 0; attempt < SEARCH_PLACE_ATTEMPTS; attempt++) {
        int dx = LCRNG_RandMod(span) - SEARCH_MAX_DISTANCE;
        int dz = LCRNG_RandMod(span) - SEARCH_MAX_DISTANCE;

        if (Abs(dx) < SEARCH_MIN_DISTANCE && Abs(dz) < SEARCH_MIN_DISTANCE) {
            continue;
        }

        if (IsSearchableTile(fieldSystem, playerX + dx, playerZ + dz, playerHeight)) {
            sSearch.x = playerX + dx;
            sSearch.z = playerZ + dz;
            sSearch.mapHeaderID = fieldSystem->location->mapHeaderID;

            if (sSearch.chainSpecies != sSearch.species || sSearch.chainMapHeaderID != sSearch.mapHeaderID) {
                sSearch.chain = 0;
            }

            sSearch.searchLevel = DexNavSave_GetSearchLevel(fieldSystem->saveData, sSearch.species);
            RollHiddenMon(fieldSystem);

            sSearch.anim = ov5_021F3154(fieldSystem, sSearch.x, sSearch.z, sSearch.shiny ? PATCH_SHAKE_SHINY : PATCH_SHAKE_SOFT);
            sSearch.active = TRUE;
            DrawHud(fieldSystem);
            return TRUE;
        }
    }

    return FALSE;
}

void DexNavSearch_Update(FieldSystem *fieldSystem)
{
    if (!sSearch.active) {
        return;
    }

    int playerX = PlayerAvatar_GetXPos(fieldSystem->playerAvatar);
    int playerZ = PlayerAvatar_GetZPos(fieldSystem->playerAvatar);

    if (fieldSystem->location->mapHeaderID != sSearch.mapHeaderID
        || Abs(playerX - sSearch.x) > SEARCH_LOST_DISTANCE
        || Abs(playerZ - sSearch.z) > SEARCH_LOST_DISTANCE) {
        // walking away breaks the chain too
        sSearch.chain = 0;
        DexNavSearch_End(TRUE);
        return;
    }

    // the shake animation runs once, so restart it to keep the patch rustling
    if (sSearch.anim != NULL && ov5_021F31A8(sSearch.anim)) {
        OverworldAnimManager_Finish(sSearch.anim);
        sSearch.anim = ov5_021F3154(fieldSystem, sSearch.x, sSearch.z, sSearch.shiny ? PATCH_SHAKE_SHINY : PATCH_SHAKE_SOFT);
    }

    // A message box may have borrowed the bar's VRAM tiles. The window's pixel buffer in RAM is
    // untouched, so re-uploading it restores the bar without re-rendering any text.
    if (sSearch.hud != NULL) {
        Window_DrawMessageBox(sSearch.hud, FIELD_MSGBOX_BASE_TILE, FIELD_MSGBOX_PALETTE);
        Window_ScheduleCopyToVRAM(sSearch.hud);
    }
}

BOOL DexNavSearch_IsPlayerInPatch(FieldSystem *fieldSystem, int playerX, int playerZ)
{
    return sSearch.active && fieldSystem->location->mapHeaderID == sSearch.mapHeaderID && playerX == sSearch.x && playerZ == sSearch.z;
}

BOOL DexNavSearch_IsActive(void)
{
    return sSearch.active;
}

BOOL DexNavSearch_IsRequested(void)
{
    return sSearch.requested;
}

Pokemon *DexNavSearch_CreateEncounterMon(FieldSystem *fieldSystem, enum HeapID heapID)
{
    u32 otID = TrainerInfo_ID(SaveData_GetTrainerInfo(fieldSystem->saveData));
    Pokemon *mon = Pokemon_New(heapID);

    Pokemon_Init(mon);
    Pokemon_InitWith(mon, sSearch.species, sSearch.level, INIT_IVS_RANDOM, TRUE, sSearch.personality, OTID_SET, otID);

    for (int stat = 0; stat < NUM_STATS; stat++) {
        u32 iv = sSearch.ivs[stat];
        Pokemon_SetValue(mon, MON_DATA_HP_IV + stat, &iv);
    }

    Pokemon_CalcLevelAndStats(mon);

    DexNavSave_IncrementSearchLevel(fieldSystem->saveData, sSearch.species);

    if (sSearch.chain < 999) {
        sSearch.chain++;
    }

    sSearch.chainSpecies = sSearch.species;
    sSearch.chainMapHeaderID = sSearch.mapHeaderID;

    DexNavSearch_End(TRUE);
    return mon;
}

void DexNavSearch_End(BOOL removeFieldObjects)
{
    if (removeFieldObjects) {
        if (sSearch.anim != NULL) {
            OverworldAnimManager_Finish(sSearch.anim);
        }

        RemoveHud();
    }

    sSearch.anim = NULL;
    sSearch.hud = NULL;
    sSearch.active = FALSE;
    sSearch.requested = FALSE;
}
