"""check_matches.py [--against <git rev>] : compare progress/jp/report.json with the committed one (default HEAD).
Lists functions that stopped matching and new matches. Exits 1 if any match was lost, so it can gate a commit.
Run after `python tools/build_all.py --report ../progress/jp/report.json`."""
import json, os, subprocess, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
REPORT = "progress/jp/report.json"


def matched(report):
    return {f["name"] for u in report["units"] for f in u.get("functions", []) if f.get("fuzzy_match_percent") == 100}


rev = sys.argv[sys.argv.index("--against") + 1] if "--against" in sys.argv else "HEAD"
old = json.loads(subprocess.run(["git", "show", "%s:%s" % (rev, REPORT)], cwd=ROOT, capture_output=True,
                                text=True, check=True).stdout)
new = json.load(open(os.path.join(ROOT, REPORT)))
before, after = matched(old), matched(new)
lost, gained = sorted(before - after), sorted(after - before)
print("matched %d -> %d (%+d)" % (len(before), len(after), len(after) - len(before)))
for name in gained:
    print("  new   " + name)
for name in lost:
    print("  LOST  " + name)
sys.exit(1 if lost else 0)
