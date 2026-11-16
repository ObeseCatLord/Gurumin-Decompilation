"""fn.py <hex addr> : Ghidra's decompilation of one function"""
import sys
from gimg import DECOMPILED
pat = " @ %s" % sys.argv[1].lower()
on = False
for line in open(DECOMPILED, encoding="utf-8", errors="replace"):
    if not on and line.startswith("// ====") and pat in line:
        on = True
    elif on and line.startswith("// ===="):
        break
    if on:
        print(line, end="")


