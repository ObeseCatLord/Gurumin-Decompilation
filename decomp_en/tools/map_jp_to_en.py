"""map_jp_to_en.py : pair EN game functions (game_en_steam.exe) with their JP twins (APP.dll, Packs.dll, SoundArc.dll,
sound3d.dll) -> decomp_en/jp_en_map.tsv and decomp_en/jp_en_coverage.txt

Run tools/classify.py first (it decides which EN functions are game code).

Method
1. Features per function (both sides): referenced C strings, float constants loaded from data, integer immediates
   (not addresses), and the library functions it calls (Win32 imports and named CRT functions, normalised: no leading
   underscores, lower case). Each feature is weighted by its rarity among EN game functions (idf).
2. Feature pass: every JP/EN pair sharing a rare feature is scored
       score = (1 - exp(-shared/12)) * sqrt(0.6 * shared/JP weight + 0.4 * shared/EN weight) * size factor,
   pairs are accepted greedily, best score first, one EN function per JP function.
3. Call-graph pass (repeated until nothing changes): an unmapped JP function whose mapped callers/callees point at
   the same unmapped EN function (callees of the callers' twins, callers of the callees' twins) is paired with it
   when it is the unique best candidate.
4. Inlining: a JP function whose best EN candidate is already the twin of one of its JP callers is recorded as
   `inlined` into that EN function (VS2013 inlines more than VS .NET 2002: /Ob2 vs /Ob1).

Confidence (0-1) = score with a margin bonus; `high` >= 0.6, `medium` >= 0.35, else `low`. Call-graph pairs carry
the number of agreeing neighbours in the evidence column. Low-confidence rows need a look before relying on them.

Feature kinds in the evidence column: s strings, f float constants, i integer immediates, c library calls.
Columns of jp_en_map.tsv: en_address en_size jp_module jp_address jp_name jp_size jp_status confidence level method
evidence. jp_status is `matched:<src file>` when the JP function is byte-matched (progress/jp/report.json), else
`unmatched`. EN game functions with no JP twin are listed with jp_module `-` and a guess in evidence (launcher,
steam, unknown).
"""
import collections, json, math, os, re, sys

from enimg import EN, ROOT, game_functions
from exe_index import Program

OUT = os.path.join(EN, "jp_en_map.tsv")
COVERAGE = os.path.join(EN, "jp_en_coverage.txt")
JP_MODULES = ["APP.dll", "Packs.dll", "SoundArc.dll", "sound3d.dll"]
APP_GAME_END = 0x101B2000          # APP.dll: CRT/library code starts here (same cut as decomp/triage.tsv users)
STEAM_IMPORTS = "STEAM_API.DLL"


def norm_name(name):
    return name.lstrip("_").lower()


def library_name(prog, callee):
    """name of a library (CRT/MFC) function, or None for game code"""
    name = prog.funcs.get(callee, ("",))[0]
    if not name or name.startswith(("FUN_", "thunk_FUN_", "LAB_")):
        return None
    return norm_name(name)


class Side:
    """the functions of one side (EN or one JP module) with their feature sets"""

    def __init__(self, prog, members, is_library):
        self.prog = prog
        self.members = members                 # set of addresses
        self.is_library = is_library           # address -> bool (callee is library code)
        self.tokens = {}
        for f in members:
            self.tokens[f] = self.features(f)

    def features(self, f):
        p = self.prog
        tok = collections.Counter()
        for s in p.strings(f):
            if len(s) >= 3:
                tok["s:" + s] += 1
        for v in p.float_constants(f):
            tok["f:%g" % v] += 1
        apis, consts = p.features(f)
        for v in consts:
            if v >= 0x40 and v not in (0xFFFFFFFF, 0xFFFF, 0xFF00, 0x100):
                tok["i:%x" % v] += 1
        for name in p.ext_calls.get(f, []):
            tok["c:" + norm_name(name)] += 1
        for c in p.callees.get(f, []):
            if self.is_library(c):
                n = library_name(p, c)
                if n:
                    tok["c:" + n] += 1
        return tok


def load_jp_status():
    status = {}
    path = os.path.join(ROOT, "progress", "jp", "report.json")
    if not os.path.exists(path):
        return status
    report = json.load(open(path, encoding="utf-8"))
    for unit in report["units"]:
        if unit["name"] == "app/unmatched":
            continue
        src = os.path.basename(unit["metadata"].get("source_path", ""))
        for f in unit["functions"]:
            if f["fuzzy_match_percent"] >= 100:
                status[int(f["metadata"]["virtual_address"])] = "matched:" + src
    return status


def load_jp_names():
    names = {}
    for line in open(os.path.join(ROOT, "symbols.tsv"), encoding="utf-8-sig").read().splitlines()[1:]:
        c = line.split("\t")
        if len(c) >= 2 and re.match(r"^10[0-9a-f]{6}$", c[0]):
            names[int(c[0], 16)] = c[1]
    return names


def main():
    en_prog = Program("game_en_steam.exe")
    en_game = {int(a, 16) for a in game_functions()}
    en = Side(en_prog, en_game, lambda c: c not in en_game)

    jp_sides = {}
    for mod in JP_MODULES:
        prog = Program(mod)
        if mod == "APP.dll":
            members = {a for a in prog.funcs if a < APP_GAME_END}
            lib = lambda c, end=APP_GAME_END: c >= end
        else:
            members = {a for a, (n, s) in prog.funcs.items() if n.startswith("FUN_") or a in prog.funcs and s > 0}
            lib = lambda c, p=prog: library_name(p, c) is not None
        jp_sides[mod] = Side(prog, members, lib)

    # rarity weights from the EN game functions
    df = collections.Counter()
    for t in en.tokens.values():
        df.update(set(t))
    n_en = len(en.tokens)

    def weight(tok):
        d = df.get(tok, 0)
        if d == 0:
            return 0.0
        w = math.log(1.0 + n_en / d)
        return w * (1.5 if tok.startswith("s:") else 1.0)

    def total(tok):
        return sum(weight(t) * min(c, 3) for t, c in tok.items())

    en_total = {f: total(t) for f, t in en.tokens.items()}
    index = collections.defaultdict(list)
    for f, t in en.tokens.items():
        for k in t:
            if df[k] <= 25:
                index[k].append(f)

    # 1. feature pass
    candidates = []          # (score, jp key, en address, shared summary)
    best_of = {}
    for mod, side in jp_sides.items():
        for f, tok in side.tokens.items():
            wj = total(tok)
            if wj <= 0:
                continue
            seen = collections.Counter()
            for k in tok:
                for e in index.get(k, ()):
                    seen[e] += 1
            scored = []
            jsize = side.prog.funcs[f][1]
            for e in seen:
                shared = [k for k in tok if k in en.tokens[e]]
                sw = sum(weight(k) * min(tok[k], en.tokens[e][k], 3) for k in shared)
                if sw <= 0:
                    continue
                esize = en_prog.funcs[e][1]
                # EN functions may be up to ~3x larger (more inlining); beyond that the pair gets less likely
                ratio = min(1.0, esize / max(jsize, 1), 3.0 * jsize / max(esize, 1))
                size_factor = 0.4 + 0.6 * ratio
                strength = 1.0 - math.exp(-sw / 12.0)  # one rare token is weak evidence, many are strong
                cover, precision = sw / wj, sw / max(en_total[e], 1e-9)
                score = strength * math.sqrt(0.6 * cover + 0.4 * precision) * size_factor
                strong = sum(1 for k in shared if df[k] <= 3)
                scored.append((score, e, shared, strong))
            scored.sort(key=lambda x: -x[0])
            if scored:
                second = scored[1][0] if len(scored) > 1 else 0.0
                best_of[(mod, f)] = scored[:3]
                for score, e, shared, strong in scored[:3]:
                    margin = score / second if second > 0 else 3.0
                    if strong >= 1 or score >= 0.5:
                        candidates.append((score * min(1.0, 0.75 + 0.1 * margin), (mod, f), e, shared, strong))
    candidates.sort(key=lambda x: -x[0])
    jp_to_en, en_to_jp = {}, {}
    evidence = {}
    for score, key, e, shared, strong in candidates:
        if key in jp_to_en or e in en_to_jp:
            continue
        if score < 0.12:
            continue
        jp_to_en[key] = e
        en_to_jp[e] = key
        kinds = collections.Counter(k.split(":", 1)[0] for k in shared)
        summary = ",".join("%s%d" % (k, v) for k, v in sorted(kinds.items()))
        # counts only: the committed map must not carry text from the game
        evidence[key] = (min(1.0, score), "features", "shared %s, %d rare" % (summary, strong))

    # 2. call-graph pass
    def neighbours(prog, f, members):
        return ([c for c in prog.callers.get(f, []) if c in members], [c for c in prog.callees.get(f, []) if c in members])

    changed = True
    rounds = 0
    while changed and rounds < 20:
        changed = False
        rounds += 1
        proposals = []
        for mod, side in jp_sides.items():
            for f in side.members:
                key = (mod, f)
                if key in jp_to_en:
                    continue
                callers, callees = neighbours(side.prog, f, side.members)
                votes = collections.Counter()
                mapped = 0
                for c in callers:
                    e = jp_to_en.get((mod, c))
                    if e is None:
                        continue
                    mapped += 1
                    for x in en_prog.callees.get(e, []):
                        if x in en_game and x not in en_to_jp:
                            votes[x] += 1
                for c in callees:
                    e = jp_to_en.get((mod, c))
                    if e is None:
                        continue
                    mapped += 1
                    for x in en_prog.callers.get(e, []):
                        if x in en_game and x not in en_to_jp:
                            votes[x] += 1
                if not votes:
                    continue
                ranked = votes.most_common(2)
                top, n = ranked[0]
                runner = ranked[1][1] if len(ranked) > 1 else 0
                jsize, esize = side.prog.funcs[f][1], en_prog.funcs[top][1]
                size_sim = min(jsize, esize) / max(jsize, esize, 1)
                if n > runner and (n >= 2 or (size_sim > 0.5 and runner == 0)):
                    conf = min(0.9, 0.25 + 0.15 * n + 0.2 * size_sim) * (n / max(mapped, 1)) ** 0.5
                    proposals.append((conf, key, top, n, mapped, size_sim))
        proposals.sort(key=lambda x: -x[0])
        for conf, key, e, n, mapped, size_sim in proposals:
            if key in jp_to_en or e in en_to_jp:
                continue
            jp_to_en[key] = e
            en_to_jp[e] = key
            evidence[key] = (conf, "callgraph", "neighbours %d of %d mapped agree, size ratio %.2f" % (n, mapped, size_sim))
            changed = True

    # 3. inlined JP functions: best feature candidate is the twin of a JP caller
    inlined = {}
    for key, scored in best_of.items():
        if key in jp_to_en:
            continue
        mod, f = key
        side = jp_sides[mod]
        callers = set(side.prog.callers.get(f, []))
        for score, e, shared, strong in scored[:2]:
            owner = en_to_jp.get(e)
            if owner and owner[0] == mod and owner[1] in callers and score >= 0.2:
                inlined[key] = e
                break

    write(en_prog, en_game, jp_sides, jp_to_en, en_to_jp, evidence, inlined)


def level(conf):
    return "high" if conf >= 0.6 else "medium" if conf >= 0.35 else "low"


def en_only_reason(prog, f):
    area_launcher = 0x6E5D60 <= f < 0x6F15B0
    if area_launcher:
        return "launcher (EN-only config dialog)"
    calls = prog.ext_calls.get(f, [])
    if any(c.startswith("Steam") or "Steam" in c for c in calls) or any("Steam" in s for s in prog.strings(f)):
        return "steam"
    return "unknown"


def write(en_prog, en_game, jp_sides, jp_to_en, en_to_jp, evidence, inlined):
    status = load_jp_status()
    names = load_jp_names()
    rows = []
    for e in sorted(en_game):
        esize = en_prog.funcs[e][1]
        key = en_to_jp.get(e)
        if key:
            mod, f = key
            conf, method, ev = evidence[key]
            jname = names.get(f, "") if mod == "APP.dll" else ""
            if not jname:
                n = jp_sides[mod].prog.funcs[f][0]
                jname = "" if n.startswith("FUN_") else n
            st = status.get(f, "unmatched") if mod == "APP.dll" else "unmatched"
            folded = [("%s:%08x" % k) for k, v in inlined.items() if v == e]
            if folded:
                ev += "; also inlines " + " ".join(folded[:6])
            rows.append(["%08x" % e, esize, mod, "%08x" % f, jname, jp_sides[mod].prog.funcs[f][1], st,
                         "%.2f" % conf, level(conf), method, ev])
        else:
            rows.append(["%08x" % e, esize, "-", "", "", "", "", "0.00", "none", "en-only", en_only_reason(en_prog, e)])
    with open(OUT, "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\t".join(["en_address", "en_size", "jp_module", "jp_address", "jp_name", "jp_size", "jp_status",
                            "confidence", "level", "method", "evidence"]) + "\n")
        for r in rows:
            fh.write("\t".join(str(x) for x in r) + "\n")

    # coverage report
    mapped = [r for r in rows if r[2] != "-"]
    lines = ["JP -> EN map coverage (decomp_en/tools/map_jp_to_en.py)", ""]
    lines.append("EN game functions: %d (%d bytes)" % (len(rows), sum(r[1] for r in rows)))
    lines.append("  mapped to a JP twin: %d (%d bytes)" % (len(mapped), sum(r[1] for r in mapped)))
    for lv in ("high", "medium", "low"):
        lines.append("    %-6s %d" % (lv, sum(1 for r in mapped if r[8] == lv)))
    for mod in JP_MODULES:
        lines.append("    from %-12s %d" % (mod, sum(1 for r in mapped if r[2] == mod)))
    lines.append("    by features %d, by call graph %d" % (sum(1 for r in mapped if r[9] == "features"),
                                                          sum(1 for r in mapped if r[9] == "callgraph")))
    lines.append("    JP twin already matched: %d" % sum(1 for r in mapped if str(r[6]).startswith("matched")))
    only = [r for r in rows if r[2] == "-"]
    lines.append("  without a JP twin: %d (%d bytes)" % (len(only), sum(r[1] for r in only)))
    for reason in sorted(set(r[10] for r in only)):
        sel = [r for r in only if r[10] == reason]
        lines.append("    %-34s %5d functions %8d bytes" % (reason, len(sel), sum(r[1] for r in sel)))
    lines.append("")
    for mod, side in jp_sides.items():
        n = len(side.members)
        m = sum(1 for k in jp_to_en if k[0] == mod)
        i = sum(1 for k in inlined if k[0] == mod)
        lines.append("JP %-12s %5d functions considered, %5d mapped, %4d inlined into a mapped EN function" % (mod, n, m, i))
    open(COVERAGE, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")
    print("\n".join(lines))


if __name__ == "__main__":
    main()
