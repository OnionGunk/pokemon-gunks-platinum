#ifndef POKEPLATINUM_DEXNAV_SEARCH_H
#define POKEPLATINUM_DEXNAV_SEARCH_H

#include "constants/heap.h"

#include "field/field_system_decl.h"

#include "pokemon.h"
#include "savedata.h"

#include "constants/species.h"

// Normal block (only ~100 bytes free there): just the player's selection.
// repel steps kept topped up while the Endless Repel is on
#define ENDLESS_REPEL_STEPS 250

typedef struct DexNavSave {
    u16 selectedSpecies;
    // quality of life, in the block's spare field: the Repel to offer when one wears off (item ids are
    // below 512), the last Poke Ball thrown for the battle's Quick Ball (0 in saves from before it,
    // which were always 0 in these bits), and the Endless Repel's on/off state
    u16 lastRepel : 9;
    u16 lastBall : 5;
    u16 endlessRepelOn : 1;
    u16 expAllOn : 1;
} DexNavSave;

// Box block: rides in the ~3.5 KB of slack left by rounding the PC boxes up to whole sectors.
// One search level per species, indexed directly by species ID.
typedef struct DexNavLevels {
    u8 searchLevel[MAX_SPECIES];
} DexNavLevels;

int DexNavSave_SaveSize(void);
void DexNavSave_Init(DexNavSave *dexNavSave);
int DexNavLevels_SaveSize(void);
void DexNavLevels_Init(DexNavLevels *dexNavLevels);
u8 DexNavSave_GetSearchLevel(SaveData *saveData, u16 species);
u16 DexNavSave_GetSelectedSpecies(SaveData *saveData);
void DexNavSave_SetSelectedSpecies(SaveData *saveData, u16 species);
u16 DexNavSave_GetLastRepel(SaveData *saveData);
void DexNavSave_SetLastRepel(SaveData *saveData, u16 item);
BOOL DexNavSave_IsEndlessRepelOn(SaveData *saveData);
void DexNavSave_SetEndlessRepelOn(SaveData *saveData, BOOL on);
u16 DexNavSave_GetLastBall(SaveData *saveData);
void DexNavSave_SetLastBall(SaveData *saveData, u16 item);
BOOL DexNavSave_IsExpAllOn(SaveData *saveData);
void DexNavSave_SetExpAllOn(SaveData *saveData, BOOL on);

// Called by the DexNav screen when the player picks a species to search for.
void DexNavSearch_Request(u16 species, u8 level);

// Called once the field is back: rolls the hidden Pokemon and places a shaking grass patch near the player.
// Returns FALSE when no searchable grass is close enough.
BOOL DexNavSearch_TryStart(FieldSystem *fieldSystem);

// Per-step upkeep: keeps the patch shaking, redraws the search bar, and drops the search if the player wanders off.
void DexNavSearch_Update(FieldSystem *fieldSystem);
BOOL DexNavSearch_Tick(FieldSystem *fieldSystem);
void DexNavSearch_OnWildEncounter(u16 species);

// Called by the field map as it shuts down and once it is running again, so a search survives menus and battles.
void DexNavSearch_OnFieldExit(void);
void DexNavSearch_OnFieldReturn(FieldSystem *fieldSystem);

// TRUE when the player has stepped into the search patch.
BOOL DexNavSearch_IsPlayerInPatch(FieldSystem *fieldSystem, int playerX, int playerZ);

BOOL DexNavSearch_IsActive(void);
BOOL DexNavSearch_IsRequested(void);

// Builds the rolled Pokemon for the battle, raises its search level and the chain, and ends the search.
Pokemon *DexNavSearch_CreateEncounterMon(FieldSystem *fieldSystem, enum HeapID heapID);

// Ends the search. Pass FALSE for removeFieldObjects when the field has already been torn down.
void DexNavSearch_End(BOOL removeFieldObjects);

#endif // POKEPLATINUM_DEXNAV_SEARCH_H
