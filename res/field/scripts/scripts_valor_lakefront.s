#include "macros/scrcmd.inc"
#include "res/text/bank/valor_lakefront.h"
#include "res/field/events/events_valor_lakefront.h"
#define LOCALID_POKEMON_FOLLOWER 253

    ScriptEntry ValorLakefront_OnTransition
    ScriptEntry ValorLakefront_OnLoad
    ScriptEntry ValorLakefront_GruntM
    ScriptEntry ValorLakefront_CameramanSouth
    ScriptEntry ValorLakefront_Beauty
    ScriptEntry ValorLakefront_CameramanNorth
    ScriptEntry ValorLakefront_SignboardSevenStarsRestaurant
    ScriptEntry ValorLakefront_Collector
    ScriptEntry ValorLakefront_CoordEvent_BlockSunyshore
    ScriptEntryEnd

ValorLakefront_OnTransition:
    CallIfSet FLAG_TALKED_TO_VALOR_LAKEFRONT_GRUNT_M, ValorLakefront_SetGruntMPositionNorth
    GoToIfUnset FLAG_GALACTIC_LEFT_LAKE_VALOR, ValorLakefront_RemoveWarpsLakeValorNormal
    GoToIfSet FLAG_GALACTIC_LEFT_LAKE_VALOR, ValorLakefront_RemoveWarpsLakeValorDrained
    End
    End

ValorLakefront_SetGruntMPositionNorth:
    SetObjectEventPos LOCALID_GRUNT_M, 723, 769
    SetObjectEventDir LOCALID_GRUNT_M, DIR_NORTH
    SetObjectEventMovementType LOCALID_GRUNT_M, MOVEMENT_TYPE_LOOK_NORTH
    Return

ValorLakefront_OnLoad:
    GoToIfUnset FLAG_GALACTIC_LEFT_LAKE_VALOR, ValorLakefront_RemoveWarpsLakeValorNormal
    GoToIfSet FLAG_GALACTIC_LEFT_LAKE_VALOR, ValorLakefront_RemoveWarpsLakeValorDrained
    End

ValorLakefront_RemoveWarpsLakeValorNormal:
    SetWarpEventPos 5, 713, 760
    SetWarpEventPos 6, 713, 761
    End

ValorLakefront_RemoveWarpsLakeValorDrained:
    SetWarpEventPos 3, 713, 760
    SetWarpEventPos 4, 713, 761
    End

ValorLakefront_GruntM:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    GoToIfSet FLAG_TALKED_TO_VALOR_LAKEFRONT_GRUNT_M, ValorLakefront_GruntMNorth
    Message ValorLakefront_Text_WhyAmIRunning
    FacePlayer
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_GruntMExclamationMark
    WaitMovement
    Message ValorLakefront_Text_YouWereEavesdropping
    CloseMessage
    GetPlayerDir VAR_0x8004
    GoToIfEq VAR_0x8004, DIR_SOUTH, ValorLakefront_PokemonFollower1
    GoToIfEq VAR_0x8004, DIR_NORTH, ValorLakefront_GruntMLeaveNorth
    GoTo ValorLakefront_GruntMLeaveWestEast

ValorLakefront_PokemonFollower1:
    ScrCmd_Unused_06A LOCALID_POKEMON_FOLLOWER, VAR_0x8006, VAR_0x8007
    GoToIfEq VAR_0x8006, 720, ValorLakefront_PokemonFollower2
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_GruntMLeaveSouth
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchGruntMLeaveSouth
    GoTo ValorLakefront_GruntMLeaveSouth

ValorLakefront_PokemonFollower2:
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_PokemonFollower3
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PokemonFollower4
ValorLakefront_GruntMLeaveSouth:
    WaitMovement
    GoTo ValorLakefront_MoveGruntMNorth

ValorLakefront_GruntMLeaveNorth:
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_GruntMLeaveNorthWestEast
    WaitMovement
    GoTo ValorLakefront_MoveGruntMNorth

ValorLakefront_GruntMLeaveWestEast:
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_GruntMLeaveNorthWestEast
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchGruntMLeaveWestEast
    WaitMovement
    GoTo ValorLakefront_MoveGruntMNorth

ValorLakefront_MoveGruntMNorth:
    SetPosition LOCALID_GRUNT_M, 723, 1, 769, DIR_NORTH
    SetFlag FLAG_TALKED_TO_VALOR_LAKEFRONT_GRUNT_M
    ReleaseAll
    End

ValorLakefront_GruntMNorth:
    GetPlayerDir VAR_0x8000
    ScrCmd_Unused_06A LOCALID_POKEMON_FOLLOWER, VAR_0x8006, VAR_0x8007
    FacePlayer
    Message ValorLakefront_Text_MyPokemonWillKOYou
    CloseMessage
    StartTrainerBattle TRAINER_GALACTIC_GRUNT_VALOR_LAKEFRONT
    CheckWonBattle VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, ValorLakefront_BlackOut
    Message ValorLakefront_Text_ICantBattleAnymore
    CloseMessage
    GetPlayerDir VAR_0x8004
    GoToIfEq VAR_0x8004, DIR_SOUTH, ValorLakefront_PokemonFollower5
    GoToIfEq VAR_0x8004, DIR_NORTH, ValorLakefront_GruntMNorthLeaveNorth
    GoTo ValorLakefront_GruntMNorthLeaveWestEast

ValorLakefront_PokemonFollower5:
    GoToIfEq VAR_0x8006, 724, ValorLakefront_PokemonFollower6
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_GruntMNorthLeaveSouth
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchGruntMNorthLeaveSouth
    GoTo ValorLakefront_GruntMNorthLeaveSouth

ValorLakefront_PokemonFollower6:
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_PokemonFollower7
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PokemonFollower8

ValorLakefront_GruntMNorthLeaveSouth:
    WaitMovement
    GoTo ValorLakefront_RemoveGruntM

ValorLakefront_GruntMNorthLeaveNorth:
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_GruntMNorthLeaveNorthWestEast
    WaitMovement
    GoTo ValorLakefront_RemoveGruntM

ValorLakefront_GruntMNorthLeaveWestEast:
    ApplyMovement LOCALID_GRUNT_M, ValorLakefront_Movement_GruntMNorthLeaveNorthWestEast
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchGruntMNorthLeaveWestEast
    WaitMovement
    GoTo ValorLakefront_RemoveGruntM

ValorLakefront_RemoveGruntM:
    RemoveObject LOCALID_GRUNT_M
    SetFlag FLAG_HIDE_GRAND_LAKE_ROUTE_213_LOBBY_LOOKER
    ClearFlag FLAG_BLOCK_PASTORIA_CITY_CROAGUNK_EVENT
    GoTo ValorLakefront_Cynthia
    End

ValorLakefront_BlackOut:
    BlackOutFromBattle
    ReleaseAll
    End

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchGruntMLeaveSouth:
    WalkOnSpotNormalEast
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower4:
    WalkOnSpotNormalWest
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchGruntMLeaveWestEast:
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchGruntMNorthLeaveSouth:
    Delay8
    WalkOnSpotNormalEast
    Delay8
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower8:
    Delay8
    WalkOnSpotNormalWest

    Delay8
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchGruntMNorthLeaveWestEast:
    Delay8
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_GruntMExclamationMark:
    EmoteExclamationMark
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_GruntMLeaveSouth:
    WalkFastEast
    WalkFastNorth 11
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower3:
    WalkFastWest
    WalkFastNorth 11
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_GruntMLeaveNorthWestEast:
    WalkFastNorth 10
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_GruntMNorthLeaveSouth:
    WalkNormalEast
    WalkSlowNorth 9
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower7:
    WalkNormalWest

    WalkSlowNorth 9
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_GruntMNorthLeaveNorthWestEast:
    WalkNormalNorth
    WalkSlowNorth 9
    EndMovement

ValorLakefront_Cynthia:
    CallIfEq VAR_0x8000, DIR_NORTH, ValorLakefront_SetCynthiaPositionNorth
    CallIfEq VAR_0x8000, DIR_SOUTH, ValorLakefront_PokemonFollower9
    CallIfEq VAR_0x8000, DIR_WEST, ValorLakefront_PokemonFollower10
    CallIfEq VAR_0x8000, DIR_EAST, ValorLakefront_PokemonFollower11
    ClearFlag FLAG_HIDE_VALOR_LAKEFRONT_CYNTHIA
    SetObjectEventDir LOCALID_CYNTHIA, DIR_SOUTH
    SetObjectEventMovementType LOCALID_CYNTHIA, MOVEMENT_TYPE_LOOK_SOUTH
    AddObject LOCALID_CYNTHIA
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_CynthiaEnter
    WaitMovement
    Message ValorLakefront_Text_IHadToTalk
    CloseMessage
    CallIfEq VAR_0x8000, DIR_NORTH, ValorLakefront_PokemonFollower12
    CallIfEq VAR_0x8000, DIR_SOUTH, ValorLakefront_PokemonFollower13
    CallIfEq VAR_0x8000, DIR_WEST, ValorLakefront_PokemonFollower14
    CallIfEq VAR_0x8000, DIR_EAST, ValorLakefront_PokemonFollower15
    ClearFlag FLAG_HIDE_VALOR_LAKEFRONT_RIVAL
    SetObjectEventDir LOCALID_RIVAL, DIR_NORTH
    SetObjectEventMovementType LOCALID_RIVAL, MOVEMENT_TYPE_LOOK_NORTH
    AddObject LOCALID_RIVAL
    Common_SetRivalBGM
    CallIfEq VAR_0x8000, DIR_NORTH, ValorLakefront_RivalEnterNorth
    CallIfEq VAR_0x8000, DIR_SOUTH, ValorLakefront_RivalEnterSouth
    CallIfEq VAR_0x8000, DIR_WEST, ValorLakefront_RivalEnterWest
    CallIfEq VAR_0x8000, DIR_EAST, ValorLakefront_RivalEnterEast
    BufferPlayerName 0
    BufferRivalName 1
    Message ValorLakefront_Text_WheredThatGoonGo
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalWalkOnSpotNorth
    WaitMovement
    Message ValorLakefront_Text_IsThisYourSister
    CallIfEq VAR_0x8000, DIR_NORTH, ValorLakefront_RivalFacePlayerNorth
    CallIfEq VAR_0x8000, DIR_SOUTH, ValorLakefront_RivalFacePlayerSouth
    CallIfEq VAR_0x8000, DIR_WEST, ValorLakefront_RivalFacePlayerWest
    CallIfEq VAR_0x8000, DIR_EAST, ValorLakefront_RivalFacePlayerEast
    Message ValorLakefront_Text_SeeYouAround
    CloseMessage
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_CynthiaWatchRivalLeave
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchRivalLeave
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalLeave
    WaitMovement
    RemoveObject LOCALID_RIVAL
    Common_FadeToDefaultMusic2
    Message ValorLakefront_Text_HesYourFriend
    CloseMessage
    CallIfEq VAR_0x8000, DIR_NORTH, ValorLakefront_CynthiaFacePlayerNorth
    CallIfEq VAR_0x8000, DIR_SOUTH, ValorLakefront_CynthiaFacePlayerSouth
    CallIfEq VAR_0x8000, DIR_WEST, ValorLakefront_CynthiaFacePlayerWest
    CallIfEq VAR_0x8000, DIR_EAST, ValorLakefront_CynthiaFacePlayerEast
    Message ValorLakefront_Text_HaveYouSeenPsyduck
    ShowYesNoMenu VAR_RESULT
    GoToIfEq VAR_RESULT, MENU_YES, ValorLakefront_YeahThatsRight
    GoToIfEq VAR_RESULT, MENU_NO, ValorLakefront_YouHaventSeen
    End

ValorLakefront_SetCynthiaPositionNorth:
    SetObjectEventPos LOCALID_CYNTHIA, 723, 761
    Return

ValorLakefront_PokemonFollower9:
    CallIfEq VAR_0x8007, 767, ValorLakefront_PokemonFollower16
ValorLakefront_SetCynthiaPositionSouth:
    SetObjectEventPos LOCALID_CYNTHIA, 723, 759
    Return

ValorLakefront_PokemonFollower16:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PokemonFollower17
    Return

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower17:
    Delay8 2
    FaceNorth
    Delay8 3
    WalkNormalWest
    WalkNormalSouth
    FaceNorth
    EndMovement

ValorLakefront_PokemonFollower10:
    CallIfEq VAR_0x8007, 768, ValorLakefront_PokemonFollower18
ValorLakefront_SetCynthiaPositionWest:
    SetObjectEventPos LOCALID_CYNTHIA, 724, 760
    Return

ValorLakefront_PokemonFollower18:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PokemonFollower19
    Return

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower19:
    Delay8 2
    FaceNorth
    Delay8 3
    WalkNormalEast
    WalkNormalSouth
    FaceNorth
    EndMovement

ValorLakefront_PokemonFollower11:
    CallIfEq VAR_0x8007, 768, ValorLakefront_PokemonFollower16
ValorLakefront_SetCynthiaPositionEast:
    SetObjectEventPos LOCALID_CYNTHIA, 722, 760
    Return

ValorLakefront_CynthiaFacePlayerNorth:
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_CynthiaWalkOnSpotSouth
    WaitMovement
    Return

ValorLakefront_CynthiaFacePlayerSouth:
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_CynthiaWalkOnSpotSouth
    WaitMovement
    Return

ValorLakefront_CynthiaFacePlayerWest:
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_CynthiaWalkOnSpotSouth
    WaitMovement
    Return

ValorLakefront_CynthiaFacePlayerEast:
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_CynthiaWalkOnSpotSouth
    WaitMovement
    Return

ValorLakefront_PokemonFollower12:
    CallIfEq VAR_0x8007, 771, ValorLakefront_PokemonFollower20
    CallIfEq VAR_0x8006, 724, ValorLakefront_PokemonFollower21
ValorLakefront_SetRivalPositionNorth:
    SetObjectEventPos LOCALID_RIVAL, 724, 780
    Return

ValorLakefront_PokemonFollower20:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PokemonFollower22
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_PokemonFollower23
    Return

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower22:
    FaceSouth
    Delay8
    JumpOnSpotFastSouth
    WalkFastEast
    WalkFastNorth 3
    WalkFastWest 2
    WalkFastSouth 2
    FaceEast
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower23:
    Delay8 2
    Delay4
    FaceEast
    Delay4 2
    FaceNorth
    Delay4 2
    FaceWest
    Delay4
    FaceSouth
    EndMovement

ValorLakefront_PokemonFollower21:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PokemonFollower24
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_PokemonFollower25
    Return

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower24:
    FaceSouth
    Delay8
    JumpOnSpotFastSouth
    WalkFastNorth 2
    WalkFastWest 2
    WalkFastSouth 2
    FaceEast
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower25:
    Delay8
    Delay4
    FaceEast
    Delay4 2
    FaceNorth
    Delay4 2
    FaceWest
    Delay4
    FaceSouth
    EndMovement

ValorLakefront_PokemonFollower13:
    CallIfEq VAR_0x8006, 724, ValorLakefront_PokemonFollower26
ValorLakefront_SetRivalPositionSouth:
    SetObjectEventPos LOCALID_RIVAL, 724, 778
    Return

ValorLakefront_PokemonFollower26:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PokemonFollower24
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_PokemonFollower25
    Return

ValorLakefront_PokemonFollower14:
    CallIfEq VAR_0x8007, 770, ValorLakefront_PokemonFollower27
ValorLakefront_SetRivalPositionWest:
    SetObjectEventPos LOCALID_RIVAL, 723, 779
    Return

ValorLakefront_PokemonFollower27:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PokemonFollower28
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_PokemonFollower29
    Return

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower28:
    FaceSouth
    Delay8
    JumpOnSpotFastSouth
    WalkFastWest
    WalkFastNorth 3
    WalkFastEast 2
    WalkFastSouth 2
    FaceWest
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PokemonFollower29:
    Delay8 2
    Delay4
    FaceWest
    Delay4 2
    FaceNorth
    Delay4 2
    FaceEast
    Delay4
    FaceSouth
    EndMovement

ValorLakefront_PokemonFollower15:
    CallIfEq VAR_0x8007, 770, ValorLakefront_PokemonFollower30
ValorLakefront_SetRivalPositionEast:
    SetObjectEventPos LOCALID_RIVAL, 723, 779
    Return

ValorLakefront_PokemonFollower30:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PokemonFollower22
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_PokemonFollower23
    Return

ValorLakefront_RivalEnterNorth:
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchRivalEnterNorthSouthEast
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalEnterNorthSouthEast
    WaitMovement
    Return

ValorLakefront_RivalEnterSouth:
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchRivalEnterNorthSouthEast
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalEnterNorthSouthEast
    WaitMovement
    Return

ValorLakefront_RivalEnterWest:
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchRivalEnterWest
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalEnterWest
    WaitMovement
    Return

ValorLakefront_RivalEnterEast:
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchRivalEnterNorthSouthEast
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalEnterNorthSouthEast
    WaitMovement
    Return

ValorLakefront_RivalFacePlayerNorth:
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalWalkOnSpotWest
    WaitMovement
    Return

ValorLakefront_RivalFacePlayerSouth:
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalWalkOnSpotWest
    WaitMovement
    Return

ValorLakefront_RivalFacePlayerWest:
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalWalkOnSpotEast
    WaitMovement
    Return

ValorLakefront_RivalFacePlayerEast:
    ApplyMovement LOCALID_RIVAL, ValorLakefront_Movement_RivalWalkOnSpotWest
    WaitMovement
    Return

ValorLakefront_YeahThatsRight:
    Message ValorLakefront_Text_YeahThatsRight
    GoTo ValorLakefront_UseMedicineOnPsyduck
    End

ValorLakefront_YouHaventSeen:
    Message ValorLakefront_Text_YouHaventSeen
    GoTo ValorLakefront_UseMedicineOnPsyduck
    End

ValorLakefront_UseMedicineOnPsyduck:
    Message ValorLakefront_Text_UseMedicineOnPsyduck
    SetVar VAR_0x8004, ITEM_SECRETPOTION
    SetVar VAR_0x8005, 1
    Common_GiveItemQuantity
    Message ValorLakefront_Text_SeeYouLater
    CloseMessage
    GoTo ValorLakefront_CynthiaLeave
    End

ValorLakefront_CynthiaLeave:
    ApplyMovement LOCALID_CYNTHIA, ValorLakefront_Movement_CynthiaLeave
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerWatchCynthiaLeave
    WaitMovement
    GoTo ValorLakefront_RemoveCynthia

ValorLakefront_RemoveCynthia:
    RemoveObject LOCALID_CYNTHIA
    SetVar VAR_DUMMY_0x4083, 2
    SetFlag FLAG_DUMMY_0x00B7
    SetFlag FLAG_HIDE_PASTORIA_CITY_RIVAL
    ClearFlag FLAG_HIDE_PASTORIA_CITY_GYM_CRASHER_WAKE
    SetVar VAR_PASTORIA_CITY_STATE, 6
    ReleaseAll
    End

    .balign 4, 0
ValorLakefront_Movement_RivalEnterNorthSouthEast:
    WalkFastNorth 10
    WalkOnSpotFastWest
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_RivalEnterWest:
    WalkFastNorth 10
    WalkOnSpotFastEast
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_RivalWalkOnSpotNorth:
    WalkOnSpotFastNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_RivalWalkOnSpotWest:
    WalkOnSpotFastWest
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_RivalWalkOnSpotEast:
    WalkOnSpotFastEast
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_RivalLeave:
    WalkFastNorth 8
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CynthiaEnter:
    WalkNormalSouth 4
    WalkOnSpotNormalNorth
    Delay8 2
    WalkNormalSouth 4
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CynthiaWatchRivalLeave:
    Delay8
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CynthiaWalkOnSpotSouth:
    WalkOnSpotNormalSouth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CynthiaLeave:
    WalkNormalNorth 9
    EndMovement

ValorLakefront_Movement_Unused:
    Delay8
    WalkOnSpotNormalEast
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchRivalEnterNorthSouthEast:
    Delay8 4
    WalkOnSpotNormalEast
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchRivalEnterWest:
    Delay8 4
    WalkOnSpotNormalWest
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchRivalLeave:
    Delay8 2
    WalkOnSpotNormalNorth
    EndMovement

ValorLakefront_Movement_Unused2:
    Delay8 3
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerWatchCynthiaLeave:
    Delay8
    FaceNorth
    EndMovement

ValorLakefront_CameramanSouth:
    NPCMessage ValorLakefront_Text_LegendaryInThisLake
    End

ValorLakefront_CameramanNorth:
    NPCMessage ValorLakefront_Text_PokemonWontComeOut
    End

ValorLakefront_Beauty:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    FacePlayer
    CheckItem ITEM_SUITE_KEY, 1, VAR_RESULT
    GoToIfEq VAR_RESULT, TRUE, ValorLakefront_FoundSuiteKey
    Message ValorLakefront_Text_DroppedSuiteKey
    WaitButton
    CloseMessage
    ReleaseAll
    End

ValorLakefront_FoundSuiteKey:
    Message ValorLakefront_Text_ThatsMySuiteKey
    CloseMessage
    RemoveItem ITEM_SUITE_KEY, 1, VAR_RESULT
    ApplyMovement LOCALID_BEAUTY, ValorLakefront_Movement_WalkOnSpotNorth
    WaitMovement
    LoadDoorAnimation 22, 24, 11, 13, ANIMATION_TAG_DOOR_1
    PlayDoorOpenAnimation ANIMATION_TAG_DOOR_1
    WaitForAnimation ANIMATION_TAG_DOOR_1
    ApplyMovement LOCALID_BEAUTY, ValorLakefront_Movement_WalkNorth
    WaitMovement
    RemoveObject LOCALID_BEAUTY
    PlayDoorCloseAnimation ANIMATION_TAG_DOOR_1
    WaitForAnimation ANIMATION_TAG_DOOR_1
    UnloadAnimation ANIMATION_TAG_DOOR_1
    ClearFlag FLAG_HIDE_GRAND_LAKE_VALOR_LAKEFRONT_EAST_HOUSE_BEAUTY
    SetFlag FLAG_HIDE_VALOR_LAKEFRONT_BEAUTY
    ReleaseAll
    End

    .balign 4, 0
ValorLakefront_Movement_WalkOnSpotNorth:
    WalkOnSpotNormalNorth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_WalkNorth:
    WalkNormalNorth
    EndMovement

ValorLakefront_SignboardSevenStarsRestaurant:
    ShowLandmarkSign ValorLakefront_Text_SignSevenStarsRestaurant
    End

ValorLakefront_Collector:
    NPCMessage ValorLakefront_Text_SunyshoreHadBlackout2
    End

ValorLakefront_CoordEvent_BlockSunyshore:
    LockAll
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorExclamationMark
    WaitMovement
    GetPlayerMapPos VAR_0x8000, VAR_0x8001
    CallIfEq VAR_0x8001, 789, ValorLakefront_CollectorBlockPlayerX789
    CallIfEq VAR_0x8001, 790, ValorLakefront_CollectorBlockPlayerX790
    CallIfEq VAR_0x8001, 791, ValorLakefront_CollectorBlockPlayerX791
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorWalkWest
    ApplyMovement LOCALID_PLAYER, ValorLakefront_Movement_PlayerGetPushedWest
    ApplyMovement LOCALID_POKEMON_FOLLOWER, ValorLakefront_Movement_PlayerGetPushedWest
    WaitMovement
    Message ValorLakefront_Text_SunyshoreHadBlackout
    CloseMessage
    CallIfEq VAR_0x8001, 789, ValorLakefront_CollectorWalkBackX789
    CallIfEq VAR_0x8001, 790, ValorLakefront_CollectorWalkBackX790
    CallIfEq VAR_0x8001, 791, ValorLakefront_CollectorWalkBackX791
    ReleaseAll
    End

ValorLakefront_CollectorBlockPlayerX789:
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorBlockPlayerX789
    WaitMovement
    Return

ValorLakefront_CollectorBlockPlayerX790:
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorBlockPlayerX790
    WaitMovement
    Return

ValorLakefront_CollectorBlockPlayerX791:
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorBlockPlayerX791
    WaitMovement
    Return

ValorLakefront_CollectorWalkBackX789:
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorWalkBackX789
    WaitMovement
    Return

ValorLakefront_CollectorWalkBackX790:
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorWalkBackX790
    WaitMovement
    Return

ValorLakefront_CollectorWalkBackX791:
    ApplyMovement LOCALID_COLLECTOR, ValorLakefront_Movement_CollectorWalkBackX791
    WaitMovement
    Return

    .balign 4, 0
ValorLakefront_Movement_CollectorExclamationMark:
    EmoteExclamationMark
    Delay16
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CollectorBlockPlayerX789:
    WalkNormalEast
    WalkNormalSouth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CollectorBlockPlayerX790:
    WalkNormalEast
    WalkNormalSouth 2
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CollectorBlockPlayerX791:
    WalkNormalEast
    WalkNormalSouth 3
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CollectorWalkBackX789:
    WalkNormalNorth
    FaceSouth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CollectorWalkBackX790:
    WalkNormalNorth 2
    FaceSouth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CollectorWalkBackX791:
    WalkNormalNorth 3
    FaceSouth
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_CollectorWalkWest:
    WalkNormalWest
    EndMovement

    .balign 4, 0
ValorLakefront_Movement_PlayerGetPushedWest:
    LockDir
    WalkNormalWest
    UnlockDir
    EndMovement
