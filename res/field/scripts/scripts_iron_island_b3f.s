#include "macros/scrcmd.inc"
#define LOCALID_POKEMON_FOLLOWER 253

    ScriptEntry IronIslandB3F_OnTransition
    ScriptEntry IronIslandB3F_PokemonFollower5
    ScriptEntry IronIslandB3F_Dummy3
    ScriptEntry IronIslandB3F_OnLoad
    ScriptEntryEnd

IronIslandB3F_OnLoad:
    CheckPartyHasFatefulEncounterRegigigas VAR_MAP_LOCAL_0x04
    GoToIfEq VAR_MAP_LOCAL_0x04, FALSE, IronIslandB3F_RemoveWarpIronRuinsWithRegisteel
    GoToIfEq VAR_MAP_LOCAL_0x04, TRUE, IronIslandB3F_RemoveWarpIronRuinsWithoutRegisteel
    End

IronIslandB3F_OnTransition:
    InitPersistedMapFeaturesForPlatformLift
    CallIfNe VAR_IRON_RUINS_STATE, RUINS_STATE_CAUGHT_REGI, IronIslandB3F_ResetIronRuinsState
    CheckPartyHasFatefulEncounterRegigigas VAR_MAP_LOCAL_0x04
    GoToIfEq VAR_MAP_LOCAL_0x04, FALSE, IronIslandB3F_RemoveWarpIronRuinsWithRegisteel
    GoToIfEq VAR_MAP_LOCAL_0x04, TRUE, IronIslandB3F_RemoveWarpIronRuinsWithoutRegisteel
    End

IronIslandB3F_RemoveWarpIronRuinsWithRegisteel:
    SetWarpEventPos 3, 17, 1
    End

IronIslandB3F_RemoveWarpIronRuinsWithoutRegisteel:
    SetWarpEventPos 2, 17, 1
    End

IronIslandB3F_PokemonFollower5:
    GetPlayerDir VAR_RESULT
    CallIfEq VAR_RESULT, 0, IronIslandB3F_PokemonFollower1
    CallIfEq VAR_RESULT, 1, IronIslandB3F_PokemonFollower2
IronIslandB3F_CoordEvent_PlatformLift:
    TriggerPlatformLift
    End

IronIslandB3F_PokemonFollower1:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, IronIslandB3F_Movement_PokemonFollower3
    WaitMovement
    Return

IronIslandB3F_PokemonFollower2:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, IronIslandB3F_Movement_PokemonFollower4
    WaitMovement
    Return

    .balign 4, 0
IronIslandB3F_Movement_PokemonFollower3:
    WalkNormalEast
    WalkNormalNorth
    FaceSouth
    EndMovement

    .balign 4, 0
IronIslandB3F_Movement_PokemonFollower4:
    WalkNormalEast
    WalkNormalSouth
    FaceSouth
    EndMovement

IronIslandB3F_ResetIronRuinsState:
    SetVar VAR_IRON_RUINS_STATE, 0
    Return

IronIslandB3F_Dummy3:
    End

    .balign 4, 0
