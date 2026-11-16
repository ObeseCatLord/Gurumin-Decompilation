import os, struct
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
DLL = os.path.join(ROOT, "re", "bin", "APP.dll")
FUNCS = os.path.join(ROOT, "re", "export", "APP.dll", "functions.tsv")
DECOMPILED = os.path.join(ROOT, "re", "export", "APP.dll", "decompiled.c")

class Image:
    def __init__(self, path=DLL):
        d = self.data = open(path, "rb").read()
        pe = struct.unpack_from("<I", d, 0x3C)[0]
        nsec, = struct.unpack_from("<H", d, pe + 6)
        opt, = struct.unpack_from("<H", d, pe + 20)
        self.base, = struct.unpack_from("<I", d, pe + 24 + 28)
        self.sections = {}
        off = pe + 24 + opt
        for _ in range(nsec):
            name = d[off:off + 8].rstrip(b"\0").decode()
            vsize, va, rsize, raw = struct.unpack_from("<IIII", d, off + 8)
            self.sections[name] = (self.base + va, vsize, raw, rsize)
            off += 40

    def read(self, va, size):
        for start, vsize, raw, rsize in self.sections.values():
            if start <= va < start + rsize:
                o = raw + va - start
                return self.data[o:o + min(size, start + rsize - va)]
        return b""

def inventory():
    inv = {}
    for line in open(FUNCS, encoding="utf-8", errors="replace").read().splitlines()[1:]:
        c = line.split("\t")
        inv[int(c[0], 16)] = (c[1], int(c[2]))
    return inv
