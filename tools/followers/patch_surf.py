"""Following Platinum's surf behaviour: the follower returns to its Poke Ball when the player starts
surfing, stays hidden on the water, and pops back out after the player lands (FP hooks the surf mount
and dismount tasks; dismount hook = fp_023cc63c)."""
import os

os.chdir('C:/Projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


RETURN_TO_PLAYER = '''static void PokemonFollower_ReturnToPlayer(FieldSystem *fieldSystem, BOOL release)
{
    MapObject *follower = PokemonFollower_GetMapObject(fieldSystem);
    MapObject *player = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);

    if (follower == NULL) {
        return;
    }

    sub_020656AC(follower);
    MapObject_SetPosDirFromCoords(follower, MapObject_GetX(player), MapObject_GetY(player), MapObject_GetZ(player), MapObject_GetFacingDir(player));
    MapObject_SetXInitial(follower, MapObject_GetX(player));
    MapObject_SetZInitial(follower, MapObject_GetZ(player));
    PokemonFollower_SetScriptHidden(follower, TRUE);
    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_ARRIVING, TRUE);
    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_RELEASE, release);
    MapObject_SwitchMovementType(follower, MOVEMENT_TYPE_POKEMON_FOLLOWER);
}
'''

# move ReturnToPlayer next to the other follower helpers so the surf code can use it
s = open('src/scrcmd.c', encoding='utf-8').read()
start = s.index('// After a cutscene the follower goes back under the player')
end = s.index(RETURN_TO_PLAYER) + len(RETURN_TO_PLAYER)
s = s[:start] + s[end:].lstrip('\n')
s = s.replace('// Following Platinum (command 850)\n', '// Following Platinum (command 850): after a cutscene the follower goes back under the player and\n// pops out of its Poke Ball once the player steps away\n', 1)
open('src/scrcmd.c', 'w', encoding='utf-8', newline='\n').write(s)

p = 'src/field_map_change.c'
s = open(p, encoding='utf-8').read()
anchor = '// Ported from Following Platinum\'s spawn routine'
assert s.count(anchor) == 1
s = s.replace(anchor, '''// The follower goes back under the player and follows again, appearing once the player steps away
// (popping out of its Poke Ball when release is set): after cutscenes and after surfing.
''' + RETURN_TO_PLAYER.replace('static void', 'void') + '''
// Starting to surf: the follower returns to its Poke Ball and waits there until the player lands.
void PokemonFollower_RecallForSurf(FieldSystem *fieldSystem)
{
    MapObject *follower = PokemonFollower_GetMapObject(fieldSystem);

    if (follower == NULL) {
        return;
    }

    sub_020656AC(follower);

    if (!MapObject_IsHidden(follower)) {
        PokemonFollowerBall_Play(follower, POKE_BALL_ANIMATION_RECALL, MapObject_GetDataAt(follower, POKEMON_FOLLOWER_DATA_SPECIES));
    }

    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_ARRIVING | POKEMON_FOLLOWER_FLAG_RELEASE, FALSE);
    PokemonFollower_SetFlag(follower, POKEMON_FOLLOWER_FLAG_SCRIPT_HIDDEN, TRUE);
}

''' + anchor, 1)
if '#include "overlay005/pokemon_follower_ball.h"' not in s:
    i = s.index('#include "overlay005/')
    s = s[:i] + '#include "overlay005/pokemon_follower_ball.h"\n' + s[i:]
open(p, 'w', encoding='utf-8', newline='\n').write(s)

edit('include/field_map_change.h', 'void PokemonFollower_SetScriptHidden(MapObject *follower, BOOL hidden);',
     'void PokemonFollower_SetScriptHidden(MapObject *follower, BOOL hidden);\n'
     'void PokemonFollower_ReturnToPlayer(FieldSystem *fieldSystem, BOOL release);\n'
     'void PokemonFollower_RecallForSurf(FieldSystem *fieldSystem);')

# surf start and dismount
p = 'src/overlay005/ov5_021DFB54.c'
edit(p, '''    switch (taskEnv->state) {
    case 0:
        if (PlayerAvatar_MapDistortionState(taskEnv->playerAvatar) == AVATAR_DISTORTION_STATE_NONE) {
            FieldBGM_SetOverride(taskEnv->fieldSystem, SEQ_NONE);
            FieldBGM_TryFadeOut(taskEnv->fieldSystem, SEQ_NAMINORI_sseq, 1);
        }
''', '''    switch (taskEnv->state) {
    case 0:
        if (PlayerAvatar_MapDistortionState(taskEnv->playerAvatar) == AVATAR_DISTORTION_STATE_NONE) {
            FieldBGM_SetOverride(taskEnv->fieldSystem, SEQ_NONE);
            FieldBGM_TryFadeOut(taskEnv->fieldSystem, SEQ_NAMINORI_sseq, 1);
        }

        PokemonFollower_RecallForSurf(taskEnv->fieldSystem);
''')
edit(p, '''        PlayerAvatar_SetSurfMountAnimManager(v0->playerAvatar, NULL);
        PlayerAvatar_SetPlayerState(v0->playerAvatar, 0x0);
        FieldBGM_TryFadeOut(v0->fieldSystem, FieldBGM_GetForMapHeader(v0->fieldSystem, v0->fieldSystem->location->mapHeaderID), 1);
        MonRideTaskEnv_Free(v0);''', '''        PlayerAvatar_SetSurfMountAnimManager(v0->playerAvatar, NULL);
        PlayerAvatar_SetPlayerState(v0->playerAvatar, 0x0);
        FieldBGM_TryFadeOut(v0->fieldSystem, FieldBGM_GetForMapHeader(v0->fieldSystem, v0->fieldSystem->location->mapHeaderID), 1);
        PokemonFollower_ReturnToPlayer(v0->fieldSystem, TRUE);
        MonRideTaskEnv_Free(v0);''')
if '#include "field_map_change.h"' not in open(p, encoding='utf-8').read():
    edit(p, '#include "field_bgm.h"\n', '#include "field_bgm.h"\n#include "field_map_change.h"\n')

# never appear on water, and a plain reveal clears the script-hidden state too
p = 'src/unk_02069BE0.c'
edit(p, '''    if (flags & POKEMON_FOLLOWER_FLAG_RELEASE) {
        PokemonFollowerBall_Play(mapObj, POKE_BALL_ANIMATION_RELEASE, species);
    } else {
        MapObject_SetHidden(mapObj, FALSE);
    }''', '''    if (flags & POKEMON_FOLLOWER_FLAG_RELEASE) {
        PokemonFollowerBall_Play(mapObj, POKE_BALL_ANIMATION_RELEASE, species);
    } else {
        PokemonFollower_SetScriptHidden(mapObj, FALSE);
    }''')
edit(p, '''    if ((flags & POKEMON_FOLLOWER_FLAG_ARRIVING) == 0) {
        return;
    }

    FieldSystem *fieldSystem = MapObject_FieldSystem(mapObj);''', '''    if ((flags & POKEMON_FOLLOWER_FLAG_ARRIVING) == 0) {
        return;
    }

    FieldSystem *fieldSystem = MapObject_FieldSystem(mapObj);

    // while surfing the follower stays in its Poke Ball
    if (PlayerAvatar_GetPlayerState(FieldSystem_GetPlayerAvatar(fieldSystem)) == PLAYER_AVATAR_SURFING) {
        return;
    }''')
print('ok')
