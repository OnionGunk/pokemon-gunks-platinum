"""Add Following Platinum's movement actions 154-171: four quick 2-tile jumps and 14 follower emotes."""
import os

os.chdir('C:/projects/pokemon-platinum-dexnav/pkplat-dexnav')


def edit(p, old, new):
    s = open(p, encoding='utf-8').read()
    assert s.count(old) == 1, (p, old[:70], s.count(old))
    open(p, 'w', encoding='utf-8', newline='\n').write(s.replace(old, new))


DIRS = ['North', 'South', 'West', 'East']
EMOTES = ['Heart', 'Face1', 'Face2', 'Face3', 'Face4', 'Face5', 'Music', 'Question', 'Exclamation', 'Sweat', 'Cry',
          'Poison', 'Silence', 'Sleep']
EMOTE_CONSTS = ['HEART', 'FACE_1', 'FACE_2', 'FACE_3', 'FACE_4', 'FACE_5', 'MUSIC', 'QUESTION', 'EXCLAMATION', 'SWEAT',
                'CRY', 'POISON', 'SILENCE', 'SLEEP']

jump_names = [f'JumpFarFaster{d}' for d in DIRS]
jump_consts = [f'MOVEMENT_ACTION_JUMP_FAR_FASTER_{d.upper()}' for d in DIRS]
emote_names = [f'FollowerEmote{e}' for e in EMOTES]
emote_consts = [f'MOVEMENT_ACTION_FOLLOWER_EMOTE_{c}' for c in EMOTE_CONSTS]

# ids 154-171, in Following Platinum's order
edit('generated/movement_actions.txt', 'MOVEMENT_ACTION_153\nMAX_MOVEMENT_ACTION\n',
     'MOVEMENT_ACTION_153\n' + ''.join(c + '\n' for c in jump_consts + emote_consts) + 'MAX_MOVEMENT_ACTION\n')

# step functions
steps = '''
// Following Platinum: a quick hop that covers two tiles in four frames
'''
for d, name in zip(DIRS, jump_names):
    steps += f'''
static BOOL MovementAction_{name}_Step0(MapObject *mapObj)
{{
    MovementAction_InitJump(mapObj, DIR_{d.upper()}, FX32_CONST(8), 4, MAP_OBJ_UNK_A0_05, JUMP_HEIGHT_HIGH, 0x100);
    return TRUE;
}}
'''
steps += '''
// Following Platinum: emote bubbles over the Pokemon follower
static void MovementAction_InitFollowerEmote(MapObject *mapObj, enum PokemonFollowerEmote emote)
{
    EmoteMovementData *data = MapObject_InitMovementData(mapObj, sizeof(EmoteMovementData));

    data->unk_00 = emote;
    data->unk_04 = PokemonFollowerEmote_Show(mapObj, emote);

    MapObject_AdvanceMovementStep(mapObj);
}

static BOOL MovementAction_FollowerEmote_Step1(MapObject *mapObj)
{
    EmoteMovementData *data = MapObject_GetMovementData(mapObj);

    if (PokemonFollowerEmote_IsDone(data->unk_04)) {
        OverworldAnimManager_Finish(data->unk_04);
        MapObject_AdvanceMovementStep(mapObj);
        return TRUE;
    }

    return FALSE;
}
'''
for c, name in zip(EMOTE_CONSTS, emote_names):
    steps += f'''
static BOOL MovementAction_{name}_Step0(MapObject *mapObj)
{{
    MovementAction_InitFollowerEmote(mapObj, POKEMON_FOLLOWER_EMOTE_{c});
    return FALSE;
}}
'''

p = 'src/unk_020655F4.c'
anchor = 'static void MovementAction_InitWalkUneven(MapObject *mapObj, int dir, s16 duration, u16 param3)\n'
edit(p, anchor, steps.lstrip('\n') + '\n' + anchor)
edit(p, '#include "overlay005/ov5_021F5A10.h"\n', '#include "overlay005/ov5_021F5A10.h"\n#include "overlay005/pokemon_follower_emote.h"\n')

lists = ''
for name in jump_names:
    lists += f'''
BOOL (*const gMovementActionFuncs_{name}[])(MapObject *) = {{
    MovementAction_{name}_Step0,
    MovementAction_Jump_Step1,
    MovementAction_End,
}};
'''
for name in emote_names:
    lists += f'''
BOOL (*const gMovementActionFuncs_{name}[])(MapObject *) = {{
    MovementAction_{name}_Step0,
    MovementAction_FollowerEmote_Step1,
    MovementAction_End,
}};
'''
s = open(p, encoding='utf-8').read()
marker = 'BOOL (*const gMovementActionFuncs_153[])(MapObject *) = {'
start = s.index(marker)
end = s.index('};\n', start) + 3
s = s[:end] + lists + s[end:]
open(p, 'w', encoding='utf-8', newline='\n').write(s)

edit('include/unk_020655F4.h', 'extern BOOL (*const gMovementActionFuncs_153[])(MapObject *);\n',
     'extern BOOL (*const gMovementActionFuncs_153[])(MapObject *);\n'
     + ''.join(f'extern BOOL (*const gMovementActionFuncs_{n}[])(MapObject *);\n' for n in jump_names + emote_names))

edit('src/unk_020EDBAC.c', '    [MOVEMENT_ACTION_153] = gMovementActionFuncs_153,\n',
     '    [MOVEMENT_ACTION_153] = gMovementActionFuncs_153,\n'
     + ''.join(f'    [{c}] = gMovementActionFuncs_{n},\n' for c, n in zip(jump_consts + emote_consts, jump_names + emote_names)))
print('ok')
