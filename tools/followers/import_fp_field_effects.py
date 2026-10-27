"""Import Following Platinum's field effects (emote bubbles, Poke Ball release, model fixes) into
res/graphics/field_effects, taken directly from its fldeff.narc (members 201-221 are new; 20, 69,
70, 72 and 73 are edited vanilla models).

Usage: py -3 -I tools/followers/import_fp_field_effects.py
"""
import os
import re
import sys

sys.path.insert(0, 'C:/projects/pokemon-platinum-dexnav/fp')
from narc import narc_members  # noqa: E402

ROOT = 'C:/projects/pokemon-platinum-dexnav/pkplat-dexnav/res/graphics/field_effects'
FP_NARC = 'C:/projects/pokemon-platinum-dexnav/fp/fp/fs/data/mmodel/fldeff.narc'

# new members, in archive order (names from each file's internal texture/model names)
NEW = {
    201: 'follower_poke_ball.nsbmd',
    202: 'follower_poke_ball_release.nsbmd',
    203: 'follower_poke_ball_release.nsbta',
    204: 'follower_emote_mask.nsbtx',
    205: 'follower_emote.nsbmd',
    206: 'follower_emote_sequence.bin',
    207: 'follower_emote_heart.nsbtx',
    208: 'follower_emote_face_1.nsbtx',
    209: 'follower_emote_face_2.nsbtx',
    210: 'follower_emote_face_3.nsbtx',
    211: 'follower_emote_face_4.nsbtx',
    212: 'follower_emote_face_5.nsbtx',
    213: 'follower_emote_music.nsbtx',
    214: 'follower_emote_question.nsbtx',
    215: 'follower_emote_exclamation.nsbtx',
    216: 'follower_emote_sweat.nsbtx',
    217: 'follower_emote_cry.nsbtx',
    218: 'follower_emote_poison.nsbtx',
    219: 'follower_emote_silence.nsbtx',
    220: 'follower_emote_sleep.nsbtx',
    221: 'follower_reflection_large.nsbmd',
}
EDITED = [20, 69, 70, 72, 73]

members = narc_members(open(FP_NARC, 'rb').read())
meson_path = f'{ROOT}/meson.build'
meson = open(meson_path, encoding='utf-8').read()
order = re.findall(r"'([^']+\.(?:nsbtx|nsbmd|nsbta|nsbca|nsbtp|bin|nsbva|nsbma))'", meson)
assert len(order) >= 201, len(order)

for index in EDITED:
    open(f'{ROOT}/{order[index]}', 'wb').write(members[index])
    print('updated', order[index])

for index, name in NEW.items():
    open(f'{ROOT}/{name}', 'wb').write(members[index])

missing = [name for index, name in sorted(NEW.items()) if f"'{name}'" not in meson]
if missing:
    last = f"'{order[200]}',"
    assert meson.count(last) == 1
    meson = meson.replace(last, last + ''.join(f"\n        '{name}'," for name in missing))
    open(meson_path, 'w', encoding='utf-8', newline='\n').write(meson)
order_path = f'{ROOT}/field_effects.order'
listed = open(order_path, encoding='utf-8').read().split()
unlisted = [name for index, name in sorted(NEW.items()) if name not in listed]
if unlisted:
    with open(order_path, 'a', encoding='utf-8', newline='\n') as f:
        f.write(''.join(f'{name}\n' for name in unlisted))
print(f'{len(NEW)} new members, {len(missing)} added to meson.build, {len(unlisted)} to field_effects.order')
