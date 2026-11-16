"""iat.py : list APP.dll imports as IAT address, dll, name -> ../iat.tsv"""
import os, struct
from gimg import Image
img = Image()
d = img.data
pe = struct.unpack_from("<I", d, 0x3C)[0]
imp_rva, imp_size = struct.unpack_from("<II", d, pe + 24 + 96 + 8)
def rd(va, n):
    return img.read(va, n)
def cstr(va):
    s = b""
    while True:
        c = rd(va, 64)
        i = c.find(b"\0")
        if i >= 0:
            return (s + c[:i]).decode("latin-1")
        s += c
        va += 64
rows = []
desc = img.base + imp_rva
while True:
    olt, ts, fc, name, ft = struct.unpack("<IIIII", rd(desc, 20))
    if name == 0:
        break
    dll = cstr(img.base + name)
    lookup = olt or ft
    i = 0
    while True:
        e, = struct.unpack("<I", rd(img.base + lookup + 4 * i, 4))
        if e == 0:
            break
        fn = "#%d" % (e & 0xFFFF) if e & 0x80000000 else cstr(img.base + e + 2)
        rows.append((img.base + ft + 4 * i, dll, fn))
        i += 1
    desc += 20
with open(os.path.join(os.path.dirname(__file__), "..", "iat.tsv"), "w") as f:
    for r in rows:
        f.write("%08x\t%s\t%s\n" % r)
print(len(rows), "imports")
