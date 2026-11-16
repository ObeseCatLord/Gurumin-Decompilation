"""xref_en.py : carry symbols.tsv names from the JP APP.dll to the Steam EN game.exe by shared string references.

For every named JP function, take the strings it references (decomp/strings.tsv). Find each string's bytes in the EN
exe, then the EN functions referencing that address (re/export/game_en_steam.exe/data_refs.tsv). A JP function maps
to the EN function that references the most of its strings, provided that function is the unique best candidate.
Output: docs/symbols_en.tsv (en_address, name, jp_address, shared strings, confidence)."""
import collections, csv, os, struct, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "decomp", "tools"))
from gimg import Image

en = Image(os.path.join(ROOT, "re", "bin", "game_en_steam.exe"))
en_export = os.path.join(ROOT, "re", "export", "game_en_steam.exe")
symbols = {}
for line in open(os.path.join(ROOT, "symbols.tsv"), encoding="utf-8-sig").read().splitlines()[1:]:
    c = line.split("\t")
    if len(c) >= 2:
        symbols[c[0].lower()] = c[1]

jp_strings = collections.defaultdict(set)
for r in csv.DictReader(open(os.path.join(ROOT, "decomp", "strings.tsv"), encoding="utf-8"), delimiter="\t"):
    if len(r["text"]) < 4:
        continue
    for fn in filter(None, r["functions"].split(",")):
        jp_strings[fn.split("_")[-1].lower()].add(r["text"])

# EN string addresses
en_data = {name: sec for name, sec in en.sections.items() if name in (".rdata", ".data")}
_cache = {}


def find_en(text):
    if text in _cache:
        return _cache[text]
    _cache[text] = hits = _find_en(text)
    return hits


def _find_en(text):
    raw = text.replace("\\t", "\t").replace("\\r", "\r").replace("\\n", "\n").encode("cp932", "replace") + b"\0"
    hits = []
    for name, (va, vs, rawoff, rs) in en_data.items():
        blob = en.data[rawoff:rawoff + rs]
        i = blob.find(raw)
        while i >= 0:
            if i == 0 or blob[i - 1] == 0:
                hits.append(va + i)
            i = blob.find(raw, i + 1)
    return hits

refs = collections.defaultdict(set)
for line in open(os.path.join(en_export, "data_refs.tsv"), encoding="utf-8", errors="replace").read().splitlines()[1:]:
    c = line.split("\t")
    if len(c) >= 3 and c[1]:
        refs[int(c[2], 16)].add(c[1].split("@")[-1] if "@" in c[1] else c[1])

ALL = "--all" in sys.argv
targets = {k: symbols.get(k, "") for k in jp_strings} if ALL else symbols
out = []
for jp, name in sorted(targets.items()):
    votes = collections.Counter()
    strings = jp_strings.get(jp, set())
    for s in strings:
        for a in find_en(s):
            for f in refs.get(a, ()):
                votes[f] += 1
    if not votes:
        continue
    best = votes.most_common(2)
    if len(best) > 1 and best[0][1] == best[1][1]:
        continue
    conf = "high" if best[0][1] >= 3 else ("medium" if best[0][1] == 2 else "low")
    out.append([best[0][0], name, jp, best[0][1], len(strings), conf])

# an EN function claimed by several JP functions keeps only the strongest claim
claims = collections.defaultdict(list)
for r in out:
    claims[r[0]].append(r)
out = [max(rs, key=lambda r: r[3]) for rs in claims.values() if len(rs) == 1 or sorted(r[3] for r in rs)[-1] > sorted(r[3] for r in rs)[-2]]
out.sort(key=lambda r: r[2])

with open(os.path.join(ROOT, "docs", "jp_en_map.tsv" if ALL else "symbols_en.tsv"), "w", encoding="utf-8") as f:
    f.write("en_function\tname\tjp_address\tshared_strings\tjp_strings\tconfidence\n")
    for r in out:
        f.write("%s\t%s\t%s\t%d\t%d\t%s\n" % tuple(r))
print(len(out), "of", len(targets), "functions mapped", collections.Counter(r[5] for r in out))
