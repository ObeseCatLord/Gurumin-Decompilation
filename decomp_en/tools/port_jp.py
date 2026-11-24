"""port_jp.py <EN address> [--brief] : everything needed to port one EN function from its JP twin.

Prints the jp_en_map.tsv row, the matched JP source of the twin (comment block + body, cut from decomp/src/<file>),
the JP symbol rows, the EN disassembly and the EN callees with their JP twins (so callee names can be carried over).
With --brief it prints a ready-to-paste subagent brief instead (the HANDOFF Standing instructions refer to it).

This is the cheap first step of each EN function: when the JP twin is matched, its source usually needs only the EN
differences (more inlining under /Ob2, SSE2 floats, EN-only fields), so a worker starts from a draft instead of from
Ghidra output.
"""
import csv, os, re, subprocess, sys

import capstone
from enimg import EN, ROOT, Image, inventory

JP_SRC = os.path.join(ROOT, "decomp", "src")


def map_rows():
    return {r["en_address"]: r for r in csv.DictReader(open(os.path.join(EN, "jp_en_map.tsv"), encoding="utf-8"),
                                                      delimiter="\t")}


def jp_symbols(address):
    """mangled names of the JP function (decomp/obj_symbols.tsv) plus the FUN_ spelling"""
    names = ["FUN_" + address]
    for line in open(os.path.join(ROOT, "decomp", "obj_symbols.tsv"), encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) > 1 and parts[1].lower() == address:
            names.append(parts[0])
    return names


def source_name(mangled):
    """Class::Method or Function from a simple MSVC mangled name (?Name@Class@@...)"""
    m = re.match(r"\?(\w+)@(\w+)@@", mangled)
    if m:
        return "%s::%s" % (m.group(2), m.group(1))
    m = re.match(r"\?(\w+)@@", mangled)
    return m.group(1) if m else mangled.lstrip("_")


def cut_function(path, name):
    """the comment block and body of the definition of `name` in a source file"""
    lines = open(path, encoding="cp932", errors="replace").read().splitlines()
    pat = re.compile(r"^[A-Za-z_].*\b%s\s*\(" % re.escape(name))
    for i, line in enumerate(lines):
        if pat.match(line) and not line.rstrip().endswith(";"):
            start = i
            while start > 0 and lines[start - 1].startswith("//"):
                start -= 1
            end = i
            while end < len(lines) and lines[end] != "}":
                end += 1
            return "\n".join(lines[start:end + 1])
    return None


def jp_source(row):
    st = row["jp_status"]
    if not st.startswith("matched:"):
        return None, None
    path = os.path.join(JP_SRC, st.split(":", 1)[1])
    for sym in jp_symbols(row["jp_address"]):
        text = cut_function(path, source_name(sym))
        if text:
            return path, text
    return path, None


def disassembly(address):
    img, inv = Image(), inventory()
    a = int(address, 16)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    return ["%08x  %-6s %s" % (i.address, i.mnemonic, i.op_str) for i in md.disasm(img.read(a, inv[a][1]), a)]


def callees(address, rows):
    out = []
    for line in disassembly(address):
        m = re.search(r"call\s+0x([0-9a-f]+)$", line)
        if m:
            t = m.group(1).zfill(8)
            r = rows.get(t)
            twin = "%s %s %s" % (r["jp_module"], r["jp_address"], r["jp_name"]) if r and r["jp_module"] != "-" else "-"
            if t not in [c[0] for c in out]:
                out.append((t, twin))
    return out


def main():
    address = sys.argv[1].lower().replace("0x", "").zfill(8)
    rows = map_rows()
    row = rows.get(address)
    if not row:
        sys.exit("%s is not an EN game function (classify.tsv)" % address)
    path, text = jp_source(row)
    asm = disassembly(address)
    calls = callees(address, rows)
    if "--brief" in sys.argv:
        print(brief(address, row, path, text, asm, calls))
        return
    print("EN %s (%s bytes) <- %s %s %s, %s, confidence %s (%s): %s" % (
        address, row["en_size"], row["jp_module"], row["jp_address"], row["jp_name"], row["jp_status"],
        row["confidence"], row["level"], row["evidence"]))
    if text:
        print("\n--- JP source (%s) ---\n%s" % (os.path.relpath(path, ROOT), text))
    print("\n--- EN callees ---")
    for t, twin in calls:
        print("  %s  JP twin: %s" % (t, twin))
    print("\n--- EN disassembly ---")
    print("\n".join(asm))


def brief(address, row, path, text, asm, calls):
    jp = "%s %s %s" % (row["jp_module"], row["jp_address"], row["jp_name"] or "")
    lines = [
        "Match EN function 0x%s (%s bytes) of Gurumin's English Steam exe in the decomp_en tree of the repository at" % (
            address, row["en_size"]),
        "E:\\Claude Typing of the Dead\\Gurumin\\gurumin-decomp (cd decomp_en). JP twin: %s (%s, map confidence %s)." % (
            jp, row["jp_status"], row["confidence"]),
        "",
        "Rules (AGENTS.md, all hard): typed members, never fixed offsets; each type declared once, in decomp_en/include/;",
        "no fakematches (no dummy locals, volatile, inline asm, steering blocks); no magic numbers (named constants);",
        "a comment above every function (what it does, who calls it) and on every field; informative names.",
        "Do not edit decomp/ (the JP tree). Do not commit. Report: MATCH or the remaining DIFF with what you tried.",
        "",
        "Tools (from decomp_en/): build.bat <file.cpp> (from bash: powershell -c \"cmd /c '.\\build.bat <file.cpp>'\")",
        "prints MATCH/DIFF per function; python tools/sbs.py build/<file>.obj <symbol> [addr] shows both side by side;",
        "python tools/asm.py <addr>, python tools/fn.py <addr> (Ghidra), python tools/port_jp.py <addr> (JP draft).",
        "C++ symbols need a row in decomp_en/obj_symbols.tsv (<mangled name><TAB><EN address>).",
        "Compiler: VS2013 Update 4, /O2 /Oy- /GS- /EHsc (SSE2 floats, /Ob2 inlining: JP helpers that were calls may",
        "be inlined in EN; EN addresses are 0x004xxxxx-0x007xxxxx, data 0x008xxxxx-0x02xxxxxx).",
        "",
        "Write the function in decomp_en/src/<same file name as the JP source, or a new file per area>.",
    ]
    if text:
        lines += ["", "JP source (%s), matched with VS .NET 2002; start from it:" % os.path.relpath(path, ROOT), text]
    else:
        lines += ["", "No matched JP source: start from python tools/fn.py %s and the disassembly." % address]
    lines += ["", "EN callees and their JP twins (use the JP names):"]
    lines += ["  %s  %s" % c for c in calls] or ["  (none)"]
    lines += ["", "EN disassembly:"] + asm
    return "\n".join(lines)


if __name__ == "__main__":
    main()
