"""xref_propagate.py : grow docs/jp_en_map.tsv through the call graph.

Anchors are the high/medium string matches. For every mapped pair, if exactly one unmapped callee remains on each
side (or exactly one unmapped caller), those two are paired. Repeats until nothing changes.
Writes docs/jp_en_map_full.tsv (en, jp, name, how)."""
import csv, os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def graph(path):
    callees, callers = {}, {}
    for line in open(path, encoding="utf-8", errors="replace").read().splitlines()[1:]:
        c = line.split("\t")
        if len(c) < 5:
            continue
        a = c[0].lower()
        callers[a] = {x.split("@")[-1].lower() for x in c[3].split(",") if "@" in x and "EXTERNAL" not in x}
        callees[a] = {x.split("@")[-1].lower() for x in c[4].split(",") if "@" in x and "EXTERNAL" not in x}
    return callees, callers


jp_callees, jp_callers = graph(os.path.join(ROOT, "re", "export", "APP.dll", "functions.tsv"))
en_callees, en_callers = graph(os.path.join(ROOT, "re", "export", "game_en_steam.exe", "functions.tsv"))
names = {}
for line in open(os.path.join(ROOT, "symbols.tsv"), encoding="utf-8-sig").read().splitlines()[1:]:
    c = line.split("\t")
    if len(c) >= 2:
        names[c[0].lower()] = c[1]

jp2en, en2jp, how = {}, {}, {}
for r in csv.DictReader(open(os.path.join(ROOT, "docs", "jp_en_map.tsv"), encoding="utf-8"), delimiter="\t"):
    if r["confidence"] in ("high", "medium"):
        en = r["en_function"].split("_")[-1].lower()
        jp = r["jp_address"].lower()
        if jp not in jp2en and en not in en2jp:
            jp2en[jp], en2jp[en], how[jp] = en, jp, "strings"

changed = True
rounds = 0
while changed and rounds < 50:
    changed = False
    rounds += 1
    for jp, en in list(jp2en.items()):
        for jg, eg, kind in ((jp_callees, en_callees, "callee"), (jp_callers, en_callers, "caller")):
            a = [x for x in jg.get(jp, ()) if x not in jp2en and x in jg]
            b = [x for x in eg.get(en, ()) if x not in en2jp and x in eg]
            if len(a) == 1 and len(b) == 1:
                jp2en[a[0]], en2jp[b[0]], how[a[0]] = b[0], a[0], "%s of %s" % (kind, jp)
                changed = True

with open(os.path.join(ROOT, "docs", "jp_en_map_full.tsv"), "w", encoding="utf-8") as f:
    f.write("en_address\tjp_address\tname\thow\n")
    for jp in sorted(jp2en):
        f.write("%s\t%s\t%s\t%s\n" % (jp2en[jp], jp, names.get(jp, ""), how[jp]))
print(len(jp2en), "pairs after", rounds, "rounds;", sum(1 for h in how.values() if h != "strings"), "from the call graph")
