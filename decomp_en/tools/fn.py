"""fn.py <hex addr> : Ghidra's decompilation of one EN function (re/export/game_en_steam.exe/decompiled.c)"""
import sys
from enimg import DECOMPILED

pat = " @ %08x" % int(sys.argv[1], 16)
on = False
for line in open(DECOMPILED, encoding="utf-8", errors="replace"):
    if not on and line.startswith("// ====") and pat in line:
        on = True
    elif on and line.startswith("// ===="):
        break
    if on:
        print(line, end="")
