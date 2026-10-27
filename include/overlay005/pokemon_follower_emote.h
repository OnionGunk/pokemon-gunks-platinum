#ifndef POKEPLATINUM_OV5_POKEMON_FOLLOWER_EMOTE_H
#define POKEPLATINUM_OV5_POKEMON_FOLLOWER_EMOTE_H

#include "struct_decls/map_object.h"

#include "overlay005/field_effect_manager.h"

#include "overworld_anim_manager.h"

// Emote bubbles shown over the Pokemon follower, ported from Following Platinum.
enum PokemonFollowerEmote {
    POKEMON_FOLLOWER_EMOTE_HEART = 0,
    POKEMON_FOLLOWER_EMOTE_FACE_1,
    POKEMON_FOLLOWER_EMOTE_FACE_2,
    POKEMON_FOLLOWER_EMOTE_FACE_3,
    POKEMON_FOLLOWER_EMOTE_FACE_4,
    POKEMON_FOLLOWER_EMOTE_FACE_5,
    POKEMON_FOLLOWER_EMOTE_MUSIC,
    POKEMON_FOLLOWER_EMOTE_QUESTION,
    POKEMON_FOLLOWER_EMOTE_EXCLAMATION,
    POKEMON_FOLLOWER_EMOTE_SWEAT,
    POKEMON_FOLLOWER_EMOTE_CRY,
    POKEMON_FOLLOWER_EMOTE_POISON,
    POKEMON_FOLLOWER_EMOTE_SILENCE,
    POKEMON_FOLLOWER_EMOTE_SLEEP,
    POKEMON_FOLLOWER_EMOTE_COUNT
};

void *PokemonFollowerEmoteRenderer_New(FieldEffectManager *fieldEffMan);
void PokemonFollowerEmoteRenderer_Free(void *context);
OverworldAnimManager *PokemonFollowerEmote_Show(MapObject *mapObj, enum PokemonFollowerEmote emote);
BOOL PokemonFollowerEmote_IsDone(OverworldAnimManager *animMan);

#endif // POKEPLATINUM_OV5_POKEMON_FOLLOWER_EMOTE_H
