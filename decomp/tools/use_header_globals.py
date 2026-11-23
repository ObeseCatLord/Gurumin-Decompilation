"""use_header_globals.py <header.h> <global> [<global> ...] : replace the per-file `extern` declarations of the
given globals in src/*.cpp and src_od/*.cpp with `#include "<header.h>"` (placed where the first removed line stood,
skipped when the file already includes the header). The header must already declare the globals.

Only single-line declarations are removed; each file keeps its own line endings. Prints the files it changed.
Build and check (build_all.py, check_matches.py) afterwards; revert a file that loses with git checkout.
"""
import glob, os, re, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")


def main():
    header, names = sys.argv[1], sys.argv[2:]
    pattern = re.compile(rb'^\s*extern\s+(?:"C"\s+)?[^;(]*?(?:\(\s*\*\s*)?\b(' + b"|".join(n.encode() for n in names)
                         + rb')\b[^;]*;[^\n]*\n?', re.M)
    files = sorted(glob.glob(os.path.join(ROOT, "src", "*.cpp")) + glob.glob(os.path.join(ROOT, "src_od", "*.cpp")))
    for path in files:
        data = open(path, "rb").read()
        matches = list(pattern.finditer(data))
        if not matches:
            continue
        newline = b"\r\n" if matches[0].group(0).endswith(b"\r\n") else b"\n"
        include = b'#include "' + header.encode() + b'"'
        already = include in data
        out, pos = [], 0
        for i, m in enumerate(matches):
            out.append(data[pos:m.start()])
            if i == 0 and not already:
                out.append(include + newline)
            pos = m.end()
        out.append(data[pos:])
        open(path, "wb").write(b"".join(out))
        print(os.path.basename(path), len(matches))


if __name__ == "__main__":
    main()
