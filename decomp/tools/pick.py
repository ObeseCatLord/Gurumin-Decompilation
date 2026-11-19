"""pick.py <file.cpp in src/> <n> : keep variant n of every '#if V == k' / '#elif V == k' / '#else' / '#endif' block.
Conditions may list several variants ('#if V == 1 || V == 3'). The file keeps its own line endings."""
import os, re, sys

path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", sys.argv[1])
want = int(sys.argv[2])
COND = re.compile(r"\s*#(if|elif) (V == \d+(?:\s*\|\|\s*V == \d+)*)\s*$")
out, stack = [], []
for line in open(path, "rb").read().decode("utf-8").splitlines(True):
    text = line.rstrip("\r\n")
    m = COND.match(text)
    if m:
        if m.group(1) == "if":
            stack.append({"done": False, "on": False})
        top = stack[-1]
        wanted = want in [int(v) for v in re.findall(r"\d+", m.group(2))]
        top["on"] = not top["done"] and wanted
        top["done"] |= top["on"]
        continue
    if stack and re.match(r"\s*#else\s*$", text):
        top = stack[-1]
        top["on"] = not top["done"]
        top["done"] = True
        continue
    if stack and re.match(r"\s*#endif\s*$", text):
        stack.pop()
        continue
    if all(s["on"] for s in stack):
        out.append(line)
open(path, "wb").write("".join(out).encode("utf-8"))
