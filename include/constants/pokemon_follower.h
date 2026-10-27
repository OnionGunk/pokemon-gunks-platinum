#ifndef POKEPLATINUM_CONSTANTS_POKEMON_FOLLOWER_H
#define POKEPLATINUM_CONSTANTS_POKEMON_FOLLOWER_H

// Pokemon follower (ported from Following Platinum)
#define POKEMON_FOLLOWER_LOCAL_ID 0xFD

// follower map object data: [0] species, [2] flags
#define POKEMON_FOLLOWER_DATA_SPECIES 0
#define POKEMON_FOLLOWER_DATA_FLAGS   2

#define POKEMON_FOLLOWER_FLAG_SHINY    0x80
// spawned on the player's arrival tile (often a door); stays hidden until it can step off it
#define POKEMON_FOLLOWER_FLAG_ARRIVING 0x20
// pops out of its Poke Ball when it next appears (after a cutscene)
#define POKEMON_FOLLOWER_FLAG_RELEASE  0x40
// hidden by a script (cutscenes); graphics refreshes keep it hidden
#define POKEMON_FOLLOWER_FLAG_SCRIPT_HIDDEN 0x02

#endif // POKEPLATINUM_CONSTANTS_POKEMON_FOLLOWER_H
