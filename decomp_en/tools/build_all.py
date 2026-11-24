"""build_all.py : compile every decomp_en/src/*.cpp with build.bat, print per-file and total matches, and write the EN
progress report.

usage: python tools/build_all.py [--report ../progress/en/report.json]

Same report format as the JP project (objdiff-style JSON for ByteMatch / decomp.dev, plus progress.svg next to it).
Game functions come from decomp_en/classify.tsv (class "game"); functions not yet decompiled are listed in the unit
"en/unmatched" at 0 %. "Matched under the rules" uses the JP project's rule_check (a matched function counts only when
its file declares no struct/class/union and no extern global of its own).
Percentages: MATCH = 100, DIFF = share of identical bytes, capped at 98.
"""
import glob, json, os, re, subprocess, sys

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(here)
sys.path.insert(0, here)
from enimg import game_functions, ROOT  # noqa: E402
sys.path.insert(1, os.path.join(ROOT, "decomp", "tools"))
from rule_check import file_violations  # noqa: E402  (read-only reuse of the JP rule checker)


def load_symbol_addresses():
    names = {}
    path = os.path.join(root, "obj_symbols.tsv")
    if os.path.exists(path):
        for line in open(path, encoding="utf-8"):
            parts = line.rstrip("\n").split("\t")
            if len(parts) > 1 and parts[1] and not line.startswith("#"):
                names[parts[0]] = parts[1].lower()
    return names


def address_of(name, symbols):
    if name in symbols:
        return symbols[name]
    m = re.search(r"(?:FUN_|M_)(00[4-8][0-9a-fA-F]{5})", name)
    return m.group(1).lower() if m else None


def measures(functions):
    total_code = sum(f["size"] for f in functions)
    matched = [f for f in functions if f["percent"] >= 100.0]
    matched_code = sum(f["size"] for f in matched)
    fuzzy = sum(f["size"] * f["percent"] for f in functions) / total_code if total_code else 0.0
    complete = bool(functions) and len(matched) == len(functions)
    ruled = [f for f in matched if f.get("rules_clean")]
    ruled_code = sum(f["size"] for f in ruled)

    def pct(a, b):
        return round(100.0 * a / b, 4) if b else 0.0
    return {
        "fuzzy_match_percent": round(fuzzy, 4),
        "total_code": str(total_code), "matched_code": str(matched_code),
        "matched_code_percent": pct(matched_code, total_code),
        "total_data": "0", "matched_data": "0", "matched_data_percent": 0.0,
        "total_functions": len(functions), "matched_functions": len(matched),
        "matched_functions_percent": pct(len(matched), len(functions)),
        "complete_code": str(total_code if complete else 0), "complete_code_percent": 100.0 if complete else 0.0,
        "complete_data": "0", "complete_data_percent": 0.0,
        "total_units": 1, "complete_units": 1 if complete else 0,
        "matched_rules_code": str(ruled_code), "matched_rules_code_percent": pct(ruled_code, total_code),
        "matched_rules_functions": len(ruled), "matched_rules_functions_percent": pct(len(ruled), len(functions)),
    }


def unit_entry(name, source, functions):
    m = measures(functions)
    return {
        "name": name,
        "measures": m,
        "sections": [{"name": ".text", "size": m["total_code"], "fuzzy_match_percent": m["fuzzy_match_percent"]}],
        "functions": [{"name": f["name"], "size": str(f["size"]), "fuzzy_match_percent": round(f["percent"], 4),
                       "metadata": {"virtual_address": str(int(f["address"], 16))}} for f in functions],
        "metadata": {"complete": m["complete_units"] == 1, "source_path": source},
    }


def main():
    args = sys.argv[1:]
    report_path = args[args.index("--report") + 1] if "--report" in args else None
    game = game_functions()
    symbols = load_symbol_addresses()
    units, all_functions, seen = [], [], set()
    unique_matched = matched_bytes = ruled_matched = ruled_bytes = 0
    for path in sorted(glob.glob(os.path.join(root, "src", "*.cpp"))):
        name = os.path.basename(path)
        rules_clean = not file_violations(path)
        out = subprocess.run(["cmd", "/c", os.path.join(root, "build.bat"), name], capture_output=True, text=True).stdout
        functions = []
        for line in out.splitlines():
            if line.startswith("DIFF") or "error" in line:
                print(name, line)
            m = re.match(r"MATCH (\S+) \((\d+) bytes", line)
            d = re.match(r"DIFF (\S+) len ours=(\d+), diffs=(\d+)", line)
            if m:
                sym, percent = m.group(1), 100.0
            elif d:
                sym = d.group(1)
                percent = min(98.0, max(0.0, 100.0 * (1 - int(d.group(3)) / max(1, int(d.group(2))))))
            else:
                continue
            addr = address_of(sym, symbols)
            if addr is None or addr not in game or addr in seen:
                continue
            seen.add(addr)
            if percent >= 100.0:
                unique_matched += 1
                matched_bytes += game[addr]
                if rules_clean:
                    ruled_matched += 1
                    ruled_bytes += game[addr]
            functions.append({"name": sym, "size": game[addr], "percent": percent, "address": addr,
                              "rules_clean": rules_clean})
        if not re.search(r"matched (\d+), differing (\d+)", out):
            print(name, "BUILD FAILED")
            print(out[-2000:])
        all_functions += functions
        if functions:
            units.append(unit_entry("en/" + os.path.splitext(name)[0],
                                    os.path.relpath(path, ROOT).replace(os.sep, "/"), functions))
    code = sum(game.values())
    print("PROGRESS %d of %d game functions matched (%.1f%%), %d of %d code bytes (%.2f%%)" % (
        unique_matched, len(game), 100.0 * unique_matched / len(game), matched_bytes, code, 100.0 * matched_bytes / code))
    print("RULES %d of %d matched functions are matched under the rules, %d code bytes (%.2f%%)" % (
        ruled_matched, unique_matched, ruled_bytes, 100.0 * ruled_bytes / code))
    if report_path:
        rest = [{"name": "FUN_" + a, "size": s, "percent": 0.0, "address": a} for a, s in sorted(game.items()) if a not in seen]
        units.append(unit_entry("en/unmatched", "", rest))
        everything = [{"size": f["size"], "percent": round(f["percent"], 4), "rules_clean": f.get("rules_clean")}
                      for f in all_functions + rest]
        total = measures(everything)
        total["total_units"] = len(units)
        total["complete_units"] = sum(1 for u in units if u["metadata"]["complete"])
        os.makedirs(os.path.dirname(os.path.abspath(report_path)), exist_ok=True)
        with open(report_path, "w", encoding="utf-8", newline="\n") as f:
            json.dump({"measures": total, "units": units}, f, indent=1)
        subprocess.run([sys.executable, os.path.join(ROOT, "decomp", "tools", "progress_svg.py"), report_path,
                        os.path.join(os.path.dirname(os.path.abspath(report_path)), "progress.svg")], check=True)
        print("report (en): %d of %d functions, %.2f%% of code matched (%d under the rules), fuzzy %.2f%%" % (
            total["matched_functions"], total["total_functions"], total["matched_code_percent"],
            total["matched_rules_functions"], total["fuzzy_match_percent"]))


if __name__ == "__main__":
    main()
