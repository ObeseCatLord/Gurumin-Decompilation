"""sync_symbols.py : copy readable names from decomp/obj_symbols.tsv into symbols.tsv.

obj_symbols.tsv maps compiler (mangled) names to addresses for functions whose source name carries no address.
This adds every such function that symbols.tsv does not list yet, as `Class::Method` (constructors `Class::Class`,
destructors `Class::~Class`, C functions by their plain name). Placeholder names (M_xxxxxxxx, FUN_xxxxxxxx) are skipped.
Existing symbols.tsv rows are never changed.
"""
import os, re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO = os.path.dirname(ROOT)


def readable(mangled):
    if mangled.startswith("_"):
        return mangled[1:].split("@")[0]
    m = re.match(r"^\?\?([01])(\w+)@@", mangled)
    if m:
        cls = m.group(2)
        return "%s::%s%s" % (cls, "~" if m.group(1) == "1" else "", cls)
    m = re.match(r"^\?(\w+)@(\w+)@@", mangled)
    if m:
        return "%s::%s" % (m.group(2), m.group(1))
    m = re.match(r"^\?(\w+)@@", mangled)
    if m:
        return m.group(1)
    return None


def main():
    path = os.path.join(REPO, "symbols.tsv")
    lines = open(path, encoding="utf-8").read().rstrip("\n").split("\n")
    known = {l.split("\t")[0] for l in lines[1:] if l}
    added = 0
    for line in open(os.path.join(ROOT, "obj_symbols.tsv"), encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 2 or line.startswith("#"):
            continue
        name, address = readable(parts[0]), parts[1].lower()
        if not name or address in known or re.search(r"(^|::)(M|FUN)_10[0-9a-f]{6}$", name):
            continue
        lines.append("%s\t%s\tmatched decompiled source (decomp/obj_symbols.tsv)" % (address, name))
        known.add(address)
        added += 1
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    print("symbols.tsv: %d names added" % added)


if __name__ == "__main__":
    main()
