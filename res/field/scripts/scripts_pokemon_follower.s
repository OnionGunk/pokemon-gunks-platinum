#include "macros/scrcmd.inc"
#include "res/text/bank/pokemon_follower.h"

// Talking to the Pokemon that follows the player. Ported from Following Platinum's added script
// (scr_seq member 1124): the reaction and the line depend on the Pokemon's friendship.
#define LOCALID_POKEMON_FOLLOWER 253


    ScriptEntry PokemonFollower_Talk
    ScriptEntryEnd

PokemonFollower_Talk:
    PlaySE SE_CONFIRM_sseq_3
    LockAll
    FacePlayer
    GetPokemonFollowerPartySlot VAR_0x800C
    BufferPartyMonNickname 0, VAR_0x800C
    BufferPlayerName 1
    GetPartyMonFriendship VAR_0x8004, VAR_0x800C
    GoToIfGe VAR_0x8004, 180, PokemonFollower_VeryFriendly
    GoToIfGe VAR_0x8004, 70, PokemonFollower_Friendly
    GoTo PokemonFollower_Unfriendly
    End

PokemonFollower_VeryFriendly:
    GetRandom VAR_0x8008, 10
    GoToIfEq VAR_0x8008, 0, PokemonFollower_CryHeart
    GoToIfEq VAR_0x8008, 1, PokemonFollower_CryHopHeart
    GoToIfEq VAR_0x8008, 2, PokemonFollower_LookAroundHop
    GoToIfEq VAR_0x8008, 3, PokemonFollower_LookAroundHopMusic
    GoToIfEq VAR_0x8008, 4, PokemonFollower_CryHopsHappy
    GoToIfEq VAR_0x8008, 5, PokemonFollower_CryHopsHappy2
    GoToIfEq VAR_0x8008, 6, PokemonFollower_CrySmile
    GoToIfEq VAR_0x8008, 7, PokemonFollower_HopCrySmile
    GoToIfEq VAR_0x8008, 8, PokemonFollower_LookAroundSilence
    GoTo PokemonFollower_Beam
    End

PokemonFollower_Friendly:
    GetRandom VAR_0x8008, 4
    GoToIfEq VAR_0x8008, 0, PokemonFollower_CrySmile
    GoToIfEq VAR_0x8008, 1, PokemonFollower_HopCrySmile
    GoToIfEq VAR_0x8008, 2, PokemonFollower_Silence
    GoTo PokemonFollower_LookAroundSilence
    End

PokemonFollower_Unfriendly:
    GetRandom VAR_0x8008, 3
    GoToIfEq VAR_0x8008, 0, PokemonFollower_CrySad
    GoToIfEq VAR_0x8008, 1, PokemonFollower_LookAway
    GoTo PokemonFollower_CryAngry
    End

// Each reaction picks one of its lines at random: PokemonFollower_ShowRandomMessage shows
// message VAR_0x8005 + a random offset below VAR_0x8006.
PokemonFollower_CryAngry:
    Call PokemonFollower_Cry
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteFace2
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_000
    SetVar VAR_0x8006, 6
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_LookAroundHop:
    Call PokemonFollower_LookAroundAndFacePlayer
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_Hop
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_018
    SetVar VAR_0x8006, 1
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_LookAroundHopMusic:
    Call PokemonFollower_LookAroundAndFacePlayer
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_Hop
    WaitMovement
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteMusic
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_019
    SetVar VAR_0x8006, 2
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_Silence:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteSilence
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_022
    SetVar VAR_0x8006, 6
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_LookAroundSilence:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_LookAroundSlowly
    WaitMovement
    FacePlayer
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteSilence
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_028
    SetVar VAR_0x8006, 2
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_CryHopsHappy:
    Call PokemonFollower_CryAndHopTwice
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteFace1
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_046
    SetVar VAR_0x8006, 7
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_CryHopsHappy2:
    Call PokemonFollower_CryAndHopTwice
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteFace1
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_053
    SetVar VAR_0x8006, 7
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_CryHeart:
    Call PokemonFollower_Cry
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteHeart
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_060
    SetVar VAR_0x8006, 1
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_CryHopHeart:
    Call PokemonFollower_CryAndHopTwice
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteHeart
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_061
    SetVar VAR_0x8006, 1
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_CrySad:
    Call PokemonFollower_Cry
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteFace3
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_113
    SetVar VAR_0x8006, 2
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_LookAway:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_LookAroundSlowly
    WaitMovement
    FacePlayer
    SetVar VAR_0x8005, PokemonFollower_Text_117
    SetVar VAR_0x8006, 1
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_CrySmile:
    Call PokemonFollower_Cry
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteFace4
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_129
    SetVar VAR_0x8006, 12
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_HopCrySmile:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_Hop
    WaitMovement
    Call PokemonFollower_Cry
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteFace4
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_141
    SetVar VAR_0x8006, 1
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_Beam:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_EmoteFace5
    WaitMovement
    SetVar VAR_0x8005, PokemonFollower_Text_152
    SetVar VAR_0x8006, 1
    GoTo PokemonFollower_ShowRandomMessage

PokemonFollower_ShowRandomMessage:
    GetRandom VAR_0x8004, VAR_0x8006
    AddVar VAR_0x8005, VAR_0x8004
    MessageVar VAR_0x8005
    WaitButton
    CloseMessage
    ReleaseAll
    End

PokemonFollower_Cry:
    GetPartyMonSpecies VAR_0x800C, VAR_0x8004
    GetPartyMonForm VAR_0x800C, VAR_0x8005
    PlayCry VAR_0x8004, VAR_0x8005
    Return

PokemonFollower_CryAndHopTwice:
    Call PokemonFollower_Cry
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_HopTwice
    WaitMovement
    Return

PokemonFollower_LookAroundAndFacePlayer:
    ApplyMovement LOCALID_POKEMON_FOLLOWER, PokemonFollower_Movement_LookAroundQuickly
    WaitMovement
    FacePlayer
    Return

    .balign 4, 0
PokemonFollower_Movement_HopTwice:
    LockDir
    JumpOnSpotSlowNorth 2
    UnlockDir
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_Hop:
    LockDir
    JumpOnSpotSlowNorth
    UnlockDir
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_LookAroundQuickly:
    Delay8
    FaceNorth
    Delay4
    FaceWest
    Delay4
    FaceSouth
    Delay4
    FaceEast
    Delay4
    FaceNorth
    Delay8
    FaceWest
    Delay8
    FaceSouth
    Delay8
    FaceEast
    Delay8
    FaceNorth
    Delay2
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
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_LookAroundSlowly:
    Delay8 2
    FaceNorth
    Delay8 4
    FaceWest
    Delay8 3
    FaceEast
    Delay8 4
    FaceSouth
    Delay8 5
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteHeart:
    FollowerEmoteHeart
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteFace1:
    FollowerEmoteFace1
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteFace2:
    FollowerEmoteFace2
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteFace3:
    FollowerEmoteFace3
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteFace4:
    FollowerEmoteFace4
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteFace5:
    FollowerEmoteFace5
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteMusic:
    FollowerEmoteMusic
    EndMovement

    .balign 4, 0
PokemonFollower_Movement_EmoteSilence:
    FollowerEmoteSilence
    EndMovement
