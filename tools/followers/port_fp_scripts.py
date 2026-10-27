"""Port Following Platinum's edits to event scripts onto our decomp sources (res/field/scripts/*.s).

For each script FP changed:
  1. assemble our source with a marker label before every line, so each source line has a byte offset
     (and check that the unmodified source assembles to the vanilla member byte for byte);
  2. decode vanilla and FP into instruction/movement items (FP's own command ids mapped to ours);
  3. align the two item streams, keeping only matches whose branch/movement targets also line up;
  4. rewrite only the source lines that changed: deleted items go away, FP's new items are inserted
     in decomp syntax (our label, text, var/flag, local id and species names where known), and a
     compound macro line that is only partly affected is expanded into its primitive commands;
  5. assemble the result and check it decodes to exactly FP's script (modulo the command renames).

Usage:
  py -3 -I tools/followers/port_fp_scripts.py [member index or script name ...]   (default: every changed script)
  add --write to overwrite res/field/scripts/*.s with ported files that validate
"""
import difflib
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import script_disasm as sd  # noqa: E402
from script_asm import AsmError, assemble  # noqa: E402

REPO = sd.ROOT
PROJECT = os.path.abspath(os.path.join(REPO, '..'))
FP_NARC = os.path.join(PROJECT, 'fp', 'fp', 'fs', 'fielddata', 'script', 'scr_seq.narc')
VANILLA_NARC = os.path.join(PROJECT, 'fp', 'vanilla', 'fs', 'fielddata', 'script', 'scr_seq.narc')
WORK = os.path.join(PROJECT, 'fp', 'port_tmp')
SCRIPTS = os.path.join(REPO, 'res', 'field', 'scripts')
BUILD = os.path.join(REPO, 'build')

FOLLOWER_LOCALID = 253
FOLLOWER_DEFINE = '#define LOCALID_POKEMON_FOLLOWER 253'

# Following Platinum's command ids -> our commands
FP_COMMAND_MAP = {
    840: 'SCRCMD_SETALLOWTWOPOKEMONCRIES',
    844: 'SCRCMD_POKEMONFOLLOWERSTARTCUTSCENE',
    845: 'SCRCMD_FACEPLAYER',  # FP's command only turns the talked-to object toward the player
    846: 'SCRCMD_POKEMONFOLLOWERSETPAUSED',
    847: 'SCRCMD_WAITFORPOKEMONFOLLOWER',
    848: 'SCRCMD_POKEMONFOLLOWERSETMOVEMENTTYPE',
    849: 'SCRCMD_POKEMONFOLLOWERPLACENEARPLAYER',
    850: 'SCRCMD_POKEMONFOLLOWERENDCUTSCENE',
    851: 'SCRCMD_POKEMONFOLLOWERRESUMEFOLLOWING',
    852: 'SCRCMD_POKEMONFOLLOWERRESPAWN',
    853: 'SCRCMD_HIDEPOKEMONFOLLOWER',
    854: 'SCRCMD_SHOWPOKEMONFOLLOWER',
    855: 'SCRCMD_PLAYPOKEBALLANIMATION',
    856: 'SCRCMD_GETPOKEMONFOLLOWERPARTYSLOT',
    857: 'SCRCMD_POKEMONFOLLOWERHAPPYHOP',
    858: 'SCRCMD_GETPARTYMONFOLLOWERGFX',
    859: 'SCRCMD_CHECKPOKEMONFOLLOWERACTIVE',
    860: 'SCRCMD_GETPOKEMONFOLLOWERDIRFROMPLAYER',
}


def read(path):
    return open(path, encoding='utf-8').read()


OVERLAY = os.path.join(WORK, 'include_overlay')


def refresh_events_overlay():
    """Same for map events: object local-id headers for maps whose JSON gained named objects."""
    events_dir = os.path.join(REPO, 'res', 'field', 'events')
    out_dir = os.path.join(OVERLAY, 'res', 'field', 'events')
    os.makedirs(out_dir, exist_ok=True)
    import json
    for f in os.listdir(events_dir):
        if not f.endswith('.json'):
            continue
        header = os.path.join(BUILD, 'res', 'field', 'events', f[:-5] + '.h')
        if not os.path.exists(header):
            continue
        objs = json.load(open(os.path.join(events_dir, f), encoding='utf-8'))['object_events']
        defines = ''.join(f'#define {o["id"]} {i}\n' for i, o in enumerate(objs) if 'id' in o)
        if all(line in read(header) for line in defines.splitlines()):
            continue
        guard = 'POKEPLATINUM_GENERATED_OBJECT_EVENTS_' + f[len('events_'):-5].upper() + '_H'
        open(os.path.join(out_dir, f[:-5] + '.h'), 'w', encoding='utf-8', newline='\n').write(
            f'#ifndef {guard}\n#define {guard}\n\n{defines}\n#endif // {guard}\n')


def refresh_text_overlay():
    """build/ may predate text we just added: write current message-id headers for any bank whose
    JSON has more messages than its generated header, so ported scripts assemble before a rebuild."""
    import json
    bank_dir = os.path.join(OVERLAY, 'res', 'text', 'bank')
    os.makedirs(bank_dir, exist_ok=True)
    for f in os.listdir(os.path.join(REPO, 'res', 'text')):
        if not f.endswith('.json'):
            continue
        header = os.path.join(BUILD, 'res', 'text', 'bank', f[:-5] + '.h')
        if not os.path.exists(header):
            continue
        messages = json.load(open(os.path.join(REPO, 'res', 'text', f), encoding='utf-8'))['messages']
        ids = [n for n in re.findall(r'^#define (\w+) \d+\s*$', read(header), re.M) if not n.endswith('_ENTRY_COUNT')]
        if len(ids) >= len(messages):
            continue
        guard = 'PORT_OVERLAY_' + f[:-5].upper()
        body = ''.join(f'#define {m["id"]} {n}\n' for n, m in enumerate(messages))
        body += f'#define TEXT_BANK_{f[:-5].upper()}_ENTRY_COUNT {len(messages)}\n'
        open(os.path.join(bank_dir, f[:-5] + '.h'), 'w', encoding='utf-8', newline='\n').write(
            f'#ifndef {guard}\n#define {guard}\n{body}#endif\n')


# --- decoding ------------------------------------------------------------------------------------------------
class Item:
    __slots__ = ('off', 'size', 'kind', 'cmd', 'params', 'act', 'count')

    def __init__(self, off, size, kind, cmd=None, params=None, act=None, count=None):
        self.off, self.size, self.kind = off, size, kind
        self.cmd, self.params, self.act, self.count = cmd, params, act, count

    def sig(self):
        if self.kind == 'mov':
            return ('M', self.act, self.count)
        return ('C', self.cmd, tuple((k, 'T' if k in ('off', 'mov') else v) for k, v in self.params))

    def targets(self):
        return [v for k, v in self.params if k in ('off', 'mov')] if self.kind == 'cmd' else []


def command_name(cid, fp):
    if fp and cid >= sd.VANILLA_CMD_COUNT:
        return FP_COMMAND_MAP.get(cid)
    if cid < len(sd.CMD_NAMES):
        return sd.CMD_NAMES[cid]
    return None


def decode(data, fp):
    """Items reachable from the script entries (code by recursive descent, then movement blocks)."""
    entries = sd.entries(data, False)
    items = {}
    movement_starts = set()
    work = [v for k, v in entries if k == 'script']
    seen = set()
    while work:
        pos = work.pop()
        while 0 <= pos < len(data) - 1 and pos not in seen:
            seen.add(pos)
            start = pos
            cid = struct.unpack_from('<H', data, pos)[0]
            pos += 2
            cmd = command_name(cid, fp)
            if cmd is None:
                raise ValueError(f'unknown command id {cid} at {start:#x}')
            _, kinds = sd.CMD_LAYOUT.get(cmd, (cmd, []))
            kinds = list(kinds)
            params = []
            extended = False
            i = 0
            while i < len(kinds):
                k = kinds[i]
                if k == 'b':
                    params.append(('b', data[pos]))
                    pos += 1
                elif k == 'h':
                    params.append(('h', struct.unpack_from('<H', data, pos)[0]))
                    pos += 2
                elif k == 'w':
                    params.append(('w', struct.unpack_from('<I', data, pos)[0]))
                    pos += 4
                else:
                    target = pos + 4 + struct.unpack_from('<i', data, pos)[0]
                    pos += 4
                    params.append((k, target))
                    if k == 'mov':
                        movement_starts.add(target)
                    else:
                        work.append(target)
                if i == len(kinds) - 1 and not extended:
                    kinds += sd.variable_params(cmd, data, pos)
                    extended = True
                i += 1
            items[start] = Item(start, pos - start, 'cmd', cmd, params)
            if cmd in sd.TERMINATORS:
                break
    for start in sorted(movement_starts):
        pos = start
        while pos + 4 <= len(data) and pos not in items:
            act, count = struct.unpack_from('<HH', data, pos)
            items[pos] = Item(pos, 4, 'mov', act=act, count=count)
            pos += 4
            if act == 254:
                break
    return [items[k] for k in sorted(items)], [v for k, v in entries if k == 'script']


# --- naming --------------------------------------------------------------------------------------------------
def load_defines(path, prefix):
    out = {}
    if os.path.exists(path):
        for name, val in re.findall(r'#define\s+(' + prefix + r'\w+)\s+(-?\w+)\s*$', read(path), re.M):
            try:
                out.setdefault(int(val, 0), []).append(name)
            except ValueError:
                pass
    return out


VARS_FLAGS = load_defines(os.path.join(BUILD, 'generated', 'vars_flags.h'), r'(?:VAR|FLAG)_')
SPECIES = {v: n[0] for v, n in load_defines(os.path.join(BUILD, 'generated', 'species.h'), 'SPECIES_').items()}

# how often each var/flag name is used across all scripts, to pick the conventional alias
NAME_USE = {}
for _f in os.listdir(SCRIPTS):
    if _f.endswith('.s'):
        for _n in re.findall(r'\b((?:VAR|FLAG)_\w+)', read(os.path.join(SCRIPTS, _f))):
            NAME_USE[_n] = NAME_USE.get(_n, 0) + 1


class Namer:
    def __init__(self, source):
        self.text = {}
        self.localids = {FOLLOWER_LOCALID: 'LOCALID_POKEMON_FOLLOWER', 0xFF: 'LOCALID_PLAYER'}
        for inc in re.findall(r'#include "(res/[^"]+\.h)"', source):
            path = os.path.join(BUILD, inc)
            if '/text/bank/' in inc:
                # message ids in bank order, from the bank's JSON (current even before a rebuild)
                bank = os.path.join(REPO, 'res', 'text', os.path.basename(inc)[:-2] + '.json')
                import json
                for v, msg in enumerate(json.load(open(bank, encoding='utf-8'))['messages']):
                    self.text.setdefault(v, msg['id'])
            elif '/events/' in inc:
                # local ids are object positions in the map's events JSON (current even before a rebuild)
                events = os.path.join(REPO, 'res', 'field', 'events', os.path.basename(inc)[:-2] + '.json')
                import json
                for v, obj in enumerate(json.load(open(events, encoding='utf-8'))['object_events']):
                    if 'id' in obj:
                        self.localids.setdefault(v, obj['id'])
        self.used = set(re.findall(r'\b((?:VAR|FLAG)_\w+)', source))

    def var_or_flag(self, value, want):
        names = [n for n in VARS_FLAGS.get(value, []) if n.startswith(want)]
        if not names:
            return None
        local = [n for n in names if n in self.used]
        return (local or sorted(names, key=lambda n: -NAME_USE.get(n, 0)))[0]

    def value(self, arg, value, kind):
        a = arg.lower()
        if a.startswith('message') and value in self.text and value < 0x4000:
            return self.text[value]
        if kind == 'b':
            return str(value)
        if 'flag' in a:
            return self.var_or_flag(value, 'FLAG_') or str(value)
        if 0x8000 <= value <= 0x8015:  # script special vars are never meant as literals
            name = self.var_or_flag(value, 'VAR_')
            if name:
                return name
        if value >= 0x4000 and ('var' in a or 'value' in a or a in ('species', 'partyslot', 'slot', 'localid', 'item',
                                                                    'messageid', 'messagevar')):
            name = self.var_or_flag(value, 'VAR_')
            if name:
                return name
        if a in ('localid', 'objectid') or 'localid' in a:
            return self.localids.get(value, str(value))
        if a.startswith('message') and value in self.text:
            return self.text[value]
        if a == 'species' and value in SPECIES:
            return SPECIES[value]
        return str(value) if value < 0x4000 else hex(value)


# --- macro templates -----------------------------------------------------------------------------------------
def load_macros():
    """SCRCMD name -> (macro name, declared args, emitted arg per param or None) for macros whose body is
    exactly the command id followed by plain argument references."""
    out = {}
    for m in re.finditer(r'\.macro\s+(\w+)([^\n]*)\n(.*?)\.endm', read(os.path.join(REPO, 'asm', 'macros', 'scrcmd.inc')), re.S):
        name, header, body = m.group(1), m.group(2), m.group(3)
        lines = [l.strip() for l in body.split('\n') if l.strip() and not l.strip().startswith('//')]
        if not lines or not re.match(r'\.short (SCRCMD_\w+)$', lines[0]):
            continue
        cmd = lines[0].split()[1]
        args = [a.strip().split('=')[0].strip() for a in header.split(',') if a.strip()]
        defaults = {a.strip().split('=')[0].strip() for a in header.split(',') if '=' in a}
        emitted, ok = [], True
        for l in lines[1:]:
            mm = re.match(r'\.(byte|short|long|word)\s+(.*)$', l)
            if not mm:
                ok = False
                break
            for part in mm.group(2).split(','):
                part = part.strip().replace(' ', '')
                ref = re.match(r'\\(\w+)(-\.-4)?$', part)
                if not ref or ref.group(1) not in args:
                    ok = False
                    break
                emitted.append(ref.group(1))
        if not ok or len(set(emitted)) != len(emitted) or any(a not in emitted and a not in defaults for a in args):
            continue
        out.setdefault(cmd, []).append((name, args, emitted))
    return out


MACROS = load_macros()
MOVE_MACRO = {}
for _name, _act in re.findall(r'\.macro\s+(\w+)[^\n]*\n\s*\.short\s+(MOVEMENT_ACTION_\w+)', read(os.path.join(REPO, 'asm', 'macros', 'movement.inc'))):
    MOVE_MACRO.setdefault(_act, _name)
MOVE_ID = {int(v): n for n, v in re.findall(r'#define (MOVEMENT_ACTION_\w+)\s+(\d+)', read(os.path.join(BUILD, 'generated', 'movement_actions.h')))}


GFX_NAMES = {int(v): n for n, v in re.findall(r'#define (OBJ_EVENT_GFX_\w+)\s+(\d+)\s*$',
                                               read(os.path.join(BUILD, 'generated', 'object_events_gfx.h')), re.M)}


# argument meanings for macros whose parameter names don't say (used only to pick value names)
ARG_HINTS = {
    'SCRCMD_UNUSED_06A': ['localID', 'destVarX', 'destVarZ'],  # gets an object's map position
    'SCRCMD_ADDOBJECT': ['localID'],
}


def format_item(item, namer, label_for):
    if item.kind == 'mov':
        if item.act == 254:
            return ['EndMovement']
        act = MOVE_ID.get(item.act)
        macro = MOVE_MACRO.get(act) if act else None
        if macro is None:
            return [f'.short {act or item.act}', f'.short {item.count}']
        return [f'{macro} {item.count}' if item.count != 1 else macro]
    candidates = MACROS.get(item.cmd, [])
    preferred = sd.CMD_LAYOUT.get(item.cmd, (None,))[0]
    candidates = sorted(candidates, key=lambda c: c[0] != preferred)
    for name, args, emitted in candidates:
        if len(emitted) != len(item.params):
            continue
        values = {}
        hints = ARG_HINTS.get(item.cmd, emitted)
        for argname, hint, (kind, v) in zip(emitted, hints, item.params):
            values[argname] = label_for(v) if kind in ('off', 'mov') else namer.value(hint, v, kind)
        rendered = [values[a] for a in args if a in values]
        if len(rendered) != len([a for a in args if a in values]) or any(a not in values for a in args[:len(rendered)]):
            continue
        # an object graphics id stored into one of the VAR_OBJ_GFX_ID_* vars
        if item.cmd in ('SCRCMD_SETVARFROMVALUE', 'SCRCMD_SETVAR') and len(rendered) == 2 \
                and rendered[0].startswith('VAR_OBJ_GFX_ID_') and item.params[1][1] in GFX_NAMES:
            rendered[1] = GFX_NAMES[item.params[1][1]]
        return [f'{name} {", ".join(rendered)}'.rstrip()]
    # no plain macro: raw directives
    out = [f'.short {item.cmd}']
    for kind, v in item.params:
        if kind == 'b':
            out.append(f'.byte {v}')
        elif kind == 'h':
            out.append(f'.short {v}')
        elif kind == 'w':
            out.append(f'.long {v:#x}')
        else:
            out.append(f'.long {label_for(v)}-.-4')
    return out


# --- source analysis -----------------------------------------------------------------------------------------
LABEL_RE = re.compile(r'^\s*([A-Za-z_]\w*):\s*$')


def is_emitting(line):
    s = line.strip()
    if not s or s.startswith('//') or s.startswith('#') or LABEL_RE.match(line):
        return False
    return True


def analyze_source(source, name):
    lines = source.split('\n')
    marked = []
    markers = []
    for i, line in enumerate(lines):
        if is_emitting(line):
            marked.append(f'__PFL{i}:')
            markers.append(i)
        marked.append(line)
    data, labels = assemble('\n'.join(marked), WORK, name + '_marked')
    line_off = {i: labels[f'__PFL{i}'] for i in markers}
    our_labels = {}
    for lab, off in labels.items():
        if lab.startswith('__PFL') or lab == '_start':
            continue
        our_labels.setdefault(off, []).append(lab)
    # label lines in source order, to prefer the first defined label at an offset
    order = {m.group(1): i for i, l in enumerate(lines) for m in [LABEL_RE.match(l)] if m}
    for off in our_labels:
        our_labels[off].sort(key=lambda n: order.get(n, 1 << 30))
    return lines, data, line_off, markers, our_labels


# --- porting -------------------------------------------------------------------------------------------------
def align(v_items, f_items):
    sm = difflib.SequenceMatcher(None, [i.sig() for i in v_items], [i.sig() for i in f_items], autojunk=False)
    pairs = {}
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            for k in range(i2 - i1):
                pairs[i1 + k] = j1 + k
    # drop matches whose targets don't correspond, until stable
    v_off = {it.off: n for n, it in enumerate(v_items)}
    f_off = {it.off: n for n, it in enumerate(f_items)}
    changed = True
    while changed:
        changed = False
        f2v = {f: v for v, f in pairs.items()}
        for v in list(pairs):
            f = pairs[v]
            for vt, ft in zip(v_items[v].targets(), f_items[f].targets()):
                vn, fn = v_off.get(vt), f_off.get(ft)
                if vn is None or fn is None or f2v.get(fn) != vn:
                    del pairs[v]
                    changed = True
                    break
    return pairs


def label_prefix(lines):
    names = [m.group(1) for l in lines for m in [LABEL_RE.match(l)] if m and not m.group(1).startswith('_')]
    prefixes = {}
    for n in names:
        p = n.split('_')[0]
        prefixes[p] = prefixes.get(p, 0) + 1
    return max(prefixes, key=prefixes.get) if prefixes else 'Script'


def port(index, name, vanilla_data, fp_data):
    path = os.path.join(SCRIPTS, name + '.s')
    source = read(path)
    lines, built, line_off, markers, our_labels = analyze_source(source, name)
    ours_extra = set()
    if built != vanilla_data:
        # our source already adds to the vanilla script (e.g. DexNav's common scripts): port onto our
        # build instead, and keep the items vanilla doesn't have
        o_items, _ = decode(built, fp=False)
        van_items, _ = decode(vanilla_data, fp=False)
        base_pairs = align(o_items, van_items)
        ours_extra = {n for n in range(len(o_items)) if n not in base_pairs}
        vanilla_data = built

    v_items, v_entries = decode(vanilla_data, fp=False)
    f_items, f_entries = decode(fp_data, fp=True)
    pairs = align(v_items, f_items)
    f2v = {f: v for v, f in pairs.items()}
    v_index = {it.off: n for n, it in enumerate(v_items)}
    f_index = {it.off: n for n, it in enumerate(f_items)}
    namer = Namer(source)
    prefix = label_prefix(lines)

    # which source line holds each vanilla item, and whether it starts that line
    line_starts = sorted((line_off[i], i) for i in markers)
    starts_by_line = {}
    owner = {}
    for n, it in enumerate(v_items):
        cands = [i for off, i in line_starts if off <= it.off]
        i = None
        for off, li in reversed(line_starts):
            if off <= it.off:
                i = li
                break
        owner[n] = i
        starts_by_line.setdefault(i, []).append(n)

    # targets FP refers to
    f_targets = {}
    for it in f_items:
        for k, v in it.params if it.kind == 'cmd' else []:
            if k in ('off', 'mov'):
                f_targets[v] = k
    for t in f_entries:
        f_targets.setdefault(t, 'off')

    # labels for vanilla offsets we need but don't have; names for FP-only targets
    new_labels_at_v = {}
    counter = [0]

    def fresh(kind):
        counter[0] += 1
        return f'{prefix}_{"Movement_" if kind == "mov" else ""}PokemonFollower{counter[0]}'

    def our_label(voff, kind):
        if voff in our_labels:
            return our_labels[voff][0]
        # FP dropped the items between an existing label and this one (e.g. a trimmed movement
        # block): the existing label now marks this item
        n = v_index[voff] - 1
        while n >= 0 and n not in kept and v_items[n].off not in our_labels:
            n -= 1
        if n >= 0 and n not in kept and all(m not in kept for m in range(n, v_index[voff])):
            return our_labels[v_items[n].off][0]
        if voff not in new_labels_at_v:
            new_labels_at_v[voff] = fresh(kind)
        return new_labels_at_v[voff]

    f_new_labels = {}

    def label_for_f(foff):
        n = f_index[foff]
        if n in f2v:
            return our_label(v_items[f2v[n]].off, f_targets.get(foff, 'off'))
        if foff not in f_new_labels:
            f_new_labels[foff] = fresh(f_targets.get(foff, 'off'))
        return f_new_labels[foff]

    def label_for_v(voff):
        return our_label(voff, 'off')

    # Walk the gaps between matched items. FP's new items in a gap go where the gap's first replaced
    # vanilla item was (so they stay ahead of any undecoded lines like a trailing End), or, if nothing
    # was replaced, right before the next matched vanilla item.
    inserts_before = {}  # vanilla item index -> [f item index]
    kept = set(pairs) | ours_extra
    matched = sorted(pairs.items())
    bounds = [(-1, -1)] + matched + [(len(v_items), len(f_items))]
    tail_inserts = []
    for (va, fa), (vb, fb) in zip(bounds, bounds[1:]):
        new = list(range(fa + 1, fb))
        if not new:
            continue
        deleted = [n for n in range(va + 1, vb) if n not in kept]
        if deleted:
            inserts_before.setdefault(deleted[0], []).extend(new)
        elif vb < len(v_items):
            inserts_before.setdefault(vb, []).extend(new)
        else:
            tail_inserts.extend(new)

    # resolve all labels first (so new vanilla-offset labels are known before writing lines)
    for foff in f_targets:
        label_for_f(foff)
    f_text = {fn: format_item(f_items[fn], namer, label_for_f) for fn in range(len(f_items)) if fn not in f2v}

    dirty = set()
    for vn, it in enumerate(v_items):
        li = owner[vn]
        first_on_line = starts_by_line[li][0] == vn and line_off[li] == it.off
        if vn not in kept:
            dirty.add(li)
        if vn in inserts_before and not first_on_line:
            dirty.add(li)
        if it.off in new_labels_at_v and not first_on_line:
            dirty.add(li)
    # a dirty line must cover only decoded items (otherwise we can't regenerate it safely)
    for li in dirty:
        vns = starts_by_line[li]
        end = min([off for off, i in line_starts if off > line_off[li]] + [len(vanilla_data)])
        covered = sum(v_items[n].size for n in vns)
        span = end - line_off[li]
        if covered != span and not (lines[li].strip().startswith('.') or covered > span):
            raise ValueError(f'line {li + 1} only partly decoded: {lines[li].strip()}')

    state = {'last': None}

    def compound(cmp_item, branch_item):
        """CompareVarToValue/CompareVarToVar + GoToIf/CallIf -> the decomp's GoToIfEq-style macro."""
        conds = ['Lt', 'Eq', 'Gt', 'Le', 'Ge', 'Ne']
        if cmp_item.cmd not in ('SCRCMD_COMPAREVARTOVALUE', 'SCRCMD_COMPAREVARTOVAR'):
            return None
        if branch_item.cmd not in ('SCRCMD_GOTOIF', 'SCRCMD_CALLIF') or branch_item.off in f_new_labels:
            return None
        (_, var), (_, value) = cmp_item.params
        (_, cond), (_, target) = branch_item.params
        is_var = 0x4000 <= value < 0x8000 or 0x8000 <= value <= 0x8015
        if cond >= len(conds) or (cmp_item.cmd == 'SCRCMD_COMPAREVARTOVALUE') == is_var:
            return None
        kind = 'GoToIf' if branch_item.cmd == 'SCRCMD_GOTOIF' else 'CallIf'
        var_name = namer.value('varID', var, 'h')
        value_name = namer.value('varID' if is_var else 'value', value, 'h')
        return f'{kind}{conds[cond]} {var_name}, {value_name}, {label_for_f(target)}'

    def emit_inserted(fns, out):
        skip = set()
        for k, fn in enumerate(fns):
            if fn in skip:
                continue
            it = f_items[fn]
            if it.off in f_new_labels:
                if it.kind == 'mov':
                    out.append('')
                    out.append('    .balign 4, 0')
                elif out and out[-1].strip():
                    out.append('')
                out.append(f'{f_new_labels[it.off]}:')
            nxt = f_items[fns[k + 1]] if k + 1 < len(fns) and fns[k + 1] == fn + 1 else None
            folded = compound(it, nxt) if nxt is not None and it.kind == 'cmd' and nxt.kind == 'cmd' else None
            if folded:
                out.append('    ' + folded)
                skip.add(fns[k + 1])
            else:
                out.extend('    ' + t for t in f_text[fn])
        if fns:
            state['last'] = f_items[fns[-1]]
            if not falls_through():
                out.append('')

    def retarget_only(i, vns, line):
        """If FP's only change to this line is where its branches/movements point, return the line
        with the label names swapped; otherwise None."""
        deleted = [vn for vn in vns if vn not in kept]
        if not deleted or deleted != list(range(deleted[0], deleted[-1] + 1)):
            return None
        anchors = [vn for vn in vns if vn in inserts_before]
        if anchors != [deleted[0]] or any(v_items[vn].off in new_labels_at_v for vn in vns[1:]):
            return None
        new = inserts_before[deleted[0]]
        if len(new) != len(deleted) or any(f_items[fn].off in f_new_labels for fn in new):
            return None
        text = line
        for vn, fn in zip(deleted, new):
            v_it, f_it = v_items[vn], f_items[fn]
            if v_it.sig() != f_it.sig():
                return None
            for vt, ft in zip(v_it.targets(), f_it.targets()):
                old_names = [n for n in our_labels.get(vt, []) if re.search(r'\b' + re.escape(n) + r'\b', text)]
                if not old_names:
                    return None
                text = re.sub(r'\b' + re.escape(old_names[0]) + r'\b', label_for_f(ft), text)
        return text

    def falls_through():
        last = state['last']
        return last is not None and last.kind == 'cmd' and last.cmd not in sd.TERMINATORS

    out = []
    pending_labels = []  # label/blank/.balign/header lines waiting for the next decoded line
    entry_k = 0
    for i, line in enumerate(lines):
        stripped = line.strip()
        if i not in starts_by_line:
            header = stripped.startswith('ScriptEntry')
            if is_emitting(line) and not header and not stripped.startswith('.balign'):
                # unreachable vanilla code (e.g. an End after a GoTo): drop it if FP's code now runs into it
                if falls_through():
                    continue
                out.extend(pending_labels)
                pending_labels = []
                out.append(line)
                continue
            if stripped.startswith('ScriptEntryEnd'):
                for t in f_entries[len(v_entries):]:
                    pending_labels.append(f'    ScriptEntry {label_for_f(t)}')
            elif header and entry_k < min(len(v_entries), len(f_entries)):
                # FP may point an existing entry at new code
                vt, ft = v_entries[entry_k], f_entries[entry_k]
                if f2v.get(f_index[ft]) != v_index[vt]:
                    line = re.sub(r'ScriptEntry\s+\w+', 'ScriptEntry ' + label_for_f(ft), line)
                entry_k += 1
            pending_labels.append(line)
            continue
        vns = starts_by_line[i]
        first = vns[0]
        retargeted = retarget_only(i, vns, line)
        if retargeted is not None:
            # FP only pointed this line's branches somewhere else: keep the line, swap the labels
            out.extend(pending_labels)
            pending_labels = []
            if v_items[first].off in new_labels_at_v and line_off[i] == v_items[first].off:
                out.append(f'{new_labels_at_v[v_items[first].off]}:')
            out.append(retargeted)
            state['last'] = v_items[vns[-1]]
            continue
        # insertions anchored at the first item of this line go above its labels
        if first in inserts_before and line_off[i] == v_items[first].off:
            # keep blank lines that separate blocks above the insertion
            lead = []
            while pending_labels and not pending_labels[0].strip():
                lead.append(pending_labels.pop(0))
            out.extend(lead)
            emit_inserted(inserts_before[first], out)
        out.extend(pending_labels)
        pending_labels = []
        if v_items[first].off in new_labels_at_v and line_off[i] == v_items[first].off:
            out.append(f'{new_labels_at_v[v_items[first].off]}:')
        if i not in dirty:
            out.append(line)
            state['last'] = v_items[vns[-1]]
            continue
        indent = re.match(r'\s*', line).group(0) or '    '
        for vn in vns:
            it = v_items[vn]
            if vn != first or line_off[i] != it.off:
                if vn in inserts_before:
                    emit_inserted(inserts_before[vn], out)
                if it.off in new_labels_at_v:
                    out.append(f'{new_labels_at_v[it.off]}:')
            if vn in kept:
                out.extend(indent + t for t in format_item(it, namer, label_for_v))
                state['last'] = it
    out.extend(pending_labels)
    if tail_inserts:
        emit_inserted(tail_inserts, out)

    text = '\n'.join(out)
    if 'LOCALID_POKEMON_FOLLOWER' in text and 'define LOCALID_POKEMON_FOLLOWER' not in text:
        incs = [n for n, l in enumerate(out) if l.startswith('#include')]
        at = (incs[-1] + 1) if incs else 0
        out.insert(at, FOLLOWER_DEFINE)
        text = '\n'.join(out)
    text = re.sub(r'\n{3,}', '\n\n', text)
    return text, [v_items[n].sig() for n in sorted(ours_extra)]


def validate(name, text, fp_data, extras=()):
    data, _ = assemble(text, WORK, name + '_ported', [OVERLAY])
    ours, our_entries = decode(data, fp=False)
    theirs, their_entries = decode(fp_data, fp=True)
    if extras:
        # our own additions must be the only difference
        a = [i.sig() for i in ours]
        b = [i.sig() for i in theirs]
        extra_found = []
        for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
            if tag == 'delete':
                extra_found += a[i1:i2]
            elif tag != 'equal':
                return f'{tag} run at item {i1}: ours {a[i1:i2][:2]} FP {b[j1:j2][:2]}'
        if sorted(map(repr, extra_found)) != sorted(map(repr, extras)):
            return f'our additions changed: {len(extra_found)} extra items, expected {len(extras)}'
        if len(our_entries) < len(their_entries):
            return 'lost script entries'
        return None
    if len(our_entries) != len(their_entries):
        return f'{len(our_entries)} script entries vs FP {len(their_entries)}'
    a = [i.sig() for i in ours]
    b = [i.sig() for i in theirs]
    if a != b:
        sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
        diffs = [op for op in sm.get_opcodes() if op[0] != 'equal']
        return f'{len(diffs)} differing runs, first at item {diffs[0][1]}: ours {a[diffs[0][1]:diffs[0][2]][:2]} FP {b[diffs[0][3]:diffs[0][4]][:2]}'
    # same items; check every branch/movement target points at the corresponding item
    oi = {it.off: n for n, it in enumerate(ours)}
    ti = {it.off: n for n, it in enumerate(theirs)}
    for x, y in zip(ours, theirs):
        for t1, t2 in zip(x.targets(), y.targets()):
            if oi.get(t1) != ti.get(t2):
                return f'target mismatch at our {x.off:#x}'
    for t1, t2 in zip(our_entries, their_entries):
        if oi.get(t1) != ti.get(t2):
            return 'script entry target mismatch'
    return None


def main():
    refresh_text_overlay()
    refresh_events_overlay()
    write = '--write' in sys.argv
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    names = sd.member_names()
    van = sd.narc_members(open(VANILLA_NARC, 'rb').read())
    fp = sd.narc_members(open(FP_NARC, 'rb').read())
    changed = [i for i in range(len(van)) if van[i] != fp[i]]
    if args:
        changed = [int(a) if a.isdigit() else names.index(a) for a in args]
    results = []
    for i in changed:
        name = names[i]
        if name.startswith('scripts_init_'):
            results.append((i, name, 'skipped', 'init script table'))
            continue
        if name == 'scripts_amity_square':
            results.append((i, name, 'skipped', 'Amity Square rework deferred'))
            continue
        try:
            text, extras = port(i, name, van[i], fp[i])
            problem = validate(name, text, fp[i], extras)
        except (AsmError, ValueError, KeyError) as e:
            results.append((i, name, 'failed', str(e).strip().splitlines()[-1][:200] if str(e).strip() else repr(e)))
            continue
        if problem:
            results.append((i, name, 'diffs', problem))
            open(os.path.join(WORK, name + '_ported.s'), 'w', encoding='utf-8', newline='\n').write(text)
        else:
            results.append((i, name, 'ok', ''))
            if write:
                open(os.path.join(SCRIPTS, name + '.s'), 'w', encoding='utf-8', newline='\n').write(text)
            else:
                open(os.path.join(WORK, name + '_ported.s'), 'w', encoding='utf-8', newline='\n').write(text)
    for i, name, status, note in results:
        print(f'{i:4d} {name:55s} {status:8s} {note}')
    return results


if __name__ == '__main__':
    main()
