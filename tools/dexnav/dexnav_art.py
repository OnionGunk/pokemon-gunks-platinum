#!/usr/bin/env python3
"""Draws the DexNav screens as pixel art and converts them into DS tile data.

Each 256x192 screen is cut into 8x8 tiles, deduplicated (including H/V flips),
and written to src/applications/dexnav_art.h as 4bpp tiles + tilemaps + palettes.
The cursor highlight is a palette swap, so selected cells cost no extra tiles.

Usage: python dexnav_art.py <repo root> [preview dir]
"""
import os
import sys

from PIL import Image, ImageDraw

W, H = 256, 192

# Layout shared with src/applications/dexnav.c
GRID_LEFT, CELL_W, CELL_H, COLS, ROWS = 8, 40, 32, 6, 2
LAND_TOP, WATER_TOP = 16, 104

# Palette 0: base art. Index 0 is never drawn (BG transparency).
BASE = [
    (0, 0, 0),         # 0  transparent
    (248, 240, 216),   # 1  cream paper
    (234, 222, 190),   # 2  paper dot
    (107, 66, 38),     # 3  wood dark
    (156, 107, 60),    # 4  wood mid
    (201, 154, 94),    # 5  wood light
    (255, 249, 236),   # 6  card
    (205, 180, 132),   # 7  card border
    (243, 230, 200),   # 8  slot fill
    (214, 192, 150),   # 9  slot shadow
    (111, 163, 90),    # 10 leaf
    (166, 207, 122),   # 11 leaf light
    (74, 56, 40),      # 12 ink
    (208, 96, 76),     # 13 berry
    (235, 211, 160),   # 14 tab
    (255, 255, 255),   # 15 shine
]
# Palette 1: identical except the slot colours, used for the selected cell.
SELECTED = list(BASE)
SELECTED[8] = (255, 214, 150)
SELECTED[9] = (232, 150, 72)
SELECTED[15] = (255, 238, 200)

# Palette for text on the text layer (font colour slots).
TEXT = [(0, 0, 0)] * 16
TEXT[1] = (74, 56, 40)      # ink
TEXT[2] = (214, 192, 150)   # soft shadow on paper/card
TEXT[3] = (255, 246, 222)   # light text on wood
TEXT[4] = (107, 66, 38)     # dark wood shadow
TEXT[15] = (255, 249, 236)


def new_screen():
    img = Image.new("P", (W, H), 1)
    img.putpalette([c for rgb in BASE for c in rgb] + [0] * (256 - 16) * 3)
    d = ImageDraw.Draw(img)
    # polka-dot paper, offset every other row
    for y in range(0, H, 8):
        for x in range(0, W, 16):
            ox = x + (8 if (y // 8) % 2 else 0) + 3
            d.point([(ox, y + 3), (ox + 1, y + 3), (ox, y + 4), (ox + 1, y + 4)], fill=2)
    return img, d


def rounded(d, x0, y0, x1, y1, fill, border, r=3):
    d.rounded_rectangle([x0, y0, x1, y1], radius=r, fill=fill, outline=border)


def plank(d, x0, y0, x1, y1):
    d.rectangle([x0, y0, x1, y1], fill=4)
    d.line([x0, y0, x1, y0], fill=5)
    d.line([x0, y1, x1, y1], fill=3)
    # wood grain and nail heads
    for gy in range(y0 + 3, y1 - 1, 5):
        for gx in range(x0 + 6 + (gy * 7) % 23, x1 - 6, 37):
            d.line([gx, gy, gx + 9, gy], fill=3)
    for nx in (x0 + 4, x1 - 5):
        d.point([(nx, y0 + 3), (nx, y1 - 3)], fill=5)


def leaf(d, x, y, flip=False):
    pts = [(0, 3), (1, 2), (2, 1), (3, 1), (4, 0), (5, 0), (6, 1), (5, 2), (4, 3), (3, 4), (2, 4), (1, 4)]
    for px, py in pts:
        d.point((x + (6 - px if flip else px), y + py), fill=10)
    d.point((x + (3 if not flip else 3), y + 2), fill=11)
    d.point((x + (4 if not flip else 2), y + 1), fill=11)


def slot(d, x, y):
    # cell is 40x32; the slot sits inside with a soft drop shadow
    rounded(d, x + 3, y + 3, x + 37, y + 30, fill=9, border=9, r=4)
    rounded(d, x + 2, y + 1, x + 36, y + 28, fill=8, border=7, r=4)
    d.line([x + 6, y + 3, x + 30, y + 3], fill=15)


def panel_with_tab(d, top, label_w):
    rounded(d, 3, top - 13, 252, top + ROWS * CELL_H + 3, fill=6, border=7, r=5)
    # tab for the section label
    rounded(d, 6, top - 12, 6 + label_w, top - 2, fill=14, border=7, r=3)
    leaf(d, 6 + label_w - 9, top - 10)
    for i in range(COLS * ROWS):
        slot(d, GRID_LEFT + (i % COLS) * CELL_W, top + (i // COLS) * CELL_H)


def draw_bottom():
    img, d = new_screen()
    panel_with_tab(d, LAND_TOP, 46)
    panel_with_tab(d, WATER_TOP, 52)
    plank(d, 0, 176, 255, 191)
    return img


def draw_top():
    img, d = new_screen()
    plank(d, 0, 0, 255, 21)
    leaf(d, 70, 8)
    leaf(d, 78, 9, flip=True)
    # info card
    rounded(d, 8, 30, 247, 124, fill=6, border=7, r=6)
    d.line([16, 58, 239, 58], fill=7)
    for yy in (82, 104):
        for xx in range(16, 240, 4):
            d.point((xx, yy), fill=9)
    # small berry + leaf decoration in the card corner
    d.ellipse([228, 36, 233, 41], fill=13)
    d.point((229, 37), fill=15)
    leaf(d, 232, 33, flip=True)
    # selected-for-search card
    rounded(d, 8, 132, 247, 186, fill=6, border=7, r=6)
    rounded(d, 12, 136, 64, 146, fill=14, border=7, r=3)
    leaf(d, 54, 138)
    return img


def to_tiles(img):
    px = img.load()
    tiles = []
    for ty in range(H // 8):
        for tx in range(W // 8):
            tiles.append(tuple(px[tx * 8 + x, ty * 8 + y] for y in range(8) for x in range(8)))
    return tiles


def flips(t):
    rows = [t[i * 8:(i + 1) * 8] for i in range(8)]
    h = tuple(v for r in rows for v in reversed(r))
    v = tuple(v for r in reversed(rows) for v in r)
    hv = tuple(v for r in reversed(rows) for v in reversed(r))
    return [(t, 0), (h, 1), (v, 2), (hv, 3)]


def build(images):
    tileset, index, maps = [], {}, []
    for img in images:
        m = []
        for t in to_tiles(img):
            for cand, flip in flips(t):
                if cand in index:
                    m.append(index[cand] | (flip << 10))
                    break
            else:
                index[t] = len(tileset)
                tileset.append(t)
                m.append(index[t])
        maps.append(m)
    return tileset, maps


def pack(t):
    words = []
    for i in range(0, 64, 8):
        w = 0
        for j in range(8):
            w |= (t[i + j] & 0xF) << (4 * j)
        words.append(w)
    return words


def rgb555(c):
    r, g, b = (v >> 3 for v in c)
    return r | (g << 5) | (b << 10)


def c_array(ctype, name, values, per_line=8, fmt="0x{:04X}"):
    lines = [f"static const {ctype} {name}[{len(values)}] = {{"]
    for i in range(0, len(values), per_line):
        lines.append("    " + ", ".join(fmt.format(v) for v in values[i:i + per_line]) + ",")
    lines.append("};")
    return "\n".join(lines)


def main():
    root = sys.argv[1]
    preview = sys.argv[2] if len(sys.argv) > 2 else None
    top, bottom = draw_top(), draw_bottom()
    tileset, (topMap, bottomMap) = build([top, bottom])

    out = [
        "// Generated by tools/dexnav/dexnav_art.py - do not edit by hand.",
        "#ifndef POKEPLATINUM_DEXNAV_ART_H",
        "#define POKEPLATINUM_DEXNAV_ART_H",
        "",
        f"#define DEXNAV_ART_NUM_TILES {len(tileset)}",
        "",
        c_array("u16", "sDexNavPalBase", [rgb555(c) for c in BASE]),
        c_array("u16", "sDexNavPalSelected", [rgb555(c) for c in SELECTED]),
        c_array("u16", "sDexNavPalText", [rgb555(c) for c in TEXT]),
        c_array("u32", "sDexNavTiles", [w for t in tileset for w in pack(t)], fmt="0x{:08X}"),
        c_array("u16", "sDexNavTopMap", topMap, per_line=16),
        c_array("u16", "sDexNavBottomMap", bottomMap, per_line=16),
        "",
        "#endif // POKEPLATINUM_DEXNAV_ART_H",
        "",
    ]
    with open(os.path.join(root, "src/applications/dexnav_art.h"), "w", newline="\n") as f:
        f.write("\n".join(out))

    if preview:
        os.makedirs(preview, exist_ok=True)
        top.convert("RGB").resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(preview, "dexnav_top.png"))
        bottom.convert("RGB").resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(preview, "dexnav_bottom.png"))

    print(f"{len(tileset)} unique tiles ({len(tileset) * 32} bytes) for 1536 screen tiles")


if __name__ == "__main__":
    main()
