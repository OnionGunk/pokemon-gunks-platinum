#include "macros/scrcmd.inc"
#include "res/text/bank/pokemon_league_champion_room.h"
#include "res/field/events/events_pokemon_league_champion_room.h"

    ScriptEntry PokemonLeagueChampionRoom_OnTransition
    ScriptEntry PokemonLeagueChampionRoom_PokemonFollower16
    ScriptEntryEnd

PokemonLeagueChampionRoom_OnTransition:
    InitPersistedMapFeaturesForPlatformLift
    GetPartyMonFollowerGfx 0, VAR_OBJ_GFX_ID_1
    GetPartyMonFollowerGfx 1, VAR_OBJ_GFX_ID_2
    GetPartyMonFollowerGfx 2, VAR_OBJ_GFX_ID_3
    GetPartyMonFollowerGfx 3, VAR_OBJ_GFX_ID_4
    GetPartyMonFollowerGfx 4, VAR_OBJ_GFX_ID_5
    GetPartyMonFollowerGfx 5, VAR_OBJ_GFX_ID_6
    End

PokemonLeagueChampionRoom_PokemonFollower16:
    CheckPokemonFollowerActive VAR_RESULT
    GoToIfEq VAR_RESULT, 0, PokemonLeagueChampionRoom_OnFrame_EnterRoom
    RemoveObject LOCALID_PARTY_MON_1
    SetObjectEventPos LOCALID_PARTY_MON_1, 8, 18
    AddObject LOCALID_PARTY_MON_1
    ApplyMovement LOCALID_PARTY_MON_1, PokemonLeagueChampionRoom_Movement_PokemonFollower1
PokemonLeagueChampionRoom_OnFrame_EnterRoom:
    ApplyMovement LOCALID_PLAYER, PokemonLeagueChampionRoom_Movement_PlayerApproachCynthia
    FadeOutBGM 0, 35
    WaitMovement
    PlayTrainerEncounterBGM TRAINER_CHAMPION_CYNTHIA
    CallIfUnset FLAG_GAME_COMPLETED, PokemonLeagueChampionRoom_CynthiaIntro
    CallIfSet FLAG_GAME_COMPLETED, PokemonLeagueChampionRoom_CynthiaGameCompletedIntro
    CloseMessage
    SetFlag FLAG_ALT_MUSIC_CHAMPION_ROOM
    SetVarFromValue VAR_0x8004, 6
    SetVarFromValue VAR_0x8005, 0
    CheckPokemonFollowerActive VAR_RESULT
    GoToIfEq VAR_RESULT, 0, PokemonLeagueChampionRoom_PokemonFollower2
    GetPokemonFollowerPartySlot VAR_0x8004
    AddVar VAR_0x8005, 1

PokemonLeagueChampionRoom_PokemonFollower2:
    FadeOutBGM 0, 32
    WaitTime 2, VAR_RESULT
    RemoveObject LOCALID_PARTY_MON_2
    RemoveObject LOCALID_PARTY_MON_3
    RemoveObject LOCALID_PARTY_MON_4
    RemoveObject LOCALID_PARTY_MON_5
    RemoveObject LOCALID_PARTY_MON_6
    SetAllowTwoPokemonCries 6
    GoToIfEq VAR_OBJ_GFX_ID_1, 0, PokemonLeagueChampionRoom_PokemonFollower3
    GoToIfEq VAR_0x8004, 0, PokemonLeagueChampionRoom_PokemonFollower3
    RemoveObject LOCALID_PARTY_MON_1
    Call PokemonLeagueChampionRoom_PokemonFollower4
    SetObjectEventPos LOCALID_PARTY_MON_1, VAR_0x8006, VAR_0x8007
    AddObject LOCALID_PARTY_MON_1
    HideObject LOCALID_PARTY_MON_1
    AddVar VAR_0x8005, 1
    SetVarFromValue VAR_0x8000, 0
    GetPartyMonSpecies VAR_0x8000, VAR_RESULT
    PlayPokeBallAnimation LOCALID_PARTY_MON_1, VAR_RESULT, 0
    WaitTime 16, VAR_RESULT

PokemonLeagueChampionRoom_PokemonFollower3:
    GoToIfEq VAR_OBJ_GFX_ID_2, 0, PokemonLeagueChampionRoom_PokemonFollower5
    GoToIfEq VAR_0x8004, 1, PokemonLeagueChampionRoom_PokemonFollower5
    Call PokemonLeagueChampionRoom_PokemonFollower4
    SetObjectEventPos LOCALID_PARTY_MON_2, VAR_0x8006, VAR_0x8007
    AddObject LOCALID_PARTY_MON_2
    HideObject LOCALID_PARTY_MON_2
    AddVar VAR_0x8005, 1
    SetVarFromValue VAR_0x8000, 1
    GetPartyMonSpecies VAR_0x8000, VAR_RESULT
    PlayPokeBallAnimation LOCALID_PARTY_MON_2, VAR_RESULT, 0
    WaitTime 16, VAR_RESULT

PokemonLeagueChampionRoom_PokemonFollower5:
    GoToIfEq VAR_OBJ_GFX_ID_3, 0, PokemonLeagueChampionRoom_PokemonFollower6
    GoToIfEq VAR_0x8004, 2, PokemonLeagueChampionRoom_PokemonFollower6
    Call PokemonLeagueChampionRoom_PokemonFollower4
    SetObjectEventPos LOCALID_PARTY_MON_3, VAR_0x8006, VAR_0x8007
    AddObject LOCALID_PARTY_MON_3
    HideObject LOCALID_PARTY_MON_3
    AddVar VAR_0x8005, 1
    SetVarFromValue VAR_0x8000, 2
    GetPartyMonSpecies VAR_0x8000, VAR_RESULT
    PlayPokeBallAnimation LOCALID_PARTY_MON_3, VAR_RESULT, 0
    WaitTime 16, VAR_RESULT

PokemonLeagueChampionRoom_PokemonFollower6:
    GoToIfEq VAR_OBJ_GFX_ID_4, 0, PokemonLeagueChampionRoom_PokemonFollower7
    GoToIfEq VAR_0x8004, 3, PokemonLeagueChampionRoom_PokemonFollower7
    Call PokemonLeagueChampionRoom_PokemonFollower4
    SetObjectEventPos LOCALID_PARTY_MON_4, VAR_0x8006, VAR_0x8007
    AddObject LOCALID_PARTY_MON_4
    HideObject LOCALID_PARTY_MON_4
    AddVar VAR_0x8005, 1
    SetVarFromValue VAR_0x8000, 3
    GetPartyMonSpecies VAR_0x8000, VAR_RESULT
    PlayPokeBallAnimation LOCALID_PARTY_MON_4, VAR_RESULT, 0
    WaitTime 16, VAR_RESULT

PokemonLeagueChampionRoom_PokemonFollower7:
    GoToIfEq VAR_OBJ_GFX_ID_5, 0, PokemonLeagueChampionRoom_PokemonFollower8
    GoToIfEq VAR_0x8004, 4, PokemonLeagueChampionRoom_PokemonFollower8
    Call PokemonLeagueChampionRoom_PokemonFollower4
    SetObjectEventPos LOCALID_PARTY_MON_5, VAR_0x8006, VAR_0x8007
    AddObject LOCALID_PARTY_MON_5
    HideObject LOCALID_PARTY_MON_5
    AddVar VAR_0x8005, 1
    SetVarFromValue VAR_0x8000, 4
    GetPartyMonSpecies VAR_0x8000, VAR_RESULT
    PlayPokeBallAnimation LOCALID_PARTY_MON_5, VAR_RESULT, 0
    WaitTime 16, VAR_RESULT

PokemonLeagueChampionRoom_PokemonFollower8:
    GoToIfEq VAR_OBJ_GFX_ID_6, 0, PokemonLeagueChampionRoom_PokemonFollower9
    GoToIfEq VAR_0x8004, 5, PokemonLeagueChampionRoom_PokemonFollower9
    Call PokemonLeagueChampionRoom_PokemonFollower4
    SetObjectEventPos LOCALID_PARTY_MON_6, VAR_0x8006, VAR_0x8007
    AddObject LOCALID_PARTY_MON_6
    HideObject LOCALID_PARTY_MON_6
    AddVar VAR_0x8005, 1
    SetVarFromValue VAR_0x8000, 5
    GetPartyMonSpecies VAR_0x8000, VAR_RESULT
    PlayPokeBallAnimation LOCALID_PARTY_MON_6, VAR_RESULT, 0
    WaitTime 16, VAR_RESULT

PokemonLeagueChampionRoom_PokemonFollower9:
    WaitCry
    SetAllowTwoPokemonCries 1
    WaitTime 2, VAR_RESULT
    CallIfUnset FLAG_ARRESTED_CHARON_STARK_MOUNTAIN, PokemonLeagueChampionRoom_StartCynthiaBattle
    CallIfSet FLAG_ARRESTED_CHARON_STARK_MOUNTAIN, PokemonLeagueChampionRoom_StartCynthiaRematchBattle
    CheckWonBattle VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, PokemonLeagueChampionRoom_BlackOut
    Message PokemonLeagueChampionRoom_Text_CynthiaDefeat
    SetFlag FLAG_DEFEATED_CYNTHIA
    CallIfUnset FLAG_ARRESTED_CHARON_STARK_MOUNTAIN, PokemonLeagueChampionRoom_CreateJournalEventDefeatedCynthia
    CallIfSet FLAG_ARRESTED_CHARON_STARK_MOUNTAIN, PokemonLeagueChampionRoom_CreateJournalEventDefeatedRematchCynthia
    Message PokemonLeagueChampionRoom_Text_ComeWithMe
    CloseMessage
    ApplyMovement LOCALID_CYNTHIA, PokemonLeagueChampionRoom_Movement_CynthiaWalkAwayFromElevator
    WaitMovement
    ApplyMovement LOCALID_PLAYER, PokemonLeagueChampionRoom_Movement_PlayerEnterElevator
    WaitMovement
    ApplyMovement LOCALID_CYNTHIA, PokemonLeagueChampionRoom_Movement_CynthiaFacePlayer
    WaitMovement
    WaitTime 30, VAR_RESULT
    TriggerPlatformLift
    WaitTime 30, VAR_RESULT
    SetFlag FLAG_HIDE_POKEMON_LEAGUE_HALLWAY_TO_HALL_OF_FAME_CYNTHIA
    SetFlag FLAG_HIDE_POKEMON_LEAGUE_HALLWAY_TO_HALL_OF_FAME_PROF_ROWAN
    ApplyMovement LOCALID_PLAYER, PokemonLeagueChampionRoom_Movement_PlayerEnterHallwayToHallOfFame
    WaitMovement
    PlaySE SEQ_SE_DP_KAIDAN2_sseq
    FadeScreenOut
    WaitFadeScreen
    Warp MAP_HEADER_POKEMON_LEAGUE_HALLWAY_TO_HALL_OF_FAME, 5, 23, DIR_NORTH
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

PokemonLeagueChampionRoom_CynthiaIntro:
    Message PokemonLeagueChampionRoom_Text_CynthiaIntro
    Return

PokemonLeagueChampionRoom_CynthiaGameCompletedIntro:
    Message PokemonLeagueChampionRoom_Text_CynthiaGameCompletedIntro
    Return

PokemonLeagueChampionRoom_StartCynthiaBattle:
    StartTrainerBattle TRAINER_CHAMPION_CYNTHIA
    Return

PokemonLeagueChampionRoom_StartCynthiaRematchBattle:
    StartTrainerBattle TRAINER_CHAMPION_CYNTHIA_REMATCH
    Return

PokemonLeagueChampionRoom_CreateJournalEventDefeatedCynthia:
    CreateJournalEvent LOCATION_EVENT_BEAT_CHAMPION, TRAINER_CHAMPION_CYNTHIA
    Return

PokemonLeagueChampionRoom_CreateJournalEventDefeatedRematchCynthia:
    CreateJournalEvent LOCATION_EVENT_BEAT_CHAMPION, TRAINER_CHAMPION_CYNTHIA_REMATCH
    Return

PokemonLeagueChampionRoom_BlackOut:
    ClearFlag FLAG_ALT_MUSIC_CHAMPION_ROOM
    BlackOutFromBattle
    ReleaseAll
    End

PokemonLeagueChampionRoom_PokemonFollower4:
    SetVarFromVar VAR_0x8008, VAR_0x8005
    GoToIfEq VAR_0x8008, 0, PokemonLeagueChampionRoom_PokemonFollower10
    GoToIfEq VAR_0x8008, 1, PokemonLeagueChampionRoom_PokemonFollower11
    GoToIfEq VAR_0x8008, 2, PokemonLeagueChampionRoom_PokemonFollower12
    GoToIfEq VAR_0x8008, 3, PokemonLeagueChampionRoom_PokemonFollower13
    GoToIfEq VAR_0x8008, 4, PokemonLeagueChampionRoom_PokemonFollower14
    GoToIfEq VAR_0x8008, 5, PokemonLeagueChampionRoom_PokemonFollower15
    Return

PokemonLeagueChampionRoom_PokemonFollower10:
    SetVarFromValue VAR_0x8006, 8
    SetVarFromValue VAR_0x8007, 15
    Return

PokemonLeagueChampionRoom_PokemonFollower11:
    SetVarFromValue VAR_0x8006, 10
    SetVarFromValue VAR_0x8007, 16
    Return

PokemonLeagueChampionRoom_PokemonFollower12:
    SetVarFromValue VAR_0x8006, 6
    SetVarFromValue VAR_0x8007, 16
    Return

PokemonLeagueChampionRoom_PokemonFollower13:
    SetVarFromValue VAR_0x8006, 11
    SetVarFromValue VAR_0x8007, 15
    Return

PokemonLeagueChampionRoom_PokemonFollower14:
    SetVarFromValue VAR_0x8006, 5
    SetVarFromValue VAR_0x8007, 15
    Return

PokemonLeagueChampionRoom_PokemonFollower15:
    SetVarFromValue VAR_0x8006, 8
    SetVarFromValue VAR_0x8007, 17
    Return

    .balign 4, 0
PokemonLeagueChampionRoom_Movement_PlayerApproachCynthia:
    WalkNormalNorth 4
    EndMovement

    .balign 4, 0
PokemonLeagueChampionRoom_Movement_PokemonFollower1:
    Delay8
    WalkNormalNorth 3

    EndMovement

    .balign 4, 0
PokemonLeagueChampionRoom_Movement_PlayerEnterElevator:
    WalkNormalNorth 6
    EndMovement

    .balign 4, 0
PokemonLeagueChampionRoom_Movement_PlayerEnterHallwayToHallOfFame:
    WalkNormalNorth 6
    EndMovement

    .balign 4, 0
PokemonLeagueChampionRoom_Movement_CynthiaWalkAwayFromElevator:
    WalkNormalWest
    WalkOnSpotNormalSouth
    EndMovement

    .balign 4, 0
PokemonLeagueChampionRoom_Movement_CynthiaFacePlayer:
    WalkOnSpotNormalNorth
    EndMovement
