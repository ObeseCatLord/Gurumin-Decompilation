r"""de2_tool.py - Gurumin .de2 music <-> WAV.

  de2_tool.py info   <file.de2>
  de2_tool.py decode <file.de2> [out.wav]        -> 16-bit PCM WAV
  de2_tool.py encode <in.wav> <out.de2> [--template orig.de2]
                                                  44.1 kHz stereo 16-bit PCM -> .de2 (MS ADPCM); the sync block is
                                                  copied from --template (or left empty)

Layout (sound3d.dll 0x10004c20, APP.dll Bgm_Play 0x1003aef0):
  u32 sum, u32 xor (name-keyed, tools/de2_header.py), u32 RIFF size, u32 block length ^ xor
  block (sync data for Bgm_Play)
  RIFF/WAVE: fmt = PCM 44100 Hz stereo 16-bit; 'data' holds chunks of
     u32 payload size (<= 0x5000), u32 decoded size (<= 0x13e20), payload = Microsoft ADPCM blocks of 2048 bytes
     (stereo, 2036 samples per block, standard 7 coefficient pairs) decoded by the Windows ACM.
"""
import array, os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import de2_header

COEF = [(256, 0), (512, -256), (0, 0), (192, 64), (240, 0), (460, -208), (392, -232)]
ADAPT = [230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230]
BLOCK = 2048
SAMPLES = (BLOCK - 14) * 2 // 2 + 2   # per channel


def split(data):
    s, x, riff_len, blk = struct.unpack_from("<4I", data, 0)
    block_len = blk ^ x
    riff = 16 + block_len
    return data[16:riff], data[riff:]


def decode_block(b, out):
    pred = [b[0], b[1]]
    delta = list(struct.unpack_from("<hh", b, 2))
    s1 = list(struct.unpack_from("<hh", b, 6))
    s2 = list(struct.unpack_from("<hh", b, 10))
    out.extend((s2[0], s2[1], s1[0], s1[1]))
    for byte in b[14:]:
        for ch, nib in ((0, byte >> 4), (1, byte & 15)):
            c1, c2 = COEF[pred[ch]]
            p = (s1[ch] * c1 + s2[ch] * c2) >> 8
            sn = nib - 16 if nib & 8 else nib
            p += sn * delta[ch]
            p = max(-32768, min(32767, p))
            out.append(p)
            s2[ch], s1[ch] = s1[ch], p
            delta[ch] = max(16, (ADAPT[nib] * delta[ch]) >> 8)


def decode(data):
    _, riff = split(data)
    di = riff.find(b"data")
    pos = di + 8
    end = len(riff)
    pcm = array.array("h")
    while pos + 8 <= end:
        size, decoded = struct.unpack_from("<II", riff, pos)
        if size == 0 or size > 0x5000 or pos + 8 + size > end:
            break
        chunk = riff[pos + 8:pos + 8 + size]
        start = len(pcm)
        for k in range(0, len(chunk) - 13, BLOCK):
            decode_block(chunk[k:k + BLOCK], pcm)
        del pcm[start + decoded // 2:]
        pos += 8 + size
    return pcm


def wav(pcm):
    raw = pcm.tobytes()
    return struct.pack("<4sI4s4sIHHIIHH4sI", b"RIFF", 36 + len(raw), b"WAVE", b"fmt ", 16, 1, 2, 44100,
                       176400, 4, 16, b"data", len(raw)) + raw


def _encode_channel(values, pred):
    c1, c2 = COEF[pred]
    first = values[2:]
    delta = max(16, abs(first[0] - values[1]) // 2) if first else 16
    delta = min(delta, 0x7FFF)
    s2, s1 = values[0], values[1]
    nibbles, err = [], 0
    d0 = delta
    for v in first:
        p = (s1 * c1 + s2 * c2) >> 8
        q = int(round((v - p) / delta))
        q = max(-8, min(7, q))
        nib = q & 15
        r = max(-32768, min(32767, p + q * delta))
        err += (v - r) * (v - r)
        s2, s1 = s1, r
        delta = max(16, (ADAPT[nib] * delta) >> 8)
        nibbles.append(nib)
    return err, d0, nibbles


def encode_block(samples):
    """samples: SAMPLES (l, r) frames -> one 2048-byte MS ADPCM block (best of the 7 predictors per channel)."""
    chans = []
    for ch in (0, 1):
        values = [f[ch] for f in samples]
        best = min((_encode_channel(values, p) + (p,) for p in range(7)), key=lambda r: r[0])
        chans.append((best[3], best[1], best[2], values))
    (p0, d0, n0, v0), (p1, d1, n1, v1) = chans
    out = bytearray([p0, p1]) + struct.pack("<hhhhhh", d0, d1, v0[1], v1[1], v0[0], v1[0])
    for a, b in zip(n0, n1):
        out.append((a << 4) | b)
    return bytes(out.ljust(BLOCK, b"\0"))

def encode(pcm_bytes, name, template=None):
    frames = array.array("h", pcm_bytes)
    pairs = list(zip(frames[0::2], frames[1::2]))
    if len(pairs) % SAMPLES:
        pairs += [(0, 0)] * (SAMPLES - len(pairs) % SAMPLES)
    body = bytearray()
    total_pcm = 0
    for c in range(0, len(pairs), SAMPLES * 10):
        group = pairs[c:c + SAMPLES * 10]
        payload = b"".join(encode_block(group[k:k + SAMPLES]) for k in range(0, len(group), SAMPLES))
        decoded = len(group) * 4
        body += struct.pack("<II", len(payload), decoded) + payload
        total_pcm += decoded
    fmt = struct.pack("<HHIIHH", 1, 2, 44100, 176400, 4, 16) + b"\0\0"
    riff = b"RIFF" + struct.pack("<I", 4 + 8 + len(fmt) + 8 + len(body)) + b"WAVE" + b"fmt " + \
        struct.pack("<I", len(fmt)) + fmt + b"data" + struct.pack("<I", total_pcm) + bytes(body)
    block = split(template)[0] if template else b""
    s, x = de2_header.expected(name)
    return struct.pack("<4I", s, x, len(riff), len(block) ^ x) + block + riff


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    mode, src = sys.argv[1], sys.argv[2]
    if mode == "info":
        d = open(src, "rb").read()
        block, riff = split(d)
        print("%s: sync block %d bytes, RIFF %d bytes, header %s" % (src, len(block), len(riff),
              "ok" if struct.unpack_from("<II", d) == de2_header.expected(src) else "MISMATCH"))
    elif mode == "decode":
        pcm = decode(open(src, "rb").read())
        dst = sys.argv[3] if len(sys.argv) > 3 else os.path.splitext(src)[0] + ".wav"
        open(dst, "wb").write(wav(pcm))
        print("%s -> %s (%.1f s)" % (src, dst, len(pcm) / 2 / 44100))
    elif mode == "encode":
        dst = sys.argv[3]
        w = open(src, "rb").read()
        di = w.find(b"data")
        template = open(sys.argv[sys.argv.index("--template") + 1], "rb").read() if "--template" in sys.argv else None
        open(dst, "wb").write(encode(w[di + 8:], dst, template))
        print("%s -> %s" % (src, dst))
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
