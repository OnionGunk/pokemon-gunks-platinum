"""Disassemble Platinum field scripts (scr_seq.narc members) into readable text using the decomp's macro names.

Usage:
  py -3 -I script_disasm.py <scr_seq.narc> <member index or script name>   print one member
  py -3 -I script_disasm.py <scr_seq.narc> all <out dir>                   write every member to <out dir>/<name>.txt
  add --fp to decode Following Platinum's extra commands (ids 840, 844-860)

Code is decoded by recursive descent from the script entries (following GoTo/Call/branch targets), so inline
movement data and text are never mistaken for code. Labels are numbered in discovery order, which keeps output
stable when code is inserted earlier in a file (useful for diffing two ROMs). Commands with ids past the vanilla
table are printed as Unknown_<id> with the rest of their bytes up to the next recognizable command.
"""
import os
import re
import struct
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
VANILLA_CMD_COUNT = None  # filled from scrcmd.h minus the commands this repo appended (see below)
OUR_EXTRA_CMDS = ('SCRCMD_OPENDEXNAV', 'SCRCMD_DEXNAVSTARTSEARCH', 'SCRCMD_DEXNAVSTARTBATTLE', 'SCRCMD_DEXNAVPRINTMESSAGE')


def read(path):
    return open(os.path.join(ROOT, path), encoding='utf-8').read()


def narc_members(data):
    pos = struct.unpack_from('<H', data, 0xC)[0]
    sections = {}
    for _ in range(struct.unpack_from('<H', data, 0xE)[0]):
        magic, size = data[pos:pos + 4], struct.unpack_from('<I', data, pos + 4)[0]
        sections[magic] = pos
        if size == 0:
            break
        pos += size
    btaf, gmif = sections[b'BTAF'], sections[b'GMIF']
    count = struct.unpack_from('<H', data, btaf + 8)[0]
    out = []
    for i in range(count):
        s, e = struct.unpack_from('<II', data, btaf + 12 + i * 8)
        out.append(data[gmif + 8 + s:gmif + 8 + e])
    return out


# --- command table -------------------------------------------------------------------------------------------
CMD_NAMES = re.findall(r'^ScriptCommand\((\w+)', read('include/data/scripts/scrcmd.h'), re.M)
CMD_ID = {n: i for i, n in enumerate(CMD_NAMES)}
VANILLA_CMD_COUNT = len([n for n in CMD_NAMES if n not in OUR_EXTRA_CMDS])

# per command: (macro name, [param kinds]); kinds: b=byte h=short w=long off=relative code offset mov=movement offset
CMD_LAYOUT = {}
for m in re.finditer(r'\.macro\s+(\w+)([^\n]*)\n(.*?)\.endm', read('asm/macros/scrcmd.inc'), re.S):
    name, body = m.group(1), m.group(3)
    lines = [l.strip() for l in body.split('\n') if l.strip() and not l.strip().startswith('//')]
    if not lines or not re.match(r'\.short (SCRCMD_|ScrCmd_)', lines[0]):
        continue
    cmd = lines[0].split()[1]
    if cmd in CMD_LAYOUT:
        continue
    params = []
    depth = 0
    for l in lines[1:]:
        if l.startswith('.if'):
            depth += 1
            continue
        if l.startswith('.endif') or l.startswith('.else'):
            depth = depth - 1 if l.startswith('.endif') else depth
            continue
        if depth:
            continue  # conditional params are handled in VARIABLE below
        directive = l.split()[0]
        if directive == '.byte':
            params.append('b')
        elif directive == '.short':
            params += ['h'] * (l.count(',') + 1)
        elif directive in ('.long', '.word'):
            params.append(('mov' if 'movement' in l.lower() else 'off') if '-.-4' in l.replace(' ', '') else 'w')
    CMD_LAYOUT[cmd] = (name, params)


def variable_params(cmd, data, pos):
    """Extra params for commands whose length depends on an earlier argument (the .if blocks in scrcmd.inc)."""
    if cmd in ('SCRCMD_DOSTRENGTHFUNC', 'SCRCMD_DOFLASHFUNC', 'SCRCMD_DODEFOGFUNC'):
        return ['h'] if data[pos - 1] == 2 else []
    if cmd == 'SCRCMD_DOGROUPCONNECTIONACTION':
        a = struct.unpack_from('<H', data, pos - 2)[0]
        return ['h', 'h'] if a in (0, 1, 2, 3) else ['h'] if a in (4, 5) else []
    if cmd == 'SCRCMD_CALLTVBROADCAST':
        a = struct.unpack_from('<H', data, pos - 2)[0]
        return ['h'] if a in (0, 6) else ['h'] * 3 if a in (1, 3, 5) else ['h', 'h'] if a == 4 else []
    if cmd == 'SCRCMD_MYSTERYGIFTGIVE':
        a = struct.unpack_from('<H', data, pos - 2)[0]
        return ['h'] if a in (1, 2, 3) else ['h', 'h'] if a in (5, 6) else []
    return []


# Following Platinum's added commands (its table lives in the code blob at 0x023D2194, count 861).
# Names are descriptive guesses from the handlers; params: bytes read after the command id.
FP_COMMANDS = {
    840: ('FP_SoundParam', ['h']),                       # 0x023CD9A5: SoundSystem_GetParam(h)
    844: ('FP_FollowerResync', []),                      # 0x023CC65: end anim, re-place follower, set graphics, waits
    845: ('FP_WaitScriptMember10', []),                  # 0x023C8D01: waits on script member 10
    846: ('FP_FollowerPauseMovement', ['h']),            # 0x023C8D35: h=1 pause, 0 unpause
    847: ('FP_WaitFollower', []),                        # 0x023C8D65: pauses script while follower busy
    848: ('FP_FollowerSetMovementType', ['h']),          # 0x023C8DB9: MapObject_SwitchMovementType(follower, h)
    849: ('FP_FollowerPlaceNearPlayer', ['b', 'b']),     # 0x023C8DE1: position follower from player pos + offsets
    850: ('FP_FollowerResumeFollowing', []),             # 0x023C8E69: back to movement type 68 (follow)
    851: ('FP_FollowerResumeFollowing2', []),            # 0x023C8EAD
    852: ('FP_FollowerRespawnBehindPlayer', []),         # 0x023C8EDD: movement 68, regraphic, unhide
    853: ('FP_FollowerHide', []),                        # 0x023C8F21: SetHidden(follower, TRUE)
    854: ('FP_FollowerShow', []),                        # 0x023C8F41: SetHidden(follower, FALSE)
    855: ('FP_SetObjectToPartyMonGfx', ['h', 'h', 'h']),  # 0x023C8F61: vars (localID, ?, ?)
    856: ('FP_GetFollowerLeadInfo', ['h']),              # 0x023CBE89: dest var <- lead party info
    857: ('FP_FollowerHappyHop', []),                    # 0x023C8FC1: fixed movement (Delay8 x3, Delay4, FaceSouth, act166, ...)
    858: ('FP_GetPartyMonFollowerGfx', ['h', 'h']),      # 0x023C8FF5: in var party slot, out var graphics id
    859: ('FP_CheckFollowerActive', ['h']),              # 0x023C9099: dest var <- follower active
    860: ('FP_GetFollowerDirFromPlayer', ['h']),         # 0x023C90BD: dest var <- follower side relative to player
}
FP_MODE = '--fp' in sys.argv

TERMINATORS = {'SCRCMD_END', 'SCRCMD_RETURN', 'SCRCMD_GOTO', 'SCRCMD_RETURNCOMMONSCRIPT'}

# --- movements -----------------------------------------------------------------------------------------------
MOVE_VALUE = {}
for name, val in re.findall(r'#define (MOVEMENT_ACTION_\w+)\s+(\d+)', read('build/generated/movement_actions.h')):
    MOVE_VALUE[int(val)] = name
MOVE_MACRO = {}
for name, act in re.findall(r'\.macro\s+(\w+)[^\n]*\n\s*\.short\s+(MOVEMENT_ACTION_\w+)', read('asm/macros/movement.inc')):
    MOVE_MACRO.setdefault(act, name)


def movement_name(value):
    act = MOVE_VALUE.get(value)
    if act is None:
        return f'FPMovementAction_{value}'
    return MOVE_MACRO.get(act, act)


def decode_movement(data, pos):
    out = []
    while pos + 4 <= len(data):
        act, count = struct.unpack_from('<HH', data, pos)
        pos += 4
        if act == 254:
            out.append('EndMovement')
            break
        out.append(f'{movement_name(act)} {count}' if count != 1 else movement_name(act))
        if len(out) > 64:
            out.append('...')
            break
    return out


# --- disassembly ---------------------------------------------------------------------------------------------
def entries(data, init_script):
    pos, out = 0, []
    if init_script:
        while pos < len(data) and data[pos] != 0:
            kind = data[pos]
            if kind == 1:  # INIT_SCRIPT_ON_FRAME_TABLE: .long label-.-4 after the type byte
                off = struct.unpack_from('<i', data, pos + 1)[0]
                out.append(('frame_table', pos + 1 + 4 + off))
                pos += 5
            else:
                sid = struct.unpack_from('<H', data, pos + 1)[0]
                out.append((f'init_type{kind}', sid))
                pos += 5
        return out
    while pos + 2 <= len(data) and struct.unpack_from('<H', data, pos)[0] != 0xFD13:
        off = struct.unpack_from('<i', data, pos)[0]
        out.append(('script', pos + 4 + off))
        pos += 4
    return out


def disassemble(data, init_script=False):
    labels, order = {}, []
    lines = {}
    movements = {}
    unknown_ids = []

    def label(target, kind='L'):
        if target not in labels:
            labels[target] = f'{kind}{len(labels)}'
            order.append(target)
        return labels[target]

    ents = entries(data, init_script)
    work = []
    header = []
    for kind, val in ents:
        if kind == 'script':
            header.append(f'ScriptEntry {label(val, "Script_")}')
            work.append(val)
        elif kind == 'frame_table':
            header.append(f'InitScriptEntry_OnFrameTable table@{val:#x}')
            pos = val
            while pos + 2 <= len(data) and struct.unpack_from('<H', data, pos)[0] != 0:
                v1, v2, sid = struct.unpack_from('<HHH', data, pos)
                header.append(f'    InitScriptGoToIfEqual {v1:#x}, {v2}, {sid}')
                pos += 6
        else:
            header.append(f'InitScriptEntry {kind} script {val}')

    seen = set()
    while work:
        pos = work.pop()
        while 0 <= pos < len(data) - 1 and pos not in seen:
            seen.add(pos)
            start = pos
            cid = struct.unpack_from('<H', data, pos)[0]
            pos += 2
            if pos + 8 > len(data) + 8:
                break
            if FP_MODE and cid >= VANILLA_CMD_COUNT and cid in FP_COMMANDS:
                cmd = f'FP_{cid}'
                name, params = FP_COMMANDS[cid]
                CMD_LAYOUT[cmd] = (name, params)
            elif cid >= len(CMD_NAMES) or (cid >= VANILLA_CMD_COUNT and CMD_NAMES[cid] in OUR_EXTRA_CMDS):
                unknown_ids.append(cid)
                rest = data[pos:pos + 8].hex(' ')
                lines[start] = f'Unknown_{cid} (next bytes: {rest})'
                break
            else:
                cmd = CMD_NAMES[cid]
            name, params = CMD_LAYOUT.get(cmd, (cmd, []))
            args = []
            kinds = list(params)
            extended = False
            i = 0
            while i < len(kinds):
                k = kinds[i]
                if k == 'b':
                    args.append(str(data[pos]))
                    pos += 1
                elif k == 'h':
                    v = struct.unpack_from('<H', data, pos)[0]
                    args.append(str(v) if v < 0x4000 else hex(v))
                    pos += 2
                elif k == 'w':
                    args.append(hex(struct.unpack_from('<I', data, pos)[0]))
                    pos += 4
                else:
                    target = pos + 4 + struct.unpack_from('<i', data, pos)[0]
                    pos += 4
                    if not 0 <= target < len(data):
                        args.append(f'<bad offset {target}>')
                    elif k == 'mov':
                        lab = label(target, 'Movement_')
                        movements[target] = decode_movement(data, target)
                    else:
                        lab = label(target)
                        work.append(target)
                    args.append(lab)
                if i == len(kinds) - 1 and not extended:
                    kinds += variable_params(cmd, data, pos)
                    extended = True
                i += 1
            lines[start] = f'{name} {", ".join(args)}'.rstrip()
            if cmd in TERMINATORS:
                break

    out = header + ['']
    for addr in sorted(set(lines) | set(movements)):
        if addr in labels:
            out.append(f'{labels[addr]}:')
        if addr in movements:
            out += ['    ' + m for m in movements[addr]]
        else:
            out.append('    ' + lines[addr])
    return '\n'.join(out) + '\n', unknown_ids


def member_names():
    return [l.strip() for l in read('res/field/scripts/scripts.order').split('\n') if l.strip()]


def main():
    sys.argv = [a for a in sys.argv if a != '--fp']
    narc = narc_members(open(sys.argv[1], 'rb').read())
    names = member_names()
    names += [f'added_member_{i}' for i in range(len(names), len(narc))]
    if sys.argv[2] == 'all':
        os.makedirs(sys.argv[3], exist_ok=True)
        for i, data in enumerate(narc):
            text, _ = disassemble(data, names[i].startswith('scripts_init_'))
            open(os.path.join(sys.argv[3], f'{i:04d}_{names[i]}.txt'), 'w', encoding='utf-8').write(text)
        return
    idx = int(sys.argv[2]) if sys.argv[2].isdigit() else names.index(sys.argv[2])
    text, unk = disassemble(narc[idx], names[idx].startswith('scripts_init_'))
    print(f'// member {idx}: {names[idx]}')
    print(text)


if __name__ == '__main__':
    main()
