#include "overlay006/field_warp.h"

#include <nitro.h>
#include <string.h>

#include "generated/journal_location_events.h"

#include "field/field_system.h"
#include "overlay005/map_object_anim_cmd.h"
#include "overlay005/ov5_021F0EB0.h"
#include "overlay006/hm_cut_in.h"

#include "field_map_change.h"
#include "field_move_tasks.h"
#include "field_overworld_state.h"
#include "field_task.h"
#include "heap.h"
#include "journal.h"
#include "location.h"
#include "player_avatar.h"
#include "screen_fade.h"
#include "sound_playback.h"
#include "spawn_locations.h"
#include "unk_020655F4.h"

enum FieldWarpStateResult {
    STATE_RESULT_NEXT_STATE = 0,
    STATE_RESULT_REPEAT_STATE,
    STATE_RESULT_END_TASK,
};

typedef enum FieldWarpStateResult (*FieldWarpStateFunc)(FieldTask *, FieldSystem *, FieldWarp *);

static FieldWarp *FieldWarp_New(enum HeapID heapID, u32 size);
static void CreateJournalEntryForTeleport(FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult StartWarpOutSpinning(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult StartFadeOut(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult FinishFadeOut(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult ChangeMap(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult StartFadeIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult FinishFadeIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult SlowDownWarpInSpinning(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult FinishWarpInAnimation(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult StartFieldMoveCutIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);
static enum FieldWarpStateResult FinishFieldMoveCutIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp);

// Following Platinum: a visible follower spins along with the player during Escape Rope, Dig,
// Teleport and their arrivals
static void FieldWarp_SetPokemonFollower(FieldWarp *fieldWarp, FieldSystem *fieldSystem)
{
    MapObject *follower = PokemonFollower_GetMapObject(fieldSystem);

    fieldWarp->follower = follower != NULL && !MapObject_IsHidden(follower) ? follower : NULL;
    fieldWarp->followerAnimationTask = NULL;
}

static void FieldWarp_StartAnimation(FieldWarp *fieldWarp, const MapObjectAnimCmd *animation)
{
    fieldWarp->animationTask = MapObject_StartAnimation(fieldWarp->player, animation);

    if (fieldWarp->follower != NULL) {
        fieldWarp->followerAnimationTask = MapObject_StartAnimation(fieldWarp->follower, animation);
    }
}

static void FieldWarp_FinishAnimation(FieldWarp *fieldWarp)
{
    MapObject_FinishAnimation(fieldWarp->animationTask);

    if (fieldWarp->followerAnimationTask != NULL) {
        MapObject_FinishAnimation(fieldWarp->followerAnimationTask);
        fieldWarp->followerAnimationTask = NULL;
    }
}

static const FieldWarpStateFunc sEscapeRopeStates[] = {
    StartWarpOutSpinning,
    StartFadeOut,
    FinishFadeOut,
    ChangeMap
};

static const FieldWarpStateFunc sFieldWarpFadeInStates[] = {
    StartFadeIn,
    FinishFadeIn,
    SlowDownWarpInSpinning,
    FinishWarpInAnimation
};

static const FieldWarpStateFunc sDigTeleportStates[] = {
    StartFieldMoveCutIn,
    FinishFieldMoveCutIn,
    StartWarpOutSpinning,
    StartFadeOut,
    FinishFadeOut,
    ChangeMap
};

static const MapObjectAnimCmd sWarpFastSpinningAnimation[] = {
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_END, 0 }
};

static const MapObjectAnimCmd sFadeOutWarpAnimation[] = {
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 2 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 2 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 2 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 2 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_END, 0 }
};

static const MapObjectAnimCmd sFadeInWarpAnimation[] = {
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 1 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 2 },
    { MOVEMENT_ACTION_FACE_WEST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 3 },
    { MOVEMENT_ACTION_FACE_NORTH, 1 },
    { MOVEMENT_ACTION_DELAY_1, 4 },
    { MOVEMENT_ACTION_FACE_EAST, 1 },
    { MOVEMENT_ACTION_DELAY_1, 5 },
    { MOVEMENT_ACTION_FACE_SOUTH, 1 },
    { MOVEMENT_ACTION_END, 0 }
};

FieldWarp *FieldWarp_InitEscapeRope(FieldSystem *fieldSystem, enum HeapID heapID)
{
    FieldWarp *fieldWarp = FieldWarp_New(heapID, sizeof(FieldWarp));

    fieldWarp->warpType = FIELD_WARP_TYPE_ESCAPE_ROPE;
    fieldWarp->fieldSystem = fieldSystem;
    fieldWarp->player = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);
    FieldWarp_SetPokemonFollower(fieldWarp, fieldSystem);

    return fieldWarp;
}

BOOL FieldWarp_EscapeRopeFadeOut(FieldTask *task)
{
    enum FieldWarpStateResult stateResult;
    FieldSystem *fieldSystem = FieldTask_GetFieldSystem(task);
    FieldWarp *fieldWarp = FieldTask_GetEnv(task);

    do {
        stateResult = sEscapeRopeStates[fieldWarp->state](task, fieldSystem, fieldWarp);

        if (stateResult == STATE_RESULT_END_TASK) {
            Heap_Free(fieldWarp);
        }
    } while (stateResult == STATE_RESULT_REPEAT_STATE);

    return FALSE;
}

static enum FieldWarpStateResult StartWarpOutSpinning(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    fieldWarp->unk_14 = ov5_021F0EB0(fieldSystem, HEAP_ID_FIELD1);
    ov5_021F0F10(fieldWarp->unk_14, 1, FX32_CONST(-150), 15);
    FieldWarp_StartAnimation(fieldWarp, sFadeOutWarpAnimation);
    fieldWarp->state++;

    switch (fieldWarp->warpType) {
    case FIELD_WARP_TYPE_ESCAPE_ROPE:
        Sound_PlayEffect(SEQ_SE_DP_KAIDAN2_sseq);
        break;
    case FIELD_WARP_TYPE_DIG:
        Sound_PlayEffect(SEQ_SE_DP_KAIDAN2_sseq);
        break;
    case FIELD_WARP_TYPE_TELEPORT:
        Sound_PlayEffect(SEQ_SE_DP_TELE_sseq);
        break;
    }

    return STATE_RESULT_NEXT_STATE;
}

static enum FieldWarpStateResult StartFadeOut(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (!MapObject_HasAnimationEnded(fieldWarp->animationTask)) {
        return STATE_RESULT_NEXT_STATE;
    }

    FieldWarp_FinishAnimation(fieldWarp);

    FieldWarp_StartAnimation(fieldWarp, sWarpFastSpinningAnimation);
    fieldWarp->animationDelay++;

    if (fieldWarp->animationDelay < 8) {
        return STATE_RESULT_NEXT_STATE;
    }

    if (fieldWarp->warpType == FIELD_WARP_TYPE_TELEPORT) {
        StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_OUT, FADE_TYPE_BRIGHTNESS_OUT, COLOR_BLACK, 6, 1, HEAP_ID_FIELD1);
    } else {
        StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_OUT, FADE_TYPE_BRIGHTNESS_OUT, COLOR_WHITE, 6, 1, HEAP_ID_FIELD1);
    }

    fieldWarp->state++;
    return STATE_RESULT_NEXT_STATE;
}

static enum FieldWarpStateResult FinishFadeOut(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (MapObject_HasAnimationEnded(fieldWarp->animationTask) == TRUE) {
        FieldWarp_FinishAnimation(fieldWarp);
        FieldWarp_StartAnimation(fieldWarp, sWarpFastSpinningAnimation);
    }

    if (!IsScreenFadeDone()) {
        return STATE_RESULT_NEXT_STATE;
    }

    FieldWarp_FinishAnimation(fieldWarp);
    ov5_021F0EFC(fieldWarp->unk_14);

    fieldWarp->state++;

    return STATE_RESULT_REPEAT_STATE;
}

static enum FieldWarpStateResult ChangeMap(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    FieldOverworldState *fieldState = SaveData_GetFieldOverworldState(fieldSystem->saveData);

    if (fieldWarp->warpType == FIELD_WARP_TYPE_TELEPORT) {
        Location location;

        u16 warpId = FieldOverworldState_GetBlackOutWarpId(fieldState);
        Location_InitFly(warpId, &location);
        FieldTask_ChangeMapByFieldWarp(task, &location, fieldWarp->warpType);
    } else {
        Location *location = FieldOverworldState_GetExitLocation(fieldState);
        FieldTask_ChangeMapByFieldWarp(task, location, fieldWarp->warpType);
    }

    return STATE_RESULT_END_TASK;
}

FieldWarp *FieldWarp_InitFadeIn(FieldSystem *fieldSystem, enum HeapID heapID, enum FieldWarpType warpType)
{
    FieldWarp *fieldWarp = FieldWarp_New(heapID, sizeof(FieldWarp));

    fieldWarp->warpType = warpType;
    fieldWarp->fieldSystem = fieldSystem;
    fieldWarp->player = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);
    FieldWarp_SetPokemonFollower(fieldWarp, fieldSystem);

    return fieldWarp;
}

BOOL FieldWarp_FadeIn(FieldTask *task)
{
    enum FieldWarpStateResult stateResult;
    FieldSystem *fieldSystem = FieldTask_GetFieldSystem(task);
    FieldWarp *fieldWarp = FieldTask_GetEnv(task);

    do {
        stateResult = sFieldWarpFadeInStates[fieldWarp->state](task, fieldSystem, fieldWarp);

        if (stateResult == STATE_RESULT_END_TASK) {
            CreateJournalEntryForTeleport(fieldSystem, fieldWarp);
            Heap_Free(fieldWarp);
            return TRUE;
        }
    } while (stateResult == STATE_RESULT_REPEAT_STATE);

    return FALSE;
}

static void CreateJournalEntryForTeleport(FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (fieldWarp->warpType != FIELD_WARP_TYPE_TELEPORT) {
        return;
    }

    void *journalEntryLocationEvent = JournalEntry_CreateEventUsedMove(FIELD_MOVE_TELEPORT, fieldSystem->location->mapHeaderID, HEAP_ID_FIELD1);
    JournalEntry_SaveData(fieldSystem->journalEntry, journalEntryLocationEvent, JOURNAL_LOCATION);
}

static enum FieldWarpStateResult StartFadeIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (fieldWarp->warpType == FIELD_WARP_TYPE_TELEPORT) {
        StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_IN, FADE_TYPE_BRIGHTNESS_IN, COLOR_BLACK, 6, 1, HEAP_ID_FIELD1);
    } else {
        StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_IN, FADE_TYPE_BRIGHTNESS_IN, COLOR_WHITE, 6, 1, HEAP_ID_FIELD1);
    }

    fieldWarp->unk_14 = ov5_021F0EB0(fieldSystem, HEAP_ID_FIELD1);
    ov5_021F0F10(fieldWarp->unk_14, 1, FX32_CONST(-150), 1);
    FieldWarp_StartAnimation(fieldWarp, sWarpFastSpinningAnimation);
    fieldWarp->state++;

    return STATE_RESULT_NEXT_STATE;
}

static enum FieldWarpStateResult FinishFadeIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (MapObject_HasAnimationEnded(fieldWarp->animationTask) == TRUE) {
        FieldWarp_FinishAnimation(fieldWarp);
        FieldWarp_StartAnimation(fieldWarp, sWarpFastSpinningAnimation);
    }

    if (!IsScreenFadeDone()) {
        return STATE_RESULT_NEXT_STATE;
    }

    ov5_021F0F10(fieldWarp->unk_14, 2, 0, 60);

    fieldWarp->state++;
    return STATE_RESULT_REPEAT_STATE;
}

static enum FieldWarpStateResult SlowDownWarpInSpinning(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (!MapObject_HasAnimationEnded(fieldWarp->animationTask)) {
        return STATE_RESULT_NEXT_STATE;
    }

    FieldWarp_FinishAnimation(fieldWarp);
    fieldWarp->animationDelay++;

    if (fieldWarp->animationDelay < 4) {
        FieldWarp_StartAnimation(fieldWarp, sWarpFastSpinningAnimation);
        return STATE_RESULT_NEXT_STATE;
    }

    FieldWarp_StartAnimation(fieldWarp, sFadeInWarpAnimation);
    fieldWarp->state++;

    return STATE_RESULT_NEXT_STATE;
}

static enum FieldWarpStateResult FinishWarpInAnimation(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (!MapObject_HasAnimationEnded(fieldWarp->animationTask)) {
        return STATE_RESULT_NEXT_STATE;
    }

    if (!ov5_021F0EF0(fieldWarp->unk_14)) {
        return STATE_RESULT_NEXT_STATE;
    }

    ov5_021F0EFC(fieldWarp->unk_14);
    FieldWarp_FinishAnimation(fieldWarp);

    return STATE_RESULT_END_TASK;
}

FieldWarp *FieldWarp_InitDig(FieldSystem *fieldSystem, Pokemon *mon, enum HeapID heapID)
{
    FieldWarp *fieldWarp = FieldWarp_New(heapID, sizeof(FieldWarp));

    fieldWarp->warpType = FIELD_WARP_TYPE_DIG;
    fieldWarp->fieldSystem = fieldSystem;
    fieldWarp->player = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);
    FieldWarp_SetPokemonFollower(fieldWarp, fieldSystem);
    fieldWarp->mon = mon;

    return fieldWarp;
}

BOOL FieldWarp_DigFadeOut(FieldTask *task)
{
    enum FieldWarpStateResult stateResult;
    FieldSystem *fieldSystem = FieldTask_GetFieldSystem(task);
    FieldWarp *fieldWarp = FieldTask_GetEnv(task);

    do {
        stateResult = sDigTeleportStates[fieldWarp->state](task, fieldSystem, fieldWarp);

        if (stateResult == STATE_RESULT_END_TASK) {
            Heap_Free(fieldWarp);
        }
    } while (stateResult == STATE_RESULT_REPEAT_STATE);

    return FALSE;
}

static enum FieldWarpStateResult StartFieldMoveCutIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    int gender = PlayerAvatar_GetGender(fieldSystem->playerAvatar);

    fieldWarp->cutInTask = HMCutIn_StartTask(fieldSystem, 0, fieldWarp->mon, gender);
    fieldWarp->state++;

    return STATE_RESULT_NEXT_STATE;
}

static enum FieldWarpStateResult FinishFieldMoveCutIn(FieldTask *task, FieldSystem *fieldSystem, FieldWarp *fieldWarp)
{
    if (HMCutIn_IsFinished(fieldWarp->cutInTask) == FALSE) {
        return STATE_RESULT_NEXT_STATE;
    }

    HMCutIn_EndTask(fieldWarp->cutInTask);
    fieldWarp->state++;
    return STATE_RESULT_REPEAT_STATE;
}

FieldWarp *FieldWarp_InitTeleport(FieldSystem *fieldSystem, Pokemon *mon, enum HeapID heapID)
{
    FieldWarp *fieldWarp = FieldWarp_New(heapID, sizeof(FieldWarp));

    fieldWarp->warpType = FIELD_WARP_TYPE_TELEPORT;
    fieldWarp->fieldSystem = fieldSystem;
    fieldWarp->player = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);
    FieldWarp_SetPokemonFollower(fieldWarp, fieldSystem);
    fieldWarp->mon = mon;

    return fieldWarp;
}

BOOL FieldWarp_TeleportFadeOut(FieldTask *task)
{
    enum FieldWarpStateResult stateResult;
    FieldSystem *fieldSystem = FieldTask_GetFieldSystem(task);
    FieldWarp *fieldWarp = FieldTask_GetEnv(task);

    do {
        stateResult = sDigTeleportStates[fieldWarp->state](task, fieldSystem, fieldWarp);

        if (stateResult == STATE_RESULT_END_TASK) {
            Heap_Free(fieldWarp);
        }
    } while (stateResult == STATE_RESULT_REPEAT_STATE);

    return FALSE;
}

static FieldWarp *FieldWarp_New(enum HeapID heapID, u32 size)
{
    FieldWarp *fieldWarp = Heap_AllocAtEnd(heapID, size);

    GF_ASSERT(fieldWarp != NULL);
    memset(fieldWarp, 0, size);
    return fieldWarp;
}
