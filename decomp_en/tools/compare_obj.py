"""compare_obj.py <file.obj> [-v] : byte-compare every function of a /Gy COFF object with game_en_steam.exe.

A function is compared when its symbol is FUN_xxxxxxxx (C linkage) or has a row in decomp_en/obj_symbols.tsv
(`<mangled name><TAB><address>`). Relocation fields are masked. Prints MATCH or DIFF per function and a summary
line; the same output format as the JP project's decomp/tools/compare_obj.py, so build_all/check_matches read it.
"""
import os, re, sys

from coff import parse
from enimg import Image, OBJ_SYMBOLS

ADDR = re.compile(r"^(?:_|\?)?(?:FUN|M)_([0-9a-fA-F]{8})(?:@|$)")


def named_symbols():
    named = {}
    if os.path.exists(OBJ_SYMBOLS):
        for line in open(OBJ_SYMBOLS, encoding="utf-8").read().splitlines():
            if line and not line.startswith("#"):
                parts = line.split("\t")
                named[parts[0]] = int(parts[1], 16)
    return named


def compare(path, verbose=False):
    img = Image()
    named = named_symbols()
    syms, secs = parse(path)
    ok = bad = 0
    for s in syms:
        # function symbols (type 0x20) with external storage class in a real section
        if not s or s[3] != 0x20 or s[4] != 2 or s[2] <= 0:
            continue
        m = ADDR.match(s[0])
        if m:
            va = int(m.group(1), 16)
        elif s[0] in named:
            va = named[s[0]]
        else:
            continue
        data, rel = secs[s[2] - 1]
        orig = img.read(va, len(data))
        mask = bytearray(len(data))
        for off, _symidx, _typ in rel:
            for k in range(4):
                if off + k < len(mask):
                    mask[off + k] = 1
        diffs = [i for i in range(len(data)) if not mask[i] and (i >= len(orig) or data[i] != orig[i])]
        if len(orig) < len(data) or diffs:
            bad += 1
            print("DIFF %s len ours=%d, diffs=%d first@%s" % (s[0], len(data), len(diffs), diffs[:1]))
        else:
            ok += 1
            print("MATCH %s (%d bytes, %d relocs)" % (s[0], len(data), len(rel)))
    print("matched %d, differing %d, equivalent 0" % (ok, bad))


if __name__ == "__main__":
    compare(sys.argv[1], "-v" in sys.argv)
