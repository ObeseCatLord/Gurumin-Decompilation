"""config_keys.py : list gurumin.def keys read with FUN_1015fe70 (GetPrivateProfileInt wrapper) and the global each lands in."""
import os, re, sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "decomp", "tools"))
from gimg import Image, DECOMPILED
img = Image()
def s(tok):
    m = re.match(r'&DAT_([0-9a-f]{8})', tok)
    if m:
        return img.read(int(m.group(1), 16), 32).split(b"\0")[0].decode("cp932", "replace")
    return tok.strip('"')
lines = open(DECOMPILED, encoding="utf-8", errors="replace").read().splitlines()
fn = None
rows = []
for i, line in enumerate(lines):
    m = re.match(r"// ==== (\S+) @ ([0-9a-f]+)", line)
    if m:
        fn = m.group(1)
    m = re.search(r'FUN_1015fe70\(([^,]+),("[^"]*"|&DAT_\w+),("[^"]*"|&DAT_\w+),', line)
    if m:
        dest = ""
        for j in range(i + 1, min(i + 4, len(lines))):
            mm = re.match(r"\s*(\w+) = uVar\d+;", lines[j]) or re.match(r"\s*(\w+) = (\w+);", lines[j])
            if mm:
                dest = mm.group(1)
                break
            if "FUN_1015fe70" in lines[j]:
                break
        rows.append((fn, s(m.group(2)), s(m.group(3)), dest))
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "docs", "config_keys.tsv")
with open(out, "w", encoding="utf-8") as f:
    f.write("function\tsection\tkey\tglobal\n")
    for r in rows:
        f.write("\t".join(r) + "\n")
print(len(rows), "keys")
