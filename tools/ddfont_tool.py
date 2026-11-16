r"""ddfont_tool.py - the game's pre-rendered font (3ddata\ddfont8.dat, ddfont8s.dat) <-> PNG atlas (needs Pillow).

  ddfont_tool.py export <ddfont8.dat> <atlas.png>             writes atlas.png + atlas.tsv (index, sjis, width)
  ddfont_tool.py import <ddfont8.dat> <atlas.png> <out.dat>   rebuilds the font from an edited atlas + its .tsv

Format (APP.dll text renderer near 0x10145380):
  one 144-byte record per Shift-JIS code: index = code - 0x8141 (codes >= 0xE040: code - 0x4043), 15,048 records.
  record = 72 little-endian u16 words: 17 rows x 4 words; each word holds 5 pixels of 3-bit coverage (0..7, pixel k in
  bits 3k..3k+2); the glyph advance width is bits 9..13 of word 3 (so a glyph uses at most 18 columns).
  The renderer turns coverage into ARGB4444 alpha (v*0x2000+0x1000).
Atlas: 64 glyphs per row, cells of 20x17 pixels, grey = coverage*36.
"""
import os, struct, sys
from PIL import Image

RECORD, ROWS, COLS, CELL_W = 144, 17, 64, 20


def code_of(index):
    c = index + 0x8141
    return c if c < 0xE040 else index + 0x4043


def export(dat, png):
    d = open(dat, "rb").read()
    n = len(d) // RECORD
    img = Image.new("L", (COLS * CELL_W, ((n + COLS - 1) // COLS) * ROWS), 0)
    px = img.load()
    with open(os.path.splitext(png)[0] + ".tsv", "w") as f:
        f.write("index\tsjis\twidth\n")
        for i in range(n):
            words = struct.unpack_from("<72H", d, i * RECORD)
            w = (words[3] >> 9) & 0x1F
            f.write("%d\t%04x\t%d\n" % (i, code_of(i), w))
            ox, oy = (i % COLS) * CELL_W, (i // COLS) * ROWS
            for row in range(ROWS):
                for x in range(min(w, 18)):
                    v = (words[row * 4 + x // 5] >> ((x % 5) * 3)) & 7
                    px[ox + x, oy + row] = v * 36
    img.save(png)
    print("exported", n, "glyphs")


def build(png, tsv, template):
    d = bytearray(template)
    img = Image.open(png).convert("L")
    px = img.load()
    widths = {}
    for line in open(tsv).read().splitlines()[1:]:
        i, _, w = line.split("\t")
        widths[int(i)] = int(w)
    for i, w in widths.items():
        ox, oy = (i % COLS) * CELL_W, (i // COLS) * ROWS
        words = [0] * 72
        for row in range(ROWS):
            for x in range(min(w, 18)):
                v = min(7, (px[ox + x, oy + row] + 18) // 36)
                words[row * 4 + x // 5] |= v << ((x % 5) * 3)
        words[3] = (words[3] & 0x1FF) | ((w & 0x1F) << 9)
        struct.pack_into("<72H", d, i * RECORD, *words)
    return bytes(d)


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    if sys.argv[1] == "export":
        export(sys.argv[2], sys.argv[3])
    elif sys.argv[1] == "import":
        png = sys.argv[3]
        out = build(png, os.path.splitext(png)[0] + ".tsv", open(sys.argv[2], "rb").read())
        open(sys.argv[4], "wb").write(out)
        print("wrote", sys.argv[4])
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
