"""script_api.py : list every script built-in registered through FUN_10036ca0 (signature, id, handler) -> docs/script_api.tsv"""
import os, re
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = open(os.path.join(ROOT, "re", "export", "APP.dll", "decompiled.c"), encoding="utf-8", errors="replace").read()
reg = re.compile(r'FUN_10036ca0\("([^"]+)",\s*(0x[0-9a-f]+|\d+),\s*&?(\w+)\)')
fn = None
rows = []
for line in src.splitlines():
    m = re.match(r"// ==== (\S+) @", line)
    if m:
        fn = m.group(1)
        continue
    for m in reg.finditer(line):
        rows.append((m.group(1), int(m.group(2), 0), m.group(3), fn))
os.makedirs(os.path.join(ROOT, "docs"), exist_ok=True)
with open(os.path.join(ROOT, "docs", "script_api.tsv"), "w", encoding="utf-8") as f:
    f.write("signature\tid\thandler\tregistered_in\n")
    for r in rows:
        f.write("%s\t%d\t%s\t%s\n" % r)
from collections import Counter
print(len(rows), "built-ins; handlers:", Counter(r[2] for r in rows).most_common(8))
print("registered in:", Counter(r[3] for r in rows))

