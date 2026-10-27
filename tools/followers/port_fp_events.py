"""Port Following Platinum's edits to zone_event.narc (map events) into res/field/events/events_*.json.

FP changed 41 maps: NPC Pokemon use its follower sprites, and Route 201 and the Pokemon League's last
rooms gained Pokemon objects that scripts dress up with party sprites. Each edited JSON is converted
with the repo's own tools/jsoncnv/event.py and must equal FP's member byte for byte.

Usage: py -3 -I tools/followers/port_fp_events.py [--write]
"""
import json
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from zone_events import decode  # noqa: E402

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
PROJECT = os.path.abspath(os.path.join(REPO, '..'))
sys.path.insert(0, os.path.join(PROJECT, 'fp'))
sys.path.insert(0, os.path.join(REPO, 'build'))
from narc import narc_members  # noqa: E402
from generated import movement_types, object_events_gfx, trainer_types, vars_flags  # noqa: E402

EVENTS = os.path.join(REPO, 'res', 'field', 'events')
WORK = os.path.join(PROJECT, 'fp', 'events_tmp')
VANILLA = os.path.join(PROJECT, 'fp', 'vanilla', 'fs', 'fielddata', 'eventdata', 'zone_event.narc')
FP = os.path.join(PROJECT, 'fp', 'fp', 'fs', 'fielddata', 'eventdata', 'zone_event.narc')

GFX = {v.value: k for k, v in object_events_gfx.ObjectEventGfx.__members__.items()}
MOVEMENT = {v.value: k for k, v in movement_types.MovementType.__members__.items()}
TRAINER = {v.value: k for k, v in trainer_types.TrainerType.__members__.items()}
VARFLAG = {v.value: k for k, v in vars_flags.VarFlag.__members__.items()}

# names for the objects FP added, by map and local id
NEW_OBJECT_IDS = {
    'events_route_201': {13: 'LOCALID_RIVAL_STARTER', 14: 'LOCALID_PLAYER_STARTER'},
    'events_pokemon_league_champion_room': {i: f'LOCALID_PARTY_MON_{i}' for i in range(1, 7)},
    'events_pokemon_league_hallway_to_hall_of_fame': {i: f'LOCALID_PARTY_MON_{i - 1}' for i in range(2, 8)},
    'events_pokemon_league_hall_of_fame': {i: f'LOCALID_PARTY_MON_{i - 1}' for i in range(2, 8)},
}


def to_json_object(rec, local_id, name):
    obj = {
        'id': name,
        'graphics_id': GFX[rec['graphics_id']],
        'movement_type': MOVEMENT[rec['movement_type']],
        'trainer_type': TRAINER[rec['trainer_type']],
        'hidden_flag': VARFLAG[rec['hidden_flag']] if rec['hidden_flag'] else '0',
        'script': rec['script'],
        'initial_dir': rec['initial_dir'],
        'data': [d for d in (rec['data0'], rec['data1'], rec['data2'])],
        'movement_range_x': rec['movement_range_x'],
        'movement_range_z': rec['movement_range_z'],
        'x': rec['x'],
        'z': rec['z'],
        'y': rec['y'],
    }
    while obj['data'] and obj['data'][-1] == 0:
        obj['data'].pop()
    if rec['clone_id'] != local_id:
        obj['clone_id'] = rec['clone_id']
    return obj


def convert(path, out):
    env = dict(os.environ, PYTHONPATH=os.pathsep.join([os.path.join(REPO, 'tools', 'jsoncnv'), os.path.join(REPO, 'build')]))
    r = subprocess.run([sys.executable, os.path.join(REPO, 'tools', 'jsoncnv', 'event.py'), path, out],
                       capture_output=True, text=True, env=env)
    if r.returncode:
        raise RuntimeError(r.stderr.strip()[-500:])
    return open(out, 'rb').read()


def port(name, van, fp):
    path = os.path.join(EVENTS, name + '.json')
    data = json.load(open(path, encoding='utf-8'))
    os.makedirs(WORK, exist_ok=True)
    if convert(path, os.path.join(WORK, name + '_ours.bin')) != van:
        raise RuntimeError('our JSON does not convert to the vanilla member')
    a, b = decode(van), decode(fp)
    notes = []
    for section in ('bg_events', 'warp_events', 'coord_events'):
        if a[section] != b[section]:
            raise RuntimeError(f'unhandled {section} change')
    objs = data['object_events']
    old, new = a['object_events'], b['object_events']
    if len(new) < len(old):
        raise RuntimeError('FP removed objects')
    for i, (x, y) in enumerate(zip(old, new)):
        changed = [k for k in x if x[k] != y[k]]
        if changed != ['graphics_id'] and changed:
            raise RuntimeError(f'object {i}: unhandled change {changed}')
        if changed:
            objs[i]['graphics_id'] = GFX[y['graphics_id']]
            notes.append(f'{objs[i].get("id", i)}: {GFX[x["graphics_id"]]} -> {GFX[y["graphics_id"]]}')
    for i in range(len(old), len(new)):
        obj_name = NEW_OBJECT_IDS.get(name, {}).get(i, f'LOCALID_FP_OBJECT_{i}')
        objs.append(to_json_object(new[i], i, obj_name))
        notes.append(f'added {obj_name} ({GFX[new[i]["graphics_id"]]} at {new[i]["x"]},{new[i]["z"]})')
    out_path = os.path.join(WORK, name + '.json')
    original = open(path, encoding='utf-8').read()
    ending = original[original.rstrip().rfind('}') + 1:]  # keep the file's own trailing newlines
    with open(out_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(json.dumps(data, ensure_ascii=False, indent=4) + ending)
    if convert(out_path, os.path.join(WORK, name + '_ported.bin')) != fp:
        raise RuntimeError('ported JSON does not convert to FP\'s member')
    return out_path, notes


def main():
    write = '--write' in sys.argv
    order = open(os.path.join(EVENTS, 'zone_event.order'), encoding='utf-8').read().split()
    van = narc_members(open(VANILLA, 'rb').read())
    fp = narc_members(open(FP, 'rb').read())
    results = []
    for i, name in enumerate(order):
        if van[i] == fp[i]:
            continue
        try:
            out_path, notes = port(name, van[i], fp[i])
        except RuntimeError as e:
            results.append((i, name, 'failed', str(e)))
            continue
        if write:
            # keep the original file's formatting: only rewrite if the JSON text differs
            src = os.path.join(EVENTS, name + '.json')
            open(src, 'w', encoding='utf-8', newline='\n').write(open(out_path, encoding='utf-8').read())
        results.append((i, name, 'ok', '; '.join(notes)))
    for i, name, status, note in results:
        print(f'{i:4d} {name:48s} {status:6s} {note}')
    return results


if __name__ == '__main__':
    main()
