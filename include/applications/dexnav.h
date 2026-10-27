#ifndef POKEPLATINUM_APPLICATIONS_DEXNAV_H
#define POKEPLATINUM_APPLICATIONS_DEXNAV_H

#include "overlay_manager.h"
#include "savedata.h"

#define DEXNAV_GRID_COLS      6
#define DEXNAV_GRID_ROWS      2
#define DEXNAV_SECTION_SLOTS  (DEXNAV_GRID_COLS * DEXNAV_GRID_ROWS)
#define DEXNAV_TOTAL_SLOTS    (DEXNAV_SECTION_SLOTS * 2)

enum DexNavMethod {
    DEXNAV_METHOD_GRASS = 0,
    DEXNAV_METHOD_MORNING,
    DEXNAV_METHOD_NIGHT,
    DEXNAV_METHOD_SWARM,
    DEXNAV_METHOD_SURF,
    DEXNAV_METHOD_OLD_ROD,
    DEXNAV_METHOD_GOOD_ROD,
    DEXNAV_METHOD_SUPER_ROD,
};

typedef struct DexNavEntry {
    u16 species;
    u8 method;
    u8 minLevel;
    u8 maxLevel;
} DexNavEntry;

// Built by the field before the app opens: slots 0-11 are land, 12-23 are water.
typedef struct DexNavData {
    SaveData *saveData;
    DexNavEntry slots[DEXNAV_TOTAL_SLOTS];
} DexNavData;

BOOL DexNav_Init(ApplicationManager *appMan, int *state);
BOOL DexNav_Main(ApplicationManager *appMan, int *state);
BOOL DexNav_Exit(ApplicationManager *appMan, int *state);

#endif // POKEPLATINUM_APPLICATIONS_DEXNAV_H
