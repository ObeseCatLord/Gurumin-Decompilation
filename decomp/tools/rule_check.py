"""rule_check.py : find the per-file declarations that break AGENTS.md rule 2 (one declaration per type and global).

usage: python tools/rule_check.py [file.cpp ...]   (default: every src/*.cpp and src_od/*.cpp)

A source file is "clean" when it declares no struct/class/union of its own (definitions with a body; forward
declarations such as `struct Node;` are fine) and no `extern` global variable of its own (function prototypes are
fine; a function-pointer global such as `extern "C" void (*DAT_11b1ba40)(void *);` is a global). Types and globals
belong in shared headers under decomp/include/, which this check does not read.

build_all.py imports file_violations() to count the functions that are "matched under the rules": matched byte for
byte and defined in a clean file. Run this script alone to list each file's violations, files with the most matched
functions first (it reads the last progress report for the counts when one exists).
"""
import glob, json, os, re, sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(here, "..")

COMMENT_OR_STRING = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.S)
TYPE_DEFINITION = re.compile(r'\b(struct|class|union)\s+(?:__declspec\s*\([^)]*\)\s*)?(\w*)\s*(?::[^{;()]*)?\{')
EXTERN_START = re.compile(r'\bextern\s+(?:"C"\s+)?')
FUNCTION_POINTER_NAME = re.compile(r'\(\s*(?:__\w+\s+)?\*+\s*(\w+)\s*\)')
DECLARED_NAME = re.compile(r'(\w+)\s*(?:\[[^\]]*\]\s*)*(?:=.*)?$', re.S)


def strip_comments(text):
    """Blank out comments; keep string literals as "" so `extern "C"` stays recognisable."""
    def replace(match):
        token = match.group(0)
        if token.startswith('"'):
            return '"C"' if token == '"C"' else '""'
        if token.startswith("'"):
            return "' '"
        return " " * len(token) if "\n" not in token else "\n" * token.count("\n")
    return COMMENT_OR_STRING.sub(replace, text)


def file_violations(path):
    """Return [(kind, name)] for each type definition ("type") and extern global ("global") declared in path."""
    with open(path, encoding="cp932", errors="replace") as f:
        text = strip_comments(f.read())
    violations = [("type", m.group(2) or "<anonymous %s>" % m.group(1)) for m in TYPE_DEFINITION.finditer(text)]
    for m in EXTERN_START.finditer(text):
        rest = text[m.end():]
        end = min([i for i in (rest.find(";"), rest.find("{")) if i >= 0] or [len(rest)])
        if end < len(rest) and rest[end] == "{":
            continue                                    # a definition (function body), not a declaration
        declaration = rest[:end].strip()
        pointer = FUNCTION_POINTER_NAME.search(declaration)
        if pointer:
            violations.append(("global", pointer.group(1)))
        elif "(" not in declaration:
            name = DECLARED_NAME.search(declaration)
            violations.append(("global", name.group(1) if name else declaration))
        # anything else with parentheses is a function prototype: allowed
    return violations


def source_files():
    return sorted(glob.glob(os.path.join(root, "src", "*.cpp"))) + sorted(glob.glob(os.path.join(root, "src_od", "*.cpp")))


def matched_counts_from_report():
    """{source file name: matched function count} from progress/jp/report.json, or {} without a report."""
    path = os.path.join(root, "..", "progress", "jp", "report.json")
    if not os.path.exists(path):
        return {}
    report = json.load(open(path, encoding="utf-8"))
    return {os.path.basename(u["metadata"]["source_path"]): u["measures"]["matched_functions"]
            for u in report["units"] if u["metadata"].get("source_path")}


def main():
    paths = sys.argv[1:] or source_files()
    matched = matched_counts_from_report()
    rows = []
    for path in paths:
        violations = file_violations(path)
        if violations:
            rows.append((matched.get(os.path.basename(path), 0), os.path.basename(path), violations))
    rows.sort(key=lambda r: (-r[0], r[1]))
    for count, name, violations in rows:
        types = [n for k, n in violations if k == "type"]
        globals_ = [n for k, n in violations if k == "global"]
        print("%-28s matched %3d  types %2d  globals %2d  %s" % (
            name, count, len(types), len(globals_), " ".join(types + globals_)[:160]))
    print("%d of %d files break rule 2; %d matched functions wait in them" % (
        len(rows), len(paths), sum(r[0] for r in rows)))


if __name__ == "__main__":
    main()
