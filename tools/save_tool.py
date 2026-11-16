r"""save_tool.py - decode/encode Gurumin save files (SaveData\gdataNN.sav). Verified on 7 real saves (decode + byte-identical re-encode).

  save_tool.py decode <gdataNN.sav> [out.bin]    -> game state (312,204 bytes in the saves tested)
  save_tool.py encode <state.bin> <gdataNN.sav> [--key N]
  save_tool.py info   <gdataNN.sav>             map, position, block sizes
  save_tool.py thumb  <gdataNN.sav> [out.png]   the 256x256 load-menu thumbnail (needs Pillow)
  save_tool.py set    <gdataNN.sav> field=value...  fields: money, hp, hp_max, costume, map (writes in place,
                                                 keeps a .bak copy)

Body layout: docs/SAVE_FORMAT.md.

Format (APP.dll 0x10011240 reader, written through 0x1005cdc0 -> 0x1003b2c0):
  u32 magic   9999 (scrambled) or 0x36b1 (old: 0x30000 plain bytes follow)
  u32 key
  body        scrambled with the .itm algorithm (tools/itm_tool.py); the name that keys it is the file's base name
              (e.g. "gdata00") with characters 0, 2, 4 and 6 increased by the low byte of `key`, then upper-cased
  u32 length  of the body (the JP 0x10011240 reader copies only the first 0x30000 bytes)
  u32 sum, u32 xor   checksums of the plain body, as in .itm files
"""
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import itm_tool


def key_name(path, key):
    base = bytearray(os.path.splitext(os.path.basename(path))[0].encode("ascii"))
    for i in (0, 2, 4, 6):
        if i < len(base):
            base[i] = (base[i] + (key & 0xFF)) & 0xFF
    return bytes(b - 32 if 97 <= b <= 122 else b for b in base)


def keystream_for(name_bytes, n):
    t = bytearray(itm_tool.TABLE)
    t[8:16] = name_bytes[:8].ljust(8, b"\0")
    period = bytes(((itm_tool.KEY[i % 15] + (i & 7)) * t[i & 63]) & 0xFF for i in range(960))
    return (period * (n // 960 + 1))[:n]


def decode(data, path):
    magic, key = struct.unpack_from("<II", data, 0)
    if magic == 0x36B1:
        return data[4:4 + 0x30000], True
    if magic != 9999:
        raise ValueError("unknown save magic %d" % magic)
    body = data[8:-12]
    _, want_sum, want_xor = struct.unpack("<III", data[-12:])
    plain = itm_tool.xor_not(body, keystream_for(key_name(path, key), len(body)))
    s, x = itm_tool.checksums(plain)
    return plain, (s == want_sum and x == want_xor)


def encode(plain, path, key=0):
    s, x = itm_tool.checksums(plain)
    body = itm_tool.xor_not(plain, keystream_for(key_name(path, key), len(plain)))
    return struct.pack("<II", 9999, key) + body + struct.pack("<III", len(plain), s, x)


FIELDS = {"money": 0x3016c + 0xb8, "hp": 0x3016c + 4, "hp_max": 0x3016c + 8,
          "costume": 0x302f4 + 0x2714, "map": 0x30000}


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    mode, src = sys.argv[1], sys.argv[2]
    if mode == "decode":
        plain, ok = decode(open(src, "rb").read(), src)
        dst = sys.argv[3] if len(sys.argv) > 3 else src + ".bin"
        open(dst, "wb").write(plain)
        print("%s -> %s (%d bytes, checksum %s)" % (src, dst, len(plain), "ok" if ok else "MISMATCH"))
    elif mode == "encode":
        dst = sys.argv[3]
        key = int(sys.argv[sys.argv.index("--key") + 1]) if "--key" in sys.argv else 0x273
        open(dst, "wb").write(encode(open(src, "rb").read(), dst, key))
        print("%s -> %s" % (src, dst))
    elif mode == "info":
        plain, ok = decode(open(src, "rb").read(), src)
        map_id, = struct.unpack_from("<i", plain, 0x30000)
        x, y, z = struct.unpack_from("<3f", plain, 0x30004)
        hp, hp_max = struct.unpack_from("<ii", plain, 0x3016c + 4)
        money, = struct.unpack_from("<i", plain, FIELDS["money"])
        costume, = struct.unpack_from("<i", plain, FIELDS["costume"])
        print("checksum %s, body %d bytes, map %d, position (%.2f, %.2f, %.2f), HP %d/%d, money %d, costume %d" % ("ok" if ok else "BAD", len(plain), map_id, x, y, z, hp, hp_max, money, costume))
    elif mode == "set":
        data = open(src, "rb").read()
        plain, ok = decode(data, src)
        if not ok:
            sys.exit("checksum mismatch, not touching the file")
        body = bytearray(plain)
        for arg in sys.argv[3:]:
            name, value = arg.split("=")
            struct.pack_into("<i", body, FIELDS[name], int(value, 0))
        key, = struct.unpack_from("<I", data, 4)
        open(src + ".bak", "wb").write(data)
        open(src, "wb").write(encode(bytes(body), src, key))
        print("updated", src, "(backup:", src + ".bak)")
    elif mode == "thumb":
        from PIL import Image
        plain, ok = decode(open(src, "rb").read(), src)
        dst = sys.argv[3] if len(sys.argv) > 3 else os.path.splitext(src)[0] + ".png"
        Image.frombytes("RGB", (256, 256), plain[:0x30000]).save(dst)
        print("wrote", dst)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
