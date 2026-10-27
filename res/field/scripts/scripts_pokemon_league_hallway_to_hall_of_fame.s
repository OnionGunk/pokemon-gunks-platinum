#include "macros/scrcmd.inc"
#include "res/text/bank/pokemon_league_hallway_to_hall_of_fame.h"
#include "res/field/events/events_pokemon_league_hallway_to_hall_of_fame.h"

    ScriptEntry PokemonLeagueHallwayToHallOfFame_OnFrame_EnterHallOfFame
    ScriptEntry PokemonLeagueHallwayToHallOfFame_OnFrame_EnterHallOfFame
    ScriptEntryEnd

PokemonLeagueHallwayToHallOfFame_OnFrame_EnterHallOfFame:
    LockAll
    SetVarFromValue VAR_0x8006, 0
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower1
    WaitMovement
    ClearFlag FLAG_HIDE_POKEMON_LEAGUE_HALLWAY_TO_HALL_OF_FAME_CYNTHIA
    GetPlayerStarterSpecies VAR_RESULT
    SetVarFromValue VAR_0x8004, 0
    GetPartyMonSpecies VAR_0x8004, VAR_0x8005
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    AddVar VAR_0x8004, 1
    GetPartyMonSpecies VAR_0x8004, VAR_0x8005
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    AddVar VAR_0x8004, 1
    GetPartyMonSpecies VAR_0x8004, VAR_0x8005
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    AddVar VAR_0x8004, 1
    GetPartyMonSpecies VAR_0x8004, VAR_0x8005
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    AddVar VAR_0x8004, 1
    GetPartyMonSpecies VAR_0x8004, VAR_0x8005
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    AddVar VAR_0x8004, 1
    GetPartyMonSpecies VAR_0x8004, VAR_0x8005
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    SubVar VAR_0x8005, 1
    GoToIfEq VAR_0x8005, VAR_RESULT, PokemonLeagueHallwayToHallOfFame_PokemonFollower2
    AddVar VAR_0x8004, 1

PokemonLeagueHallwayToHallOfFame_PokemonFollower2:
    GoToIfEq VAR_OBJ_GFX_ID_1, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower3
    GoToIfEq VAR_0x8004, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower3
    CallIfEq VAR_0x8006, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower4
    AddObject LOCALID_PARTY_MON_1
    ApplyMovement LOCALID_PARTY_MON_1, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower5
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower6
    WaitTime 32, VAR_RESULT

PokemonLeagueHallwayToHallOfFame_PokemonFollower3:
    GoToIfEq VAR_OBJ_GFX_ID_2, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower7
    GoToIfEq VAR_0x8004, 1, PokemonLeagueHallwayToHallOfFame_PokemonFollower7
    CallIfEq VAR_0x8006, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower4
    AddObject LOCALID_PARTY_MON_2
    ApplyMovement LOCALID_PARTY_MON_2, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower8
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower9
    WaitTime 32, VAR_RESULT

PokemonLeagueHallwayToHallOfFame_PokemonFollower7:
    GoToIfEq VAR_OBJ_GFX_ID_3, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower10
    GoToIfEq VAR_0x8004, 2, PokemonLeagueHallwayToHallOfFame_PokemonFollower10
    CallIfEq VAR_0x8006, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower4
    AddObject LOCALID_PARTY_MON_3
    ApplyMovement LOCALID_PARTY_MON_3, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower11
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower12
    WaitTime 32, VAR_RESULT

PokemonLeagueHallwayToHallOfFame_PokemonFollower10:
    GoToIfEq VAR_OBJ_GFX_ID_4, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower13
    GoToIfEq VAR_0x8004, 3, PokemonLeagueHallwayToHallOfFame_PokemonFollower13
    CallIfEq VAR_0x8006, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower4
    AddObject LOCALID_PARTY_MON_4
    ApplyMovement LOCALID_PARTY_MON_4, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower14
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower15
    WaitTime 40, VAR_RESULT

PokemonLeagueHallwayToHallOfFame_PokemonFollower13:
    GoToIfEq VAR_OBJ_GFX_ID_5, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower16
    GoToIfEq VAR_0x8004, 4, PokemonLeagueHallwayToHallOfFame_PokemonFollower16
    CallIfEq VAR_0x8006, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower4
    AddObject LOCALID_PARTY_MON_5
    ApplyMovement LOCALID_PARTY_MON_5, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower17
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower18
    WaitTime 120, VAR_RESULT

PokemonLeagueHallwayToHallOfFame_PokemonFollower16:
    GoToIfEq VAR_OBJ_GFX_ID_6, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower19
    GoToIfEq VAR_0x8004, 5, PokemonLeagueHallwayToHallOfFame_PokemonFollower19
    CallIfEq VAR_0x8006, 0, PokemonLeagueHallwayToHallOfFame_PokemonFollower4
    AddObject LOCALID_PARTY_MON_6
    ApplyMovement LOCALID_PARTY_MON_6, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower20
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower21
    WaitTime 32, VAR_RESULT

PokemonLeagueHallwayToHallOfFame_PokemonFollower19:
    WaitTime 40, VAR_RESULT
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower22
    WaitMovement
    GoToIfEq VAR_0x8004, 6, PokemonLeagueHallwayToHallOfFame_PokemonFollower23
    SetVarFromVar VAR_0x8000, VAR_0x8004
    AddVar VAR_0x8000, 2
    SetObjectEventPos VAR_0x8000, 5, 23
    AddObject VAR_0x8000
    ApplyMovement VAR_0x8000, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower24
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower25
    WaitMovement
    WaitTime 40, VAR_RESULT
    ApplyMovement VAR_0x8000, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower26

PokemonLeagueHallwayToHallOfFame_PokemonFollower23:
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower26
    WaitMovement
    SetObjectEventPos LOCALID_CYNTHIA, 6, 14
    AddObject LOCALID_CYNTHIA
    LockObject LOCALID_CYNTHIA
    ApplyMovement LOCALID_CYNTHIA, PokemonLeagueHallwayToHallOfFame_Movement_CynthiaApproachPlayer
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PlayerFaceCynthia
    GoToIfEq VAR_0x8004, 6, PokemonLeagueHallwayToHallOfFame_PokemonFollower27
    ApplyMovement VAR_0x8000, PokemonLeagueHallwayToHallOfFame_Movement_PlayerFaceCynthia
PokemonLeagueHallwayToHallOfFame_PokemonFollower27:
    WaitMovement
    Message PokemonLeagueHallwayToHallOfFame_Text_RoomAheadIsHallOfFame
    CloseMessage
    SetObjectEventPos LOCALID_PROF_ROWAN, 4, 14
    ClearFlag FLAG_HIDE_POKEMON_LEAGUE_HALLWAY_TO_HALL_OF_FAME_PROF_ROWAN
    AddObject LOCALID_PROF_ROWAN
    LockObject LOCALID_PROF_ROWAN
    ApplyMovement LOCALID_PROF_ROWAN, PokemonLeagueHallwayToHallOfFame_Movement_RowanApproachPlayer
    WaitMovement
    GoToIfEq VAR_0x8004, 6, PokemonLeagueHallwayToHallOfFame_PokemonFollower28
    ApplyMovement VAR_0x8000, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower29
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower30
    ApplyMovement LOCALID_PROF_ROWAN, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower31
    ApplyMovement LOCALID_CYNTHIA, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower32
    WaitMovement
PokemonLeagueHallwayToHallOfFame_PokemonFollower28:
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PlayerFaceRowan
    WaitMovement
    BufferPlayerName 0
    Message PokemonLeagueHallwayToHallOfFame_Text_YourLastBattleWasSplendid
    CloseMessage
    ApplyMovement LOCALID_CYNTHIA, PokemonLeagueHallwayToHallOfFame_Movement_ExclamationMark
    WaitMovement
    Message PokemonLeagueHallwayToHallOfFame_Text_HelloProfessorRowan
    BufferPlayerName 0
    Message PokemonLeagueHallwayToHallOfFame_Text_GrownIntoRealChampion
    ApplyMovement LOCALID_CYNTHIA, PokemonLeagueHallwayToHallOfFame_Movement_WalkInPlaceWest
    WaitMovement
    Message PokemonLeagueHallwayToHallOfFame_Text_YouStillEnjoyEnthusiasm
    GetPlayerGender VAR_RESULT
    GoToIfEq VAR_RESULT, GENDER_MALE, PokemonLeagueHallwayToHallOfFame_StepThisWayPleaseMale
    GoToIfEq VAR_RESULT, GENDER_FEMALE, PokemonLeagueHallwayToHallOfFame_StepThisWayPleaseFemale
    End

PokemonLeagueHallwayToHallOfFame_StepThisWayPleaseMale:
    BufferPlayerName 0
    Message PokemonLeagueHallwayToHallOfFame_Text_StepThisWayPleaseMale
    GoTo PokemonLeagueHallwayToHallOfFame_EnterHallOfFameRoom

PokemonLeagueHallwayToHallOfFame_StepThisWayPleaseFemale:
    BufferPlayerName 0
    Message PokemonLeagueHallwayToHallOfFame_Text_StepThisWayPleaseFemale
    GoTo PokemonLeagueHallwayToHallOfFame_EnterHallOfFameRoom

PokemonLeagueHallwayToHallOfFame_EnterHallOfFameRoom:
    Message PokemonLeagueHallwayToHallOfFame_Text_ProfessorPleaseJoinUs
    CloseMessage
    GoToIfEq VAR_0x8004, 6, PokemonLeagueHallwayToHallOfFame_PokemonFollower33
    ApplyMovement VAR_0x8000, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower34
    WaitTime 12, VAR_RESULT
PokemonLeagueHallwayToHallOfFame_PokemonFollower33:
    ApplyMovement LOCALID_CYNTHIA, PokemonLeagueHallwayToHallOfFame_Movement_CynthiaEnterHallOfFameRoom
    ApplyMovement LOCALID_PROF_ROWAN, PokemonLeagueHallwayToHallOfFame_Movement_RowanEnterHallOfFameRoom
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PlayerEnterHallOfFameRoom
    WaitMovement
    PlaySE SEQ_SE_DP_KAIDAN2_sseq
    FadeScreenOut
    WaitFadeScreen
    Warp MAP_HEADER_POKEMON_LEAGUE_HALL_OF_FAME, 8, 15, DIR_NORTH
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

PokemonLeagueHallwayToHallOfFame_PokemonFollower4:
    SetVarFromValue VAR_0x8006, 1
    ApplyMovement LOCALID_PLAYER, PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower35
    Return

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_RowanApproachPlayer:
    WalkNormalNorth 8
    WalkOnSpotNormalEast
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_RowanEnterHallOfFameRoom:
    Delay8 3
    WalkNormalNorth 2
    WalkNormalEast
    WalkNormalNorth 2
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_CynthiaApproachPlayer:
    WalkNormalNorth 8
    WalkOnSpotNormalWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_ExclamationMark:
    WalkOnSpotNormalWest
    EmoteExclamationMark
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_WalkInPlaceWest:
    WalkOnSpotNormalWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_CynthiaEnterHallOfFameRoom:
    WalkNormalNorth 2
    WalkNormalWest
    WalkNormalNorth 2
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower1:
    WalkNormalNorth 7
    Delay8
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower35:
    WalkOnSpotNormalSouth
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower25:
    Delay8
    WalkOnSpotNormalSouth
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower26:
    WalkNormalNorth 10

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PlayerEnterHallway:
    Delay8
    WalkOnSpotNormalSouth
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PlayerFaceCynthia:
    Delay8 6
    WalkOnSpotNormalEast
    EndMovement

PokemonLeagueElevatorToHallOfFame_Movement_Unused:
    WalkOnSpotNormalEast
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PlayerFaceRowan:
    WalkOnSpotNormalWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PlayerEnterHallOfFameRoom:
    Delay8
    WalkOnSpotNormalNorth
    Delay8 6
    WalkNormalNorth 4
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower5:
    WalkFastNorth 8
    JumpOnSpotFastWest 2
    WalkFastNorth 11
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower8:
    WalkFastNorth 12
    FaceSouth
    Delay8 2
    WalkFastNorth 9
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower11:
    WalkFastNorth 8
    FaceWest
    Delay8 3
    WalkFastNorth 11
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower14:
    WalkFastNorth 8
    FaceWest
    Delay2
    FaceSouth
    Delay2
    FaceEast
    Delay2
    FaceNorth
    Delay2
    FaceWest
    Delay2
    FaceSouth
    Delay2
    FaceEast
    Delay2
    JumpOnSpotFastEast 2
    FollowerEmoteMusic
    WalkFastNorth 11
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower17:
    Delay8 4
    WalkNormalNorth 9
    FaceEast
    Delay8 4
    WalkNormalNorth 11
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower20:
    WalkFastNorth 9
    WalkFastNorth 11
    SetInvisible
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower6:
    Delay4 7
    FaceEast
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower9:
    Delay4 6
    FaceWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower12:
    Delay4 6
    FaceEast
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower15:
    Delay4 6
    FaceWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower18:
    Delay8 12
    FaceWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower21:
    Delay4 7
    FaceEast
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower24:
    WalkNormalNorth 4
    Delay8 8
    WalkNormalNorth 2
    Delay8
    FollowerEmoteHeart
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower22:
    FaceNorth
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower29:
    WalkFastWest
    JumpOnSpotFastNorth 2
    WalkFastWest
    WalkFastNorth 2
    WalkFastEast 4
    WalkFastSouth 2
    WalkFastWest 4
    WalkFastNorth 2
    WalkFastEast 4
    WalkFastSouth 2
    WalkFastWest 3
    FaceNorth
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower31:
    WalkOnSpotNormalSouth
    Delay4 3
    WalkOnSpotNormalWest
    WalkOnSpotNormalNorth
    Delay4
    WalkOnSpotNormalEast
    Delay4
    WalkOnSpotNormalSouth
    Delay4 2
    WalkOnSpotNormalWest
    WalkOnSpotNormalNorth
    Delay4 2
    WalkOnSpotNormalEast
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower30:
    Delay4 5
    FaceWest
    Delay4 2
    FaceNorth
    Delay4 2
    FaceEast
    Delay4 3
    FaceSouth
    Delay4 2
    FaceWest
    Delay4 2
    FaceNorth
    Delay4 2
    FaceEast
    Delay4 2
    FaceSouth
    Delay4 2
    FaceWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower32:
    Delay4 5
    FaceWest
    Delay4 3
    FaceNorth
    Delay4
    FaceEast
    Delay4 2
    FaceSouth
    Delay4 2
    FaceWest
    Delay4 3
    FaceNorth
    Delay4
    FaceEast
    Delay4 2
    FaceSouth
    Delay4 2
    FaceWest
    EndMovement

    .balign 4, 0
PokemonLeagueHallwayToHallOfFame_Movement_PokemonFollower34:
    WalkFastWest
    WalkFastNorth 3
    WalkFastEast 2
    WalkFastNorth 2
    SetInvisible
    EndMovement
