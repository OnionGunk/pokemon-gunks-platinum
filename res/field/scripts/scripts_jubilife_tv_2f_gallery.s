#include "macros/scrcmd.inc"
#include "res/text/bank/jubilife_tv_2f_gallery.h"
#include "res/field/events/events_jubilife_tv_2f_gallery.h"
#define LOCALID_POKEMON_FOLLOWER 253

    ScriptEntry JubilifeTV2FGallery_Hiker
    ScriptEntry JubilifeTV2FGallery_Frame0
    ScriptEntry JubilifeTV2FGallery_Frame1
    ScriptEntry JubilifeTV2FGallery_Frame2
    ScriptEntry JubilifeTV2FGallery_Frame3
    ScriptEntry JubilifeTV2FGallery_Frame4
    ScriptEntry JubilifeTV2FGallery_Frame5
    ScriptEntry JubilifeTV2FGallery_Frame6
    ScriptEntry JubilifeTV2FGallery_Frame7
    ScriptEntry JubilifeTV2FGallery_Frame8
    ScriptEntry JubilifeTV2FGallery_Frame9
    ScriptEntry JubilifeTV2FGallery_Frame10
    ScriptEntry JubilifeTV2FGallery_PokemonBreederM
    ScriptEntry JubilifeTV2FGallery_Twin
    ScriptEntryEnd

JubilifeTV2FGallery_Hiker:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    FacePlayer
    SetVar VAR_0x8007, 0
    CallIfUnset FLAG_ENTERED_DRESSING_ROOM, _006F
    GoToIfEq VAR_MAP_LOCAL_0x01, 0, JubilifeTV2FGallery_AskDressUpPokemon
    GoToIfEq VAR_MAP_LOCAL_0x01, 1, JubilifeTV2FGallery_AskDressUpPokemonChangePhoto
    End

_006F:
    SetVar VAR_0x8007, 1
    Return

JubilifeTV2FGallery_AskDressUpPokemon:
    Message JubilifeTV2FGallery_Text_DressUpYourPokemon
    GoTo JubilifeTV2FGallery_YesNoDressUpPokemon

JubilifeTV2FGallery_AskDressUpPokemonChangePhoto:
    Message JubilifeTV2FGallery_Text_WouldYouLikeToDressUp
    GoTo JubilifeTV2FGallery_YesNoDressUpPokemon

JubilifeTV2FGallery_ShouldIExplainDressUp:
    Message JubilifeTV2FGallery_Text_ShouldIExplainDressUp
    ShowYesNoMenu VAR_RESULT
    GoToIfEq VAR_RESULT, MENU_YES, JubilifeTV2FGallery_ExplainDressUp
    GoToIfEq VAR_RESULT, MENU_NO, JubilifeTV2FGallery_SetPokemonForDressUp
    End

JubilifeTV2FGallery_ExplainDressUp:
    Message JubilifeTV2FGallery_Text_ExplainDressUp
    GoTo JubilifeTV2FGallery_YesNoDressUpPokemonAfterExplanation

JubilifeTV2FGallery_YesNoDressUpPokemon:
    ShowYesNoMenu VAR_RESULT
    GoToIfEq VAR_RESULT, MENU_YES, JubilifeTV2FGallery_ShouldIExplainDressUp
    GoToIfEq VAR_RESULT, MENU_NO, JubilifeTV2FGallery_WheresYourSenseOfAdventure
    End

JubilifeTV2FGallery_YesNoDressUpPokemonAfterExplanation:
    ShowYesNoMenu VAR_RESULT
    GoToIfEq VAR_RESULT, MENU_YES, JubilifeTV2FGallery_SetPokemonForDressUp
    GoToIfEq VAR_RESULT, MENU_NO, JubilifeTV2FGallery_WheresYourSenseOfAdventure
    End

JubilifeTV2FGallery_SetPokemonForDressUp:
    GetPartyCount VAR_RESULT
    GoToIfEq VAR_RESULT, 1, JubilifeTV2FGallery_SetOnlyPokemonForDressUp
    GoToIfNe VAR_RESULT, 1, JubilifeTV2FGallery_ChoosePokemonForDressUp
    End

JubilifeTV2FGallery_SetOnlyPokemonForDressUp:
    SetVar VAR_MAP_LOCAL_0x02, 0
    GoTo JubilifeTV2FGallery_DressItUpAdorably

JubilifeTV2FGallery_ChoosePokemonForDressUp:
    Message JubilifeTV2FGallery_Text_DressUpWhichPokemon
    CloseMessage
    FadeScreenOut
    WaitFadeScreen
    SelectMoveTutorPokemon
    GetSelectedPartySlot VAR_MAP_LOCAL_0x02
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    GoToIfEq VAR_MAP_LOCAL_0x02, 0xFF, JubilifeTV2FGallery_WheresYourSenseOfAdventure
    GetPartyMonSpecies VAR_MAP_LOCAL_0x02, VAR_RESULT
    GoToIfEq VAR_RESULT, SPECIES_NONE, JubilifeTV2FGallery_ThatsAnEgg
    GoTo JubilifeTV2FGallery_DressItUpAdorably

JubilifeTV2FGallery_DressItUpAdorably:
    Message JubilifeTV2FGallery_Text_DressItUpAdorably
    CloseMessage
    ScrCmd_Unused_06A LOCALID_POKEMON_FOLLOWER, VAR_0x8000, VAR_0x8001
    GetPlayerDir VAR_0x8006
    GoToIfEq VAR_0x8006, DIR_NORTH, JubilifeTV2FGallery_EnterDressingRoomNorth
    GoToIfEq VAR_0x8006, DIR_WEST, JubilifeTV2FGallery_EnterDressingRoomWest
    GoToIfEq VAR_0x8006, DIR_EAST, JubilifeTV2FGallery_EnterDressingRoomEast
    End

JubilifeTV2FGallery_EnterDressingRoomNorth:
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerMoveOutOfTheWayNorth
    WaitMovement
    ApplyMovement LOCALID_PLAYER, JubilifeTV2FGallery_Movement_PlayerEnterDressingRoomNorth
    CallIfEq VAR_0x8000, 2, JubilifeTV2FGallery_PokemonFollower1
    CallIfEq VAR_0x8000, 3, JubilifeTV2FGallery_PokemonFollower2
    CallIfEq VAR_0x8000, 4, JubilifeTV2FGallery_PokemonFollower3
    WaitMovement
    GoTo JubilifeTV2FGallery_EnterDressingRoom

JubilifeTV2FGallery_PokemonFollower1:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower4
    Return

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower4:
    WalkSlowEast
    WalkSlowNorth 2
    SetInvisible
    EndMovement

JubilifeTV2FGallery_PokemonFollower2:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower5
    Return

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower5:
    WalkSlowNorth 3
    SetInvisible
    EndMovement

JubilifeTV2FGallery_PokemonFollower3:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower6
    Return

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower6:
    WalkSlowWest
    WalkSlowNorth 2
    SetInvisible
    EndMovement

JubilifeTV2FGallery_EnterDressingRoomWest:
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerMoveOutOfTheWayWest
    WaitMovement
    ApplyMovement LOCALID_PLAYER, JubilifeTV2FGallery_Movement_PlayerEnterDressingRoomWest
    CallIfEq VAR_0x8000, 4, JubilifeTV2FGallery_PokemonFollower7
    CallIfEq VAR_0x8000, 5, JubilifeTV2FGallery_PokemonFollower8
    WaitMovement
    GoTo JubilifeTV2FGallery_EnterDressingRoom

JubilifeTV2FGallery_PokemonFollower11:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower9
    Return

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower9:
    WalkSlowEast 2
    WalkSlowNorth
    SetInvisible
    EndMovement

JubilifeTV2FGallery_PokemonFollower12:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower10
    Return

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower10:
    WalkSlowNorth
    WalkSlowEast
    WalkSlowNorth
    SetInvisible
    EndMovement

JubilifeTV2FGallery_EnterDressingRoomEast:
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerMoveOutOfTheWayEast
    WaitMovement
    ApplyMovement LOCALID_PLAYER, JubilifeTV2FGallery_Movement_PlayerEnterDressingRoomEast
    CallIfEq VAR_0x8000, 1, JubilifeTV2FGallery_PokemonFollower11
    CallIfEq VAR_0x8000, 2, JubilifeTV2FGallery_PokemonFollower12
    WaitMovement
    GoTo JubilifeTV2FGallery_EnterDressingRoom

JubilifeTV2FGallery_PokemonFollower7:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower13
    Return

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower13:
    WalkSlowNorth
    WalkSlowWest
    WalkSlowNorth
    SetInvisible
    EndMovement

JubilifeTV2FGallery_PokemonFollower8:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower14
    Return

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower14:
    WalkSlowWest 2
    WalkSlowNorth
    SetInvisible
    EndMovement

JubilifeTV2FGallery_EnterDressingRoom:
    SetFlag FLAG_ENTERED_DRESSING_ROOM
    FadeScreenOut
    WaitFadeScreen
    ScrCmd_0A6 VAR_MAP_LOCAL_0x02, VAR_0x8005, VAR_0x8007
    ReturnToField
    ApplyMovement LOCALID_PLAYER, JubilifeTV2FGallery_Movement_PlayerFaceSouth
    WaitMovement
    FadeScreenIn
    WaitFadeScreen
    GoToIfEq VAR_0x8006, DIR_NORTH, JubilifeTV2FGallery_PokemonFollower15
    GoToIfEq VAR_0x8006, DIR_WEST, JubilifeTV2FGallery_PokemonFollower16
    GoToIfEq VAR_0x8006, DIR_EAST, JubilifeTV2FGallery_PokemonFollower17
    End

JubilifeTV2FGallery_PokemonFollower15:
    ApplyMovement LOCALID_PLAYER, JubilifeTV2FGallery_Movement_PokemonFollower18
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower19
JubilifeTV2FGallery_ExitDressingRoomNorth:
    WaitMovement
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerBlockDoorNorth
    WaitMovement
    GoTo JubilifeTV2FGallery_CheckDressUpPokemonResult

JubilifeTV2FGallery_PokemonFollower16:
    ApplyMovement LOCALID_PLAYER, JubilifeTV2FGallery_Movement_PokemonFollower18
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower19
JubilifeTV2FGallery_ExitDressingRoomWest:
    WaitMovement
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerBlockDoorWest
    WaitMovement
    GoTo JubilifeTV2FGallery_CheckDressUpPokemonResult

JubilifeTV2FGallery_PokemonFollower17:
    ApplyMovement LOCALID_PLAYER, JubilifeTV2FGallery_Movement_PokemonFollower18
    ApplyMovement LOCALID_POKEMON_FOLLOWER, JubilifeTV2FGallery_Movement_PokemonFollower19
JubilifeTV2FGallery_ExitDressingRoomEast:
    WaitMovement
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerBlockDoorEast
    WaitMovement
    GoTo JubilifeTV2FGallery_CheckDressUpPokemonResult

JubilifeTV2FGallery_CheckDressUpPokemonResult:
    GoToIfEq VAR_0x8005, 0, JubilifeTV2FGallery_CameOutWithoutDoingAnything
    SetVar VAR_MAP_LOCAL_0x01, 1
    Message JubilifeTV2FGallery_Text_ItNeedsATitle
    WaitButton
    CloseMessage
    GoTo JubilifeTV2FGallery_SetDressUpPhotoTitle

JubilifeTV2FGallery_WheresYourSenseOfAdventure:
    Message JubilifeTV2FGallery_Text_WheresYourSenseOfAdventure
    WaitButton
    CloseMessage
    ReleaseAll
    End

JubilifeTV2FGallery_ThatsAnEgg:
    Message JubilifeTV2FGallery_Text_ThatsAnEgg
    GoTo JubilifeTV2FGallery_ChoosePokemonForDressUp

JubilifeTV2FGallery_SetDressUpPhotoTitle:
    FadeScreenOut
    WaitFadeScreen
    ChooseCustomMessageWord VAR_RESULT, VAR_0x8004
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    GoToIfEq VAR_RESULT, 0, JubilifeTV2FGallery_ForlornWithoutATitle
    SetDressUpPhotoTitle VAR_0x8004
    IncrementGameRecord RECORD_TIMES_DRESSED_UP_POKEMON
    Dummy316
    BufferCustomMessageWord 0, VAR_0x8004
    Message JubilifeTV2FGallery_Text_IGetItNow
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerFaceEast
    WaitMovement
    Message JubilifeTV2FGallery_Text_IPutPhotoRightHere
    WaitButton
    CloseMessage
    ApplyMovement LOCALID_HIKER, JubilifeTV2FGallery_Movement_HikerFaceSouth
    WaitMovement
    ReleaseAll
    End

JubilifeTV2FGallery_ForlornWithoutATitle:
    Message JubilifeTV2FGallery_Text_ForlornWithoutATitle
    WaitButton
    CloseMessage
    GoTo JubilifeTV2FGallery_SetDressUpPhotoTitle

JubilifeTV2FGallery_CameOutWithoutDoingAnything:
    Message JubilifeTV2FGallery_Text_CameOutWithoutDoingAnything
    WaitButton
    CloseMessage
    ReleaseAll
    End

    .balign 4, 0
JubilifeTV2FGallery_Movement_PlayerEnterDressingRoomNorth:
    WalkSlowNorth 2
    SetInvisible

    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_PlayerEnterDressingRoomWest:
    WalkSlowWest
    WalkSlowNorth
    SetInvisible

    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_PlayerEnterDressingRoomEast:
    WalkSlowEast
    WalkSlowNorth
    SetInvisible

    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_PlayerFaceSouth:
    FaceSouth
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower18:
    Delay16
    SetVisible

    .balign 4, 0
JubilifeTV2FGallery_Movement_PlayerExitDressingRoom:
    WalkSlowSouth 2
    FaceNorth
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_PokemonFollower19:
    SetVisible
    WalkSlowSouth 3

    FaceNorth
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerMoveOutOfTheWayNorth:
    WalkNormalWest
    FaceEast
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerMoveOutOfTheWayWest:
    WalkNormalWest
    FaceEast
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerMoveOutOfTheWayEast:
    WalkNormalEast
    FaceWest
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerBlockDoorNorth:
    WalkNormalEast
    FaceSouth
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerBlockDoorWest:
    WalkNormalEast
    FaceSouth
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerBlockDoorEast:
    WalkNormalWest
    FaceSouth
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerFaceEast:
    FaceEast
    EndMovement

    .balign 4, 0
JubilifeTV2FGallery_Movement_HikerFaceSouth:
    FaceSouth
    EndMovement

JubilifeTV2FGallery_Frame0:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 0, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 0, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame1:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 1, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 1, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame2:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 2, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 2, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame3:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 3, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 3, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame4:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 4, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 4, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame5:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 5, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 5, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame6:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 6, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 6, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame7:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 7, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 7, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame8:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 8, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 8, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame9:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 9, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 9, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_Frame10:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    DressUpPhotoHasData 10, VAR_RESULT
    GoToIfEq VAR_RESULT, FALSE, JubilifeTV2FGallery_FrameForHoldingPortrait
    FadeScreenOut
    WaitFadeScreen
    ShowDressUpPhoto 10, VAR_RESULT
    ReturnToField
    FadeScreenIn
    WaitFadeScreen
    ReleaseAll
    End

JubilifeTV2FGallery_FrameForHoldingPortrait:
    Message JubilifeTV2FGallery_Text_FrameForHoldingPortrait
    WaitButton
    CloseMessage
    ReleaseAll
    End

JubilifeTV2FGallery_PokemonBreederM:
    NPCMessage JubilifeTV2FGallery_Text_DisplayTrainersMixedRecordsWith
    End

JubilifeTV2FGallery_Twin:
    NPCMessage JubilifeTV2FGallery_Text_SomeoneGaveNewBackdrop
    End
