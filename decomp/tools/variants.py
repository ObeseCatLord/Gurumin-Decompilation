"""variants.py <file.cpp in src/> <count> [extra cl flags] : build the file with /DV=1..count and print each result."""
import os, subprocess, sys

here = os.path.dirname(os.path.abspath(__file__))
build = os.path.join(here, "..", "build.bat")
name, count = sys.argv[1], int(sys.argv[2])
extra = " ".join(sys.argv[3:])
for v in range(1, count + 1):
    out = subprocess.run('cmd /c ""%s" %s "/DV=%d" %s"' % (build, name, v, extra), capture_output=True, text=True).stdout
    lines = [l for l in out.splitlines() if l.startswith(("MATCH", "DIFF", "EQUIV")) or "error" in l]
    print("V=%d" % v, " | ".join(lines))
