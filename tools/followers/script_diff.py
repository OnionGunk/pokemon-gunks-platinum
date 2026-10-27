"""Diff Following Platinum's event scripts against vanilla, member by member.

Usage: py -3 -I script_diff.py <vanilla scr_seq.narc> <fp scr_seq.narc> <vanilla dump dir> <fp dump dir> <out dir>
(dump dirs come from `script_disasm.py ... all <dir>`, the FP one with --fp)

Writes <out dir>/<member>_<name>.diff for every member whose bytes differ, with label names normalized so
inserted code doesn't renumber everything, and prints a one-line summary per member.
"""
import difflib
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from script_disasm import narc_members  # noqa: E402

van = narc_members(open(sys.argv[1], 'rb').read())
fp = narc_members(open(sys.argv[2], 'rb').read())
vdir, fdir, out = sys.argv[3], sys.argv[4], sys.argv[5]
os.makedirs(out, exist_ok=True)

LABEL = re.compile(r'\b(L|Movement_|Script_)\d+\b')


def norm(text):
    return [LABEL.sub(lambda m: m.group(1) + '#', l) for l in text.split('\n')]


files = {int(f[:4]): f for f in os.listdir(fdir)}
vfiles = {int(f[:4]): f for f in os.listdir(vdir)}
for i in range(len(fp)):
    if i < len(van) and van[i] == fp[i]:
        continue
    f = files[i]
    a = norm(open(os.path.join(vdir, vfiles[i]), encoding='utf-8').read()) if i in vfiles else []
    b = norm(open(os.path.join(fdir, f), encoding='utf-8').read())
    diff = list(difflib.unified_diff(a, b, 'vanilla', 'fp', n=3, lineterm=''))
    open(os.path.join(out, f.replace('.txt', '.diff')), 'w', encoding='utf-8').write('\n'.join(diff) + '\n')
    added = [l[1:].strip() for l in diff if l.startswith('+') and not l.startswith('+++')]
    removed = [l for l in diff if l.startswith('-') and not l.startswith('---')]
    fpcmds = sorted({l.split()[0] for l in added if l.startswith('FP_')})
    print(f'{f[:-4]}: +{len(added)} -{len(removed)} {" ".join(fpcmds)}')
