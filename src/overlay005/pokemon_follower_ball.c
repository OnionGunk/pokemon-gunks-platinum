#include "overlay005/pokemon_follower_ball.h"

#include <nitro.h>

#include "constants/field/field_effect_renderer.h"
#include "constants/heap.h"
#include "constants/pokemon_follower.h"

#include "overlay005/field_effect_manager.h"
#include "overlay005/ov5_021ECA70.h"

#include "billboard.h"
#include "billboard_vram_transfer.h"
#include "easy3d_object.h"
#include "field_map_change.h"
#include "heap.h"
#include "map_object.h"
#include "overworld_anim_manager.h"
#include "sound_playback.h"
#include "vram_transfer.h"

#include "res/graphics/field_effects/field_effects.naix"

// Ported from Following Platinum (fp_023c9248 and its renderer). Releasing: a Poke Ball sits on the
// spot for a moment, then opens in a flash that reveals the Pokemon, which cries. Recalling: the
// Pokemon shrinks while turning white, then the ball appears in its place and fades away.

#define BALL_HEIGHT            FX32_CONST(6)
#define RELEASE_DELAY          2
#define RECALL_SHRINK_FRAMES   6
#define RECALL_BALL_FADE       6
#define PLTT_COLORS            16

typedef struct FollowerBallRenderer {
    FieldEffectManager *fieldEffMan;
    NNSFndAllocator allocator;
    Easy3DModel ballModel;
    Easy3DModel releaseModel;
    Easy3DObject ballObj;
    Easy3DObject releaseObj;
    Easy3DAnim releaseAnim;
} FollowerBallRenderer;

typedef struct FollowerBallParams {
    FollowerBallRenderer *renderer;
    MapObject *mapObj;
    int mode;
    u16 species;
} FollowerBallParams;

typedef struct FollowerBall {
    FollowerBallParams params;
    int state;
    int timer;
    int graphicsID;
    int localID;
    int mapHeaderID;
    u16 *pltt;
} FollowerBall;

static const OverworldAnimManagerFuncs sFollowerBallFuncs;

void *PokemonFollowerBallRenderer_New(FieldEffectManager *fieldEffMan)
{
    FollowerBallRenderer *renderer = FieldEffectManager_HeapAllocInit(fieldEffMan, sizeof(FollowerBallRenderer), FALSE, 0);

    renderer->fieldEffMan = fieldEffMan;
    HeapExp_FndInitAllocator(&renderer->allocator, HEAP_ID_FIELD1, 4);

    Easy3DModel_LoadFrom(&renderer->ballModel, fieldEffMan->fldEffNARC, follower_poke_ball_nsbmd, HEAP_ID_FIELD1);
    Easy3DObject_Init(&renderer->ballObj, &renderer->ballModel);
    Easy3DObject_SetVisible(&renderer->ballObj, FALSE);

    Easy3DModel_LoadFrom(&renderer->releaseModel, fieldEffMan->fldEffNARC, follower_poke_ball_release_nsbmd, HEAP_ID_FIELD1);
    Easy3DObject_Init(&renderer->releaseObj, &renderer->releaseModel);
    Easy3DAnim_LoadFrom(&renderer->releaseAnim, &renderer->releaseModel, fieldEffMan->fldEffNARC, follower_poke_ball_release_nsbta, HEAP_ID_FIELD1, &renderer->allocator);
    Easy3DObject_AddAnim(&renderer->releaseObj, &renderer->releaseAnim);
    Easy3DObject_SetVisible(&renderer->releaseObj, FALSE);

    return renderer;
}

void PokemonFollowerBallRenderer_Free(void *context)
{
    FollowerBallRenderer *renderer = context;

    Easy3DAnim_Release(&renderer->releaseAnim, &renderer->allocator);
    Easy3DModel_Release(&renderer->releaseModel);
    Easy3DModel_Release(&renderer->ballModel);
    FieldEffectManager_HeapFree(renderer);
}

OverworldAnimManager *PokemonFollowerBall_Play(MapObject *mapObj, enum PokeBallAnimationMode mode, u16 species)
{
    FieldEffectManager *fieldEffMan = MapObject_GetFieldEffectManager(mapObj);
    FollowerBallParams params;
    VecFx32 pos;

    params.renderer = FieldEffectManager_GetRendererContext(fieldEffMan, FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_BALL);
    params.mapObj = mapObj;
    params.mode = mode;
    params.species = species;

    MapObject_GetPosPtr(mapObj, &pos);
    pos.y += BALL_HEIGHT;

    return FieldEffectManager_InitAnimManager(fieldEffMan, &sFollowerBallFuncs, &pos, 0, &params, MapObject_CalculateTaskPriority(mapObj, 2));
}

static BOOL FollowerBall_IsObjectValid(FollowerBall *ball)
{
    return sub_020627B4(ball->params.mapObj, ball->graphicsID, ball->localID, ball->mapHeaderID);
}

static int FollowerBall_Init(OverworldAnimManager *animMan, void *context)
{
    FollowerBall *ball = context;
    FollowerBallRenderer *renderer;
    VecFx32 pos;

    ball->params = *(const FollowerBallParams *)OverworldAnimManager_GetUserData(animMan);
    renderer = ball->params.renderer;
    ball->graphicsID = MapObject_GetGraphicsID(ball->params.mapObj);
    ball->localID = MapObject_GetLocalID(ball->params.mapObj);
    ball->mapHeaderID = MapObject_GetMapHeaderID(ball->params.mapObj);

    OverworldAnimManager_GetPosition(animMan, &pos);
    Easy3DObject_SetPosition(&renderer->ballObj, pos.x, pos.y, pos.z);
    Easy3DObject_SetPosition(&renderer->releaseObj, pos.x, pos.y, pos.z);
    NNS_G3dMdlSetMdlAlphaAll(renderer->ballModel.model, 31);
    Easy3DObject_SetVisible(&renderer->ballObj, ball->params.mode == POKE_BALL_ANIMATION_RELEASE);
    Easy3DObject_SetVisible(&renderer->releaseObj, FALSE);

    return TRUE;
}

static void FollowerBall_Delete(OverworldAnimManager *animMan, void *context)
{
    FollowerBall *ball = context;

    Easy3DObject_SetVisible(&ball->params.renderer->ballObj, FALSE);
    Easy3DObject_SetVisible(&ball->params.renderer->releaseObj, FALSE);

    if (ball->pltt != NULL) {
        Heap_Free(ball->pltt);
    }
}

static void FollowerBall_SetObjectHidden(MapObject *mapObj, BOOL hidden)
{
    if (MapObject_GetLocalID(mapObj) == POKEMON_FOLLOWER_LOCAL_ID) {
        PokemonFollower_SetScriptHidden(mapObj, hidden);
    } else {
        MapObject_SetHidden(mapObj, hidden);
    }
}

static void FollowerBall_UpdateRelease(OverworldAnimManager *animMan, FollowerBall *ball)
{
    FollowerBallRenderer *renderer = ball->params.renderer;

    switch (ball->state) {
    case 0:
        if (++ball->timer >= RELEASE_DELAY) {
            ball->state++;
        }
        break;
    case 1:
        Sound_PlayEffect(SEQ_SE_DP_KON3_sseq);
        Easy3DObject_SetVisible(&renderer->ballObj, FALSE);
        Easy3DObject_SetVisible(&renderer->releaseObj, TRUE);
        FollowerBall_SetObjectHidden(ball->params.mapObj, FALSE);
        Easy3DAnim_SetFrame(&renderer->releaseAnim, 0);
        ball->state++;
        // fall through
    case 2:
        if (Easy3DAnim_Update(&renderer->releaseAnim, FX32_ONE) == TRUE) {
            Sound_PlayPokemonCry(ball->params.species, 0);
            FieldEffectManager_FinishAnimManager(animMan);
        }
        break;
    }
}

static void FollowerBall_UploadPltt(Billboard *billboard, void *pltt)
{
    VramTransfer_Request(NNS_GFD_DST_3D_TEX_PLTT, NNS_GfdGetPlttKeyAddr(billboard->plttKey), pltt, PLTT_COLORS * sizeof(u16));
}

static void FollowerBall_UpdateRecall(OverworldAnimManager *animMan, FollowerBall *ball)
{
    FollowerBallRenderer *renderer = ball->params.renderer;
    Billboard *billboard = ov5_021ECB80(ball->params.mapObj);
    VecFx32 scale = { FX32_ONE, FX32_ONE, FX32_ONE };

    switch (ball->state) {
    case 0: {
        // the billboard's own palette, white-blended into a heap copy (DMA can't read the stack)
        int plttIdx = (MapObject_GetDataAt(ball->params.mapObj, POKEMON_FOLLOWER_DATA_FLAGS) & POKEMON_FOLLOWER_FLAG_SHINY) ? 1 : 0;

        ball->pltt = Heap_Alloc(HEAP_ID_FIELD1, PLTT_COLORS * sizeof(u16) * 2);
        MI_CpuCopy16(GetPlttDataVRAMBuffer(billboard->animTexture, plttIdx), ball->pltt, PLTT_COLORS * sizeof(u16));
        Sound_PlayEffect(SEQ_SE_DP_BOWA_sseq);
        ball->state++;
        break;
    }
    case 1: {
        u16 *white = ball->pltt + PLTT_COLORS;

        for (int i = 0; i < PLTT_COLORS; i++) {
            u16 c = ball->pltt[i];
            int strength = ball->timer * 3;
            int r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;

            r += ((31 - r) * strength) >> 4;
            g += ((31 - g) * strength) >> 4;
            b += ((31 - b) * strength) >> 4;
            white[i] = (c & 0x8000) | (b << 10) | (g << 5) | r;
        }

        FollowerBall_UploadPltt(billboard, white);

        scale.x = scale.y = FX32_ONE - ball->timer * (FX32_ONE / 8);
        Billboard_SetScale(billboard, &scale);

        if (++ball->timer >= RECALL_SHRINK_FRAMES) {
            Easy3DObject_SetVisible(&renderer->ballObj, TRUE);
            FollowerBall_UploadPltt(billboard, ball->pltt);
            FollowerBall_SetObjectHidden(ball->params.mapObj, TRUE);
            ball->timer = 0;
            ball->state++;
        }
        break;
    }
    case 2:
        NNS_G3dMdlSetMdlAlphaAll(renderer->ballModel.model, 31 - ball->timer * 5);

        if (++ball->timer >= RECALL_BALL_FADE) {
            Billboard_SetScale(billboard, &scale);
            FieldEffectManager_FinishAnimManager(animMan);
        }
        break;
    }
}

static void FollowerBall_Update(OverworldAnimManager *animMan, void *context)
{
    FollowerBall *ball = context;

    if (!FollowerBall_IsObjectValid(ball)) {
        FieldEffectManager_FinishAnimManager(animMan);
        return;
    }

    if (ball->params.mode == POKE_BALL_ANIMATION_RELEASE) {
        FollowerBall_UpdateRelease(animMan, ball);
    } else {
        FollowerBall_UpdateRecall(animMan, ball);
    }
}

static void FollowerBall_Draw(OverworldAnimManager *animMan, void *context)
{
    FollowerBall *ball = context;

    Easy3DObject_Draw(&ball->params.renderer->ballObj);
    Easy3DObject_Draw(&ball->params.renderer->releaseObj);
}

static const OverworldAnimManagerFuncs sFollowerBallFuncs = {
    sizeof(FollowerBall),
    FollowerBall_Init,
    FollowerBall_Delete,
    FollowerBall_Update,
    FollowerBall_Draw
};
