#include "macros/scrcmd.inc"
#define LOCALID_POKEMON_FOLLOWER 253

    ScriptEntry IronIslandB1FRightRoom_OnTransition
    ScriptEntry IronIslandB1FRightRoom_PokemonFollower5
    ScriptEntryEnd

IronIslandB1FRightRoom_OnTransition:
    InitPersistedMapFeaturesForPlatformLift
    End

IronIslandB1FRightRoom_PokemonFollower5:
    GetPlayerDir VAR_RESULT
    CallIfEq VAR_RESULT, 0, IronIslandB1FRightRoom_PokemonFollower1
    CallIfEq VAR_RESULT, 1, IronIslandB1FRightRoom_PokemonFollower2
IronIslandB1FRightRoom_CoordEvent_PlatformLift:
    TriggerPlatformLift
    End

    .balign 4, 0

IronIslandB1FRightRoom_PokemonFollower1:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, IronIslandB1FRightRoom_Movement_PokemonFollower3
    WaitMovement
    Return

IronIslandB1FRightRoom_PokemonFollower2:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, IronIslandB1FRightRoom_Movement_PokemonFollower4
    WaitMovement
    Return

    .balign 4, 0
IronIslandB1FRightRoom_Movement_PokemonFollower3:
    WalkNormalEast
    WalkNormalNorth
    FaceSouth
    EndMovement

    .balign 4, 0
IronIslandB1FRightRoom_Movement_PokemonFollower4:
    WalkNormalEast
    WalkNormalSouth
    FaceSouth
    EndMovement
