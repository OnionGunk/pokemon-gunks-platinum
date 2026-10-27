#include "overlay005/ov5_021ECA70.h"

#include <nitro.h>
#include <string.h>

#include "struct_decls/map_object.h"

#include "overlay005/ov5_021ECE40.h"
#include "overlay005/struct_ov5_021ECB8C_decl.h"
#include "overlay005/struct_ov5_021ED01C.h"

#include "billboard.h"
#include "map_object.h"
#include "pokemon.h"
#include "fx_util.h"
#include "constants/map_object.h"
#include "field/field_system.h"
#include "field_map_change.h"

typedef struct UnkStruct_ov5_021ECB8C_t {
    Billboard *unk_00;
    UnkStruct_ov5_021ED01C unk_04;
};

static void ov5_021ECB90(MapObject *param0);
static void ov5_021ECBA4(MapObject *param0);
static void ov5_021ECBCC(MapObject *param0, Billboard *param1);
static void ov5_021ECC0C(MapObject *param0);

void ov5_021ECA70(MapObject *param0)
{
    VecFx32 v0;

    ov5_021ECB90(param0);

    v0.x = 0;
    v0.y = (FX32_CONST(0));
    v0.z = 0;

    MapObject_SetSpritePosOffset(param0, &v0);
}

void ov5_021ECA90(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062AF0(param0);
    Billboard *v1 = v0->unk_00;

    if (ov5_021EDD94(param0) == 1) {
        return;
    }

    if (v1 == NULL) {
        return;
    }

    ov5_021ECBA4(param0);
    Billboard_AdvanceAnim(v1, FX32_ONE);
}

void ov5_021ECABC(MapObject *param0)
{
    ov5_021ECC0C(param0);
}

void ov5_021ECAC4(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062AF0(param0);

    if (v0->unk_00 != NULL) {
        ov5_021ED01C(v0->unk_00, &v0->unk_04);
    }

    ov5_021ECFA4(param0, &v0->unk_00);
    MapObject_SetStatusFlagOn(param0, MAP_OBJ_STATUS_21);
}

void ov5_021ECAF0(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062AF0(param0);

    if (ov5_021EDD94(param0) == 1) {
        return;
    }

    if (v0->unk_00 == NULL) {
        ov5_021ECF04(param0, &v0->unk_00);
    }

    if (v0->unk_00 != NULL) {
        ov5_021ED03C(v0->unk_00, &v0->unk_04);
        ov5_021EDEB4(param0, v0->unk_00);
        MapObject_SetStatusFlagOff(param0, MAP_OBJ_STATUS_21);
    }
}

void ov5_021ECB34(MapObject *param0)
{
    VecFx32 v0;

    ov5_021ECB90(param0);

    v0.x = 0;
    v0.y = (FX32_CONST(24));
    v0.z = 0;

    MapObject_SetSpritePosOffset(param0, &v0);
}

void ov5_021ECB58(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062AF0(param0);
    Billboard *v1 = v0->unk_00;

    if (ov5_021EDD94(param0) == 1) {
        return;
    }

    if (v1 == NULL) {
        return;
    }

    ov5_021ECBA4(param0);
}

void ov5_021ECB78(MapObject *param0)
{
    ov5_021ECC0C(param0);
}

Billboard *ov5_021ECB80(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062AF0(param0);
    return ov5_021ECB8C(v0);
}

Billboard *ov5_021ECB8C(UnkStruct_ov5_021ECB8C *param0)
{
    return param0->unk_00;
}

static void ov5_021ECB90(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062ACC(param0, (sizeof(UnkStruct_ov5_021ECB8C)));

    ov5_021ECF04(param0, &v0->unk_00);
}

static void ov5_021ECBA4(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062AF0(param0);
    Billboard *v1 = v0->unk_00;

    ov5_021EDEB4(param0, v1);
    Billboard_AdvanceAnim(v1, (FX32_ONE));
    ov5_021ECBCC(param0, v1);
}

static void ov5_021ECBCC(MapObject *param0, Billboard *param1)
{
    int v0 = 1;

    if (MapObject_CheckStatusFlag(param0, MAP_OBJ_STATUS_HIDE) == 1) {
        v0 = 0;
    }

    if (MapObject_CheckStatusFlag(param0, MAP_OBJ_STATUS_12) == 1) {
        if (!MapObject_CheckStatusFlag(param0, MAP_OBJ_STATUS_13)) {
            v0 = 0;
        }
    }

    Billboard_SetDrawFlag(param1, v0);
}

static void ov5_021ECC0C(MapObject *param0)
{
    UnkStruct_ov5_021ECB8C *v0 = sub_02062AF0(param0);
    ov5_021ECFA4(param0, &v0->unk_00);
}

// ---------------------------------------------------------------------------------------------
// Pokemon follower renderer, ported from Following Platinum (Mikelan98 et al.).
// Differences from the vanilla billboard renderer:
//  - the animation is chosen from the facing direction (follower sprites have 4 direction loops),
//  - it always animates, so the Pokemon "walks in place" while idle,
//  - a 2 px bob every half of a 10-frame cycle,
//  - a per-sprite push-back away from the player so big Pokemon don't overlap them.
#include "overlay005/followers/offsets.h"

#define FOLLOWER_FIRST_GFX    276
#define FOLLOWER_SHINY_DATA   POKEMON_FOLLOWER_DATA_FLAGS
#define FOLLOWER_SHINY_FLAG   POKEMON_FOLLOWER_FLAG_SHINY

// Every follower texture carries two palettes, normal then shiny. A shiny follower points its
// frame sequence's palette indexes at palette 1 for every frame (Following Platinum's approach).
static const u8 sFollowerShinyPlttIdx[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

static void PokemonFollowerRenderer_ApplyShiny(MapObject *mapObj, Billboard *billboard)
{
    if ((MapObject_GetDataAt(mapObj, FOLLOWER_SHINY_DATA) & FOLLOWER_SHINY_FLAG)
        && billboard->gfxSequence.seqCount <= NELEMS(sFollowerShinyPlttIdx)) {
        billboard->gfxSequence.plttIdx = sFollowerShinyPlttIdx;
    }
}

static const u8 *PokemonFollowerRenderer_GetSpriteOffsets(MapObject *mapObj)
{
    int index = MapObject_GetGraphicsID(mapObj) - FOLLOWER_FIRST_GFX;

    if (index < 0 || index >= NELEMS(sFollowerSpriteOffsets)) {
        return NULL;
    }

    return sFollowerSpriteOffsets[index];
}

BOOL PokemonFollowerRenderer_IsLargeSprite(MapObject *mapObj)
{
    const u8 *offsets = PokemonFollowerRenderer_GetSpriteOffsets(mapObj);

    return offsets != NULL && offsets[3] == 2;
}

BOOL PokemonFollowerRenderer_IsFollowerSprite(MapObject *mapObj)
{
    return PokemonFollowerRenderer_GetSpriteOffsets(mapObj) != NULL;
}

void PokemonFollowerRenderer_ApplyOffset(MapObject *mapObj, int dir, VecFx32 *offset)
{
    const u8 *push = PokemonFollowerRenderer_GetSpriteOffsets(mapObj);

    if (push == NULL) {
        return;
    }

    switch (dir) {
    case DIR_NORTH:
        offset->z += push[2] * FX32_ONE;
        break;
    case DIR_SOUTH:
        offset->z -= push[1] * FX32_ONE;
        break;
    case DIR_WEST:
        offset->x += push[0] * FX32_ONE;
        break;
    case DIR_EAST:
        offset->x -= push[0] * FX32_ONE;
        break;
    }
}

void PokemonFollowerRenderer_Update(MapObject *mapObj)
{
    UnkStruct_ov5_021ECB8C *data = sub_02062AF0(mapObj);
    Billboard *billboard = data->unk_00;

    if (ov5_021EDD94(mapObj) == 1 || billboard == NULL) {
        return;
    }

    PokemonFollowerRenderer_ApplyShiny(mapObj, billboard);

    int dir = MapObject_GetFacingDir(mapObj);

    Billboard_SetAnimNum(billboard, ov5_021EDF18(dir));
    Billboard_AdvanceAnim(billboard, FX32_ONE);

    VecFx32 offset;
    int frame = Billboard_GetFrameNum(billboard) / FX32_ONE;

    MapObject_GetSpriteJumpOffset(mapObj, &offset);
    offset.y = (frame % 10 < 5) ? -FX32_CONST(2) : -0x100;
    PokemonFollowerRenderer_ApplyOffset(mapObj, dir, &offset);
    MapObject_SetSpritePosOffset(mapObj, &offset);

    ov5_021EDEB4(mapObj, billboard);
    ov5_021ECBCC(mapObj, billboard);
}

// Runs when the object's graphics are (re)loaded: picks up a new lead Pokemon, then does the
// vanilla billboard refresh.
void PokemonFollowerRenderer_Refresh(MapObject *mapObj)
{
    if (MapObject_GetLocalID(mapObj) == POKEMON_FOLLOWER_LOCAL_ID) {
        FieldSystem *fieldSystem = MapObject_FieldSystem(mapObj);
        Pokemon *lead = PokemonFollower_GetLead(fieldSystem->saveData);

        if (lead != NULL) {
            int species = Pokemon_GetValue(lead, MON_DATA_SPECIES, NULL);

            MapObject_SetHidden(mapObj, !PokemonFollower_CanSpawn(species, fieldSystem->location->mapHeaderID)
                    || (MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_FLAGS) & (POKEMON_FOLLOWER_FLAG_ARRIVING | POKEMON_FOLLOWER_FLAG_SCRIPT_HIDDEN)));

            int gfx = PokemonFollower_GraphicsID(lead);

            if (gfx >= 0) {
                MapObject_SetGraphicsID(mapObj, gfx);
                MapObject_SetDataAt(mapObj, Pokemon_GetValue(lead, MON_DATA_SPECIES, NULL), 0);
            }

            int flags = MapObject_GetDataAt(mapObj, FOLLOWER_SHINY_DATA) & ~FOLLOWER_SHINY_FLAG;

            MapObject_SetDataAt(mapObj, Pokemon_IsShiny(lead) ? flags | FOLLOWER_SHINY_FLAG : flags, FOLLOWER_SHINY_DATA);
        }
    }

    ov5_021ECAF0(mapObj);
}
