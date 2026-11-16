import glob, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import itm_tool
roots = sys.argv[1:]
ok = bad = rt = 0
for root in roots:
    for p in glob.glob(os.path.join(root, "**", "*.itm"), recursive=True):
        d = open(p, "rb").read()
        if len(d) < 12:
            continue
        plain, good, unk = itm_tool.decode(d, p)
        if not good:
            bad += 1
            continue
        ok += 1
        if itm_tool.encode(plain, p, unk) == d:
            rt += 1
        txt = os.path.splitext(p)[0] + ".txt"
        if os.path.exists(txt) and open(txt, "rb").read() != plain:
            print("text differs:", txt)
print("checksum ok", ok, "failed", bad, "byte-identical re-encode", rt)
