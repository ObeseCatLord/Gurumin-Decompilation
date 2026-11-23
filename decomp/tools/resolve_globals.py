"""resolve_globals.py <header.h> <global> [<global> ...] : settle globals that files declare with different types
(AGENTS.md rule 2: one declaration each) by trying each file's declaration as the shared one.

For each global, every distinct per-file declaration (`extern "C" <type> <name><dims>;`) is tried in turn: it is
added to include/<header.h> (in address order among its extern lines), the per-file declarations are replaced by
`#include "<header.h>"` (tools/use_header_globals.py), and every file that declared the global is built and compared
with HEAD's report.json. The first declaration with no lost match, no fuzzy drop of more than a point and no compile
error is kept; otherwise everything is restored and the global is reported with the files that lost under each try.

A declaration whose type is neither builtin nor declared in include/*.h is not tried (files that include the
header without that type would stop compiling). Each failed try restores the files as they were before it. Run build_all.py and
check_matches.py afterwards, as always.
"""
import collections, glob, json, os, re, subprocess, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from globals_driver import BUILTIN, header_types

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SKIP_FILES = {"npc_greet.cpp"}


def declarations(name):
    """{declaration text without comment: [files]} over src/*.cpp."""
    found = collections.defaultdict(list)
    pattern = re.compile(r'^extern\s+"C"\s+([^;{(]*?\b' + name + r'\b[^;{(]*);', re.M)
    for path in sorted(glob.glob(os.path.join(ROOT, "src", "*.cpp"))):
        if os.path.basename(path) in SKIP_FILES:
            continue
        text = open(path, "rb").read().decode("latin-1")
        for m in pattern.finditer(text):
            found[" ".join(m.group(1).split())].append(os.path.basename(path))
    return found


def comment_of(name, files):
    for f in files:
        text = open(os.path.join(ROOT, "src", f), "rb").read().decode("latin-1")
        m = re.search(r'^extern\s+"C"[^;\n]*\b' + name + r'\b[^;\n]*;[ \t]*//\s*([^\r\n]*)', text, re.M)
        if m:
            return m.group(1).strip()
    return ""


def add_to_header(header, line):
    path = os.path.join(ROOT, "include", header)
    text = open(path, "rb").read().decode("latin-1")
    newline = "\r\n" if "\r\n" in text else "\n"
    lines = text.split(newline)
    address = re.search(r"DAT_([0-9a-f]{8})", line).group(1)
    externs = [k for k, l in enumerate(lines) if l.startswith('extern "C"')]
    later = [k for k in externs if (re.search(r"DAT_([0-9a-f]{8})", lines[k]) or [None, "z"])[1] > address]
    if later:
        at = later[0]
    elif externs:
        at = externs[-1] + 1
    else:
        at = max(k for k, l in enumerate(lines) if l.startswith("#endif"))
    lines.insert(at, line)
    open(path, "wb").write(newline.join(lines).encode("latin-1"))


def baseline_by_file():
    report = json.loads(subprocess.check_output(["git", "show", "HEAD:progress/jp/report.json"],
                                                cwd=os.path.join(ROOT, "..")))
    result = collections.defaultdict(dict)
    for unit in report["units"]:
        source = os.path.basename(unit["metadata"].get("source_path", ""))
        for fn in unit["functions"]:
            result[source][fn["name"]] = fn["fuzzy_match_percent"]
    return result


def build_file(f):
    out = subprocess.run(["cmd", "/c", os.path.join(ROOT, "build.bat"), f], capture_output=True, text=True).stdout
    if not re.search(r"matched (\d+), differing", out):
        return None
    result = {}
    for line in out.splitlines():
        m = re.match(r"(MATCH|EQUIV) (\S+)", line)
        d = re.match(r"DIFF (\S+) len ours=(\d+), diffs=(\d+)", line)
        if m:
            result[m.group(2)] = 100.0 if m.group(1) == "MATCH" else 99.0
        elif d:
            result[d.group(1)] = min(98.0, max(0.0, 100.0 * (1 - int(d.group(3)) / max(1, int(d.group(2))))))
    return result


def losses(files, baseline):
    lost = []
    for f in files:
        result = build_file(f)
        if result is None:
            lost.append(f + " (compile error)")
            continue
        for fn, percent in baseline.get(f, {}).items():
            now = result.get(fn, 0.0)
            if (percent >= 100.0 and now < 100.0) or now < percent - 1.0:
                lost.append("%s %s %.1f->%.1f" % (f, fn, percent, now))
                break
    return lost


def snapshot(files, header):
    paths = [os.path.join(ROOT, "src", f) for f in files] + [os.path.join(ROOT, "include", header)]
    return {p: open(p, "rb").read() for p in paths}


def restore(saved):
    for path, data in saved.items():
        open(path, "wb").write(data)


def main():
    header, names = sys.argv[1], sys.argv[2:]
    baseline = baseline_by_file()
    for name in names:
        decls = declarations(name)
        files = sorted({f for fs in decls.values() for f in fs})
        if not decls:
            print("%s: no per-file declaration" % name)
            continue
        comment = comment_of(name, files)
        order = sorted(decls, key=lambda d: -len(decls[d]))     # the most common spelling first
        # a declaration whose type only some source file defines cannot go into a shared header
        known = set(header_types()) | BUILTIN | {"DWORD", "HWND", "HINSTANCE", "GUID"}
        order = [d for d in order if set(re.findall(r"[A-Za-z_]\w*", d.split(name)[0])) <= known]
        if not order:
            print("%s: every declaration uses a type no include/*.h header declares" % name)
            continue
        report = []
        for decl in order:
            saved = snapshot(files, header)
            line = 'extern "C" %s;' % decl
            if comment:
                line = line.ljust(58) + "// " + comment
            add_to_header(header, line)
            subprocess.run([sys.executable, os.path.join(ROOT, "tools", "use_header_globals.py"), header, name],
                           cwd=ROOT, capture_output=True)
            lost = losses(files, baseline)
            if not lost:
                print("%s: kept `%s` in %s (%d files)" % (name, decl, header, len(files)))
                break
            report.append("`%s`: %s" % (decl, "; ".join(lost)))
            restore(saved)
        else:
            print("%s: no declaration fits all files" % name)
            for r in report:
                print("    " + r)


if __name__ == "__main__":
    main()
