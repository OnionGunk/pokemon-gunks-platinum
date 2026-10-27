"""Import Following Platinum's follower interaction text (pl_msg.narc bank 724) as res/text/pokemon_follower.json.
Message order is kept: the talk script picks lines by numeric range.

Usage: py -3 -I tools/followers/import_fp_text.py
"""
import json
import re
import sys

sys.path.insert(0, 'C:/projects/pokemon-platinum-dexnav/fp')
from narc import narc_members  # noqa: E402

FP_MSG = 'C:/projects/pokemon-platinum-dexnav/fp/fp/fs/msgdata/pl_msg.narc'
CHARMAP = 'C:/projects/pokemon-platinum-dexnav/pkplat-dexnav/tools/msgenc/charmap.txt'
OUT = 'C:/projects/pokemon-platinum-dexnav/pkplat-dexnav/res/text/pokemon_follower.json'
BANK = 724

chars = {}
for line in open(CHARMAP, encoding='utf-8'):
    line = line.rstrip('\n').lstrip(' \t')
    if line.startswith('//') or '=' not in line:
        continue
    code, ch = line.split('=', 1)
    try:
        chars[int(code, 16)] = ch
    except ValueError:
        pass


def decode_bank(data):
    import struct
    count, seed = struct.unpack_from('<HH', data, 0)
    out = []
    for i in range(count):
        key = (seed * 0x2FD * (i + 1)) & 0xFFFF
        key32 = key | (key << 16)
        off, length = struct.unpack_from('<II', data, 4 + i * 8)
        off ^= key32
        length ^= key32
        k = (0x91BD3 * (i + 1)) & 0xFFFF
        codes = []
        for j in range(length):
            (c,) = struct.unpack_from('<H', data, off + j * 2)
            codes.append(c ^ k)
            k = (k + 0x493D) & 0xFFFF
        s, j = [], 0
        while j < len(codes) and codes[j] != 0xFFFF:
            c = codes[j]
            if c == 0xFFFE:
                cmd, n = codes[j + 1], codes[j + 2]
                params = codes[j + 3:j + 3 + n]
                if cmd >> 8 == 0x01:  # string variable: {STRVAR_1 type, index, flags}
                    s.append('{STRVAR_1 %d, %s}' % (cmd & 0xFF, ', '.join(map(str, params))))
                else:
                    raise ValueError(f'unhandled text command {cmd:04X} in message {i}')
                j += 3 + n
                continue
            s.append(chars[c])
            j += 1
        out.append(''.join(s))
    return seed, out


def split_lines(text):
    # one array element per line, ending in its line-break code, like the other banks; the charmap
    # spells line breaks as two characters ("\\n") but the bank JSON stores real control characters
    for escape, char in (('\\n', '\n'), ('\\r', '\r'), ('\\f', '\f')):
        text = text.replace(escape, char)
    parts = re.findall(r'[^\n\r\f]*(?:[\n\r\f]|$)', text)
    parts = [p for p in parts if p]
    return parts[0] if len(parts) == 1 else parts


seed, messages = decode_bank(narc_members(open(FP_MSG, 'rb').read())[BANK])
bank = {'key': seed, 'messages': []}
for i, text in enumerate(messages):
    entry = {'id': f'PokemonFollower_Text_{i:03d}'}
    entry['en_US'] = split_lines(text) if text else ''
    bank['messages'].append(entry)

with open(OUT, 'w', encoding='utf-8', newline='\n') as f:
    json.dump(bank, f, ensure_ascii=False, indent=2)
    f.write('\n')
print(f'{len(messages)} messages -> {OUT}')
