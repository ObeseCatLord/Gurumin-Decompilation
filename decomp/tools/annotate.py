"""annotate.py <notes.py> : add comment blocks to decomp sources.

notes.py defines NOTES = {"file.cpp": {"header": "...", "functions": {"signature start": "comment", ...}}}.
The header goes at the top of the file (if the file does not start with a comment yet); each function comment goes
directly above the first line that starts with the given signature text (skipped when a comment is already there).
Comment text may span several lines; every line is prefixed with "// ". Comments never change the compiled code.
"""
import os, runpy, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def block(text, indent=""):
    return "".join(indent + ("// " + line if line else "//") + "\n" for line in text.strip("\n").split("\n"))


def main():
    notes = runpy.run_path(sys.argv[1])["NOTES"]
    for name, spec in notes.items():
        path = os.path.join(ROOT, "src", name)
        text = open(path, encoding="utf-8").read()
        lines = text.split("\n")
        out = []
        pending = dict(spec.get("functions", {}))
        for i, line in enumerate(lines):
            for sig in list(pending):
                if line.startswith(sig):
                    previous = out[-1] if out else ""
                    if not previous.lstrip().startswith("//"):
                        out.append(block(pending[sig]).rstrip("\n"))
                    del pending[sig]
                    break
            out.append(line)
        text = "\n".join(out)
        if spec.get("header") and not text.lstrip().startswith("//"):
            text = block(spec["header"]) + "\n" + text
        open(path, "w", encoding="utf-8", newline="\n").write(text)
        for sig in pending:
            print("%s: signature not found: %s" % (name, sig))
    print("annotated %d files" % len(notes))


if __name__ == "__main__":
    main()
