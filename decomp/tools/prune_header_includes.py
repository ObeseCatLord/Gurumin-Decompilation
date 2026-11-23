"""prune_header_includes.py <header.h> ... : drop the #include lines a header does not need to compile on its own.

Each include line is removed in turn and kept out when the header still compiles alone (`cl /Zs` on a one-line
probe file). Used after tools/type_move_driver.py, which copies every include of the source file into a new header.
Line endings are kept (bytes are edited directly).
"""
import os, re, subprocess, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
INCLUDE = os.path.join(ROOT, "include")


def compiles_alone(header):
    probe = os.path.join(ROOT, "build", "header_probe.cpp")
    open(probe, "w").write('#include "%s"\n' % header)
    out = subprocess.run(["cmd", "/c", "call", os.path.join(ROOT, "env.bat"), "&&", "cl", "/nologo", "/c", "/Zs",
                          "/I", INCLUDE, probe], capture_output=True, text=True, cwd=os.path.join(ROOT, "build"))
    return out.returncode == 0


def prune(header):
    path = os.path.join(INCLUDE, header)
    if not compiles_alone(header):
        return "does not compile on its own"
    lines = open(path, "rb").read().split(b"\n")
    removed = []
    i = 0
    while i < len(lines):
        if re.match(rb"\s*#include\s", lines[i]):
            trial = lines[:i] + lines[i + 1:]
            open(path, "wb").write(b"\n".join(trial))
            if compiles_alone(header):
                removed.append(lines[i].strip().decode("latin-1"))
                lines = trial
                continue
            open(path, "wb").write(b"\n".join(lines))
        i += 1
    # collapse a blank line pair left where the include block was
    text = b"\n".join(lines)
    text = re.sub(rb"(\r?\n)(\r?\n)(\r?\n)+", rb"\1\2", text)
    open(path, "wb").write(text)
    return "removed " + " ".join(removed) if removed else "nothing removed"


if __name__ == "__main__":
    for h in sys.argv[1:]:
        print(os.path.basename(h), prune(os.path.basename(h)))
