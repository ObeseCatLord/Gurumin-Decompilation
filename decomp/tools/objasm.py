"""objasm.py <obj> <symbol-substring> : disassemble a function from a COFF object"""
import sys
import capstone
import compare_obj as c
syms, secs = c.parse(sys.argv[1])
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
for s in syms:
    if s and s[3] == 0x20 and s[2] > 0 and sys.argv[2] in s[0]:
        print(s[0])
        data, rel = secs[s[2] - 1]
        for i in md.disasm(data, 0):
            print("%04x  %-6s %s" % (i.address, i.mnemonic, i.op_str))
