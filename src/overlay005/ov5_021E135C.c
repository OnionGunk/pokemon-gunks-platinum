#include "overlay005/ov5_021E135C.h"

#include <nitro.h>
#include <nitro/os.h>
#include <string.h>

#include "struct_decls/map_object.h"

#include "field/field_system.h"
#include "generated/movement_actions.h"
#include "overlay005/ov5_021ECC20.h"

#include "field_map_change.h"
#include "heap.h"
#include "map_object.h"
#include "map_object_move.h"
#include "player_avatar.h"
#include "screen_fade.h"
#include "sound_playback.h"
#include "sys_task.h"
#include "sys_task_manager.h"
#include "unk_020655F4.h"

typedef struct {
    FieldSystem *fieldSystem;
    BOOL *unk_04;
    int unk_08;
    int unk_0C;
    int unk_10;
} UnkStruct_ov5_021E135C;

static void ov5_021E135C(UnkStruct_ov5_021E135C *param0)
{
    switch (param0->unk_10) {
    case 0:
        param0->unk_10 = 2;
        break;
    case 2:
        param0->unk_10 = 1;
        break;
    case 1:
        param0->unk_10 = 3;
        break;
    case 3:
        param0->unk_10 = 0;
        break;
    }

    PlayerAvatar_TryFace(param0->fieldSystem->playerAvatar, param0->unk_10);
}

// Following Platinum: the follower steps onto the warp panel, then spins and rises 12 frames behind the player
static void ov5_021E139C_UpdatePokemonFollower(UnkStruct_ov5_021E135C *param0)
{
    MapObject *follower = PokemonFollower_GetMapObject(param0->fieldSystem);
    PlayerAvatar *playerAvatar = param0->fieldSystem->playerAvatar;
    VecFx32 offset;
    int frame;

    if (follower == NULL || MapObject_IsHidden(follower)) {
        return;
    }

    if (param0->unk_0C == 4 && LocalMapObj_IsAnimationSet(follower)) {
        int playerX = PlayerAvatar_GetXPos(playerAvatar);
        int playerZ = PlayerAvatar_GetZPos(playerAvatar);

        if (MapObject_GetX(follower) != playerX || MapObject_GetZ(follower) != playerZ) {
            int dir = GetDirectionBetweenPoints(MapObject_GetX(follower), MapObject_GetZ(follower), playerX, playerZ);

            LocalMapObj_SetAnimationCode(follower, MovementAction_TurnActionTowardsDir(dir, MOVEMENT_ACTION_WALK_NORMAL_NORTH));
        }
    }

    if (param0->unk_0C > 12) {
        frame = param0->unk_0C - 12;

        MapObject_TryFace(follower, param0->unk_10);
        MapObject_GetSpriteJumpOffset(follower, &offset);
        offset.y = ((FX32_ONE * 2.2) + ((FX32_ONE / 2) * frame)) * frame;
        MapObject_SetSpriteJumpOffset(follower, &offset);
    }
}

static void ov5_021E139C(SysTask *param0, void *param1)
{
    UnkStruct_ov5_021E135C *v0 = param1;
    MapObject *v1 = PlayerAvatar_GetMapObject(v0->fieldSystem->playerAvatar);
    VecFx32 v2;

    switch (v0->unk_08) {
    case 0:
        v0->unk_08 = 1;
        Sound_PlayEffect(SEQ_SE_DP_TELE2_sseq);
    case 1:
        if (v0->unk_0C % 2) {
            ov5_021E135C(v0);
        }

        MapObject_GetSpritePosOffset(v1, &v2);
        v2.y = ((FX32_ONE * 2.2) + ((FX32_ONE / 2) * v0->unk_0C)) * v0->unk_0C;

        MapObject_SetSpritePosOffset(v1, &v2);
        ov5_021E139C_UpdatePokemonFollower(v0);
        v0->unk_0C++;

        // Following Platinum fades out 2 frames later, once the follower has risen too
        if (v0->unk_0C == 22) {
            StartScreenFade(FADE_SUB_THEN_MAIN, FADE_TYPE_BRIGHTNESS_OUT, FADE_TYPE_BRIGHTNESS_OUT, COLOR_BLACK, 6, 1, HEAP_ID_FIELD1);
        } else if ((v0->unk_0C > 22) && IsScreenFadeDone()) {
            if (PokemonFollower_GetMapObject(v0->fieldSystem) != NULL) {
                sub_020656AC(PokemonFollower_GetMapObject(v0->fieldSystem));
            }

            *v0->unk_04 = 1;
            Heap_Free(v0);
            SysTask_Done(param0);
        } else {
            break;
        }
    }
}

static void ov5_021E1470(SysTask *param0, void *param1)
{
    UnkStruct_ov5_021E135C *v0 = param1;
    MapObject *v1 = PlayerAvatar_GetMapObject(v0->fieldSystem->playerAvatar);
    VecFx32 v2;
    int v3;

    switch (v0->unk_08) {
    case 0: {
        MapObject_SetPauseMovementOff(v1);
        MapObject_GetSpritePosOffset(v1, &v2);
        v3 = (20 - v0->unk_0C);
        v2.y = ((FX32_ONE * 2.2) + ((FX32_ONE / 2) * v3)) * v3;
        MapObject_SetSpritePosOffset(v1, &v2);
        MapObject_Draw(v1);
    }

        Sound_PlayEffect(SEQ_SE_DP_TELE2_sseq);
        v0->unk_08 = 1;
    case 1:
        if (v0->unk_0C % 2) {
            ov5_021E135C(v0);
        }

        MapObject_GetSpritePosOffset(v1, &v2);
        v3 = (20 - v0->unk_0C);
        v2.y = ((FX32_ONE * 2.2) + ((FX32_ONE / 2) * v3)) * v3;
        MapObject_SetSpritePosOffset(v1, &v2);
        v0->unk_0C++;

        if (v0->unk_0C == 2) {
            StartScreenFade(FADE_MAIN_THEN_SUB, FADE_TYPE_BRIGHTNESS_IN, FADE_TYPE_BRIGHTNESS_IN, COLOR_BLACK, 6, 1, HEAP_ID_FIELD1);
        }

        if (v0->unk_0C > 20) {
            v0->unk_08 = 2;
        }
        break;
    case 2:
        if (IsScreenFadeDone()) {
            PlayerAvatar_TryFace(v0->fieldSystem->playerAvatar, 1);

            if (PokemonFollower_GetMapObject(v0->fieldSystem) != NULL) {
                MapObject_TryFace(PokemonFollower_GetMapObject(v0->fieldSystem), 1);
            }
            *v0->unk_04 = 1;
            Heap_Free(v0);
            SysTask_Done(param0);
        }
        break;
    }
}

void FieldSystem_StartWarpAnimation(FieldSystem *fieldSystem, BOOL param1, BOOL *param2)
{
    UnkStruct_ov5_021E135C *v0 = Heap_AllocAtEnd(HEAP_ID_FIELD1, sizeof(UnkStruct_ov5_021E135C));

    MI_CpuClear8(v0, sizeof(UnkStruct_ov5_021E135C));

    v0->fieldSystem = fieldSystem;
    v0->unk_04 = param2;
    v0->unk_10 = PlayerAvatar_GetFacingDir(fieldSystem->playerAvatar);

    if (param1) {
        SysTask_Start(ov5_021E139C, v0, 100);
    } else {
        SysTask_Start(ov5_021E1470, v0, 100);
    }
}
