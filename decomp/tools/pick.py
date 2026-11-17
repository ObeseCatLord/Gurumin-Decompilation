"""pick.py <file.cpp in src/> <n> : keep variant n of every '#if V == k' / '#elif V == k' / '#else' / '#endif' block."""
import os, re, sys

path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", sys.argv[1])
want = int(sys.argv[2])
out, stack = [], []
for line in open(path, encoding="utf-8").read().split("\n"):
    m = re.match(r"\s*#(if|elif) V == (\d+)\s*$", line)
    if m:
        if m.group(1) == "if":
            stack.append({"done": False, "on": False})
        top = stack[-1]
        top["on"] = not top["done"] and int(m.group(2)) == want
        top["done"] |= top["on"]
        continue
    if stack and re.match(r"\s*#else\s*$", line):
        top = stack[-1]
        top["on"] = not top["done"]
        top["done"] = True
        continue
    if stack and re.match(r"\s*#endif\s*$", line):
        stack.pop()
        continue
    if all(s["on"] for s in stack):
        out.append(line)
open(path, "w", encoding="utf-8", newline="\n").write("\n".join(out))
