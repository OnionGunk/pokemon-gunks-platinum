#include "applications/poketch/dexnav/graphics.h"

#include <nitro.h>

#include "applications/poketch/poketch_animation.h"
#include "applications/poketch/poketch_graphics.h"
#include "applications/poketch/poketch_task.h"

#include "bg_window.h"
#include "font.h"
#include "heap.h"
#include "message.h"
#include "pokemon_icon.h"
#include "string_gf.h"
#include "sys_task_manager.h"
#include "text.h"

#include "res/graphics/poketch/poketch.naix"

// Same layout as the Pokemon History app: a 4x3 grid of Pokemon icons under a title line.
#define MON_ANIM_DATA(r, c)                                                \
    {                                                                      \
        .translation = { FX32_CONST(48 + POKETCH_DEXNAV_ICON_STEP_X * c),  \
            FX32_CONST(48 + POKETCH_DEXNAV_ICON_STEP_Y * r) },             \
        .animIdx = 0,                                                      \
        .flip = NNS_G2D_RENDERERFLIP_NONE,                                 \
        .oamPriority = 2,                                                  \
        .priority = POKETCH_DEXNAV_MAX_MONS - (4 * r) - c,                 \
        .hasAffineTransform = TRUE,                                        \
    }

#define ICON_ANIM_STILL   0
#define ICON_ANIM_BOUNCE  4

static void EndTask(PoketchTaskManager *taskMan);
static void Task_DrawBackground(SysTask *task, void *taskMan);
static void Task_FreeBackground(SysTask *task, void *taskMan);
static void Task_UpdateTitle(SysTask *task, void *taskMan);
static void DrawTitle(PoketchDexNavGraphics *graphics, const PoketchDexNavData *data);
static void SetupSprites(PoketchDexNavGraphics *graphics, const PoketchDexNavData *data);
static void UpdateSelection(PoketchDexNavGraphics *graphics, const PoketchDexNavData *data);
static void UnloadSprites(PoketchDexNavGraphics *graphics);

BOOL PoketchDexNavGraphics_New(PoketchDexNavGraphics **dest, const PoketchDexNavData *data, BgConfig *bgConfig)
{
    PoketchDexNavGraphics *graphics = Heap_Alloc(HEAP_ID_POKETCH_APP, sizeof(PoketchDexNavGraphics));

    if (graphics != NULL) {
        PoketchTask_InitActiveTaskList(graphics->activeTasks, POKETCH_DEXNAV_TASK_SLOTS);
        graphics->data = data;
        graphics->bgConfig = PoketchGraphics_GetBgConfig();
        graphics->animMan = PoketchGraphics_GetAnimationManager();
        *dest = graphics;

        return TRUE;
    }

    return FALSE;
}

void PoketchDexNavGraphics_Free(PoketchDexNavGraphics *graphics)
{
    if (graphics != NULL) {
        Heap_Free(graphics);
    }
}

static const PoketchTask sDexNavTasks[] = {
    { POKETCH_DEXNAV_GRAPHICS_INIT, Task_DrawBackground, 0 },
    { POKETCH_DEXNAV_GRAPHICS_FREE, Task_FreeBackground, 0 },
    { POKETCH_DEXNAV_GRAPHICS_UPDATE_TITLE, Task_UpdateTitle, 0 },
    { 0 }
};

void PoketchDexNavGraphics_StartTask(PoketchDexNavGraphics *graphics, enum PoketchDexNavGraphicsTask taskID)
{
    PoketchTask_Start(sDexNavTasks, taskID, graphics, graphics->data, graphics->activeTasks, 2, HEAP_ID_POKETCH_APP);
}

BOOL PoketchDexNavGraphics_TaskIsNotActive(PoketchDexNavGraphics *graphics, enum PoketchDexNavGraphicsTask taskID)
{
    return PoketchTask_TaskIsNotActive(graphics->activeTasks, taskID);
}

BOOL PoketchDexNavGraphics_NoActiveTasks(PoketchDexNavGraphics *graphics)
{
    return PoketchTask_NoActiveTasks(graphics->activeTasks);
}

static void EndTask(PoketchTaskManager *taskMan)
{
    PoketchDexNavGraphics *graphics = PoketchTask_GetTaskData(taskMan);
    PoketchTask_EndTask(graphics->activeTasks, taskMan);
}

static void Task_DrawBackground(SysTask *task, void *taskMan)
{
    static const BgTemplate bgTemplate = {
        .x = 0,
        .y = 0,
        .bufferSize = 0x800,
        .baseTile = 0,
        .screenSize = BG_SCREEN_SIZE_256x256,
        .colorMode = GX_BG_COLORMODE_16,
        .screenBase = GX_BG_SCRBASE_0x7000,
        .charBase = GX_BG_CHARBASE_0x00000,
        .bgExtPltt = GX_BG_EXTPLTT_01,
        .priority = 2,
        .areaOver = 0,
        .mosaic = FALSE,
    };

    PoketchDexNavGraphics *graphics = PoketchTask_GetTaskData(taskMan);
    const PoketchDexNavData *data = PoketchTask_GetConstTaskData(taskMan);

    Bg_InitFromTemplate(graphics->bgConfig, BG_LAYER_SUB_2, &bgTemplate, BG_TYPE_STATIC);
    Bg_FillTilesRange(graphics->bgConfig, BG_LAYER_SUB_2, 4, 1, 0);
    Bg_FillTilemapRect(graphics->bgConfig, BG_LAYER_SUB_2, 0, 0, 0, POKETCH_WIDTH_TILES, POKETCH_HEIGHT_TILES, 0);

    PoketchGraphics_LoadActivePalette(0, 0);
    DrawTitle(graphics, data);
    SetupSprites(graphics, data);

    GXSDispCnt dispCnt = GXS_GetDispCnt();
    GXS_SetVisiblePlane(dispCnt.visiblePlane | GX_PLANEMASK_BG2);

    EndTask(taskMan);
}

static void Task_UpdateTitle(SysTask *task, void *taskMan)
{
    PoketchDexNavGraphics *graphics = PoketchTask_GetTaskData(taskMan);
    const PoketchDexNavData *data = PoketchTask_GetConstTaskData(taskMan);

    DrawTitle(graphics, data);
    UpdateSelection(graphics, data);
    EndTask(taskMan);
}

static void Task_FreeBackground(SysTask *task, void *taskMan)
{
    PoketchDexNavGraphics *graphics = PoketchTask_GetTaskData(taskMan);

    UnloadSprites(graphics);
    Bg_FreeTilemapBuffer(graphics->bgConfig, BG_LAYER_SUB_2);
    EndTask(taskMan);
}

// "DEXNAV" on the left, the species being searched for on the right (or a hint when nothing is here).
static void DrawTitle(PoketchDexNavGraphics *graphics, const PoketchDexNavData *data)
{
    Window window;
    String *string;

    Window_Add(graphics->bgConfig, &window, BG_LAYER_SUB_2, 2, 2, 24, 2, 0, 1);
    Window_FillTilemap(&window, 4);
    Window_PutToTilemap(&window);

    string = MessageBank_GetNewStringFromNARC(NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_POKETCH_DEXNAV, data->count == 0 ? 1 : 0, HEAP_ID_POKETCH_APP);

    if (string) {
        Text_AddPrinterWithParamsAndColor(&window, FONT_SYSTEM, string, 0, 0, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 8, 4), NULL);
        String_Free(string);
    }

    if (data->count != 0 && data->selectedSpecies != 0) {
        string = MessageBank_GetNewStringFromNARC(NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_SPECIES_NAME, data->selectedSpecies, HEAP_ID_POKETCH_APP);

        if (string) {
            Text_AddPrinterWithParamsAndColor(&window, FONT_SYSTEM, string, 192 - Font_CalcStringWidth(FONT_SYSTEM, string, 0), 0, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 8, 4), NULL);
            String_Free(string);
        }
    }

    Window_LoadTiles(&window);
    Window_Remove(&window);
    Bg_CopyTilemapBufferToVRAM(graphics->bgConfig, BG_LAYER_SUB_2);
}

static void SetupSprites(PoketchDexNavGraphics *graphics, const PoketchDexNavData *data)
{
    static const PoketchAnimation_AnimationData animData[] = {
        MON_ANIM_DATA(0, 0),
        MON_ANIM_DATA(0, 1),
        MON_ANIM_DATA(0, 2),
        MON_ANIM_DATA(0, 3),
        MON_ANIM_DATA(1, 0),
        MON_ANIM_DATA(1, 1),
        MON_ANIM_DATA(1, 2),
        MON_ANIM_DATA(1, 3),
        MON_ANIM_DATA(2, 0),
        MON_ANIM_DATA(2, 1),
        MON_ANIM_DATA(2, 2),
        MON_ANIM_DATA(2, 3),
    };

    PoketchTask_LoadPokemonIconLuminancePalette(0);
    PoketchAnimation_LoadSpriteFromNARC(&graphics->spriteData, NARC_INDEX_GRAPHIC__POKETCH, poke_icon_cell_NCER_lz, poke_icon_anim_NANR_lz, HEAP_ID_POKETCH_APP);

    int i;
    for (i = 0; i < data->count; i++) {
        graphics->iconIndices[i] = PokeIconSpriteIndex(data->species[i], FALSE, 0);
        graphics->sprites[i] = PoketchAnimation_SetupNewAnimatedSprite(graphics->animMan, &animData[i], &graphics->spriteData);

        PoketchAnimation_SetSpriteCharNo(graphics->sprites[i], i * 16);
        PoketchAnimation_SetCParam(graphics->sprites[i], PokeIconPaletteIndex(data->species[i], 0, FALSE));
    }

    for (; i < POKETCH_DEXNAV_MAX_MONS; i++) {
        graphics->sprites[i] = NULL;
    }

    PoketchTask_LoadPokemonIcons(0, graphics->iconIndices, data->count, FALSE);
    UpdateSelection(graphics, data);
}

// the Pokemon the DexNav is set to search for hops; the rest stay still
static void UpdateSelection(PoketchDexNavGraphics *graphics, const PoketchDexNavData *data)
{
    for (int i = 0; i < data->count; i++) {
        PoketchAnimation_UpdateAnimationIdx(graphics->sprites[i], data->species[i] == data->selectedSpecies ? ICON_ANIM_BOUNCE : ICON_ANIM_STILL);
    }
}

static void UnloadSprites(PoketchDexNavGraphics *graphics)
{
    for (int i = 0; i < POKETCH_DEXNAV_MAX_MONS; i++) {
        if (graphics->sprites[i] != NULL) {
            PoketchAnimation_RemoveAnimatedSprite(graphics->animMan, graphics->sprites[i]);
            graphics->sprites[i] = NULL;
        }
    }

    PoketchAnimation_FreeSpriteData(&graphics->spriteData);
}
