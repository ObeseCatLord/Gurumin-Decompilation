r"""save_tool.py - decode/encode Gurumin save files (SaveData\gdataNN.sav). UNTESTED: no save files were available.

  save_tool.py decode <gdataNN.sav> [out.bin]    -> 0x30000-byte game state
  save_tool.py encode <state.bin> <gdataNN.sav> [--key N]

Format (APP.dll 0x10011240 reader, written through 0x1005cdc0 -> 0x1003b2c0):
  u32 magic   9999 (scrambled) or 0x36b1 (old: 0x30000 plain bytes follow)
  u32 key
  body        scrambled with the .itm algorithm (tools/itm_tool.py); the name that keys it is the file's base name
              (e.g. "gdata00") with characters 0, 2, 4 and 6 increased by the low byte of `key`, then upper-cased
  u32 ?       (reader adds 0x30000 to it and ignores it)
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
    return struct.pack("<II", 9999, key) + body + struct.pack("<III", 0, s, x)


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
        key = int(sys.argv[sys.argv.index("--key") + 1]) if "--key" in sys.argv else 0
        open(dst, "wb").write(encode(open(src, "rb").read(), dst, key))
        print("%s -> %s" % (src, dst))
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
