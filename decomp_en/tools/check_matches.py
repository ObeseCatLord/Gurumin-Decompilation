"""check_matches.py [--against <git rev>] : the commit gate for the EN tree.

Compares progress/en/report.json with the committed one (default HEAD), lists functions that stopped matching and new
matches, and exits 1 if any match was lost. Run after `python tools/build_all.py --report ../progress/en/report.json`.
A report that is not committed yet (first run) counts as "nothing matched before".
Same logic as decomp/tools/check_matches.py (the JP gate), for the EN report.
"""
import json, os, subprocess, sys

from enimg import ROOT, REPORT

REL = os.path.relpath(REPORT, ROOT).replace(os.sep, "/")


def matched(report):
    return {f["name"] for u in report["units"] for f in u.get("functions", []) if f.get("fuzzy_match_percent") == 100}


rev = sys.argv[sys.argv.index("--against") + 1] if "--against" in sys.argv else "HEAD"
shown = subprocess.run(["git", "show", "%s:%s" % (rev, REL)], cwd=ROOT, capture_output=True, text=True)
old = json.loads(shown.stdout) if shown.returncode == 0 else {"units": []}
new = json.load(open(REPORT, encoding="utf-8"))
before, after = matched(old), matched(new)
lost, gained = sorted(before - after), sorted(after - before)
print("matched %d -> %d (%+d)" % (len(before), len(after), len(after) - len(before)))
for name in gained:
    print("  new   " + name)
for name in lost:
    print("  LOST  " + name)
sys.exit(1 if lost else 0)
