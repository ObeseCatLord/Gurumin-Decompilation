r"""mmv_tool.py - decode and encode Gurumin .mmv movies (needs Pillow).

  mmv_tool.py info <file.mmv>
  mmv_tool.py frames <file.mmv> <out_dir> [--every N]   writes frame_NNNN.png
  mmv_tool.py gif <file.mmv> <out.gif> [--fps 15]
  mmv_tool.py encode <frames_dir | in.gif> <out.mmv> [--template orig.mmv] [--dither] [--original-frame1]
                                         PNG frames (sorted by name) or an animated GIF -> .mmv.
                                         Frames written by `frames` keep their palette and indexes exactly.
                                         Other images are reduced to 255 colours: the --template movie's palette
                                         if given (recommended: same colours, and the size is checked), else one
                                         palette made from all frames. --dither: Floyd-Steinberg instead of nearest.
                                         --original-frame1: frame 1 as in the original files (see below); with
                                         unedited frames the rebuilt movie is then byte-identical.
                                         HD: frames exactly 2x or 4x the --template movie's size make an HD movie
                                         (--scale 2|4 sets it without a template). HD movies need the mod DLL
                                         (mod/, docs/MOVIES_HD.md): it shows them at the original size with the
                                         extra detail. Without the DLL the game would draw them 2x or 4x too big.

HD movies: the unused fourth bytes of palette entries 0, 1 and 2 hold "H", "D" and the scale (2 or 4); the game
ignores them (no original movie uses them).

Format (APP.dll Mmv_Open 0x10160a90, frame decoder 0x10160a20):
  "MV", u16 width, u16 height, 256 palette entries of 4 bytes (B, G, R, unused) at offset 6,
  then frames: u32 length + length bytes of RLE that update the previous frame in place:
    0xFC n     skip n pixels (keep the previous frame's pixels)
    0xFD n v   n copies of palette index v
    other b    one pixel of palette index b
The game converts the palette to RGB565 and forces entry 255 to 0xFBFF.

Encoder rules (from MmvPlayer::Open/ReadFrame/DecodeFrameRle in decomp/src/mmv_texreg.cpp and the original files):
  - the frame buffer is not cleared, so frame 0 paints every pixel (no skips);
  - a movie held in memory loops back to frame 1 (not 0) and decodes it over the last frame, so frame 1 only skips
    pixels that are unchanged against both frame 0 and the last frame;
  - the decoder is a do-while over exactly `length` bytes: a frame is never empty (length 0 would hang the game)
    and the codes add up to the length exactly; `0xFD 0 v` would write 256 pixels, so run lengths are 1..255;
  - codes as in the original files (all 67 JP movies rebuild byte-identical with --original-frame1): an equal run
    of 3+ pixels becomes 0xFD when it reaches past the unchanged stretch, else unchanged stretches of 2+ become
    0xFC; pairs are two literals; runs over 255 are split; indexes 0xFC/0xFD always go through 0xFD; palette
    index 255 is not used (the game replaces its colour);
  - the original files encode frame 1 against frame 0 only (by default the encoder plays safe, see above);
  - streamed movies reject frames longer than 0x25800 bytes (a warning is printed).
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


MAX_RUN = 255
MAX_STREAMED_FRAME = 0x25800


def encode_frame(pixels, can_skip=None):
    """RLE codes for one frame, chosen like the original encoder (byte-identical on the game's movies).
    can_skip[i] true: pixel i may keep the previous frame's value (None: paint every pixel)."""
    out = bytearray()

    def chunks(code, total, value=None):
        # runs longer than 255 are split into 255-pixel pieces (0xFD 0 would mean 256 to the game)
        while total > 0:
            n = min(total, MAX_RUN)
            out.extend(bytes((code, n)) if value is None else bytes((code, n, value)))
            total -= n

    size = len(pixels)
    o = 0
    while o < size:
        unchanged = 0
        if can_skip is not None:
            while o + unchanged < size and can_skip[o + unchanged]:
                unchanged += 1
        v = pixels[o]
        same = 1
        while o + same < size and pixels[o + same] == v:
            same += 1
        if same >= 3 and same > unchanged:
            chunks(0xFD, same, v)            # a run wins when it reaches past the unchanged stretch
            o += same
        elif unchanged >= 2:
            chunks(0xFC, unchanged)
            o += unchanged
        elif v in (0xFC, 0xFD):
            chunks(0xFD, same, v)            # these indexes can't be literals
            o += same
        elif same == 2:
            out += bytes((v, v))             # a pair is written as two literals
            o += 2
        else:
            out.append(v)
            o += 1
    if not out:
        out += bytes((0xFC, 1))              # never an empty frame
    return bytes(out)


HD_SCALES = (2, 4)


def hd_scale(data):
    """Scale of an HD movie (its palette's spare bytes hold "H", "D", scale), else 1."""
    if len(data) > 17 and data[9:10] == b"H" and data[13:14] == b"D" and data[17] in HD_SCALES:
        return data[17]
    return 1


def scale_for(size, original_size):
    """1 for the original size, 2 or 4 for exactly that many times it, else None."""
    for scale in (1,) + HD_SCALES:
        if size == (original_size[0] * scale, original_size[1] * scale):
            return scale
    return None


def encode(w, h, palette, frame_list, loop_safe=True, scale=1):
    """palette: 768 RGB values; frame_list: bytes of w*h palette indexes each -> .mmv file bytes.
    loop_safe=False encodes frame 1 against frame 0 only, as the original files do (byte-identical rebuilds).
    scale 2 or 4 marks an HD movie (shown at w/scale x h/scale by the mod DLL)."""
    head = bytearray(b"MV" + struct.pack("<HH", w, h))
    marker = {0: ord("H"), 1: ord("D"), 2: scale} if scale > 1 else {}
    for i in range(256):
        r, g, b = palette[3 * i:3 * i + 3] if i < 255 else (0, 0, 0)
        head += bytes((b, g, r, marker.get(i, 0)))
    body = bytearray()
    last = frame_list[-1]
    for k, px in enumerate(frame_list):
        if k == 0:
            mask = None
        elif k == 1 and loop_safe:
            prev = frame_list[0]
            mask = [a == b and a == c for a, b, c in zip(px, prev, last)]
        else:
            prev = frame_list[k - 1]
            mask = [a == b for a, b in zip(px, prev)]
        codes = encode_frame(px, mask)
        if len(codes) > MAX_STREAMED_FRAME:
            print("warning: frame %d is %d bytes (streamed movies allow %d)" % (k, len(codes), MAX_STREAMED_FRAME))
        body += struct.pack("<I", len(codes)) + codes
    return bytes(head + body)


def load_images(src):
    from PIL import Image, ImageSequence
    if os.path.isdir(src):
        names = sorted(n for n in os.listdir(src) if n.lower().endswith(".png"))
        if not names:
            sys.exit("no .png frames in " + src)
        images = []
        for n in names:
            with Image.open(os.path.join(src, n)) as im:
                im.load()
                images.append(im.copy())
        return images
    with Image.open(src) as im:
        frames_and_times = [(frame.copy(), frame.info.get("duration", 0)) for frame in ImageSequence.Iterator(im)]
    # GIF writers merge identical consecutive frames into one longer frame: repeat those to get the frames back
    times = [t for _, t in frames_and_times]
    base = gif_frame_time(times)
    return [frame for frame, t in frames_and_times for _ in range(max(1, int(round(t / base))) if base else 1)]


def gif_frame_time(times):
    """The movie's frame time in ms from GIF frame durations (stored in whole centiseconds, so a 66 ms frame reads
    as 60 and three merged frames as 190): the time in [shortest, shortest + 10) that explains most durations."""
    positive = [t for t in times if t > 0]
    if not positive:
        return 0
    shortest = min(positive)

    def fits(b):
        return sum(1 for t in positive if any(int(k * b) // 10 * 10 == t for k in (round(t / b), round(t / b) + 1)))
    return max((shortest + d / 4.0 for d in range(40)), key=lambda b: (fits(b), -b))


def padded_palette(image):
    pal = list(image.getpalette() or [])
    return (pal + [0] * 768)[:768]


def indexed_frames(images):
    """Frames that are already palettised with one shared palette (as `frames` writes them): used as they are."""
    if not all(im.mode == "P" for im in images):
        return None
    pal = padded_palette(images[0])
    if any(padded_palette(im) != pal for im in images[1:]):
        return None
    frames_px = [im.tobytes() for im in images]
    if any(b"\xff" in px for px in frames_px):
        return None      # index 255 is replaced by the game: reduce the colours instead
    return pal, frames_px


def quantised_frames(images, palette=None, dither=False):
    """Reduce RGB frames to palette indexes 0..254 (palette: 768 values, or None to build one from the frames)."""
    from PIL import Image
    rgb = [im.convert("RGB") for im in images]
    if palette is None:
        step = max(1, len(rgb) // 32)
        sample = rgb[::step]
        w, h = sample[0].size
        sheet = Image.new("RGB", (w, h * len(sample)))
        for k, im in enumerate(sample):
            sheet.paste(im, (0, k * h))
        palette = padded_palette(sheet.quantize(colors=255, method=Image.Quantize.MEDIANCUT,
                                                dither=Image.Dither.NONE))
    palette = palette[:765] + palette[0:3]          # entry 255 = copy of entry 0, so it is never needed
    if dither:
        holder = Image.new("P", (1, 1))
        holder.putpalette(palette)
        table = bytes(range(255)) + b"\x00"        # any 255 picked anyway -> its twin, entry 0
        frames_px = [im.quantize(palette=holder, dither=Image.Dither.FLOYDSTEINBERG).tobytes().translate(table)
                     for im in rgb]
    else:
        frames_px = [nearest_indexes(im.tobytes(), palette) for im in rgb]
    return palette, frames_px


def nearest_indexes(rgb_bytes, palette):
    """Exact nearest palette entry (0..254) per pixel; exact colours map to their first entry. (Pillow's own
    conversion to a given palette works at reduced precision and picks neighbouring colours.)"""
    entries = [tuple(palette[3 * i:3 * i + 3]) for i in range(255)]
    cache = {}
    for i in range(254, -1, -1):
        cache[bytes(entries[i])] = i
    out = bytearray(len(rgb_bytes) // 3)
    for j in range(len(out)):
        colour = rgb_bytes[3 * j:3 * j + 3]
        index = cache.get(colour)
        if index is None:
            r, g, b = colour
            index = min(range(255), key=lambda i: (entries[i][0] - r) ** 2 + (entries[i][1] - g) ** 2
                        + (entries[i][2] - b) ** 2)
            cache[colour] = index
        out[j] = index
    return bytes(out)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    mode, src = sys.argv[1], sys.argv[2]
    if mode == "encode":
        if len(sys.argv) < 4:
            sys.exit(__doc__)
        dst = sys.argv[3]
        images = load_images(src)
        w, h = images[0].size
        if any(im.size != (w, h) for im in images):
            sys.exit("all frames must have the same size")
        template = sys.argv[sys.argv.index("--template") + 1] if "--template" in sys.argv else None
        template_palette = None
        scale = int(sys.argv[sys.argv.index("--scale") + 1]) if "--scale" in sys.argv else None
        if scale not in (None, 1) + HD_SCALES:
            sys.exit("--scale must be 1, 2 or 4")
        if template:
            template_data = open(template, "rb").read()
            tw, th, template_palette = parse(template_data)
            ts = hd_scale(template_data)        # an HD template: compare against its original size
            original = (tw // ts, th // ts)
            if scale is None:
                scale = scale_for((w, h), original)
                if scale is None:
                    print("warning: frames are %dx%d, the template movie is %dx%d (not 1x, 2x or 4x): "
                          "the game will draw it at %dx%d" % (w, h, original[0], original[1], w, h))
            elif (w, h) != (original[0] * scale, original[1] * scale):
                print("warning: frames are %dx%d, %dx the template movie would be %dx%d"
                      % (w, h, scale, original[0] * scale, original[1] * scale))
        scale = scale or 1
        if w % scale or h % scale:
            sys.exit("an HD x%d movie needs a size divisible by %d (frames are %dx%d)" % (scale, scale, w, h))
        exact = indexed_frames(images)
        if exact:
            palette, frame_list = exact
            how = "kept the frames' own palette"
        else:
            palette, frame_list = quantised_frames(images, template_palette, "--dither" in sys.argv)
            how = "colours reduced to the template's palette" if template else "colours reduced to 255"
        out = encode(w, h, palette, frame_list, "--original-frame1" not in sys.argv, scale)
        open(dst, "wb").write(out)
        hd = ", HD x%d: needs the mod DLL" % scale if scale > 1 else ""
        print("%s -> %s (%dx%d, %d frames, %d bytes, %s%s)" % (src, dst, w, h, len(frame_list), len(out), how, hd))
        return
    data = open(src, "rb").read()
    w, h, palette = parse(data)
    if mode == "info":
        n = sum(1 for _ in frames(data))
        scale = hd_scale(data)
        hd = ", HD x%d (shown as %dx%d with the mod DLL)" % (scale, w // scale, h // scale) if scale > 1 else ""
        print("%s: %dx%d, %d frames%s" % (src, w, h, n, hd))
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
        ims = [image(px) for px in frames(data)]     # palettised: the GIF keeps the movie's exact colours
        ims[0].save(sys.argv[3], save_all=True, append_images=ims[1:], duration=int(1000 / fps), loop=0)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
