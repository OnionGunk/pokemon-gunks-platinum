#include "overlay005/pokemon_follower_emote.h"

#include <nitro.h>

#include "constants/field/field_effect_renderer.h"

#include "overlay005/field_effect_manager.h"
#include "overlay005/ov5_021ECC20.h"

#include "billboard.h"
#include "map_object.h"
#include "overworld_anim_manager.h"
#include "sound_playback.h"

#include "res/graphics/field_effects/field_effects.naix"

// Emote bubbles over the Pokemon follower, ported from Following Platinum (fp_023c98e0 and its
// renderer). The bubble model and frame sequence are shared; each emote has its own 2-frame texture,
// loaded while one of its bubbles is on screen.

// field effect resource keys; 0-16 are used by the vanilla renderers and 17 marks a free slot
#define EMOTE_KEY_MODEL    18
#define EMOTE_KEY_SEQUENCE 19
#define EMOTE_KEY_TEXTURE  20
#define EMOTE_KEY_RES      (EMOTE_KEY_TEXTURE + POKEMON_FOLLOWER_EMOTE_COUNT)

// pop up like the vanilla "!" bubble, flicker through the frame sequence, then hold
#define EMOTE_HEIGHT     FX32_CONST(40)
#define EMOTE_DEPTH      FX32_CONST(1.25)
#define EMOTE_HOLD_TIME  10

typedef struct FollowerEmoteRenderer {
    FieldEffectManager *fieldEffMan;
    u8 visibleCount[POKEMON_FOLLOWER_EMOTE_COUNT];
} FollowerEmoteRenderer;

typedef struct FollowerEmoteParams {
    FieldEffectManager *fieldEffMan;
    FollowerEmoteRenderer *renderer;
    MapObject *mapObj;
    int emote;
} FollowerEmoteParams;

typedef struct FollowerEmote {
    FollowerEmoteParams params;
    int state;
    int timer;
    int localID;
    int mapHeaderID;
    BOOL done;
    fx32 offsetY;
    fx32 velocityY;
    Billboard *billboard;
} FollowerEmote;

enum FollowerEmoteState {
    EMOTE_STATE_POP_UP = 0,
    EMOTE_STATE_ANIMATE,
    EMOTE_STATE_HOLD,
    EMOTE_STATE_DONE,
};

static const BillboardAnim sEmoteAnims[] = {
    { 0, 15, BILLBOARD_ANIM_TYPE_ONESHOT },
    { 0, 0, BILLBOARD_ANIM_TYPE_TABLE_END }
};

static const u16 sEmoteTextures[POKEMON_FOLLOWER_EMOTE_COUNT] = {
    [POKEMON_FOLLOWER_EMOTE_HEART] = follower_emote_heart_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_FACE_1] = follower_emote_face_1_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_FACE_2] = follower_emote_face_2_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_FACE_3] = follower_emote_face_3_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_FACE_4] = follower_emote_face_4_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_FACE_5] = follower_emote_face_5_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_MUSIC] = follower_emote_music_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_QUESTION] = follower_emote_question_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_EXCLAMATION] = follower_emote_exclamation_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_SWEAT] = follower_emote_sweat_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_CRY] = follower_emote_cry_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_POISON] = follower_emote_poison_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_SILENCE] = follower_emote_silence_nsbtx,
    [POKEMON_FOLLOWER_EMOTE_SLEEP] = follower_emote_sleep_nsbtx,
};

static const OverworldAnimManagerFuncs sFollowerEmoteFuncs;

void *PokemonFollowerEmoteRenderer_New(FieldEffectManager *fieldEffMan)
{
    FollowerEmoteRenderer *renderer = FieldEffectManager_HeapAllocInit(fieldEffMan, sizeof(FollowerEmoteRenderer), FALSE, 0);

    renderer->fieldEffMan = fieldEffMan;
    ov5_021DF9E0(fieldEffMan, EMOTE_KEY_MODEL, follower_emote_nsbmd);
    ov5_021DFA14(fieldEffMan, EMOTE_KEY_SEQUENCE, follower_emote_sequence_bin);

    return renderer;
}

static void FollowerEmoteRenderer_LoadEmote(FollowerEmoteRenderer *renderer, int emote)
{
    if (renderer->visibleCount[emote]++ == 0) {
        ov5_021DFA3C(renderer->fieldEffMan, EMOTE_KEY_TEXTURE + emote, sEmoteTextures[emote], TRUE);
        ov5_021DF864(renderer->fieldEffMan, EMOTE_KEY_RES + emote, EMOTE_KEY_MODEL, EMOTE_KEY_SEQUENCE, EMOTE_KEY_TEXTURE + emote, FALSE, sEmoteAnims);
    }
}

static void FollowerEmoteRenderer_UnloadEmote(FollowerEmoteRenderer *renderer, int emote)
{
    GF_ASSERT(renderer->visibleCount[emote] > 0);

    if (--renderer->visibleCount[emote] == 0) {
        ov5_021DF9D4(renderer->fieldEffMan, EMOTE_KEY_RES + emote);
        ov5_021DFA7C(renderer->fieldEffMan, EMOTE_KEY_TEXTURE + emote);
    }
}

void PokemonFollowerEmoteRenderer_Free(void *context)
{
    FollowerEmoteRenderer *renderer = context;

    for (int emote = 0; emote < POKEMON_FOLLOWER_EMOTE_COUNT; emote++) {
        if (renderer->visibleCount[emote] != 0) {
            renderer->visibleCount[emote] = 1;
            FollowerEmoteRenderer_UnloadEmote(renderer, emote);
        }
    }

    ov5_021DFA30(renderer->fieldEffMan, EMOTE_KEY_SEQUENCE);
    ov5_021DFA08(renderer->fieldEffMan, EMOTE_KEY_MODEL);
    FieldEffectManager_HeapFree(renderer);
}

OverworldAnimManager *PokemonFollowerEmote_Show(MapObject *mapObj, enum PokemonFollowerEmote emote)
{
    FollowerEmoteParams params;
    VecFx32 pos;

    params.fieldEffMan = MapObject_GetFieldEffectManager(mapObj);
    params.renderer = FieldEffectManager_GetRendererContext(params.fieldEffMan, FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_EMOTE);
    params.mapObj = mapObj;
    params.emote = emote;

    MapObject_GetPosPtr(mapObj, &pos);
    return FieldEffectManager_InitAnimManager(params.fieldEffMan, &sFollowerEmoteFuncs, &pos, 0, &params, MapObject_GetTaskBasePriority(mapObj) + 1);
}

BOOL PokemonFollowerEmote_IsDone(OverworldAnimManager *animMan)
{
    FollowerEmote *emote = OverworldAnimManager_GetFuncsContext(animMan);
    return emote->done;
}

static int FollowerEmote_Init(OverworldAnimManager *animMan, void *context)
{
    FollowerEmote *emote = context;
    VecFx32 pos;

    emote->params = *(const FollowerEmoteParams *)OverworldAnimManager_GetUserData(animMan);
    emote->localID = MapObject_GetLocalID(emote->params.mapObj);
    emote->mapHeaderID = MapObject_GetMapHeaderID(emote->params.mapObj);
    emote->velocityY = FX32_CONST(6);

    FollowerEmoteRenderer_LoadEmote(emote->params.renderer, emote->params.emote);

    OverworldAnimManager_GetPosition(animMan, &pos);
    emote->billboard = ov5_021DF84C(emote->params.fieldEffMan, EMOTE_KEY_RES + emote->params.emote, &pos);
    Billboard_SetDrawFlag(emote->billboard, FALSE);
    Sound_PlayEffect(SEQ_SE_DP_DECIDE_sseq);

    return TRUE;
}

static void FollowerEmote_Delete(OverworldAnimManager *animMan, void *context)
{
    FollowerEmote *emote = context;

    Billboard_Delete(emote->billboard);
    FollowerEmoteRenderer_UnloadEmote(emote->params.renderer, emote->params.emote);
}

static void FollowerEmote_Update(OverworldAnimManager *animMan, void *context)
{
    FollowerEmote *emote = context;
    MapObject *mapObj = emote->params.mapObj;
    VecFx32 pos, spriteOffset;

    GF_ASSERT(sub_02062764(mapObj, emote->localID, emote->mapHeaderID));

    // sit above the follower's drawn position, riding along with its idle bob
    ov5_021ECDA0(mapObj, &pos);
    pos.y += EMOTE_HEIGHT;
    pos.z += EMOTE_DEPTH;
    MapObject_GetSpritePosOffset(mapObj, &spriteOffset);
    pos.y -= spriteOffset.y;

    Billboard_SetDrawFlag(emote->billboard, TRUE);

    switch (emote->state) {
    case EMOTE_STATE_POP_UP:
        emote->offsetY += emote->velocityY;

        if (emote->offsetY) {
            emote->velocityY -= FX32_CONST(2);
        } else {
            emote->velocityY = 0;
            emote->state++;
        }
        break;
    case EMOTE_STATE_ANIMATE:
        if (Billboard_AdvanceAnim(emote->billboard, FX32_ONE) == BILLBOARD_ANIM_STATUS_FINISHED) {
            emote->state++;
        }
        break;
    case EMOTE_STATE_HOLD:
        if (++emote->timer >= EMOTE_HOLD_TIME) {
            emote->state++;
            emote->done = TRUE;
        }
        break;
    }

    OverworldAnimManager_SetPosition(animMan, &pos);
}

static void FollowerEmote_Draw(OverworldAnimManager *animMan, void *context)
{
    FollowerEmote *emote = context;
    VecFx32 pos;

    OverworldAnimManager_GetPosition(animMan, &pos);
    pos.y += emote->offsetY;
    Billboard_SetPos(emote->billboard, &pos);
}

static const OverworldAnimManagerFuncs sFollowerEmoteFuncs = {
    sizeof(FollowerEmote),
    FollowerEmote_Init,
    FollowerEmote_Delete,
    FollowerEmote_Update,
    FollowerEmote_Draw
};
