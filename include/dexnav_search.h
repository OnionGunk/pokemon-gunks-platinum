#ifndef POKEPLATINUM_DEXNAV_SEARCH_H
#define POKEPLATINUM_DEXNAV_SEARCH_H

#include "constants/heap.h"

#include "field/field_system_decl.h"

#include "pokemon.h"
#include "savedata.h"

#include "constants/species.h"

// Normal block (only ~100 bytes free there): just the player's selection.
typedef struct DexNavSave {
    u16 selectedSpecies;
    u16 unused;
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

// Called by the DexNav screen when the player picks a species to search for.
void DexNavSearch_Request(u16 species, u8 level);

// Called once the field is back: rolls the hidden Pokemon and places a shaking grass patch near the player.
// Returns FALSE when no searchable grass is close enough.
BOOL DexNavSearch_TryStart(FieldSystem *fieldSystem);

// Per-step upkeep: keeps the patch shaking, redraws the search bar, and drops the search if the player wanders off.
void DexNavSearch_Update(FieldSystem *fieldSystem);

// TRUE when the player has stepped into the search patch.
BOOL DexNavSearch_IsPlayerInPatch(FieldSystem *fieldSystem, int playerX, int playerZ);

BOOL DexNavSearch_IsActive(void);
BOOL DexNavSearch_IsRequested(void);

// Builds the rolled Pokemon for the battle, raises its search level and the chain, and ends the search.
Pokemon *DexNavSearch_CreateEncounterMon(FieldSystem *fieldSystem, enum HeapID heapID);

// Ends the search. Pass FALSE for removeFieldObjects when the field has already been torn down.
void DexNavSearch_End(BOOL removeFieldObjects);

#endif // POKEPLATINUM_DEXNAV_SEARCH_H
