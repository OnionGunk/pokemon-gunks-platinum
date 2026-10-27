#include <nitro.h>
#include <nitro/sinit.h>
#include <string.h>

#include "applications/dexnav.h"
#include "applications/poketch/dexnav/graphics.h"
#include "applications/poketch/poketch_button.h"
#include "applications/poketch/poketch_system.h"
#include "field/field_system.h"

#include "bg_window.h"
#include "dexnav_search.h"
#include "heap.h"
#include "location.h"
#include "pokedex.h"
#include "poketch.h"
#include "sys_task.h"
#include "sys_task_manager.h"
#include "touch_screen.h"

// DexNav Pokétch app: the Pokemon of the current area the player has seen. Tapping one sets it as the
// DexNav's target (the R button then searches for it), plays its cry and makes its icon hop.

#define MON_HITBOX(r, c)                                                               \
    {                                                                                  \
        .rect = {                                                                      \
            .top = (48 + POKETCH_DEXNAV_ICON_STEP_Y * (r)) - (36 / 2),                 \
            .bottom = (48 + POKETCH_DEXNAV_ICON_STEP_Y * (r)) + (36 / 2),              \
            .left = (48 + POKETCH_DEXNAV_ICON_STEP_X * (c)) - (48 / 2),                \
            .right = (48 + POKETCH_DEXNAV_ICON_STEP_X * (c)) + (48 / 2)                \
        }                                                                              \
    }

typedef struct PoketchDexNav {
    u8 state;
    u8 subState;
    u8 shouldExit;
    PoketchDexNavData data;
    PoketchDexNavGraphics *graphics;
    PoketchSystem *poketchSys;
    PoketchButtonManager *buttonManager;
    u32 buttonState;
    u32 pressedIdx;
    int mapHeaderID;
} PoketchDexNav;

typedef BOOL (*StateFunc)(PoketchDexNav *);

enum PoketchDexNavState {
    STATE_LOAD_APP = 0,
    STATE_UPDATE_LOOP,
    STATE_RELOAD,
    STATE_SHUTDOWN,
};

static void NitroStaticInit(void);

static BOOL New(void **appData, PoketchSystem *poketchSys, BgConfig *bgConfig, u32 appID);
static BOOL Init(PoketchDexNav *appData, PoketchSystem *poketchSys, BgConfig *bgConfig, u32 appID);
static void Free(PoketchDexNav *appData);
static void Exit(void *appData);
static void TouchCallback(u32 buttonID, u32 buttonState, u32 touchState, void *appData);

static void Task_Main(SysTask *task, void *appData);
static void ChangeState(PoketchDexNav *appData, enum PoketchDexNavState newState);
static BOOL State_LoadApp(PoketchDexNav *appData);
static BOOL State_UpdateApp(PoketchDexNav *appData);
static BOOL State_Reload(PoketchDexNav *appData);
static BOOL State_UnloadApp(PoketchDexNav *appData);

static void NitroStaticInit(void)
{
    PoketchSystem_SetAppFunctions(New, Exit);
}

static BOOL New(void **appData, PoketchSystem *poketchSys, BgConfig *bgConfig, u32 appID)
{
    PoketchDexNav *dexNav = Heap_Alloc(HEAP_ID_POKETCH_APP, sizeof(PoketchDexNav));

    if (dexNav != NULL) {
        if (Init(dexNav, poketchSys, bgConfig, appID)) {
            if (SysTask_Start(Task_Main, dexNav, 1) != NULL) {
                *appData = dexNav;
                return TRUE;
            }
        }

        Heap_Free(dexNav);
    }

    return FALSE;
}

// The seen Pokemon of this area, land first then water, without repeats.
static void BuildAreaList(PoketchDexNav *appData)
{
    FieldSystem *fieldSystem = appData->poketchSys->fieldSystem;
    Pokedex *pokedex = SaveData_GetPokedex(appData->poketchSys->saveData);
    DexNavData *area = Heap_Alloc(HEAP_ID_POKETCH_APP, sizeof(DexNavData));

    memset(area, 0, sizeof(DexNavData));
    area->saveData = appData->poketchSys->saveData;
    DexNav_BuildAreaData(fieldSystem, area);

    appData->data.count = 0;

    for (int i = 0; i < DEXNAV_TOTAL_SLOTS && appData->data.count < POKETCH_DEXNAV_MAX_MONS; i++) {
        u16 species = area->slots[i].species;
        BOOL listed = FALSE;

        if (species == SPECIES_NONE || !Pokedex_HasSeenSpecies(pokedex, species)) {
            continue;
        }

        for (int j = 0; j < appData->data.count; j++) {
            listed |= appData->data.species[j] == species;
        }

        if (!listed) {
            appData->data.species[appData->data.count++] = species;
        }
    }

    Heap_Free(area);

    appData->data.selectedSpecies = DexNavSave_GetSelectedSpecies(appData->poketchSys->saveData);
    appData->mapHeaderID = fieldSystem->location->mapHeaderID;
}

static BOOL Init(PoketchDexNav *appData, PoketchSystem *poketchSys, BgConfig *bgConfig, u32 appID)
{
    static const TouchScreenRect sHitboxes[] = {
        MON_HITBOX(0, 0),
        MON_HITBOX(0, 1),
        MON_HITBOX(0, 2),
        MON_HITBOX(0, 3),
        MON_HITBOX(1, 0),
        MON_HITBOX(1, 1),
        MON_HITBOX(1, 2),
        MON_HITBOX(1, 3),
        MON_HITBOX(2, 0),
        MON_HITBOX(2, 1),
        MON_HITBOX(2, 2),
        MON_HITBOX(2, 3),
    };

    appData->poketchSys = poketchSys;
    BuildAreaList(appData);

    if (PoketchDexNavGraphics_New(&appData->graphics, &appData->data, bgConfig)) {
        appData->state = STATE_LOAD_APP;
        appData->subState = 0;
        appData->shouldExit = FALSE;
        appData->buttonManager = PoketchButtonManager_New(sHitboxes, NELEMS(sHitboxes), TouchCallback, appData, HEAP_ID_POKETCH_APP);
        appData->buttonState = BUTTON_MANAGER_STATE_NULL;
        appData->pressedIdx = 0;

        return TRUE;
    }

    return FALSE;
}

static void Free(PoketchDexNav *appData)
{
    PoketchButtonManager_Free(appData->buttonManager);
    PoketchDexNavGraphics_Free(appData->graphics);

    Heap_Free(appData);
}

static void Task_Main(SysTask *task, void *appData)
{
    static const StateFunc stateFuncs[] = {
        State_LoadApp,
        State_UpdateApp,
        State_Reload,
        State_UnloadApp
    };

    PoketchDexNav *dexNav = appData;

    if (dexNav->state < NELEMS(stateFuncs)) {
        PoketechSystem_UpdateButtonManager(dexNav->poketchSys, dexNav->buttonManager);

        if (stateFuncs[dexNav->state](dexNav)) {
            Free(dexNav);
            SysTask_Done(task);
            PoketchSystem_NotifyAppUnloaded(dexNav->poketchSys);
        }
    }
}

static void Exit(void *appData)
{
    ((PoketchDexNav *)appData)->shouldExit = TRUE;
}

static void TouchCallback(u32 buttonID, u32 buttonState, u32 touchState, void *appData)
{
    PoketchDexNav *dexNav = appData;

    dexNav->pressedIdx = buttonID;
    dexNav->buttonState = buttonState;
}

static void ChangeState(PoketchDexNav *appData, enum PoketchDexNavState newState)
{
    if (!appData->shouldExit) {
        appData->state = newState;
    } else {
        appData->state = STATE_SHUTDOWN;
    }

    appData->subState = 0;
}

static BOOL State_LoadApp(PoketchDexNav *appData)
{
    switch (appData->subState) {
    case 0:
        PoketchDexNavGraphics_StartTask(appData->graphics, POKETCH_DEXNAV_GRAPHICS_INIT);
        appData->subState++;
        break;
    case 1:
        if (PoketchDexNavGraphics_TaskIsNotActive(appData->graphics, POKETCH_DEXNAV_GRAPHICS_INIT)) {
            PoketchSystem_NotifyAppLoaded(appData->poketchSys);
            ChangeState(appData, STATE_UPDATE_LOOP);
        }
        break;
    }

    return FALSE;
}

static BOOL State_UpdateApp(PoketchDexNav *appData)
{
    if (appData->shouldExit) {
        ChangeState(appData, STATE_SHUTDOWN);
        return FALSE;
    }

    // walking onto another map changes which Pokemon can be found
    if (appData->poketchSys->fieldSystem->location->mapHeaderID != appData->mapHeaderID) {
        ChangeState(appData, STATE_RELOAD);
        return FALSE;
    }

    if (appData->buttonState == BUTTON_MANAGER_STATE_TOUCH) {
        if (appData->pressedIdx < appData->data.count) {
            u16 species = appData->data.species[appData->pressedIdx];

            DexNavSave_SetSelectedSpecies(appData->poketchSys->saveData, species);
            appData->data.selectedSpecies = species;
            PoketchSystem_PlayCry(species, 0);
            PoketchDexNavGraphics_StartTask(appData->graphics, POKETCH_DEXNAV_GRAPHICS_UPDATE_TITLE);
        }

        appData->buttonState = BUTTON_MANAGER_STATE_NULL;
    }

    return FALSE;
}

static BOOL State_Reload(PoketchDexNav *appData)
{
    switch (appData->subState) {
    case 0:
        PoketchDexNavGraphics_StartTask(appData->graphics, POKETCH_DEXNAV_GRAPHICS_FREE);
        appData->subState++;
        break;
    case 1:
        if (PoketchDexNavGraphics_NoActiveTasks(appData->graphics)) {
            BuildAreaList(appData);
            PoketchDexNavGraphics_StartTask(appData->graphics, POKETCH_DEXNAV_GRAPHICS_INIT);
            appData->subState++;
        }
        break;
    case 2:
        if (PoketchDexNavGraphics_TaskIsNotActive(appData->graphics, POKETCH_DEXNAV_GRAPHICS_INIT)) {
            ChangeState(appData, STATE_UPDATE_LOOP);
        }
        break;
    }

    return FALSE;
}

static BOOL State_UnloadApp(PoketchDexNav *appData)
{
    switch (appData->subState) {
    case 0:
        PoketchDexNavGraphics_StartTask(appData->graphics, POKETCH_DEXNAV_GRAPHICS_FREE);
        appData->subState++;
        break;
    case 1:
        if (PoketchDexNavGraphics_NoActiveTasks(appData->graphics)) {
            return TRUE;
        }
        break;
    }

    return FALSE;
}
