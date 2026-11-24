"""coff.py : minimal PE image reader and COFF object parser (copied from decomp/tools/gimg.py and compare_obj.py so
the EN tools keep working while the JP session edits its own tools)."""
import struct


class PeImage:
    """read bytes of a PE file by virtual address"""

    def __init__(self, path):
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

    def section_of(self, va):
        for name, (start, vsize, raw, rsize) in self.sections.items():
            if start <= va < start + max(vsize, rsize):
                return name
        return None


def parse(path):
    """symbols [(name, value, section, type, storage class) or None for aux records] and sections [(bytes, relocs)]"""
    d = open(path, "rb").read()
    machine, nsec, _, symptr, nsym, optsize, _ = struct.unpack_from("<HHIIIHH", d, 0)
    strtab = symptr + 18 * nsym

    def symname(o):
        if d[o:o + 4] == b"\0\0\0\0":
            so, = struct.unpack_from("<I", d, o + 4)
            return d[strtab + so:d.index(b"\0", strtab + so)].decode("latin-1")
        return d[o:o + 8].rstrip(b"\0").decode("latin-1")
    syms = []
    i = 0
    while i < nsym:
        o = symptr + 18 * i
        value, sec, typ, cls, aux = struct.unpack_from("<IhHBB", d, o + 8)
        syms.append((symname(o), value, sec, typ, cls))
        syms.extend([None] * aux)
        i += 1 + aux
    secs = []
    for i in range(nsec):
        o = 20 + optsize + 40 * i
        size, raw, relptr, _, nrel, _, chars = struct.unpack_from("<IIIIHHI", d, o + 16)
        rel = [struct.unpack_from("<IIH", d, relptr + 10 * r) for r in range(nrel)]
        secs.append((d[raw:raw + size] if raw else b"", rel))
    return syms, secs
