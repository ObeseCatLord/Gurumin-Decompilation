"""build_all.py : compile every src/*.cpp with build.bat and print per-file and total matches."""
import glob, os, re, subprocess
here = os.path.dirname(os.path.abspath(__file__))
total_m = total_d = total_e = 0
matched_bytes = 0
for path in sorted(glob.glob(os.path.join(here, "..", "src", "*.cpp"))):
    name = os.path.basename(path)
    out = subprocess.run(["cmd", "/c", os.path.join(here, "..", "build.bat"), name], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith("DIFF") or "error" in line:
            print(name, line)
    matched_bytes += sum(int(b) for b in re.findall(r"^MATCH \S+ \((\d+) bytes", out, re.M))
    m = re.search(r"matched (\d+), differing (\d+), equivalent (\d+)", out)
    if m:
        total_m += int(m.group(1)); total_d += int(m.group(2)); total_e += int(m.group(3))
    else:
        print(name, "BUILD FAILED"); print(out[-2000:])
print("TOTAL matched %d, equivalent (13.00 encoding only) %d, differing %d" % (total_m, total_e, total_d))
import csv
game = [r for r in csv.DictReader(open(os.path.join(here, "..", "triage.tsv")), delimiter="\t") if r["address"] < "101b2000"]
code = sum(int(r["size"]) for r in game)
print("PROGRESS %d of %d game functions matched (%.1f%%), %d of %d code bytes (%.2f%%)" % (
    total_m, len(game), 100.0 * total_m / len(game), matched_bytes, code, 100.0 * matched_bytes / code))
