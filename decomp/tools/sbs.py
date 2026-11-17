"""sbs.py <obj> <symbol-substring> [hex addr] : side-by-side disassembly of our function and the original.
Lines marked '*' differ (relocated operands are shown as written; compare_obj decides the match)."""
import difflib, re, sys
import capstone
import compare_obj as c
from gimg import Image, inventory

syms, secs = c.parse(sys.argv[1])
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
img, inv = Image(), inventory()
for s in syms:
    if s and s[3] == 0x20 and s[2] > 0 and sys.argv[2] in s[0]:
        name = s[0]
        data, rel = secs[s[2] - 1]
        break
else:
    sys.exit("symbol not found")
if len(sys.argv) > 3:
    addr = int(sys.argv[3], 16)
else:
    addr = c.NAMED.get(name) or int(re.search(r"(10[0-9a-fA-F]{6})", name).group(1), 16)
size = inv.get(addr, ("", len(data)))[1]


def norm(text):
    return re.sub(r"0x10[0-9a-f]{6}|0x[0-9a-f]{1,8}\b", "N", text)


ours = ["%-6s %s" % (i.mnemonic, i.op_str) for i in md.disasm(data, 0)]
orig = ["%-6s %s" % (i.mnemonic, i.op_str) for i in md.disasm(img.read(addr, size), addr)]
while ours and ours[-1].startswith(("nop", "int3")):
    ours.pop()
while orig and orig[-1].startswith(("nop", "int3")):
    orig.pop()
print(name, "@ %08x" % addr)
sm = difflib.SequenceMatcher(None, [norm(x) for x in ours], [norm(x) for x in orig], autojunk=False)
for op, a1, a2, b1, b2 in sm.get_opcodes():
    n = max(a2 - a1, b2 - b1)
    for k in range(n):
        left = ours[a1 + k] if a1 + k < a2 else ""
        right = orig[b1 + k] if b1 + k < b2 else ""
        print("%s %-44s %s" % (" " if op == "equal" else "*", left[:44], right))
