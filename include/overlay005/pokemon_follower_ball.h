#ifndef POKEPLATINUM_OV5_POKEMON_FOLLOWER_BALL_H
#define POKEPLATINUM_OV5_POKEMON_FOLLOWER_BALL_H

#include "struct_decls/map_object.h"

#include "overlay005/field_effect_manager.h"

#include "overworld_anim_manager.h"

// A Pokemon popping out of (or returning to) its Poke Ball in the overworld, ported from Following
// Platinum (fp_023c9248 and its renderer).
enum PokeBallAnimationMode {
    POKE_BALL_ANIMATION_RELEASE = 0,
    POKE_BALL_ANIMATION_RECALL,
};

void *PokemonFollowerBallRenderer_New(FieldEffectManager *fieldEffMan);
void PokemonFollowerBallRenderer_Free(void *context);
OverworldAnimManager *PokemonFollowerBall_Play(MapObject *mapObj, enum PokeBallAnimationMode mode, u16 species);

#endif // POKEPLATINUM_OV5_POKEMON_FOLLOWER_BALL_H
