"""Per-member summary of Following Platinum's script edits: FP commands added, every follower (local id 253)
movement with its steps, and the other added lines.

Usage: py -3 -I script_report.py <fp dump dir> <diff dir>   (from script_disasm.py --fp and script_diff.py)
"""
import os
import re
import sys
from collections import Counter

fdir, ddir = sys.argv[1], sys.argv[2]


def movements(text):
    bodies, cur = {}, None
    for line in text.split('\n'):
        m = re.match(r'^(Movement_\d+):$', line)
        if m:
            cur = m.group(1)
            bodies[cur] = []
        elif re.match(r'^\S', line):
            cur = None
        elif cur and line.strip() and line.strip() != 'EndMovement':
            bodies[cur].append(line.strip())
    return bodies


for name in sorted(os.listdir(ddir)):
    diff = open(os.path.join(ddir, name), encoding='utf-8').read().split('\n')
    text = open(os.path.join(fdir, name.replace('.diff', '.txt')), encoding='utf-8').read()
    bodies = movements(text)
    added = [l[1:].strip() for l in diff if l.startswith('+') and not l.startswith('+++')]
    removed = [l[1:].strip() for l in diff if l.startswith('-') and not l.startswith('---')]
    fp = Counter(l.split()[0] for l in added if l.startswith('FP_'))
    follower = [(m, bodies.get(m, [])) for m in re.findall(r'ApplyMovement 253, (Movement_\d+)', text)]
    print(f'### {name[:-5]}  (+{len(added)} / -{len(removed)} lines)')
    if fp:
        print('- FP commands: ' + ', '.join(f'{k} x{v}' for k, v in fp.most_common()))
    if follower:
        print(f'- follower movements ({len(follower)}):')
        seen = set()
        for m, body in follower:
            key = ', '.join(body)
            if key in seen:
                continue
            seen.add(key)
            print(f'  - {key}')
    other = Counter(re.sub(r'\b(L|Movement_|Script_)#?\d*\b', '', l).strip() for l in added
                    if not l.startswith('FP_') and not l.endswith(':') and '253' not in l)
    interesting = [k for k, _ in other.most_common() if k and not k.startswith(('Walk', 'Face', 'Delay', 'End', 'Jump', 'Lock', 'Unlock', 'Emote'))]
    if interesting:
        print('- other added: ' + '; '.join(interesting[:10]))
    if removed:
        rem = Counter(re.sub(r'\b(L|Movement_|Script_)#?\d*\b', '', l).strip() for l in removed)
        print('- removed: ' + '; '.join(k for k, _ in rem.most_common(8) if k))
    print()
