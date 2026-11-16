r"""itm_tool.py - decrypt/encrypt Gurumin .itm files (map.itm and 3ddata\*.itm scripts).

  itm_tool.py decode <in.itm> [out.txt]
  itm_tool.py encode <in.txt> [out.itm] [--name NAME]

Format (APP.dll 0x10002440 / 0x101615d0): the body is the plain text XOR-scrambled with a key derived
from the string "GURUguruMinmIN_", a 64-byte table whose bytes 8..15 are replaced by the upper-cased file
base name (first 8 characters), and the byte position. A 12-byte trailer follows: int32 unknown
(written as 0), int32 sum of plain bytes, int32 running xor of plain bytes seeded with 0xde2f.
The game checks sum and xor and refuses the file if either differs.
"""
import os
import struct
import sys

KEY = b"GURUguruMinmIN_"
TABLE = bytes.fromhex(
    "21 7e 5f 51 23 78 44 52 67 64 2d 64 26 79 66 67 26 27 28 38 29 28 35 28 35 39 34 65 72 32 66 34"
    " 61 73 66 35 66 36 65 35 66 34 73 35 66 76 77 79 6a 6b 25 66 67 71 54 55 43 46 44 34 35 36 38 72")

def table_for(name):
    base = os.path.splitext(os.path.basename(name))[0].upper().encode("cp932")[:8]
    t = bytearray(TABLE)
    t[8:16] = base.ljust(8, b"\0")
    return t


def keystream(name, n):
    t = table_for(name)
    period = bytes(((KEY[i % 15] + (i & 7)) * t[i & 63]) & 0xFF for i in range(960))
    return (period * (n // 960 + 1))[:n]


def xor_not(data, ks):
    n = len(data)
    if n == 0:
        return b""
    v = int.from_bytes(data, "little") ^ int.from_bytes(ks, "little") ^ ((1 << (8 * n)) - 1)
    return v.to_bytes(n, "little")


def checksums(plain):
    x = 0
    for b in plain:
        x ^= b
    return sum(plain) & 0xFFFFFFFF, 0xDE2F ^ x

def decode(data, name):
    body, trailer = data[:-12], data[-12:]
    unknown, want_sum, want_xor = struct.unpack("<iII", trailer)
    plain = xor_not(body, keystream(name, len(body)))
    s, x = checksums(plain)
    return plain, s == want_sum and x == want_xor, unknown


def encode(text, name, unknown=0):
    s, x = checksums(text)
    return xor_not(text, keystream(name, len(text))) + struct.pack("<iII", unknown, s, x)

def main():
    if len(sys.argv) < 3 or sys.argv[1] not in ("decode", "encode"):
        sys.exit(__doc__)
    mode, src = sys.argv[1], sys.argv[2]
    args = [a for a in sys.argv[3:] if not a.startswith("--")]
    name = src
    if "--name" in sys.argv:
        name = sys.argv[sys.argv.index("--name") + 1]
        args = [a for a in args if a != name]
    data = open(src, "rb").read()
    if mode == "decode":
        dst = args[0] if args else os.path.splitext(src)[0] + ".txt"
        plain, ok, _ = decode(data, name)
        open(dst, "wb").write(plain)
        print("%s -> %s (%d bytes, checksum %s)" % (src, dst, len(plain), "ok" if ok else "MISMATCH"))
    else:
        dst = args[0] if args else os.path.splitext(src)[0] + ".itm"
        open(dst, "wb").write(encode(data, name))
        print("%s -> %s" % (src, dst))


if __name__ == "__main__":
    main()




