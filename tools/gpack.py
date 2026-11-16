r"""gpack.py - Gurumin packs.dll ExecUnpack (LZ77 with 16-bit flag words) in Python.

unpack(data) takes the buffer handed to ExecUnpack: u32 unpacked size, u32 (progress hint), then blocks of
  u16 block length (only a sanity bound), block bytes, u8 more-blocks flag right where the block's end code stops.
Each block starts with one ignored byte and one byte of 8 flag bits; afterwards flag bits come 16 at a time
(little-endian u16, LSB first), interleaved with data bytes:
  0            literal byte
  1 0 <b>      match, offset b (1..255), length code follows
  1 1 <5 bits><b> offset = bits*256 + b; 0 = end of block, 1 = run: (bit ? 4 bits*256+byte : 4 bits) + 14
               copies of the next byte; else match with that offset
  length code: 1 -> 2, 01 -> 3, 001 -> 4, 0001 -> 5, 00001 + 3 bits -> 6..13, 00000 + byte -> 14..269
Matches never reach into the previous block (the window is reset per block).
Source: Packs.dll 0x100070e0 ExecUnpack, 0x100066d0 (blocks), 0x100063c0 (block decoder).
"""
import struct


class _Bits:
    def __init__(self, data, pos):
        self.d = data
        self.p = pos
        self.buf = data[pos + 1]
        self.n = 8
        self.p += 2

    def bit(self):
        if self.n == 0:
            self.buf = self.d[self.p] | (self.d[self.p + 1] << 8)
            self.p += 2
            self.n = 16
        b = self.buf & 1
        self.buf >>= 1
        self.n -= 1
        return b

    def bits(self, k):
        v = 0
        for _ in range(k):
            v = v * 2 + self.bit()
        return v

    def byte(self):
        b = self.d[self.p]
        self.p += 1
        return b


def _block(data, pos, out_limit):
    r = _Bits(data, pos)
    out = bytearray()
    while True:
        if r.bit() == 0:
            out.append(r.byte())
            continue
        if r.bit() == 0:
            off = r.byte()
        else:
            off = r.bits(5) * 256 + r.byte()
            if off == 0:
                return bytes(out), r.p
            if off == 1:
                n = r.bits(4) * 256 + r.byte() if r.bit() else r.bits(4)
                c = r.byte()
                out += bytes([c]) * (n + 14)
                continue
        if r.bit():
            length = 2
        elif r.bit():
            length = 3
        elif r.bit():
            length = 4
        elif r.bit():
            length = 5
        elif r.bit():
            length = r.bits(3) + 6
        else:
            length = r.byte() + 14
        if len(out) + length > out_limit:
            raise ValueError("match runs past the output size")
        start = len(out) - off
        for i in range(length):
            out.append(out[start + i])


def unpack(data):
    size, _hint = struct.unpack_from("<II", data, 0)
    pos = 8
    out = bytearray()
    while len(out) < size:
        pos += 2
        block, pos = _block(data, pos, size - len(out))
        out += block
        more = data[pos]
        pos += 1
        if not more:
            break
    return bytes(out[:size])
