"""Cutscene support for the Pokemon follower (Following Platinum's script commands 840 and 844-860):
flags, shared helpers, the Poke Ball renderer registration, the reveal-with-release path and the
script command handlers."""
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


# --- flags
edit('include/constants/pokemon_follower.h', '''#define POKEMON_FOLLOWER_FLAG_ARRIVING 0x20
''', '''#define POKEMON_FOLLOWER_FLAG_ARRIVING 0x20
// pops out of its Poke Ball when it next appears (after a cutscene)
#define POKEMON_FOLLOWER_FLAG_RELEASE  0x40
// hidden by a script (cutscenes); graphics refreshes keep it hidden
#define POKEMON_FOLLOWER_FLAG_SCRIPT_HIDDEN 0x02
''')

# --- shared helpers (resident)
edit('include/field_map_change.h', 'BOOL PokemonFollower_CanSpawn(int species, enum MapHeaderID mapHeaderID);',
     'BOOL PokemonFollower_CanSpawn(int species, enum MapHeaderID mapHeaderID);\n'
     'MapObject *PokemonFollower_GetMapObject(FieldSystem *fieldSystem);\n'
     'void PokemonFollower_SetFlag(MapObject *follower, int flag, BOOL set);\n'
     'void PokemonFollower_SetScriptHidden(MapObject *follower, BOOL hidden);')
s = open('src/field_map_change.c', encoding='utf-8').read()
anchor = '// Ported from Following Platinum\'s spawn routine'
assert s.count(anchor) == 1
s = s.replace(anchor, '''MapObject *PokemonFollower_GetMapObject(FieldSystem *fieldSystem)
{
    return MapObjMan_LocalMapObjByIndex(fieldSystem->mapObjMan, POKEMON_FOLLOWER_LOCAL_ID);
}

void PokemonFollower_SetFlag(MapObject *follower, int flag, BOOL set)
{
    int flags = MapObject_GetDataAt(follower, POKEMON_FOLLOWER_DATA_FLAGS);

    MapObject_SetDataAt(follower, set ? (flags | flag) : (flags & ~flag), POKEMON_FOLLOWER_DATA_FLAGS);
}

// Following Platinum's hide helper (fp_023c864c): hidden, status 19 and its own flag move together
void PokemonFollower_SetScriptHidden(MapObject *follower, BOOL hidden)
{
    MapObject_SetHidden(follower, hidden);
    sub_02062DB4(follower, hidden);
    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_SCRIPT_HIDDEN, hidden);
}

''' + anchor, 1)
open('src/field_map_change.c', 'w', encoding='utf-8', newline='\n').write(s)

# --- refresh keeps script-hidden followers hidden
edit('src/overlay005/ov5_021ECA70.c', '''                    || (MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_FLAGS) & POKEMON_FOLLOWER_FLAG_ARRIVING));''',
     '''                    || (MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_FLAGS) & (POKEMON_FOLLOWER_FLAG_ARRIVING | POKEMON_FOLLOWER_FLAG_SCRIPT_HIDDEN)));''')

# --- reveal: after a cutscene the follower pops out of its Poke Ball
edit('src/unk_02069BE0.c', '''    MapObject_SetDataAt(mapObj, flags & ~POKEMON_FOLLOWER_FLAG_ARRIVING, POKEMON_FOLLOWER_DATA_FLAGS);

    int species = MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_SPECIES);

    if (PokemonFollower_CanSpawn(species, fieldSystem->location->mapHeaderID)) {
        MapObject_SetHidden(mapObj, FALSE);
    }''', '''    MapObject_SetDataAt(mapObj, flags & ~(POKEMON_FOLLOWER_FLAG_ARRIVING | POKEMON_FOLLOWER_FLAG_RELEASE), POKEMON_FOLLOWER_DATA_FLAGS);

    int species = MapObject_GetDataAt(mapObj, POKEMON_FOLLOWER_DATA_SPECIES);

    if (!PokemonFollower_CanSpawn(species, fieldSystem->location->mapHeaderID)) {
        return;
    }

    if (flags & POKEMON_FOLLOWER_FLAG_RELEASE) {
        PokemonFollowerBall_Play(mapObj, POKE_BALL_ANIMATION_RELEASE, species);
    } else {
        MapObject_SetHidden(mapObj, FALSE);
    }''')
edit('src/unk_02069BE0.c', '#include "overlay005/ov5_021F3D00.h"\n', '#include "overlay005/ov5_021F3D00.h"\n#include "overlay005/pokemon_follower_ball.h"\n')

# --- renderer registration
edit('include/constants/field/field_effect_renderer.h', '    FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_EMOTE,\n',
     '    FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_EMOTE,\n    FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_BALL,\n')
p = 'src/overlay005/field_effect_renderer.c'
edit(p, '#include "overlay005/pokemon_follower_emote.h"\n', '#include "overlay005/pokemon_follower_ball.h"\n#include "overlay005/pokemon_follower_emote.h"\n')
edit(p, '    { FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_EMOTE, PokemonFollowerEmoteRenderer_New, PokemonFollowerEmoteRenderer_Free },\n',
     '    { FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_EMOTE, PokemonFollowerEmoteRenderer_New, PokemonFollowerEmoteRenderer_Free },\n'
     '    { FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_BALL, PokemonFollowerBallRenderer_New, PokemonFollowerBallRenderer_Free },\n')
edit(p, '''    FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_EMOTE,
    FIELD_EFFECT_RENDERER_INVALID
};''', '''    FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_EMOTE,
    FIELD_EFFECT_RENDERER_POKEMON_FOLLOWER_BALL,
    FIELD_EFFECT_RENDERER_INVALID
};''')
edit('src/meson.build', "    'overlay005/pokemon_follower_emote.c',\n", "    'overlay005/pokemon_follower_emote.c',\n    'overlay005/pokemon_follower_ball.c',\n")
edit('platinum.us/main.lsf.in', '\tObject src/overlay005/pokemon_follower_emote.c\n', '\tObject src/overlay005/pokemon_follower_emote.c\n\tObject src/overlay005/pokemon_follower_ball.c\n')
print('ok')
