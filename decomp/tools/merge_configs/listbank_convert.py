"""listbank_convert.py (tools/merge_configs, done 2026-11-22; kept as an example of a hand-written merge):
ListBank merge (listbank.cpp, listbank_vars.cpp, resource_banks.cpp, type_parse.cpp, game_reset.cpp,
script_queue.cpp) onto include/listbank.h. Byte-level, keeps line endings. Run from decomp/: python tools/merge_configs/listbank_convert.py <file.cpp>..."""
import re, sys
def edit(path, fn):
    raw = open(path, "rb").read().decode("utf-8")
    out = []
    for line in raw.split("\n"):
        c, s, k = line.partition("//")
        out.append(fn(c) + s + k)
    open(path, "wb").write("\n".join(out).encode("utf-8"))
def cut(path, pats, include):
    d = open(path, "rb").read().decode("utf-8")
    first = None
    for p in pats:
        m = re.search(p, d, re.S | re.M)
        if not m:
            raise SystemExit("%s: %s not found" % (path, p))
        first = m.start() if first is None or m.start() < first else first
        d = d[:m.start()] + "\x00" + d[m.end():]
    nl = "\r\n" if "\r\n" in d else "\n"
    i = d.index("\x00"); d = d[:i] + '#include "%s"%s' % (include, nl) + d[i + 1:]
    d = d.replace("\x00", "")
    open(path, "wb").write(d.encode("utf-8"))
STRUCT = lambda n: r"((?:[ \t]*//[^\r\n]*\r?\n)*)struct %s\r?\n\{.*?\n\};[ \t]*\r?\n(?:[ \t]*\r?\n)?" % n
TYPEDEF = r"((?:[ \t]*//[^\r\n]*\r?\n)*)typedef [^\r\n]*\(\*%s\)[^\r\n]*\r?\n(?:[ \t]*\r?\n)?"
EXT = r'extern "C" \w+ DAT_104687b8;[^\r\n]*\r?\n'
BASE = {"m_000c": "variableNames", "m_400c": "variableTypes", "m_2a5dc": "variableCounts", "m_e80c": "borrowed",
        "names": "builtinNames", "nameCount": "builtinCount", "globalNameCount": "globalBuiltinCount",
        "m_3632c": "failed", "counts": "variableCounts"}
def ren(code, extra=()):
    code = re.sub(r"\(int\)new int\b", "new int", code)
    code = re.sub(r"\bm_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\]) = new int", r"values\1.ints = new int", code)
    code = re.sub(r"\*\(int \*\)m_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\])", r"*values\1.ints", code)
    code = re.sub(r"\bm_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\]) = \(int\)new char\[0x100\]", r"values\1.texts = new char[1][LISTBANK_TEXT_SIZE]", code)
    code = re.sub(r"\(char \*\)m_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\]) \+ element \* 0x100", r"values\1.texts[element]", code)
    code = re.sub(r"\(char \*\)m_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\])", r"values\1.texts[0]", code)
    code = re.sub(r"\(\(int \*\)m_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\])\)", r"values\1.ints", code)
    code = re.sub(r"\(\(float \*\)m_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\])\)", r"values\1.floats", code)
    code = re.sub(r"\bm_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\]) = value;", r"values\1.value = value;", code)
    code = re.sub(r"\bm_440c(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\]) = 0;", r"values\1.storage = 0;", code)
    code = code.replace("result = (int *)((char *)this + 0x2240c);", "result = resultBuffer;")
    m = dict(BASE); m.update(dict(extra))
    for a, b in m.items():
        code = re.sub(r"\b%s\b" % a, b, code)
    return code
which = sys.argv[1:]
if "listbank.cpp" in which:
    p = "src/listbank.cpp"
    cut(p, [TYPEDEF % "ParserCallback", STRUCT("ScriptBlock"), STRUCT("ListBank")], "listbank.h")
    d = open(p, "rb").read().decode("utf-8"); d = re.sub(EXT, "", d); open(p, "wb").write(d.encode("utf-8"))
    edit(p, ren)
if "listbank_vars.cpp" in which:
    p = "src/listbank_vars.cpp"
    cut(p, [TYPEDEF % "ParserCallback", TYPEDEF % "BuiltinHandler", STRUCT("ListBank")], "listbank.h")
    edit(p, lambda c: ren(c, [("names", "variableNames")]))
if "resource_banks.cpp" in which:
    p = "src/resource_banks.cpp"
    cut(p, [STRUCT("ResourceBanks")], "listbank.h")
    def r(c):
        c = re.sub(r"\bitems(\[(?:[^\[\]]|\[[^\[\]]*\])*\]\[(?:[^\[\]]|\[[^\[\]]*\])*\])", r"values\1.storage", c)
        c = re.sub(r"\bm_36324\b", "depth", c)
        return ren(c.replace("ResourceBanks", "ListBank"))
    edit(p, r)
if "type_parse.cpp" in which:
    p = "src/type_parse.cpp"
    cut(p, [STRUCT("TokenReader")], "listbank.h")
    edit(p, lambda c: c.replace("TokenReader::M_100364a0(int source)", "ListBank::ReadType(char **text)")
         .replace("M_10035d20(source,", "ReadArgument(text,").replace("TokenReader", "ListBank"))
if "game_reset.cpp" in which:
    p = "src/game_reset.cpp"
    cut(p, [STRUCT("ResourceBanks")], "listbank.h")
    d = open(p, "rb").read().decode("utf-8"); d = re.sub(EXT, "", d); open(p, "wb").write(d.encode("utf-8"))
    edit(p, lambda c: c.replace("DAT_104687b8.M_10007d50(", "DAT_104687b8.FreeAll("))
if "script_queue.cpp" in which:
    p = "src/script_queue.cpp"
    cut(p, [STRUCT("ScriptCompiler")], "listbank.h")
    d = open(p, "rb").read().decode("utf-8"); d = re.sub(EXT, "", d); open(p, "wb").write(d.encode("utf-8"))
    edit(p, lambda c: c.replace("DAT_104687b8.Load(", "DAT_104687b8.LoadText("))
