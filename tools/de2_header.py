r"""de2_header.py - the name-keyed header check of Gurumin .de2 music files (APP.dll 0x10001cb0).

  de2_header.py check <dir-or-file>...    verify every .de2 header against its file name
  de2_header.py expect <name>             the two header words a file called <name>.de2 must start with

Header: u32 sum, u32 xor, u32 ?, u32 (length xor the second word) of an extra block read by Bgm_Play.
sum/xor are computed from the upper-cased base name (8 characters, zero padded) mixed with fixed constants; the
audio itself is decoded by soundarc.dll. A file whose header does not match makes the game's anti-tamper code
close it a while later, so renamed or new music must get a matching header.
"""
import glob, os, struct, sys


def expected(name):
    n = os.path.splitext(os.path.basename(name))[0].upper().encode("cp932")[:8].ljust(8, b"\0")
    k = bytearray(32)
    p = n
    k[1] = p[4] ^ 0x0D; k[0] = p[6] ^ 0x0F; k[2] = p[5] ^ 0x10; k[4] = p[5] ^ 0x11; k[3] = p[1] ^ 0x42
    k[6] = p[7] ^ 0x3F; k[5] = p[3] ^ 0x2A; k[8] = p[3] ^ 0x23; k[7] = p[2] ^ 0x25; k[9] = p[1] ^ 0x49
    k[10] = p[0] ^ 0x2B; k[14] = p[7] ^ 0x2B; k[15] = p[2] ^ 3; k[11] = p[1] ^ 0x57; k[12] = p[1] ^ 0x4D
    k[19] = p[2] ^ 0x53; k[17] = k[6]; k[18] = p[4] ^ 0x0C; k[13] = p[4] ^ 0x11; k[25] = p[1] ^ 0x3E
    k[16] = p[6] ^ 0x3F; k[24] = p[5] ^ 0x18; k[20] = p[5] ^ 0x35; k[28] = p[3] ^ 1; k[31] = p[4] ^ 0x17
    k[21] = p[6] ^ 0x2C; k[22] = 0x0E; k[27] = 0x49; k[29] = 0x2C; k[23] = p[3] ^ 0x49; k[30] = p[0] ^ 0x71
    total, mixed = 0, 0x37
    for b in k:
        s = b - 256 if b >= 128 else b
        total += s
        mixed ^= s
    return total & 0xFFFFFFFF, mixed & 0xFFFFFFFF


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    if sys.argv[1] == "expect":
        print("sum 0x%08x xor 0x%08x" % expected(sys.argv[2]))
        return
    ok = bad = 0
    for a in sys.argv[2:]:
        files = glob.glob(os.path.join(a, "**", "*.de2"), recursive=True) if os.path.isdir(a) else [a]
        for p in files:
            s, x = struct.unpack_from("<II", open(p, "rb").read(8))
            if (s, x) == expected(p):
                ok += 1
            else:
                bad += 1
                print("mismatch", p, hex(s), hex(x), [hex(v) for v in expected(p)])
    print("ok", ok, "mismatch", bad)


if __name__ == "__main__":
    main()
