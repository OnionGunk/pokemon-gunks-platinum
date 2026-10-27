#ifndef POKEPLATINUM_FIELD_MAP_CHANGE_H
#define POKEPLATINUM_FIELD_MAP_CHANGE_H

#include "pokemon.h"
#include "savedata.h"
#include "generated/map_headers.h"

#include "constants/pokemon_follower.h"

#include "field/field_system_decl.h"
#include "overlay005/save_info_window.h"
#include "overlay006/field_warp.h"

#include "bg_window.h"
#include "field_task.h"
#include "location.h"
#include "menu.h"
#include "overworld_anim_manager.h"
#include "string.h"

typedef struct MapChangeUndergroundContext {
    int state;
    int transitionState;
    enum MapHeaderID mapHeaderID;
    int dummy;
    int destX;
    int destZ;
    BOOL animationDone;
    u16 saveResult;
    SaveInfoWindow *saveInfoWin;
    Window window;
    String *string;
    u8 printerID;
    Menu *menu;
} MapChangeUndergroundContext;

void FieldMapChange_Set3DDisplay(FieldSystem *fieldSystem);
void FieldMapChange_UpdateGameData(FieldSystem *fieldSystem, BOOL param1);
void FieldMapChange_UpdateGameDataDistortionWorld(FieldSystem *fieldSystem, BOOL param1);
void sub_02053494(FieldSystem *fieldSystem);
void FieldSystem_SetLoadNewGameSpawnTask(FieldSystem *fieldSystem);
void FieldSystem_SetLoadSavedGameMapTask(FieldSystem *fieldSystem);
void FieldSystem_StartLoadMapFromErrorTask(FieldSystem *fieldSystem);
void FieldSystem_StartChangeMapTask(FieldTask *task, const Location *param1);
void FieldTask_ChangeMapByLocation(FieldTask *task, const Location *param1);
void FieldTask_ChangeMapToLocation(FieldTask *task, enum MapHeaderID mapHeaderID, int warpId, int x, int z, int dir);
void FieldTask_StartMapChangeFull(FieldTask *task, enum MapHeaderID mapHeaderID, int warpId, int x, int z, int dir);
void FieldTask_StartMapChangeFly(FieldSystem *fieldSystem, enum MapHeaderID mapHeaderID, int warpId, int x, int z, int dir);
void FieldTask_ChangeMapChangeFly(FieldTask *task, enum MapHeaderID mapHeaderID, int warpId, int x, int z, int dir);
void FieldTask_ChangeMapByFieldWarp(FieldTask *task, const Location *location, enum FieldWarpType param2);
void FieldSystem_StartMapChangeWarpTask(FieldSystem *fieldSystem, int param1, int param2);
MapChangeUndergroundContext *MapChangeUndergroundContext_New(FieldSystem *fieldSystem);
void FieldTask_SetUndergroundMapChange(FieldSystem *fieldSystem);
BOOL FieldTask_MapChangeToUnderground(FieldTask *task);
BOOL FieldTask_MapChangeFromUnderground(FieldTask *task);
FieldTaskFunc FieldMapChange_GetMapChangeUndergroundTask(const FieldSystem *fieldSystem);
void sub_020544F0(FieldTask *task, const Location *param1);
void sub_020545EC(FieldSystem *fieldSystem);
void sub_02054708(FieldTask *task);
void FieldTask_StartChangeMapColosseum(FieldTask *task, enum MapHeaderID mapHeaderID, int warpId, int x, int z, int dir);
void sub_02054864(FieldTask *task);

int PokemonFollower_GetLeadSlot(SaveData *saveData);
Pokemon *PokemonFollower_GetLead(SaveData *saveData);
int PokemonFollower_GraphicsID(Pokemon *mon);
BOOL PokemonFollower_CanSpawn(int species, enum MapHeaderID mapHeaderID);
MapObject *PokemonFollower_GetMapObject(FieldSystem *fieldSystem);
void PokemonFollower_SetFlag(MapObject *follower, int flag, BOOL set);
void PokemonFollower_SetScriptHidden(MapObject *follower, BOOL hidden);
void PokemonFollower_ReturnToPlayer(FieldSystem *fieldSystem, BOOL release);
OverworldAnimManager *PokemonFollower_RecallToBall(FieldSystem *fieldSystem);
void FieldMapChange_CreatePokemonFollower(FieldSystem *fieldSystem);
OverworldAnimManager *PokemonFollower_RecallIfCannotEnter(FieldSystem *fieldSystem, enum MapHeaderID mapHeaderID);

#endif // POKEPLATINUM_FIELD_MAP_CHANGE_H
