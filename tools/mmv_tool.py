r"""mmv_tool.py - decode Gurumin .mmv movies (needs Pillow).

  mmv_tool.py info <file.mmv>
  mmv_tool.py frames <file.mmv> <out_dir> [--every N]   writes frame_NNNN.png
  mmv_tool.py gif <file.mmv> <out.gif> [--fps 15]

Format (APP.dll Mmv_Open 0x10160a90, frame decoder 0x10160a20):
  "MV", u16 width, u16 height, 256 palette entries of 4 bytes (B, G, R, unused) at offset 6,
  then frames: u32 length + length bytes of RLE that update the previous frame in place:
    0xFC n     skip n pixels (keep the previous frame's pixels)
    0xFD n v   n copies of palette index v
    other b    one pixel of palette index b
The game converts the palette to RGB565 and forces entry 255 to 0xFBFF.
"""
import os, struct, sys


def parse(data):
    if data[:2] != b"MV":
        raise ValueError("not an MV file")
    w, h = struct.unpack_from("<HH", data, 2)
    palette = []
    for i in range(256):
        b, g, r = data[6 + 4 * i:9 + 4 * i]
        palette += [r, g, b]
    return w, h, palette


def frames(data):
    w, h, _ = parse(data)
    pixels = bytearray(w * h)
    pos = 0x406
    while pos + 4 <= len(data):
        length, = struct.unpack_from("<I", data, pos)
        if length == 0 or pos + 4 + length > len(data):
            break
        src = data[pos + 4:pos + 4 + length]
        i = o = 0
        while i < len(src) and o <= len(pixels):
            c = src[i]
            if c == 0xFC:
                o += src[i + 1]
                i += 2
            elif c == 0xFD:
                n, v = src[i + 1], src[i + 2]
                pixels[o:o + n] = bytes([v]) * n
                o += n
                i += 3
            else:
                if o < len(pixels):
                    pixels[o] = c
                o += 1
                i += 1
        yield bytes(pixels[:w * h])
        pos += 4 + length


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    mode, src = sys.argv[1], sys.argv[2]
    data = open(src, "rb").read()
    w, h, palette = parse(data)
    if mode == "info":
        n = sum(1 for _ in frames(data))
        print("%s: %dx%d, %d frames" % (src, w, h, n))
        return
    from PIL import Image

    def image(px):
        im = Image.frombytes("P", (w, h), px)
        im.putpalette(palette)
        return im
    if mode == "frames":
        out = sys.argv[3]
        every = int(sys.argv[sys.argv.index("--every") + 1]) if "--every" in sys.argv else 1
        os.makedirs(out, exist_ok=True)
        for k, px in enumerate(frames(data)):
            if k % every == 0:
                image(px).save(os.path.join(out, "frame_%04d.png" % k))
    elif mode == "gif":
        fps = float(sys.argv[sys.argv.index("--fps") + 1]) if "--fps" in sys.argv else 15
        ims = [image(px).convert("RGB") for px in frames(data)]
        ims[0].save(sys.argv[3], save_all=True, append_images=ims[1:], duration=int(1000 / fps), loop=0)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
