r"""iti_tool.py - Gurumin .iti textures <-> PNG (needs Pillow).

  iti_tool.py decode <in.iti> [out.png] [--packs path\to\packs.dll]
                                             types 0, 1, 888, 889 in Python; 666/667 through the game's own packs.dll
                                             (native/iti_unpack.exe, built with VS2003; packs.dll is looked up in
                                             ..\bin\ next to the .iti if --packs is not given)
  iti_tool.py encode <in.png> [out.iti]      writes type 0 (RGB) or type 1 (RGB + alpha) - raw, no packs.dll needed
  iti_tool.py info <dir>                     type/size histogram

Format (APP.dll Texture_LoadIti 0x10174ce0):
  u32 type, u32 width, u32 height, then
  type 0      : RGB  (w*h*3, top-down)
  type 1      : RGB, then alpha as w*h*3 grey bytes
  type 888    : u32 n, JPEG[n] (decoded by packs.dll BMPLoad), then the same image raw RGB
  type 889    : u32 n, JPEG colour, u32 n2, JPEG alpha, then raw RGB and raw alpha
  type 666/667: u32 n, data packed with packs.dll BMPUnpack (667: + alpha block)
"""
import glob, io, os, struct, subprocess, sys, tempfile
from collections import Counter
from PIL import Image


HELPER = os.path.join(os.path.dirname(os.path.abspath(__file__)), "native", "iti_unpack.exe")


def find_packs(path, explicit=None):
    if explicit:
        return explicit
    d = os.path.dirname(os.path.abspath(path))
    for cand in (os.path.join(d, "..", "bin", "Packs.dll"), os.path.join(d, "Packs.dll"), os.path.join(d, "..", "Packs.dll")):
        if os.path.exists(cand):
            return cand
    return None


def decode_packed(path, packs):
    if not packs or not os.path.exists(HELPER):
        raise ValueError("type 666/667 needs packs.dll (--packs) and native/iti_unpack.exe")
    fd, tmp = tempfile.mkstemp(suffix=".raw")
    os.close(fd)
    try:
        subprocess.run([HELPER, packs, path, tmp], check=True, capture_output=True)
        raw = open(tmp, "rb").read()
    finally:
        os.remove(tmp)
    w, h, alpha = struct.unpack_from("<III", raw, 0)
    img = Image.frombytes("RGB", (w, h), raw[12:12 + w * h * 3])
    if alpha:
        img.putalpha(Image.frombytes("RGB", (w, h), raw[12 + w * h * 3:12 + 2 * w * h * 3]).convert("L"))
    return img


def decode(d, path=None, packs=None):
    t, w, h = struct.unpack_from("<III", d, 0)
    size = w * h * 3
    alpha = None
    if t in (0, 1):
        rgb = d[12:12 + size]
        if t == 1:
            alpha = d[12 + size:12 + 2 * size]
    elif t in (888, 889):
        n, = struct.unpack_from("<I", d, 12)
        off = 16 + n
        if t == 889:
            n2, = struct.unpack_from("<I", d, off)
            off += 4 + n2
        if len(d) >= off + size:
            rgb = d[off:off + size]
            if t == 889 and len(d) >= off + 2 * size:
                alpha = d[off + size:off + 2 * size]
        else:
            rgb = Image.open(io.BytesIO(d[16:16 + n])).convert("RGB").tobytes()
    elif t in (666, 667) and path:
        return t, decode_packed(path, find_packs(path, packs))
    else:
        raise ValueError("type %d not supported yet" % t)
    img = Image.frombytes("RGB", (w, h), rgb)
    if alpha is not None:
        img.putalpha(Image.frombytes("RGB", (w, h), alpha).convert("L"))
    return t, img


def encode(img):
    w, h = img.size
    has_alpha = img.mode in ("RGBA", "LA") or "transparency" in img.info
    out = struct.pack("<III", 1 if has_alpha else 0, w, h) + img.convert("RGB").tobytes()
    if has_alpha:
        out += img.convert("RGBA").getchannel("A").convert("RGB").tobytes()
    return out


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    mode, src = sys.argv[1], sys.argv[2]
    if mode == "decode":
        packs = sys.argv[sys.argv.index("--packs") + 1] if "--packs" in sys.argv else None
        t, img = decode(open(src, "rb").read(), src, packs)
        dst = sys.argv[3] if len(sys.argv) > 3 and not sys.argv[3].startswith("--") else os.path.splitext(src)[0] + ".png"
        img.save(dst)
        print("%s (type %d, %dx%d) -> %s" % (src, t, img.size[0], img.size[1], dst))
    elif mode == "encode":
        dst = sys.argv[3] if len(sys.argv) > 3 else os.path.splitext(src)[0] + ".iti"
        open(dst, "wb").write(encode(Image.open(src)))
        print("%s -> %s" % (src, dst))
    elif mode == "info":
        c = Counter()
        bad = Counter()
        for p in glob.glob(os.path.join(src, "**", "*.iti"), recursive=True):
            d = open(p, "rb").read()
            t = struct.unpack_from("<I", d, 0)[0]
            c[t] += 1
            try:
                decode(d, p)
            except Exception:
                bad[t] += 1
        print("types:", dict(c), "undecodable:", dict(bad))
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
