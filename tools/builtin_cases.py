"""builtin_cases.py : split Script_GameBuiltin (0x10138280) into one decompiled snippet per built-in id,
labelled with its signature -> re/export/builtins.txt (local only: contains decompiled code)."""
import csv, os, re, subprocess, sys
here = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(here, "..")
code = subprocess.run([sys.executable, os.path.join(root, "decomp", "tools", "fn.py"), "10138280"],
                      capture_output=True, text=True).stdout.splitlines()
sig = {int(r["id"]): r["signature"] for r in csv.DictReader(open(os.path.join(root, "docs", "script_api.tsv"), encoding="utf-8"), delimiter="\t")}
blocks = {}
current = []
ids = []
for line in code:
    m = re.match(r"\s*case (0x[0-9a-f]+|\d+):", line)
    if m:
        if current and ids and any(l.strip() and not l.strip().startswith("case") for l in current):
            for i in ids:
                blocks.setdefault(i, current)
            ids = []
            current = []
        ids.append(int(m.group(1), 0))
        continue
    current.append(line)
if ids:
    for i in ids:
        blocks.setdefault(i, current)
out = os.path.join(root, "re", "export", "builtins.txt")
with open(out, "w", encoding="utf-8") as f:
    for i in sorted(sig):
        f.write("=" * 70 + "\n%d (0x%x)  %s\n" % (i, i, sig[i]))
        f.write("\n".join(blocks.get(i, ["  (no case block found; probably shares code)"])[:60]) + "\n")
print(len(blocks), "case blocks;", sum(1 for i in sig if i in blocks), "of", len(sig), "built-ins found ->", out)
