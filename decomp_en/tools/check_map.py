"""check_map.py : compare decomp_en/jp_en_map.tsv with the older string-only map docs/jp_en_map_full.tsv"""
import collections, csv, os
from enimg import EN, ROOT

m = {r["en_address"]: r for r in csv.DictReader(open(os.path.join(EN, "jp_en_map.tsv"), encoding="utf-8"), delimiter="\t")}
c = collections.Counter()
for r in csv.DictReader(open(os.path.join(ROOT, "docs", "jp_en_map_full.tsv"), encoding="utf-8"), delimiter="\t"):
    e = r["en_address"].lower().replace("fun_", "").zfill(8)
    x = m.get(e)
    if not x:
        c["not an EN game function"] += 1
    elif x["jp_address"] == r["jp_address"].lower() and x["jp_module"] == "APP.dll":
        c["agree"] += 1
    elif x["jp_module"] == "-":
        c["unmapped here"] += 1
    else:
        c["disagree (%s)" % x["level"]] += 1
        print("disagree", e, "docs:", r["jp_address"], r["how"], "| here:", x["jp_module"], x["jp_address"], x["level"], x["evidence"][:50])
print(dict(c))
