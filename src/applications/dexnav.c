#include "applications/dexnav.h"

#include <nitro.h>
#include <string.h>

#include "constants/charcode.h"
#include "constants/species.h"

#include "dexnav_search.h"

#include "bg_window.h"
#include "char_transfer.h"
#include "font.h"
#include "graphics.h"
#include "gx_layers.h"
#include "heap.h"
#include "inlines.h"
#include "message_util.h"
#include "narc.h"
#include "overlay_manager.h"
#include "pltt_transfer.h"
#include "pokedex.h"
#include "pokemon_icon.h"
#include "render_oam.h"
#include "screen_fade.h"
#include "sound_playback.h"
#include "sprite.h"
#include "sprite_resource.h"
#include "sprite_transfer.h"
#include "sprite_util.h"
#include "string_gf.h"
#include "system.h"
#include "text.h"
#include "vram_transfer.h"

#include "dexnav_art.h"

// The diploma never runs at the same time, so its heap is free to borrow.
#define HEAP_ID_DEXNAV HEAP_ID_DIPLOMA

#define GREY_PALETTE_SLOT 8

// BG palette slots: 0 = art, 1 = art with the selected-cell colours, 2 = text
#define PAL_ART        0
#define PAL_SELECTED   1
#define PAL_TEXT       2
#define TEXT_DARK      TEXT_COLOR(1, 2, 0)
#define TEXT_ON_WOOD   TEXT_COLOR(3, 4, 0)

#define CELL_W         40
#define CELL_H         32
#define GRID_LEFT      8
#define LAND_TOP       16
#define WATER_TOP      104

enum {
    RES_ICON_PALETTE = 0,
    RES_ICON_CELL = 0,
    RES_ICON_ANIM = 0,
};

typedef struct DexNav {
    enum HeapID heapID;
    DexNavData *args;
    const Pokedex *pokedex;
    BgConfig *bgConfig;
    Window topWindow;
    Window bottomWindow;
    SpriteList *spriteList;
    G2dRenderer renderer;
    SpriteResourceCollection *resourceCollection[4];
    SpriteResource *iconTiles[DEXNAV_TOTAL_SLOTS];
    SpriteResource *iconPalette;
    SpriteResource *iconCell;
    SpriteResource *iconAnim;
    Sprite *icons[DEXNAV_TOTAL_SLOTS];
    int cursor;
} DexNav;

static void DexNav_VBlankCallback(void *data);
static void DexNav_InitBgs(DexNav *dexNav);
static void DexNav_FreeBgs(DexNav *dexNav);
static void DexNav_InitSprites(DexNav *dexNav);
static void DexNav_FreeSprites(DexNav *dexNav);
static void DexNav_DrawGrid(DexNav *dexNav);
static void DexNav_DrawDetails(DexNav *dexNav);
static BOOL DexNav_HandleInput(DexNav *dexNav);

static const char *const sMethodNames[] = {
    [DEXNAV_METHOD_GRASS] = "Tall grass",
    [DEXNAV_METHOD_MORNING] = "Grass - Morning",
    [DEXNAV_METHOD_NIGHT] = "Grass - Night",
    [DEXNAV_METHOD_SWARM] = "Swarm",
    [DEXNAV_METHOD_SURF] = "Surfing",
    [DEXNAV_METHOD_OLD_ROD] = "Old Rod",
    [DEXNAV_METHOD_GOOD_ROD] = "Good Rod",
    [DEXNAV_METHOD_SUPER_ROD] = "Super Rod",
};

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
        } else if (c == '/') {
            String_AppendChar(dst, CHAR_SLASH);
        } else if (c == ':') {
            String_AppendChar(dst, CHAR_COLON);
        } else if (c == '-') {
            String_AppendChar(dst, CHAR_MINUS);
        } else if (c == '?') {
            String_AppendChar(dst, CHAR_QUESTION);
        } else if (c == '.') {
            String_AppendChar(dst, CHAR_PERIOD);
        } else {
            String_AppendChar(dst, CHAR_SPACE);
        }
    }
}

static void AppendNumber(DexNav *dexNav, String *dst, int num)
{
    String *numStr = String_Init(8, dexNav->heapID);

    String_FormatInt(numStr, num, 3, PADDING_MODE_NONE, CHARSET_MODE_EN);
    String_Concat(dst, numStr);
    String_Free(numStr);
}

static void PrintAscii(DexNav *dexNav, Window *window, const char *text, int x, int y, u32 color)
{
    String *string = String_Init(64, dexNav->heapID);

    AppendAscii(string, text);
    Text_AddPrinterWithParamsAndColor(window, FONT_SYSTEM, string, x, y, TEXT_SPEED_INSTANT, color, NULL);
    String_Free(string);
}

static void PrintString(Window *window, String *string, int x, int y, u32 color)
{
    Text_AddPrinterWithParamsAndColor(window, FONT_SYSTEM, string, x, y, TEXT_SPEED_INSTANT, color, NULL);
}

static void SlotPosition(int slot, int *x, int *y)
{
    int section = slot / DEXNAV_SECTION_SLOTS;
    int index = slot % DEXNAV_SECTION_SLOTS;

    *x = GRID_LEFT + (index % DEXNAV_GRID_COLS) * CELL_W;
    *y = (section == 0 ? LAND_TOP : WATER_TOP) + (index / DEXNAV_GRID_COLS) * CELL_H;
}

static BOOL SlotIsSeen(DexNav *dexNav, int slot)
{
    u16 species = dexNav->args->slots[slot].species;
    return species != SPECIES_NONE && Pokedex_HasSeenSpecies(dexNav->pokedex, species);
}

// Land Pokemon are searched for in tall grass and water Pokemon while surfing; one only fished up can't be searched for.
// A species on both lists keeps its first water method (the surf table is read first).
static BOOL SlotHasSearch(DexNav *dexNav, int slot)
{
    return dexNav->args->slots[slot].species != SPECIES_NONE && (slot < DEXNAV_SECTION_SLOTS || dexNav->args->slots[slot].method == DEXNAV_METHOD_SURF);
}

// only a Pokemon the player has seen can be chosen
static BOOL SlotIsSearchable(DexNav *dexNav, int slot)
{
    return SlotIsSeen(dexNav, slot) && SlotHasSearch(dexNav, slot);
}

BOOL DexNav_Init(ApplicationManager *appMan, int *state)
{
    // measured peak is ~68 KB with two icons; 24 icons add ~24 KB, so 128 KB leaves a safe margin
    Heap_Create(HEAP_ID_APPLICATION, HEAP_ID_DEXNAV, 0x20000);

    DexNav *dexNav = ApplicationManager_NewData(appMan, sizeof(DexNav), HEAP_ID_DEXNAV);
    memset(dexNav, 0, sizeof(DexNav));

    dexNav->heapID = HEAP_ID_DEXNAV;
    dexNav->args = ApplicationManager_Args(appMan);
    dexNav->pokedex = SaveData_GetPokedex(dexNav->args->saveData);

    // start the cursor on the first occupied slot
    for (int i = 0; i < DEXNAV_TOTAL_SLOTS; i++) {
        if (dexNav->args->slots[i].species != SPECIES_NONE) {
            dexNav->cursor = i;
            break;
        }
    }

    SetScreenColorBrightness(DS_SCREEN_MAIN, COLOR_BLACK);
    SetScreenColorBrightness(DS_SCREEN_SUB, COLOR_BLACK);
    SetVBlankCallback(NULL, NULL);
    SetHBlankCallback(NULL, NULL);
    GXLayers_DisableEngineALayers();
    GXLayers_DisableEngineBLayers();
    GX_SetVisiblePlane(0);
    GXS_SetVisiblePlane(0);
    SetAutorepeat(4, 8);

    DexNav_InitBgs(dexNav);
    DexNav_InitSprites(dexNav);

    SetVBlankCallback(DexNav_VBlankCallback, dexNav);
    GXLayers_TurnBothDispOn();

    return TRUE;
}

BOOL DexNav_Main(ApplicationManager *appMan, int *state)
{
    DexNav *dexNav = ApplicationManager_Data(appMan);

    switch (*state) {
    case 0:
        DexNav_DrawGrid(dexNav);
        DexNav_DrawDetails(dexNav);
        Bg_ToggleLayer(BG_LAYER_MAIN_0, TRUE);
        Bg_ToggleLayer(BG_LAYER_SUB_0, TRUE);
        Bg_ToggleLayer(BG_LAYER_MAIN_1, TRUE);
        Bg_ToggleLayer(BG_LAYER_SUB_1, TRUE);
        StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_IN, FADE_TYPE_BRIGHTNESS_IN, COLOR_BLACK, 6, 1, dexNav->heapID);
        *state = 1;
        break;
    case 1:
        if (IsScreenFadeDone() == TRUE) {
            *state = 2;
        }
        break;
    case 2:
        if (DexNav_HandleInput(dexNav) == TRUE) {
            StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_OUT, FADE_TYPE_BRIGHTNESS_OUT, COLOR_BLACK, 6, 1, dexNav->heapID);
            *state = 3;
        }
        break;
    case 3:
        if (IsScreenFadeDone() == TRUE) {
            return TRUE;
        }
        break;
    }

    SpriteList_Update(dexNav->spriteList);
    return FALSE;
}

BOOL DexNav_Exit(ApplicationManager *appMan, int *state)
{
    DexNav *dexNav = ApplicationManager_Data(appMan);
    enum HeapID heapID = dexNav->heapID;

    SetVBlankCallback(NULL, NULL);
    DexNav_FreeSprites(dexNav);
    DexNav_FreeBgs(dexNav);
    ApplicationManager_FreeData(appMan);
    Heap_Destroy(heapID);

    return TRUE;
}

static void DexNav_VBlankCallback(void *data)
{
    DexNav *dexNav = data;

    Bg_RunScheduledUpdates(dexNav->bgConfig);
    VramTransfer_Process();
    RenderOam_Transfer();
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static BOOL DexNav_HandleInput(DexNav *dexNav)
{
    int section = dexNav->cursor / DEXNAV_SECTION_SLOTS;
    int index = dexNav->cursor % DEXNAV_SECTION_SLOTS;
    int col = index % DEXNAV_GRID_COLS;
    int row = section * DEXNAV_GRID_ROWS + index / DEXNAV_GRID_COLS;
    int oldCursor = dexNav->cursor;

    if (JOY_NEW(PAD_BUTTON_A) && SlotIsSearchable(dexNav, dexNav->cursor)) {
        Sound_PlayEffect(SEQ_SE_DP_DECIDE_sseq);
        DexNavSave_SetSelectedSpecies(dexNav->args->saveData, dexNav->args->slots[dexNav->cursor].species);
        DexNav_DrawGrid(dexNav);
        DexNav_DrawDetails(dexNav);
        return FALSE;
    }

    if (JOY_NEW(PAD_BUTTON_B)) {
        Sound_PlayEffect(SEQ_SE_DP_DECIDE_sseq);
        return TRUE;
    }

    if (JOY_REPEAT(PAD_KEY_LEFT)) {
        col = (col + DEXNAV_GRID_COLS - 1) % DEXNAV_GRID_COLS;
    } else if (JOY_REPEAT(PAD_KEY_RIGHT)) {
        col = (col + 1) % DEXNAV_GRID_COLS;
    } else if (JOY_REPEAT(PAD_KEY_UP)) {
        row = (row + DEXNAV_GRID_ROWS * 2 - 1) % (DEXNAV_GRID_ROWS * 2);
    } else if (JOY_REPEAT(PAD_KEY_DOWN)) {
        row = (row + 1) % (DEXNAV_GRID_ROWS * 2);
    }

    dexNav->cursor = (row / DEXNAV_GRID_ROWS) * DEXNAV_SECTION_SLOTS + (row % DEXNAV_GRID_ROWS) * DEXNAV_GRID_COLS + col;

    if (dexNav->cursor != oldCursor) {
        Sound_PlayEffect(SEQ_SE_DP_SELECT5_sseq);
        DexNav_DrawGrid(dexNav);
        DexNav_DrawDetails(dexNav);
    }

    return FALSE;
}

static void DexNav_DrawGrid(DexNav *dexNav)
{
    Window *window = &dexNav->bottomWindow;
    u16 selected = DexNavSave_GetSelectedSpecies(dexNav->args->saveData);
    int x, y;

    // the cursor is a palette swap on the art layer: same tiles, warmer slot colours
    Bg_LoadTilemapBuffer(dexNav->bgConfig, BG_LAYER_SUB_1, (void *)sDexNavBottomMap, sizeof(sDexNavBottomMap));
    SlotPosition(dexNav->cursor, &x, &y);
    Bg_ChangeTilemapRectPalette(dexNav->bgConfig, BG_LAYER_SUB_1, x / 8, y / 8, CELL_W / 8, CELL_H / 8, PAL_SELECTED);
    Bg_CopyTilemapBufferToVRAM(dexNav->bgConfig, BG_LAYER_SUB_1);

    Window_FillTilemap(window, 0);
    PrintAscii(dexNav, window, "LAND", 12, LAND_TOP - 13, TEXT_DARK);
    PrintAscii(dexNav, window, "WATER", 12, WATER_TOP - 13, TEXT_DARK);
    PrintAscii(dexNav, window, "A: Select   B: Close", 112, 177, TEXT_ON_WOOD);

    for (int slot = 0; slot < DEXNAV_TOTAL_SLOTS; slot++) {
        u16 species = dexNav->args->slots[slot].species;

        if (species == SPECIES_NONE) {
            continue;
        }

        SlotPosition(slot, &x, &y);

        if (!SlotIsSeen(dexNav, slot)) {
            PrintAscii(dexNav, window, "?", x + 17, y + 7, TEXT_DARK);
        }

        // caught: a Poke Ball in the top left corner (the Bag's pocket icon)
        if (Pokedex_HasCaughtSpecies(dexNav->pokedex, species)) {
            String *ball = String_Init(4, dexNav->heapID);

            String_AppendChar(ball, CHAR_POCKET_ICON_POKEBALLS);
            PrintString(window, ball, x + 2, y + 1, TEXT_DARK);
            String_Free(ball);
        }

        if (species == selected) {
            String *star = String_Init(4, dexNav->heapID);

            String_AppendChar(star, CHAR_STAR);
            PrintString(window, star, x + 27, y + 1, TEXT_DARK);
            String_Free(star);
        }
    }

    Window_CopyToVRAM(window);
}

static void DexNav_DrawDetails(DexNav *dexNav)
{
    Window *window = &dexNav->topWindow;
    const DexNavEntry *entry = &dexNav->args->slots[dexNav->cursor];
    u16 selected = DexNavSave_GetSelectedSpecies(dexNav->args->saveData);
    int numCaught = 0, numSpecies = 0;

    for (int i = 0; i < DEXNAV_TOTAL_SLOTS; i++) {
        if (dexNav->args->slots[i].species != SPECIES_NONE) {
            numSpecies++;
            numCaught += Pokedex_HasCaughtSpecies(dexNav->pokedex, dexNav->args->slots[i].species);
        }
    }

    Window_FillTilemap(window, 0);
    PrintAscii(dexNav, window, "DexNav", 12, 3, TEXT_ON_WOOD);

    String *string = String_Init(64, dexNav->heapID);
    AppendAscii(string, "Caught ");
    AppendNumber(dexNav, string, numCaught);
    AppendAscii(string, "/");
    AppendNumber(dexNav, string, numSpecies);
    PrintString(window, string, 172, 3, TEXT_ON_WOOD);

    // info card
    if (numSpecies == 0) {
        PrintAscii(dexNav, window, "No wild Pokemon live here.", 20, 38, TEXT_DARK);
    } else if (entry->species == SPECIES_NONE) {
        PrintAscii(dexNav, window, "-----", 20, 38, TEXT_DARK);
    } else {
        BOOL seen = Pokedex_HasSeenSpecies(dexNav->pokedex, entry->species);
        BOOL caught = Pokedex_HasCaughtSpecies(dexNav->pokedex, entry->species);

        if (seen) {
            String *name = MessageUtil_SpeciesName(entry->species, dexNav->heapID);
            PrintString(window, name, 20, 38, TEXT_DARK);
            String_Free(name);
        } else {
            PrintAscii(dexNav, window, "?????", 20, 38, TEXT_DARK);
        }

        PrintAscii(dexNav, window, caught ? "Caught" : seen ? "Seen" : "Not seen yet", 150, 38, TEXT_DARK);
        PrintAscii(dexNav, window, "Found", 20, 66, TEXT_DARK);
        PrintAscii(dexNav, window, sMethodNames[entry->method], 84, 66, TEXT_DARK);

        String_Clear(string);
        AppendAscii(string, "Lv. ");
        AppendNumber(dexNav, string, entry->minLevel);
        if (entry->maxLevel != entry->minLevel) {
            AppendAscii(string, "-");
            AppendNumber(dexNav, string, entry->maxLevel);
        }
        PrintAscii(dexNav, window, "Level", 20, 88, TEXT_DARK);
        PrintString(window, string, 84, 88, TEXT_DARK);

        String_Clear(string);
        AppendAscii(string, "Search Lv. ");
        AppendNumber(dexNav, string, DexNavSave_GetSearchLevel(dexNav->args->saveData, entry->species));
        PrintString(window, string, 20, 108, TEXT_DARK);
    }

    // search card: what R will look for
    PrintAscii(dexNav, window, "Search", 16, 136, TEXT_DARK);

    if (selected != SPECIES_NONE) {
        BOOL onLand = FALSE, onWater = FALSE;

        for (int i = 0; i < DEXNAV_TOTAL_SLOTS; i++) {
            if (dexNav->args->slots[i].species == selected && SlotHasSearch(dexNav, i)) {
                onLand |= i < DEXNAV_SECTION_SLOTS;
                onWater |= i >= DEXNAV_SECTION_SLOTS;
            }
        }

        String *name = MessageUtil_SpeciesName(selected, dexNav->heapID);
        PrintString(window, name, 20, 152, TEXT_DARK);
        String_Free(name);
        PrintAscii(dexNav, window, onWater ? (onLand ? "Press R in grass or while surfing." : "Press R while surfing to search.") : "Press R in tall grass to search.", 20, 168, TEXT_DARK);
    } else {
        PrintAscii(dexNav, window, "Press A on a Pokemon to choose it.", 20, 156, TEXT_DARK);
    }

    String_Free(string);
    Window_CopyToVRAM(window);
}

static void DexNav_InitBgs(DexNav *dexNav)
{
    GXBanks banks = {
        GX_VRAM_BG_128_B,
        GX_VRAM_BGEXTPLTT_NONE,
        GX_VRAM_SUB_BG_128_C,
        GX_VRAM_SUB_BGEXTPLTT_NONE,
        GX_VRAM_OBJ_NONE,
        GX_VRAM_OBJEXTPLTT_NONE,
        GX_VRAM_SUB_OBJ_128_D,
        GX_VRAM_SUB_OBJEXTPLTT_NONE,
        GX_VRAM_TEX_NONE,
        GX_VRAM_TEXPLTT_NONE,
    };

    GXLayers_SetBanks(&banks);

    dexNav->bgConfig = BgConfig_New(dexNav->heapID);

    GraphicsModes graphicsModes = {
        .displayMode = GX_DISPMODE_GRAPHICS,
        .mainBgMode = GX_BGMODE_0,
        .subBgMode = GX_BGMODE_0,
        .bg0As2DOr3D = GX_BG0_AS_2D,
    };

    SetAllGraphicsModes(&graphicsModes);

    BgTemplate template = {
        .x = 0,
        .y = 0,
        .bufferSize = 0x800,
        .baseTile = 0,
        .screenSize = BG_SCREEN_SIZE_256x256,
        .colorMode = GX_BG_COLORMODE_16,
        .screenBase = GX_BG_SCRBASE_0x0000,
        .charBase = GX_BG_CHARBASE_0x04000,
        .bgExtPltt = GX_BG_EXTPLTT_01,
        .priority = 0,
        .areaOver = 0,
        .mosaic = FALSE,
    };

    // art layer under the text: one shared tileset for both screens
    BgTemplate artTemplate = template;
    artTemplate.screenBase = GX_BG_SCRBASE_0x0800;
    artTemplate.charBase = GX_BG_CHARBASE_0x10000;
    artTemplate.priority = 1;

    Bg_InitFromTemplate(dexNav->bgConfig, BG_LAYER_MAIN_1, &artTemplate, BG_TYPE_STATIC);
    Bg_InitFromTemplate(dexNav->bgConfig, BG_LAYER_SUB_1, &artTemplate, BG_TYPE_STATIC);
    Bg_LoadTiles(dexNav->bgConfig, BG_LAYER_MAIN_1, (void *)sDexNavTiles, sizeof(sDexNavTiles), 0);
    Bg_LoadTiles(dexNav->bgConfig, BG_LAYER_SUB_1, (void *)sDexNavTiles, sizeof(sDexNavTiles), 0);
    Bg_LoadTilemapBuffer(dexNav->bgConfig, BG_LAYER_MAIN_1, (void *)sDexNavTopMap, sizeof(sDexNavTopMap));
    Bg_CopyTilemapBufferToVRAM(dexNav->bgConfig, BG_LAYER_MAIN_1);

    Bg_InitFromTemplate(dexNav->bgConfig, BG_LAYER_MAIN_0, &template, BG_TYPE_STATIC);
    Bg_ClearTilemap(dexNav->bgConfig, BG_LAYER_MAIN_0);
    Bg_InitFromTemplate(dexNav->bgConfig, BG_LAYER_SUB_0, &template, BG_TYPE_STATIC);
    Bg_ClearTilemap(dexNav->bgConfig, BG_LAYER_SUB_0);

    GX_LoadBGPltt(sDexNavPalBase, PLTT_OFFSET(PAL_ART), sizeof(sDexNavPalBase));
    GX_LoadBGPltt(sDexNavPalSelected, PLTT_OFFSET(PAL_SELECTED), sizeof(sDexNavPalSelected));
    GX_LoadBGPltt(sDexNavPalText, PLTT_OFFSET(PAL_TEXT), sizeof(sDexNavPalText));
    GXS_LoadBGPltt(sDexNavPalBase, PLTT_OFFSET(PAL_ART), sizeof(sDexNavPalBase));
    GXS_LoadBGPltt(sDexNavPalSelected, PLTT_OFFSET(PAL_SELECTED), sizeof(sDexNavPalSelected));
    GXS_LoadBGPltt(sDexNavPalText, PLTT_OFFSET(PAL_TEXT), sizeof(sDexNavPalText));

    WindowTemplate windowTemplate = {
        .bgLayer = BG_LAYER_MAIN_0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 24,
        .palette = PAL_TEXT,
        .baseTile = 1,
    };

    Text_ResetAllPrinters();
    Window_AddFromTemplate(dexNav->bgConfig, &dexNav->topWindow, &windowTemplate);
    windowTemplate.bgLayer = BG_LAYER_SUB_0;
    Window_AddFromTemplate(dexNav->bgConfig, &dexNav->bottomWindow, &windowTemplate);
}

static void DexNav_FreeBgs(DexNav *dexNav)
{
    Window_Remove(&dexNav->topWindow);
    Window_Remove(&dexNav->bottomWindow);
    Bg_ToggleLayer(BG_LAYER_MAIN_0, FALSE);
    Bg_ToggleLayer(BG_LAYER_SUB_0, FALSE);
    Bg_ToggleLayer(BG_LAYER_MAIN_1, FALSE);
    Bg_ToggleLayer(BG_LAYER_SUB_1, FALSE);
    Bg_FreeTilemapBuffer(dexNav->bgConfig, BG_LAYER_MAIN_0);
    Bg_FreeTilemapBuffer(dexNav->bgConfig, BG_LAYER_SUB_0);
    Bg_FreeTilemapBuffer(dexNav->bgConfig, BG_LAYER_MAIN_1);
    Bg_FreeTilemapBuffer(dexNav->bgConfig, BG_LAYER_SUB_1);
    Heap_Free(dexNav->bgConfig);
}

static void DexNav_InitSprites(DexNav *dexNav)
{
    CharTransferTemplate transferTemplate = {
        DEXNAV_TOTAL_SLOTS + 4, 0x800, 0x8000, dexNav->heapID
    };

    CharTransfer_InitWithVramModes(&transferTemplate, GX_OBJVRAMMODE_CHAR_1D_32K, GX_OBJVRAMMODE_CHAR_1D_32K);
    PlttTransfer_Init(4, dexNav->heapID);
    CharTransfer_ClearBuffers();
    PlttTransfer_Clear();

    NNS_G2dInitOamManagerModule();
    RenderOam_Init(0, 128, 0, 32, 0, 128, 0, 32, dexNav->heapID);
    dexNav->spriteList = SpriteList_InitRendering(DEXNAV_TOTAL_SLOTS, &dexNav->renderer, dexNav->heapID);

    dexNav->resourceCollection[SPRITE_RESOURCE_CHAR] = SpriteResourceCollection_New(DEXNAV_TOTAL_SLOTS, SPRITE_RESOURCE_CHAR, dexNav->heapID);
    dexNav->resourceCollection[SPRITE_RESOURCE_PLTT] = SpriteResourceCollection_New(1, SPRITE_RESOURCE_PLTT, dexNav->heapID);
    dexNav->resourceCollection[SPRITE_RESOURCE_CELL] = SpriteResourceCollection_New(1, SPRITE_RESOURCE_CELL, dexNav->heapID);
    dexNav->resourceCollection[SPRITE_RESOURCE_ANIM] = SpriteResourceCollection_New(1, SPRITE_RESOURCE_ANIM, dexNav->heapID);

    NARC *narc = NARC_ctor(NARC_INDEX_POKETOOL__ICONGRA__PL_POKE_ICON, dexNav->heapID);

    dexNav->iconPalette = SpriteResourceCollection_AddPalette(dexNav->resourceCollection[SPRITE_RESOURCE_PLTT], NARC_INDEX_POKETOOL__ICONGRA__PL_POKE_ICON, PokeIconPalettesFileIndex(), FALSE, RES_ICON_PALETTE, NNS_G2D_VRAM_TYPE_2DSUB, 3, dexNav->heapID);
    dexNav->iconCell = SpriteResourceCollection_AddFrom(dexNav->resourceCollection[SPRITE_RESOURCE_CELL], narc, PokeIcon32KCellsFileIndex(), FALSE, RES_ICON_CELL, SPRITE_RESOURCE_CELL, dexNav->heapID);
    dexNav->iconAnim = SpriteResourceCollection_AddFrom(dexNav->resourceCollection[SPRITE_RESOURCE_ANIM], narc, PokeIcon32KAnimationFileIndex(), FALSE, RES_ICON_ANIM, SPRITE_RESOURCE_ANIM, dexNav->heapID);
    SpriteTransfer_RequestPlttWholeRange(dexNav->iconPalette);

    // greyscale copies of the three icon palettes go in the slots right after them,
    // so seen-but-not-caught species can be drawn faded
    NNSG2dPaletteData *paletteData;
    void *paletteFile = Graphics_GetPlttDataFromOpenNARC(narc, PokeIconPalettesFileIndex(), &paletteData, dexNav->heapID);
    const u16 *colors = paletteData->pRawData;
    // heap, not stack: the stack lives in DTCM, which the palette DMA cannot read
    u16 *greyColors = Heap_Alloc(dexNav->heapID, sizeof(u16) * 16 * 3);

    for (int i = 0; i < 16 * 3; i++) {
        int r = colors[i] & 0x1F, g = (colors[i] >> 5) & 0x1F, b = (colors[i] >> 10) & 0x1F;
        int luma = (r * 77 + g * 151 + b * 28) >> 8;
        int grey = 12 + luma / 3;

        greyColors[i] = GX_RGB(grey, grey, grey);
    }

    DC_FlushRange(greyColors, sizeof(u16) * 16 * 3);
    GXS_LoadOBJPltt(greyColors, GREY_PALETTE_SLOT * PALETTE_SIZE_BYTES, sizeof(u16) * 16 * 3);
    Heap_Free(greyColors);
    Heap_Free(paletteFile);

    for (int slot = 0; slot < DEXNAV_TOTAL_SLOTS; slot++) {
        u16 species = dexNav->args->slots[slot].species;

        if (!SlotIsSeen(dexNav, slot)) {
            continue;
        }

        dexNav->iconTiles[slot] = SpriteResourceCollection_AddTilesFrom(dexNav->resourceCollection[SPRITE_RESOURCE_CHAR], narc, PokeIconSpriteIndex(species, FALSE, 0), FALSE, slot, NNS_G2D_VRAM_TYPE_2DSUB, dexNav->heapID);
        SpriteTransfer_RequestChar(dexNav->iconTiles[slot]);

        SpriteResourcesHeader header;
        SpriteResourcesHeader_Init(&header, slot, RES_ICON_PALETTE, RES_ICON_CELL, RES_ICON_ANIM, -1, -1, FALSE, 0, dexNav->resourceCollection[SPRITE_RESOURCE_CHAR], dexNav->resourceCollection[SPRITE_RESOURCE_PLTT], dexNav->resourceCollection[SPRITE_RESOURCE_CELL], dexNav->resourceCollection[SPRITE_RESOURCE_ANIM], NULL, NULL);

        int x, y;
        SlotPosition(slot, &x, &y);

        AffineSpriteListTemplate template;
        template.list = dexNav->spriteList;
        template.resourceData = &header;
        template.position.x = FX32_CONST(x + CELL_W / 2);
        template.position.y = FX32_CONST(y + CELL_H / 2 + 192);
        template.position.z = 0;
        template.affineScale.x = FX32_ONE;
        template.affineScale.y = FX32_ONE;
        template.affineScale.z = FX32_ONE;
        template.affineZRotation = 0;
        template.priority = 0;
        template.vramType = NNS_G2D_VRAM_TYPE_2DSUB;
        template.heapID = dexNav->heapID;

        dexNav->icons[slot] = SpriteList_AddAffine(&template);
        Sprite_SetAnimateFlag(dexNav->icons[slot], TRUE);
        Sprite_SetAnimSpeed(dexNav->icons[slot], FX32_ONE);
        Sprite_SetAnim(dexNav->icons[slot], 1);
        if (Pokedex_HasCaughtSpecies(dexNav->pokedex, species)) {
            Sprite_SetExplicitPaletteOffsetAutoAdjust(dexNav->icons[slot], PokeIconPaletteIndex(species, 0, FALSE));
        } else {
            Sprite_SetExplicitPaletteOffset(dexNav->icons[slot], GREY_PALETTE_SLOT + PokeIconPaletteIndex(species, 0, FALSE));
        }
    }

    NARC_dtor(narc);
    GXLayers_EngineBToggleLayers(GX_PLANEMASK_OBJ, TRUE);
}

static void DexNav_FreeSprites(DexNav *dexNav)
{
    for (int slot = 0; slot < DEXNAV_TOTAL_SLOTS; slot++) {
        if (dexNav->iconTiles[slot] != NULL) {
            SpriteTransfer_ResetCharTransfer(dexNav->iconTiles[slot]);
        }
    }

    SpriteTransfer_ResetPlttTransfer(dexNav->iconPalette);

    for (int i = 0; i < 4; i++) {
        SpriteResourceCollection_Delete(dexNav->resourceCollection[i]);
    }

    SpriteList_Delete(dexNav->spriteList);
    RenderOam_Free();
    CharTransfer_Free();
    PlttTransfer_Free();
}
