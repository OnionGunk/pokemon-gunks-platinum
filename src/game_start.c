#include "game_start.h"

#include "constants/game_options.h"
#include "constants/heap.h"

#ifdef TEST_SKIP_INTRO
#include "constants/charcode.h"
#include "constants/items.h"
#include "constants/species.h"
#include "generated/first_arrival_to_zones.h"
#include "generated/moves.h"

#include "bag.h"
#include "pokedex.h"
#include "poketch.h"
#include "system_vars.h"
#include "unk_02054884.h"
#endif

#include "appearance.h"
#include "berry_patches.h"
#include "game_options.h"
#include "heap.h"
#include "location.h"
#include "main.h"
#include "math_util.h"
#include "overlay_manager.h"
#include "party.h"
#include "play_time_manager.h"
#include "pokemon.h"
#include "record_mixed_rng.h"
#include "save_player.h"
#include "savedata.h"
#include "savedata_misc.h"
#include "system_data.h"
#include "system_flags.h"
#include "trainer_info.h"
#include "unk_0206B9D8.h"
#include "vars_flags.h"

#include "constdata/const_020EA10C.h"
#include "constdata/const_020EA11C.h"

static int GameStartRowanIntro_Init(ApplicationManager *appMan, int *state);
static int GameStartRowanIntro_Main(ApplicationManager *appMan, int *state);
static int GameStartRowanIntro_Exit(ApplicationManager *appMan, int *state);
static int GameStartNewSave_Init(ApplicationManager *appMan, int *state);
static int GameStartNewSave_Main(ApplicationManager *appMan, int *state);
static int GameStartNewSave_Exit(ApplicationManager *appMan, int *state);
static int GameStartLoadSave_Init(ApplicationManager *appMan, int *state);
static int GameStartLoadSave_Main(ApplicationManager *appMan, int *state);
static int GameStartLoadSave_Exit(ApplicationManager *appMan, int *state);

static void InitializeNewSave(enum HeapID heapID, SaveData *saveData, BOOL setTrainerID);
static void TryLoadingSave(int unused, SaveData *saveData);
static void StartNewSave(int unused, SaveData *saveData);

extern const ApplicationManagerTemplate gRowanIntroAppTemplate;

const ApplicationManagerTemplate gGameStartRowanIntroAppTemplate = {
    .init = GameStartRowanIntro_Init,
    .main = GameStartRowanIntro_Main,
    .exit = GameStartRowanIntro_Exit,
    .overlayID = FS_OVERLAY_ID_NONE,
};

const ApplicationManagerTemplate gGameStartNewSaveAppTemplate = {
    .init = GameStartNewSave_Init,
    .main = GameStartNewSave_Main,
    .exit = GameStartNewSave_Exit,
    .overlayID = FS_OVERLAY_ID_NONE,
};

const ApplicationManagerTemplate gGameStartLoadSaveAppTemplate = {
    .init = GameStartLoadSave_Init,
    .main = GameStartLoadSave_Main,
    .exit = GameStartLoadSave_Exit,
    .overlayID = FS_OVERLAY_ID_NONE,
};

static BOOL GameStartRowanIntro_Init(ApplicationManager *appMan, int *state)
{
    Heap_Create(HEAP_ID_APPLICATION, HEAP_ID_GAME_START, HEAP_SIZE_GAME_START);
    InitRNG();
    return TRUE;
}

static BOOL GameStartRowanIntro_Main(ApplicationManager *appMan, int *state)
{
    SaveData *saveData = ((ApplicationArgs *)ApplicationManager_Args(appMan))->saveData;
    StartNewSave(HEAP_ID_GAME_START, saveData);
    return TRUE;
}

static int GameStartRowanIntro_Exit(ApplicationManager *appMan, int *state)
{
    Heap_Destroy(HEAP_ID_GAME_START);
#ifdef TEST_SKIP_INTRO
    // test builds go straight to the overworld so the emulator harness needs no touch input
    EnqueueApplication(FS_OVERLAY_ID_NONE, &gGameStartNewSaveAppTemplate);
#else
    EnqueueApplication(FS_OVERLAY_ID_NONE, &gRowanIntroAppTemplate);
#endif
    return TRUE;
}

static int GameStartNewSave_Init(ApplicationManager *appMan, int *state)
{
    Heap_Create(HEAP_ID_APPLICATION, HEAP_ID_GAME_START, HEAP_SIZE_GAME_START);
    InitRNG();
    return TRUE;
}

#ifdef TEST_SKIP_INTRO
// lives in the resident binary (dexnav_search.c): this file is an overlay whose .bss is cleared on load
extern u16 gTestLeadSpecies;
#endif

static int GameStartNewSave_Main(ApplicationManager *appMan, int *state)
{
    SaveData *saveData = ((ApplicationArgs *)ApplicationManager_Args(appMan))->saveData;
    InitializeNewSave(HEAP_ID_GAME_START, saveData, 1);
    PlayTime_Start(SaveData_GetPlayTime(saveData));

#ifdef TEST_SKIP_INTRO
    // test save: Poketch, Pokedex, all badges, every town visited, Master Balls and a flyer
    VarsFlags *varsFlags = SaveData_GetVarsFlags(saveData);
    TrainerInfo *trainerInfo = SaveData_GetTrainerInfo(saveData);

    Poketch_Enable(SaveData_GetPoketch(saveData));
    Pokedex_ObtainPokedex(SaveData_GetPokedex(saveData));
    Pokedex_ObtainNationalDex(SaveData_GetPokedex(saveData));
    SystemFlag_SetBagAcquired(varsFlags);
    SystemVars_SetPlayerStarter(varsFlags, SPECIES_PIPLUP);

    static const charcode_t testName[] = { CHAR_T, CHAR_E, CHAR_S, CHAR_T, CHAR_EOS };
    TrainerInfo_SetName(trainerInfo, testName);

    for (int badge = 0; badge < 8; badge++) {
        TrainerInfo_SetBadge(trainerInfo, badge);
    }

    for (int zone = 0; zone < FIRST_ARRIVAL_MAX; zone++) {
        SystemFlag_HandleFirstArrivalToZone(varsFlags, HANDLE_FLAG_SET, zone);
    }

    Bag_TryAddItem(SaveData_GetBag(saveData), ITEM_MASTER_BALL, 99, HEAP_ID_GAME_START);
    Bag_TryAddItem(SaveData_GetBag(saveData), ITEM_DEXNAV, 1, HEAP_ID_GAME_START);
    Bag_RegisterItem(SaveData_GetBag(saveData), ITEM_DEXNAV);
    // six varied Pokemon for follower checks; the emulator harness picks the lead via gTestLeadSpecies
    static const u16 sTestParty[] = { SPECIES_PIKACHU, SPECIES_PIPLUP, SPECIES_LUCARIO, SPECIES_STARAPTOR, SPECIES_STEELIX, SPECIES_GYARADOS };
    // bit 15 of gTestLeadSpecies makes the lead shiny
    u16 lead = (gTestLeadSpecies & 0x7FFF) != SPECIES_NONE ? (gTestLeadSpecies & 0x7FFF) : SPECIES_PIPLUP;

    Pokemon_GiveMonFromScript(HEAP_ID_GAME_START, saveData, lead, 30, 0, 0, 0);

    if (gTestLeadSpecies & 0x8000) {
        Pokemon *leadMon = Party_GetPokemonBySlotIndex(SaveData_GetParty(saveData), 0);
        u32 personality = Pokemon_GetValue(leadMon, MON_DATA_PERSONALITY, NULL);
        u32 otID = ((personality >> 16) ^ (personality & 0xFFFF)) << 16;

        Pokemon_SetValue(leadMon, MON_DATA_OT_ID, &otID);
    }

    for (int i = 0; i < NELEMS(sTestParty); i++) {
        if (sTestParty[i] != lead) {
            Pokemon_GiveMonFromScript(HEAP_ID_GAME_START, saveData, sTestParty[i], 30, 0, 0, 0);
        }
    }

    for (int i = 0; i < Party_GetCurrentCount(SaveData_GetParty(saveData)); i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(SaveData_GetParty(saveData), i);

        if (Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL) == SPECIES_STARAPTOR) {
            Pokemon_ResetMoveSlot(mon, MOVE_FLY, 0);
        }
    }

    // Route 201 dex state for DexNav testing: Starly caught, Bidoof seen, the rest unseen
    Pokemon *dexMon = Pokemon_New(HEAP_ID_GAME_START);

    Pokemon_InitWith(dexMon, SPECIES_STARLY, 5, INIT_IVS_RANDOM, FALSE, 0, OTID_NOT_SET, 0);
    Pokedex_Capture(SaveData_GetPokedex(saveData), dexMon);
    Pokemon_InitWith(dexMon, SPECIES_BIDOOF, 5, INIT_IVS_RANDOM, FALSE, 0, OTID_NOT_SET, 0);
    Pokedex_Encounter(SaveData_GetPokedex(saveData), dexMon);
    Pokemon_InitWith(dexMon, SPECIES_STARAPTOR, 50, INIT_IVS_RANDOM, FALSE, 0, OTID_NOT_SET, 0);
    Pokedex_Capture(SaveData_GetPokedex(saveData), dexMon);
    Heap_Free(dexMon);
#endif
    return TRUE;
}

static int GameStartNewSave_Exit(ApplicationManager *appMan, int *state)
{
    Heap_Destroy(HEAP_ID_GAME_START);
    EnqueueApplication(FS_OVERLAY_ID_NONE, &gFieldSystemNewGameTemplate);
    return TRUE;
}

static int GameStartLoadSave_Init(ApplicationManager *appMan, int *state)
{
    Heap_Create(HEAP_ID_APPLICATION, HEAP_ID_GAME_START, HEAP_SIZE_GAME_START);
    InitRNG();
    return TRUE;
}

static int GameStartLoadSave_Main(ApplicationManager *appMan, int *state)
{
    SaveData *saveData = ((ApplicationArgs *)ApplicationManager_Args(appMan))->saveData;
    SystemData *systemData = SaveData_GetSystemData(saveData);

    TryLoadingSave(HEAP_ID_GAME_START, saveData);
    Options_SetSystemButtonMode(saveData, OPTIONS_BUTTON_MODE_NORMAL);

    if (!SystemData_MatchesCurrentSystem(systemData) || !SystemData_MatchesCurrentRTCOffset(systemData)) {
        GameTime_StartPenalty(SaveData_GetGameTime(saveData));
        SystemData_Init(systemData);
        Party_SetShayminLandForm(SaveData_GetParty(saveData));
    }

    PlayTime_Start(SaveData_GetPlayTime(saveData));
    return TRUE;
}

static int GameStartLoadSave_Exit(ApplicationManager *appMan, int *state)
{
    Heap_Destroy(HEAP_ID_GAME_START);
    EnqueueApplication(FS_OVERLAY_ID_NONE, &gFieldSystemContinueTemplate);
    return TRUE;
}

#include "data/berry_init.h"

static void InitializeNewSave(enum HeapID heapID, SaveData *saveData, BOOL setTrainerID)
{
    u32 rnd;
    BerryPatch *berryPatches;
    TrainerInfo *trainerInfo;
    GameTime *gameTime;
    RecordMixedRNG *rngCollection;

    SystemData_Init(SaveData_GetSystemData(saveData));

    gameTime = SaveData_GetGameTime(saveData);
    GameTime_Clear(gameTime);

    rngCollection = SaveData_GetRecordMixedRNG(saveData);
    RecordMixedRNG_SetEntrySeed(rngCollection, RECORD_MIXED_RNG_PLAYER_OVERRIDE, MTRNG_Next());
    sub_0206C008(saveData);

    trainerInfo = SaveData_GetTrainerInfo(saveData);
    rnd = MTRNG_Next();

    if (setTrainerID) {
        TrainerInfo_SetID(trainerInfo, rnd);
    }

    TrainerInfo_SetAppearance(trainerInfo, Appearance_CalculateFromTrainerInfo(rnd, TrainerInfo_Gender(trainerInfo), 0));

    berryPatches = MiscSaveBlock_GetBerryPatches(saveData);
    BerryPatches_Init(berryPatches, heapID, (const u16 *)sBerryInitTable, NELEMS(sBerryInitTable));
}

static void TryLoadingSave(int unused, SaveData *saveData)
{
    if (!SaveData_Load(saveData)) {
        OS_ResetSystem(RESET_CLEAN);
    }
}

static void StartNewSave(int unused, SaveData *saveData)
{
    SaveData_Clear(saveData);
    InitPlayerStartLocation(saveData);
    TrainerInfo_SetMoney(SaveData_GetTrainerInfo(saveData), 3000);
    SystemFlag_SetBagAcquired(SaveData_GetVarsFlags(saveData));
}
