"""save_layout.py : offsets of the save-file body -> globals, from Save_Load (0x1003b2c0) -> docs/save_layout.tsv"""
import os, re, subprocess, sys
here = os.path.dirname(os.path.abspath(__file__))
code = subprocess.run([sys.executable, os.path.join(here, "..", "decomp", "tools", "fn.py"), "1003b2c0"],
                      capture_output=True, text=True).stdout.splitlines()
rows = []
src = None
for i, line in enumerate(code):
    l = line.strip()
    m = re.match(r"(\*?\(?[\w &*()]*?(DAT_[0-9a-f]+|local_\w+)\)?) = \*\((\w+) \*\)\(\(int\)_Memory \+ (0x[0-9a-f]+)\);", l)
    if m:
        rows.append((int(m.group(4), 16), 4 if m.group(3) in ("undefined4", "int", "uint", "float") else 1, m.group(2), ""))
        continue
    m = re.match(r"puVar6 = \(\w+ \*\)\(\(int\)_Memory \+ (0x[0-9a-f]+)\);", l)
    if m:
        src = int(m.group(1), 16)
        continue
    m = re.match(r"puVar7 = (?:\(\w+ \*\))?&(DAT_[0-9a-f]+);", l)
    if m and src is not None:
        dst = m.group(1)
        for j in range(i + 1, min(i + 3, len(code))):
            mm = re.search(r"for \(iVar3 = (0x[0-9a-f]+|\d+);", code[j])
            if mm:
                rows.append((src, int(mm.group(1), 0) * 4, dst, "block copy"))
                break
        src = None
rows.sort()
with open(os.path.join(here, "..", "docs", "save_layout.tsv"), "w") as f:
    f.write("body_offset\tbytes\tdestination\tnote\n")
    for r in rows:
        f.write("%#07x\t%d\t%s\t%s\n" % r)
print(len(rows), "entries; covered", sum(r[1] for r in rows), "bytes")
