r"""it3_tool.py - inspect Gurumin .it3 models and export their geometry.

  it3_tool.py info <file.it3>             node list: name, chunks, mesh sizes
  it3_tool.py obj  <file.it3> [out.obj]   positions + triangles of every VPA2 mesh (node-local coordinates)

Format (APP.dll It3_ParseChunks 0x101a58a0): a flat list of chunks `tag[4] u32 size data[size]`.
Each INFO chunk starts a node: name[64], float matrix[16] (row-major 4x4), u32 flags, f32, u32.
Following chunks belong to that node (RTY2 render type, LIG3 lighting, KAN3/KAN2/KANI key animation,
MAN2/MANI, MAT2/3/4/MATE materials, VPA2/VPAC meshes, CHID children, BONE/BON2, CAME, CDLO, IKNG, PLU2/PLUG, INFS).
VPA2: u32 packCount; per pack: u32 vertexCount, 0x70-byte pack header, then (if vertexCount)
u32 packedSize + ExecUnpack data of vertexCount*0x70 bytes, then u32 ?, u32 indexCount,
(if indexCount) u32 packedSize + ExecUnpack data of indexCount u32 indices (triangle list).
Vertex = 28 floats (0x70 bytes), per the vertex initialiser 0x101acf10: 0-3 position xyzw (w=1), 4-6 normal (7 = 1),
8-11 colour RGBA (0..1), 12-13 texture UV, 14-23 skinning data (bone indices suspected in 14-15/20-21), 24-27 blend weights.
"""
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gpack


def chunks(d):
    off = 0
    while off + 8 <= len(d):
        tag = d[off:off + 4].decode("latin-1")
        size, = struct.unpack_from("<I", d, off + 4)
        yield tag, d[off + 8:off + 8 + size]
        off += 8 + size


def read_vpa2(body):
    n, = struct.unpack_from("<I", body, 0)
    q = 4
    packs = []
    for _ in range(n):
        vc, = struct.unpack_from("<I", body, q)
        q += 4 + 0x70
        verts = b""
        if vc:
            cs, = struct.unpack_from("<I", body, q)
            verts = gpack.unpack(body[q + 4:q + 4 + cs])
            q += 4 + cs
        _, ic = struct.unpack_from("<II", body, q)
        q += 8
        idx = ()
        if ic > 0:
            cs, = struct.unpack_from("<I", body, q)
            raw = gpack.unpack(body[q + 4:q + 4 + cs])
            idx = struct.unpack("<%dI" % ic, raw[:ic * 4])
            q += 4 + cs
        packs.append((vc, verts, idx))
    return packs


def nodes(d):
    out = []
    cur = None
    for tag, body in chunks(d):
        if tag == "INFO":
            name = body[:64].split(b"\0")[0].decode("cp932", "replace")
            cur = {"name": name, "matrix": struct.unpack_from("<16f", body, 64), "chunks": [], "packs": []}
            out.append(cur)
        elif cur is not None:
            cur["chunks"].append((tag, len(body)))
            if tag == "VPA2":
                cur["packs"] = read_vpa2(body)
    return out


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    mode, src = sys.argv[1], sys.argv[2]
    d = open(src, "rb").read()
    if mode == "info":
        for n in nodes(d):
            mesh = ", ".join("%dv/%dt" % (vc, len(idx) // 3) for vc, _, idx in n["packs"])
            print("%-28s %-40s %s" % (n["name"], " ".join(t for t, _ in n["chunks"]), mesh))
    elif mode == "obj":
        dst = sys.argv[3] if len(sys.argv) > 3 else os.path.splitext(src)[0] + ".obj"
        base = 1
        with open(dst, "w") as f:
            for n in nodes(d):
                for k, (vc, verts, idx) in enumerate(n["packs"]):
                    if not vc:
                        continue
                    f.write("o %s_%d\n" % (n["name"].replace(" ", "_"), k))
                    for i in range(vc):
                        v = struct.unpack_from("<14f", verts, i * 0x70)
                        f.write("v %g %g %g\nvn %g %g %g\nvt %g %g\n" % (v[0], v[1], v[2], v[4], v[5], v[6], v[12], 1 - v[13]))
                    for i in range(0, len(idx) - 2, 3):
                        a, b, c = base + idx[i], base + idx[i + 1], base + idx[i + 2]
                        f.write("f %d/%d/%d %d/%d/%d %d/%d/%d\n" % (a, a, a, b, b, b, c, c, c))
                    base += vc
        print("wrote", dst)
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
