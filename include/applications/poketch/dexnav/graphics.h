#ifndef POKEPLATINUM_POKETCH_DEXNAV_GRAPHICS_H
#define POKEPLATINUM_POKETCH_DEXNAV_GRAPHICS_H

#include "applications/poketch/poketch_animation.h"
#include "applications/poketch/poketch_task.h"

#include "bg_window.h"

#define POKETCH_DEXNAV_TASK_SLOTS 4

#define POKETCH_DEXNAV_MAX_MONS 12

#define POKETCH_DEXNAV_ICON_STEP_X 40
#define POKETCH_DEXNAV_ICON_STEP_Y 48

// how each Pokemon of the area is drawn
enum PoketchDexNavMonStatus {
    POKETCH_DEXNAV_MON_UNSEEN = 0, // a question mark
    POKETCH_DEXNAV_MON_SEEN, // a silhouette
    POKETCH_DEXNAV_MON_CAUGHT, // the full icon
};

// The Pokemon that can be found in the current area, laid out like the Pokemon History app.
typedef struct PoketchDexNavData {
    u16 species[POKETCH_DEXNAV_MAX_MONS];
    u8 status[POKETCH_DEXNAV_MAX_MONS];
    u32 count;
    u16 selectedSpecies;
} PoketchDexNavData;

typedef struct PoketchDexNavGraphics {
    const PoketchDexNavData *data;
    BgConfig *bgConfig;
    u32 activeTasks[POKETCH_TASK_SLOT_BASE + POKETCH_DEXNAV_TASK_SLOTS];
    PoketchAnimation_AnimationManager *animMan;
    PoketchAnimation_AnimatedSpriteData *sprites[POKETCH_DEXNAV_MAX_MONS];
    PoketchAnimation_SpriteData spriteData;
    u32 iconIndices[POKETCH_DEXNAV_MAX_MONS];
} PoketchDexNavGraphics;

enum PoketchDexNavGraphicsTask {
    POKETCH_DEXNAV_GRAPHICS_INIT = 0,
    POKETCH_DEXNAV_GRAPHICS_FREE,
    POKETCH_DEXNAV_GRAPHICS_UPDATE_TITLE,
};

BOOL PoketchDexNavGraphics_New(PoketchDexNavGraphics **dest, const PoketchDexNavData *data, BgConfig *bgConfig);
void PoketchDexNavGraphics_Free(PoketchDexNavGraphics *graphics);
void PoketchDexNavGraphics_StartTask(PoketchDexNavGraphics *graphics, enum PoketchDexNavGraphicsTask taskID);
BOOL PoketchDexNavGraphics_TaskIsNotActive(PoketchDexNavGraphics *graphics, enum PoketchDexNavGraphicsTask taskID);
BOOL PoketchDexNavGraphics_NoActiveTasks(PoketchDexNavGraphics *graphics);

#endif // POKEPLATINUM_POKETCH_DEXNAV_GRAPHICS_H
