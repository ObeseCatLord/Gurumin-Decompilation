"""type_move_driver.py: move the file-local struct/class/union definitions of src/*.cpp into shared headers
(AGENTS.md rule 2), keeping every file whose matches survive and reverting the others.

Usage (run from decomp/ on a clean tree):
    python tools/type_move_driver.py plan            # list what would move where, write nothing
    python tools/type_move_driver.py apply [files]   # move, build each file, revert files that lose
    python tools/type_move_driver.py apply --dry     # write without building

A type moves when it is defined at file scope (not inside a function) by a plain `struct X { ... };` or
`typedef struct X { ... } Y;` statement, no include/*.h header declares the name, and either only one source file
defines it or every file that defines it has the same text (comments and spacing aside). Types defined in several files
with different texts are struct views: they need a merge by hand and stay where they are.

Headers: the types a file alone defines go to include/<snake_case_type>.h when there is one, otherwise to
include/<file stem>.h (or <stem>_types.h when that name is taken); a type shared by identical copies gets its own
include/<snake_case_type>.h. Each header carries the comment lines above each definition, the `#include` lines that
came before the first definition in the source file, and the `#define` constants the definitions use. The source file
gets `#include "<header>"` where its first moved definition stood.

After the rewrite each file is built with build.bat and compared with HEAD's report.json: a file that loses a match,
drops more than a point of fuzzy on a function, or fails to compile is restored from git (its headers go with it unless
another kept file uses them). Headers that do not compile on their own are listed for a manual fix.

Files are edited as bytes (latin-1 round trip) so cp932 text and each line's ending are kept.
"""
import collections, glob, json, os, re, subprocess, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SRC = os.path.join(ROOT, "src")
INCLUDE = os.path.join(ROOT, "include")
REPORT_IN_GIT = "HEAD:progress/jp/report.json"

TYPE_DEFINITION = re.compile(r'\b(struct|class|union)\s+(?:__declspec\s*\([^)]*\)\s*)?(\w*)\s*(?::[^{;()]*)?\{')
SKIP_FILES = {"npc_greet.cpp"}      # open.tsv 100b8220: any shared header tips its register allocation


def blank_comments_and_strings(text):
    """Same-length copy of text with comments and literals replaced by spaces (newlines kept)."""
    out = list(text)
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(n, j + 1)
        else:
            i += 1
            continue
        for k in range(i, j):
            if out[k] != "\n":
                out[k] = " "
        i = j
    return "".join(out)


def snake_case(name):
    return re.sub(r"(?<=[a-z0-9])([A-Z])|(?<=[A-Z])([A-Z][a-z])", r"_\1\2", name).lower()


class Block:
    """One file-scope type definition: raw text span [start, end) including the comment lines above it."""
    def __init__(self, name, start, end, body_start, normalized):
        self.name, self.start, self.end, self.body_start, self.normalized = name, start, end, body_start, normalized


def find_blocks(raw):
    blank = blank_comments_and_strings(raw)
    depth = [0] * (len(blank) + 1)
    d = 0
    for i, c in enumerate(blank):
        depth[i] = d
        if c == "{":
            d += 1
        elif c == "}":
            d -= 1
    blocks, unmovable = [], []
    taken_until = -1
    for m in TYPE_DEFINITION.finditer(blank):
        if m.start() < taken_until:
            continue
        name = m.group(2)
        if depth[m.start()] != 0:
            unmovable.append(name or "<anonymous>")
            continue
        line_start = blank.rfind("\n", 0, m.start()) + 1
        prefix = blank[line_start:m.start()].strip()
        if prefix not in ("", "typedef"):
            unmovable.append(name or "<anonymous>")
            continue
        open_brace = m.end() - 1
        level, j = 0, open_brace
        while j < len(blank):
            if blank[j] == "{":
                level += 1
            elif blank[j] == "}":
                level -= 1
                if level == 0:
                    break
            j += 1
        semicolon = blank.find(";", j)
        tail = blank[j + 1:semicolon].strip()
        if semicolon < 0 or (tail and not (prefix == "typedef" and re.fullmatch(r"\w+", tail))):
            unmovable.append(name or "<anonymous>")
            taken_until = j
            continue
        if prefix == "typedef" and tail:
            name = tail
        if not name:
            unmovable.append("<anonymous>")
            continue
        end = raw.find("\n", semicolon)
        end = len(raw) if end < 0 else end + 1
        start = line_start
        while start > 0:
            previous = raw.rfind("\n", 0, start - 1) + 1
            if raw[previous:start].strip().startswith("//"):
                start = previous
            else:
                break
        normalized = " ".join(blank[line_start:end].split())
        blocks.append(Block(name, start, end, line_start, normalized))
        taken_until = end
    return blocks, unmovable


def header_names_in_include():
    names = set()
    for path in glob.glob(os.path.join(INCLUDE, "*.h")):
        blank = blank_comments_and_strings(open(path, encoding="latin-1").read())
        names |= {m.group(2) for m in TYPE_DEFINITION.finditer(blank) if m.group(2)}
        names |= set(re.findall(r"typedef\s+[^;{]*?\b(\w+)\s*;", blank))
    return names


def plan(files):
    raws = {f: open(os.path.join(SRC, f), "rb").read().decode("latin-1") for f in files}
    blocks = {f: find_blocks(raws[f])[0] for f in files}
    in_headers = header_names_in_include()
    definitions = collections.defaultdict(list)
    for f, bs in blocks.items():
        for b in bs:
            definitions[b.name].append((f, b.normalized))
    movable, shared = {}, set()
    for name, defs in definitions.items():
        if name in in_headers:
            continue
        if len({d[0] for d in defs}) != len(defs):
            continue                                   # defined twice in one file (#if variants)
        if len(defs) == 1:
            movable[name] = defs[0][0]
        elif len({d[1] for d in defs}) == 1:
            movable[name] = None
            shared.add(name)
    taken = {os.path.basename(p) for p in glob.glob(os.path.join(INCLUDE, "*.h"))}
    headers = {}                                      # type name -> header file name
    for name in sorted(shared):
        headers[name] = snake_case(name) + ".h"
        taken.add(headers[name])
    per_file = collections.defaultdict(list)
    for f in files:
        own = [b.name for b in blocks[f] if movable.get(b.name, 0) == f]
        if not own:
            continue
        stem = os.path.splitext(f)[0]
        header = snake_case(own[0]) + ".h" if len(own) == 1 else stem + ".h"
        if header in taken:
            header = stem + "_types.h"
        if header in taken:
            continue
        taken.add(header)
        for name in own:
            headers[name] = header
    for f in files:
        for b in blocks[f]:
            if b.name in headers:
                per_file[f].append(b)
    return raws, per_file, headers, shared


def header_text(header, file_name, raw, moved, newline, include_lines, defines):
    names = ", ".join(b.name for b in moved)
    guard = "GURUMIN_" + re.sub(r"\W", "_", header.upper())
    lines = ["// %s: declared here once (first used by src/%s)." % (names, file_name),
             "#ifndef " + guard, "#define " + guard, ""]
    if include_lines:
        lines += include_lines + [""]
    if defines:
        lines += defines + [""]
    body = "".join(raw[b.start:b.end] for b in moved).replace("\r\n", "\n").rstrip("\n").split("\n")
    lines += body + ["", "#endif", ""]
    return newline.join(lines)


def rewrite(f, raw, moved, headers, written):
    """Return (new source text, {header: text}) for file f."""
    newline = "\r\n" if "\r\n" in raw else "\n"
    first = min(b.start for b in moved)
    include_lines = [l.strip() for l in raw[:first].splitlines() if re.match(r"\s*#include\s*[<\"]", l)]
    used = set(re.findall(r"\w+", " ".join(b.normalized for b in moved)))
    defines, define_spans = [], []
    for m in re.finditer(r"^[ \t]*#define[ \t]+(\w+)\b[^\n]*\n?", raw[:first], re.M):
        if m.group(1) in used:
            defines.append(m.group(0).rstrip("\r\n"))
            define_spans.append((m.start(), m.end()))
    by_header = collections.OrderedDict()
    for b in sorted(moved, key=lambda b: b.start):
        by_header.setdefault(headers[b.name], []).append(b)
    texts = {}
    for header, bs in by_header.items():
        if header not in written:
            own_defines = defines if header == headers[moved[0].name] else []
            texts[header] = header_text(header, f, raw, bs, newline, include_lines, own_defines)
    spans = sorted([(b.start, b.end) for b in moved] + define_spans)
    out, pos, placed = [], 0, False
    includes = "".join('#include "%s"%s' % (h, newline) for h in by_header)
    for s, e in spans:
        out.append(raw[pos:s])
        if not placed and s == first:
            out.append(includes)
            placed = True
        pos = e
        if raw.startswith(newline, pos) and (s, e) not in define_spans and out and out[-1].endswith(newline * 2):
            pos += len(newline)                       # drop the blank line that followed a removed block
    out.append(raw[pos:])
    return "".join(out), texts


def baseline_by_file():
    report = json.loads(subprocess.check_output(["git", "show", REPORT_IN_GIT], cwd=os.path.join(ROOT, "..")))
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
        m = re.match(r"MATCH (\S+)", line)
        d = re.match(r"DIFF (\S+) len ours=(\d+), diffs=(\d+)", line)
        e = re.match(r"EQUIV (\S+)", line)
        if m:
            result[m.group(1)] = 100.0
        elif e:
            result[e.group(1)] = 99.0
        elif d:
            result[d.group(1)] = min(98.0, max(0.0, 100.0 * (1 - int(d.group(3)) / max(1, int(d.group(2))))))
    return result


def standalone_ok(header):
    probe = os.path.join(ROOT, "build", "header_probe.cpp")
    open(probe, "w").write('#include "%s"\n' % header)
    env = dict(os.environ)
    out = subprocess.run(["cmd", "/c", "call", os.path.join(ROOT, "env.bat"), "&&", "cl", "/nologo", "/c", "/Zs",
                          "/I", INCLUDE, probe], capture_output=True, text=True, env=env, cwd=os.path.join(ROOT, "build"))
    return out.returncode == 0


def main():
    args = sys.argv[1:]
    if not args or args[0] not in ("plan", "apply"):
        print(__doc__)
        return 2
    chosen = [a for a in args[1:] if a.endswith(".cpp")]
    files = sorted(os.path.basename(p) for p in glob.glob(os.path.join(SRC, "*.cpp"))
                   if os.path.basename(p) not in SKIP_FILES)
    raws, per_file, headers, shared = plan(files)
    targets = [f for f in (chosen or files) if per_file.get(f)]
    if args[0] == "plan":
        for f in targets:
            print("%-28s -> %s" % (f, ", ".join("%s (%s)" % (b.name, headers[b.name]) for b in per_file[f])))
        print("%d files, %d types (%d shared by identical copies)" % (
            len(targets), sum(len(per_file[f]) for f in targets), len(shared)))
        return 0
    baseline = baseline_by_file()
    written, kept, reverted, header_users = set(), [], [], collections.defaultdict(list)
    for f in targets:
        source, texts = rewrite(f, raws[f], per_file[f], headers, written)
        new_headers = [h for h in texts if not os.path.exists(os.path.join(INCLUDE, h))]
        for h, text in texts.items():
            open(os.path.join(INCLUDE, h), "wb").write(text.encode("latin-1"))
        open(os.path.join(SRC, f), "wb").write(source.encode("latin-1"))
        if "--dry" in args:
            written |= set(texts)
            continue
        result = build_file(f)
        before = baseline.get(f, {})
        lost = None if result is not None else "compile error"
        if result is not None:
            for name, percent in before.items():
                now = result.get(name, 0.0)
                if (percent >= 100.0 and now < 100.0) or now < percent - 1.0:
                    lost = "%s %.1f -> %.1f" % (name, percent, now)
                    break
        if lost:
            open(os.path.join(SRC, f), "wb").write(raws[f].encode("latin-1"))
            for h in new_headers:
                os.remove(os.path.join(INCLUDE, h))
            reverted.append((f, lost))
            print("revert %-28s %s" % (f, lost))
        else:
            written |= set(texts)
            for b in per_file[f]:
                header_users[headers[b.name]].append(f)
            kept.append(f)
            print("kept   %-28s %s" % (f, " ".join(sorted({headers[b.name] for b in per_file[f]}))))
    not_standalone = [h for h in sorted(written) if not standalone_ok(h)]
    print("kept %d files, reverted %d; headers written %d" % (len(kept), len(reverted), len(written)))
    if not_standalone:
        print("headers that do not compile on their own:", " ".join(not_standalone))
    return 0


if __name__ == "__main__":
    sys.exit(main())
