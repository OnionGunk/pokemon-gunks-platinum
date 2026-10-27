"""Decode zone_event.narc members (the binary that tools/jsoncnv/event.py writes) into plain records."""
import struct

BG = ('script', 'type', 'x', 'z', 'y', 'player_facing_dir')
OBJ = ('clone_id', 'graphics_id', 'movement_type', 'trainer_type', 'hidden_flag', 'script', 'initial_dir',
       'data0', 'data1', 'data2', 'movement_range_x', 'movement_range_z', 'x', 'z', 'y')
WARP = ('x', 'z', 'dest_header_id', 'dest_warp_id')
COORD = ('script', 'x', 'z', 'width', 'length', 'y', 'value', 'var')


def decode(data):
    pos = 0
    out = {}

    def count():
        nonlocal pos
        (n,) = struct.unpack_from('<I', data, pos)
        pos += 4
        return n

    bgs = []
    for _ in range(count()):
        script, kind, x, z, y, facing = struct.unpack_from('<HHiiiH', data, pos)
        bgs.append(dict(zip(BG, (script, kind, x, z, y, facing))))
        pos += 20
    out['bg_events'] = bgs

    objs = []
    for _ in range(count()):
        vals = struct.unpack_from('<14Hi', data, pos)
        rec = dict(zip(OBJ, vals))
        rec['y'] //= 0x10000
        objs.append(rec)
        pos += 32
    out['object_events'] = objs

    warps = []
    for _ in range(count()):
        warps.append(dict(zip(WARP, struct.unpack_from('<4H', data, pos))))
        pos += 12
    out['warp_events'] = warps

    coords = []
    for _ in range(count()):
        coords.append(dict(zip(COORD, struct.unpack_from('<8H', data, pos))))
        pos += 16
    out['coord_events'] = coords
    assert pos == len(data), (pos, len(data))
    return out
