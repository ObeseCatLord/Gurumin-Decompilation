"""merge_driver.py: move per-file struct views onto one shared header, file by file, reverting any file that loses
a match.

Usage (run from decomp/):
    python tools/merge_driver.py <config.py> <tag> <file.cpp>... [--keep] [--nobuild]

    <config.py>  describes one type merge (see tools/merge_configs/*.py for the ones used so far)
    <tag>        names the log file build/merge_<tag>.log
    <file.cpp>   source files under src/ to convert (run several copies in parallel on disjoint file lists)
    --keep       leave a failed file converted (for inspection) instead of reverting it with git checkout
    --nobuild    convert only, no build or comparison

For each file the driver:
  1. removes the local view structs named in the config (VIEWS + FILE_VIEWS[file]) together with the comment lines
     directly above them, and writes `#include "<HEADER>"` where the first removed struct stood (or after the leading
     #include lines);
  2. removes the extern lines of the global (regex EXTERN) and applies PRE_RAW[file] regexes to the whole file;
  3. per line, in the code part only (comments untouched): applies RAW[file] regexes, renames view type names to
     the header's (TYPES, and every removed view -> CANON), and renames fields reached through a root - the global
     GLOBAL with its indexes, or a pointer variable declared with a view type - using COMMON and RENAME[file];
  4. rebuilds the file with build.bat and compares the matched addresses with HEAD's progress/jp/report.json;
  5. when matched C++ symbols vanished (new mangled names), adds obj_symbols.tsv rows that point each new
     `?Method@Class@@...` at the address of the existing row with the same method and class (classes renamed
     through ROW_CLASS count as the same), or uses ROWS(file) from the config if it defines one, and rebuilds;
  6. reverts the file (git checkout) on a compile error or any lost match, and logs why.

Files are edited as bytes so each line keeps its own ending (some files mix CRLF and LF). After a batch, run
`python tools/build_all.py --report ../progress/jp/report.json` and `python tools/check_matches.py`; a renamed
symbol shows as LOST + new at the same address, which is fine.

Config module names:
    HEADER      header file name to include, e.g. "effect_manager.h"
    CANON       the header's type name
    GLOBAL      the global (an extern "C" name) whose fields are renamed through it
    EXTERN      regex matching the extern line(s) to drop
    VIEWS       struct names to remove where present; FILE_VIEWS = {file: [more names]}
    TYPES       {old type name: new type name}; views not listed become CANON
    COMMON      {old field: new field} for every file; RENAME = {file: {old: new}}
    RAW         {file: [(regex, replacement)]} applied per line to code; PRE_RAW the same on the whole file first
    NO_INCLUDE  {file: [headers not to add]}
    optional: FILE_PTRS {file: [extra root variable names]}, ROW_CLASS {old class: new class}, ROWS(file)
"""
import importlib.util
import json
import os
import re
import subprocess
import sys

# A balanced index expression, up to three levels deep: [i], [a[i]], [a[b[i]]].
INDEX = r"\[(?:[^\[\]]|\[(?:[^\[\]]|\[[^\[\]]*\])*\])*\]"
# A symbol whose name carries its address (FUN_xxxxxxxx, DAT_xxxxxxxx, M_xxxxxxxx).
ADDRESS_NAME = re.compile(r"^(?:_|\?)?(?:FUN|DAT|M)_([0-9a-fA-F]{8})(?:@|$)")
# compare_obj symbol tuple: (name, section, size, type, storage class); 0x20 = function, 2 = external.
SYMBOL_TYPE_FUNCTION = 0x20
STORAGE_EXTERNAL = 2


def edit_struct_blocks(path, remove=(), include=(), raw_subs=()):
    """Removes each `struct Name { ... };` block in `remove` (and the comment lines directly above it), writes the
    #include lines for `include` where the first removed block stood (or after the leading #include lines), then
    applies the whole-file regexes `raw_subs`. Keeps the file's newline style."""
    data = open(path, "rb").read().decode("latin-1")
    newline = "\r\n" if "\r\n" in data else "\n"
    for name in remove:
        pattern = re.compile(r"((?:[ \t]*//[^\r\n]*\r?\n)*)[ \t]*struct %s[ \t]*\r?\n\{.*?\n\};[ \t]*\r?\n"
                             r"(?:[ \t]*\r?\n)?" % name, re.S)
        found = pattern.search(data)
        if not found:
            raise SystemExit("%s: struct %s not found" % (path, name))
        data = data[:found.start()] + "\x00" + data[found.end():]
    includes = "".join('#include "%s"%s' % (h, newline) for h in include if ('#include "%s"' % h) not in data)
    if "\x00" in data:
        i = data.index("\x00")
        data = data[:i] + (includes + newline if includes else "") + data[i + 1:]
        data = data.replace("\x00", "")
    elif includes:
        # After the last #include that comes before the first declaration (some files include more at the end).
        first_code = re.search(r"\n(?:extern|struct|typedef|enum|class|union)\b", data)
        limit = first_code.start() if first_code else len(data)
        leading = [m for m in re.finditer(r"#include [^\r\n]*\r?\n", data) if m.start() < limit]
        position = leading[-1].end() if leading else 0
        data = data[:position] + includes + data[position:]
    for pattern, replacement in raw_subs:
        data = re.sub(pattern, replacement, data)
    while newline * 3 in data:
        data = data.replace(newline * 3, newline * 2)
    open(path, "wb").write(data.encode("latin-1"))


def substitute_code(data, function):
    """Applies `function` to the code part of every line (the text before `//`), keeping comments and endings."""
    out = []
    for line in data.split("\n"):
        code, separator, comment = line.partition("//")
        out.append(function(code) + separator + comment)
    return "\n".join(out)


def load_config(path):
    spec = importlib.util.spec_from_file_location("merge_config", path)
    config = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(config)
    return config


def baseline_matches():
    """Matched addresses per source file, from the committed report (HEAD)."""
    report = json.loads(subprocess.run(["git", "show", "HEAD:progress/jp/report.json"], capture_output=True,
                                       text=True, check=True).stdout)
    matches = {}
    for unit in report["units"]:
        matches[unit["name"].split("/")[-1] + ".cpp"] = {
            "%08x" % int(f["metadata"]["virtual_address"])
            for f in unit.get("functions", []) if f.get("fuzzy_match_percent") == 100}
    return matches


def symbol_addresses():
    """obj_symbols.tsv as {mangled name: address}."""
    rows = {}
    for line in open("obj_symbols.tsv"):
        parts = line.rstrip("\r\n").split("\t")
        if len(parts) == 2:
            rows[parts[0]] = parts[1].lower()
    return rows


def build(source):
    """Builds one file; returns (exit code, set of matched addresses, build output)."""
    result = subprocess.run(["cmd", "/c", os.path.abspath("build.bat"), source], capture_output=True, text=True)
    output = result.stdout + result.stderr
    rows = symbol_addresses()
    matched = set()
    for line in output.splitlines():
        found = re.match(r"MATCH (\S+)", line)
        if found:
            by_name = ADDRESS_NAME.match(found.group(1))
            matched.add(by_name.group(1).lower() if by_name else rows.get(found.group(1), found.group(1)))
    return result.returncode, matched, output


def renamed_symbol_rows(source, config):
    """obj_symbols rows for the object's C++ functions that have no row yet: the address of the existing row with
    the same ?Method@Class@@ prefix (classes mapped through ROW_CLASS count as the same), when that is unique."""
    sys.path.insert(0, "tools")
    from compare_obj import parse as parse_object
    rows = symbol_addresses()
    by_prefix = {}
    for name, address in rows.items():
        prefix = re.match(r"(\?\??\w+@\w+@@)", name)
        if prefix:
            by_prefix.setdefault(prefix.group(1), set()).add(address)
    class_renames = getattr(config, "ROW_CLASS", {})
    symbols, _ = parse_object(os.path.join("build", source[:-4] + ".obj"))
    new_rows = []
    for symbol in symbols:
        if not (symbol and symbol[3] == SYMBOL_TYPE_FUNCTION and symbol[4] == STORAGE_EXTERNAL and symbol[2] > 0):
            continue
        name = symbol[0]
        if name in rows:
            continue
        parts = re.match(r"(\?\??)(\w+)@(\w+)@@", name)
        if not parts:
            continue
        keys = [parts.group(0)]
        keys += ["%s%s@%s@@" % (parts.group(1), parts.group(2), old)
                 for old, new in class_renames.items() if new == parts.group(3)]
        keys += ["%s%s@%s@@" % (parts.group(1), old, parts.group(3))
                 for old, new in class_renames.items() if new == parts.group(2)]
        candidates = set()
        for key in keys:
            candidates |= by_prefix.get(key, set())
        if len(candidates) == 1:
            new_rows.append((name, candidates.pop()))
    return new_rows


def convert(source, config):
    """Converts one file (steps 1-3 of the module docstring). Returns the view names it removed."""
    path = os.path.join("src", source)
    text = open(path, "rb").read().decode("latin-1").replace("\r", "")
    views = [v for v in getattr(config, "VIEWS", []) + config.FILE_VIEWS.get(source, [])
             if re.search(r"^struct %s\b[^;\n]*\n\{" % v, text, re.M)]
    pointer_types = set(views) | {config.CANON} | set(getattr(config, "TYPES", {}))
    pointers = set(re.findall(r"\b(?:%s)\s*\*\s*(\w+)" % "|".join(sorted(pointer_types)), text))
    pointers |= set(getattr(config, "FILE_PTRS", {}).get(source, []))
    include = [] if config.HEADER in config.NO_INCLUDE.get(source, ()) else [config.HEADER]
    edit_struct_blocks(path, remove=views, include=include,
                       raw_subs=[(config.EXTERN, "")] + config.PRE_RAW.get(source, []))
    field_renames = dict(config.COMMON)
    field_renames.update(config.RENAME.get(source, {}))
    roots = [config.GLOBAL + "(?:%s)*" % INDEX] + [r"\b%s" % p for p in sorted(pointers)]
    root = "(?:%s)" % "|".join(roots)
    type_renames = dict(getattr(config, "TYPES", {}))
    for view in views:
        type_renames.setdefault(view, config.CANON)

    def rename(code):
        for pattern, replacement in config.RAW.get(source, []):
            code = re.sub(pattern, replacement, code)
        for old, new in type_renames.items():
            code = re.sub(r"\b%s\b" % old, new, code)
        for old, new in field_renames.items():
            code = re.sub(r"(%s)(\.|->)%s\b" % (root, old), lambda k: k.group(1) + k.group(2) + new, code)
        return code

    data = open(path, "rb").read().decode("latin-1")
    open(path, "wb").write(substitute_code(data, rename).encode("latin-1"))
    return views


def main():
    args = sys.argv[1:]
    if len(args) < 3:
        raise SystemExit(__doc__)
    config = load_config(args[0])
    tag = args[1]
    keep = "--keep" in args
    no_build = "--nobuild" in args
    sources = [a for a in args[2:] if not a.startswith("--")]
    os.makedirs("build", exist_ok=True)
    log = open(os.path.join("build", "merge_%s.log" % tag), "a")
    baseline = baseline_matches()
    for source in sources:
        path = os.path.join("src", source)
        views = convert(source, config)
        if no_build:
            print(source, "converted", views)
            continue
        code, matched, output = build(source)
        before = baseline.get(source, set())
        rows = []
        if code == 0 and before - matched:
            rows = config.ROWS(source) if hasattr(config, "ROWS") else renamed_symbol_rows(source, config)
            if rows:
                table = open("obj_symbols.tsv", "rb").read()
                table += b"".join(("%s\t%s\n" % row).encode() for row in rows)
                open("obj_symbols.tsv", "wb").write(table)
                code, matched, output = build(source)
        lost = before - matched
        if code or lost:
            why = "compile error" if code else "lost " + ",".join(sorted(lost))
            if code:
                why += ": " + " | ".join([l.strip()[-170:] for l in output.splitlines() if "error" in l][:4])
            if not keep:
                subprocess.run(["git", "checkout", "--", path])
            log.write("%s\tFAIL\t%s\n" % (source, why))
            print(source, "FAIL", why)
        else:
            log.write("%s\tOK\t%d/%d\trows %s\n" % (source, len(matched & before), len(before), rows))
            print(source, "OK", len(matched & before), "/", len(before), "new", sorted(matched - before), "rows", rows)
        log.flush()


if __name__ == "__main__":
    main()
