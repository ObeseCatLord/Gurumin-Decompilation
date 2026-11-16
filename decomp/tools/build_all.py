"""build_all.py : compile every src/*.cpp with build.bat and print per-file and total matches."""
import glob, os, re, subprocess
here = os.path.dirname(os.path.abspath(__file__))
total_m = total_d = total_e = 0
for path in sorted(glob.glob(os.path.join(here, "..", "src", "*.cpp"))):
    name = os.path.basename(path)
    out = subprocess.run(["cmd", "/c", os.path.join(here, "..", "build.bat"), name], capture_output=True, text=True).stdout
    for line in out.splitlines():
        if line.startswith("DIFF") or "error" in line:
            print(name, line)
    m = re.search(r"matched (\d+), differing (\d+), equivalent (\d+)", out)
    if m:
        total_m += int(m.group(1)); total_d += int(m.group(2)); total_e += int(m.group(3))
    else:
        print(name, "BUILD FAILED"); print(out[-2000:])
print("TOTAL matched %d, equivalent (13.00 encoding only) %d, differing %d" % (total_m, total_e, total_d))
