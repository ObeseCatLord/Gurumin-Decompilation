"""asm.py <hex addr> [length] : disassemble a function of the EN exe (game_en_steam.exe)"""
import sys
import capstone
from enimg import Image, inventory

img, inv = Image(), inventory()
a = int(sys.argv[1], 16)
if len(sys.argv) > 2:
    inv[a] = ("", int(sys.argv[2], 0))
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
for i in md.disasm(img.read(a, inv.get(a, ("", 64))[1]), a):
    print("%08x  %-6s %s" % (i.address, i.mnemonic, i.op_str))
