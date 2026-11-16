"""strings_map.py : find cp932/ASCII strings in APP.dll data sections and the functions referencing them -> ../strings.tsv"""
import os, re, collections
from gimg import Image, ROOT
img = Image()
refs = collections.defaultdict(set)
for line in open(os.path.join(ROOT, "re", "export", "APP.dll", "data_refs.tsv"), encoding="utf-8", errors="replace").read().splitlines()[1:]:
    c = line.split("\t")
    if len(c) >= 3 and c[1]:
        refs[int(c[2], 16)].add(c[1].split("@")[0])
pat = re.compile(rb"(?:[\x20-\x7e\t\r\n]|[\x81-\x9f\xe0-\xfc][\x40-\xfc]|[\xa1-\xdf]){4,}\x00")
rows = []
for name in (".rdata", ".data"):
    va, vsize, raw, rsize = img.sections[name]
    blob = img.data[raw:raw + rsize]
    for m in pat.finditer(blob):
        addr = va + m.start()
        s = m.group()[:-1].decode("cp932", "replace").replace("\t", "\\t").replace("\r", "\\r").replace("\n", "\\n")
        rows.append((addr, ",".join(sorted(refs.get(addr, ()))), s))
out = os.path.join(os.path.dirname(__file__), "..", "strings.tsv")
with open(out, "w", encoding="utf-8") as f:
    f.write("address\tfunctions\ttext\n")
    for r in rows:
        f.write("%08x\t%s\t%s\n" % r)
print(len(rows), "strings,", sum(1 for r in rows if r[1]), "referenced")
