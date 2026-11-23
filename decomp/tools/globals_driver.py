"""globals_driver.py: move the per-file `extern "C"` global declarations into shared headers (AGENTS.md rule 2),
reverting every file that loses a match.

Usage (run from decomp/):
    python tools/globals_driver.py plan              # print what would move where, write nothing
    python tools/globals_driver.py apply             # write the headers, rewrite the files, build, revert losers
    python tools/globals_driver.py apply --nobuild   # write and rewrite only

A global moves when every file declares it with the same type (an unsized `[]` takes the others' size; different sizes keep the global per file) and that
type is a builtin or a type declared in include/*.h. It goes to one header by the subsystem of the files that use it
(SUBSYSTEMS below; per-character tables, whose first dimension is CHAR_SLOT_COUNT, go to char_tables.h). A global
already declared in an include/*.h header keeps that declaration and only loses its per-file copies. The extern
comments of all files are merged into the header line (the longest one, then any other that adds words).

In each converted file the moved extern lines are removed and the headers it now needs are included where the first
removed line stood. After the build, every file with a lost match (by address, against HEAD's report.json) or a
compile error is restored from git; its globals stay declared locally until a later pass.

Files are edited as bytes so each line keeps its own ending.
"""
import collections
import glob
import json
import os
import re
import subprocess
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SRC = os.path.join(ROOT, "src")
INCLUDE = os.path.join(ROOT, "include")
REPORT = os.path.join(ROOT, "..", "progress", "jp", "report.json")

BUILTIN = {"int", "char", "float", "short", "long", "double", "unsigned", "signed", "void", "const", "__int64",
           "bool"}

# header -> (description, file name prefixes). The first header whose prefix matches a file owns the file.
SUBSYSTEMS = [
    ("camera_vars.h", "camera, projection and visibility state",
     ["camera", "projection", "point_transform", "visibility", "sight_check"]),
    ("sound_vars.h", "music, sound effects and movie streams",
     ["bgm_", "sound", "dshow_stream", "volume_fade"]),
    ("script_vars.h", "script lists, events and flags",
     ["listbank", "script_queue", "type_parse", "event_slots", "flag_list", "text_token"]),
    ("ui_vars.h", "menus, text, input and the HUD panels",
     ["ime", "input_", "joystick_buttons", "menu_", "text_", "font_metrics", "digit_counter", "status_panel",
      "tutorial_page_next", "message_open", "dialog_yesno", "name_banners", "key_prompt_draw", "ability_text",
      "game_panels", "item_caption", "item_icon", "map_icons", "map_movie_select", "hud_fade", "message_fades"]),
    ("map_vars.h", "maps, regions, routes and collision",
     ["map_", "region_view", "group_tables", "route_", "path_", "collide_", "ground_height", "wall_flags",
      "ledge_probe", "water_ice", "circle_intersect", "box_overlap"]),
    ("char_vars.h", "characters, enemies, NPCs and the objects they use",
     ["char_", "actor_scan", "companion_update", "player_", "weapon_attach", "carry_stack", "head_look",
      "helmet_face", "face_anim", "pickups", "damage_zone", "party_block", "npc_", "enemy_", "kind4a_turret",
      "shaga_", "ai_pick", "object_spawn", "debris", "falling_drops", "balloon_burst", "switch", "pushing_platform",
      "inventory", "itm_scramble", "slot_alloc", "slot_defaults"]),
    ("render_vars.h", "the renderer, textures, meshes, scene nodes and effects",
     ["render_", "engine_", "draw_list", "buffer_pool", "screen_", "surface_copy", "textur", "texcache", "mesh_",
      "model_", "node_", "scene_", "world_", "tri_batch", "vertex", "overlay_draw", "colour_fades", "mmv_texreg",
      "effect_", "anim_", "bmp_flip", "rle_decode", "window_create", "screenshot", "resource_banks",
      "pulse_scale", "mesh", "vec", "math_float", "containers", "frame_pacer"]),
    ("game_vars.h", "game progress, saves, settings and other game-wide state",
     ["game_", "anti_tamper", "save_", "system_sav", "test_dat", "config_ini", "user_folder", "file_check",
      "loading_tick", "demo_record", "itm_", "app_"]),
]
CHAR_TABLES = ("char_tables.h", "per-character tables, indexed by character slot (0..CHAR_SLOT_COUNT-1)")
FALLBACK = "game_vars.h"
# Files that touch everything count less when choosing a global's home.
GENERIC_FILES = {"game_reset.cpp": 0.25}
# Files never converted (see open.tsv).
SKIP_FILES = {"npc_greet.cpp"}

EXTERN_LINE = re.compile(rb'^extern "C" ([\w\s\*]+?)\s*(\*?\s*)(\w+)\s*((?:\[[^\]]*\])*)\s*;\s*(//.*)?$')


def subsystem_of(file_name):
    for header, _, prefixes in SUBSYSTEMS:
        if any(file_name.startswith(p) for p in prefixes):
            return header
    return FALLBACK


GENERATED_MARK = "tools/globals_driver.py"


def hand_written_headers():
    """include/*.h except the headers this tool generates (they are rewritten from scratch on every run)."""
    return [p for p in glob.glob(os.path.join(INCLUDE, "*.h"))
            if GENERATED_MARK not in open(p, encoding="latin-1").read(400)]


def header_types():
    """Names of the types declared in include/*.h, and the header declaring each."""
    owners = {}
    for path in hand_written_headers():
        text = open(path, encoding="latin-1").read()
        for m in re.finditer(r"^(?:struct|class|union|enum)\s+(\w+)\s*(?::[^{;]*)?\s*\{?\s*$", text, re.M):
            owners.setdefault(m.group(1), os.path.basename(path))
        for m in re.finditer(r"^typedef\s+[^;]*?(\w+)\s*;", text, re.M):
            owners.setdefault(m.group(1), os.path.basename(path))
    return owners


def header_globals():
    """Globals already declared extern in include/*.h (name -> header)."""
    found = {}
    for path in hand_written_headers():
        for line in open(path, "rb").read().split(b"\n"):
            m = EXTERN_LINE.match(line.rstrip(b"\r"))
            if m and b"(" not in line.split(b"//")[0]:
                found[m.group(3).decode()] = os.path.basename(path)
    return found


def normalise_type(base, star):
    text = (base + b" " + star).decode()
    return " ".join(text.replace("*", " * ").split()).replace(" *", "*")


def parse_dims(dims):
    return re.findall(r"\[([^\]]*)\]", dims.decode())


def collect():
    """name -> list of declarations {file, type, dims, comment}."""
    decls = collections.defaultdict(list)
    for path in sorted(glob.glob(os.path.join(SRC, "*.cpp"))):
        name = os.path.basename(path)
        for line in open(path, "rb").read().split(b"\n"):
            line = line.rstrip(b"\r")
            if not line.startswith(b'extern "C"') or b"(" in line.split(b"//")[0]:
                continue
            m = EXTERN_LINE.match(line)
            if not m:
                continue
            decls[m.group(3).decode()].append({
                "file": name, "type": normalise_type(m.group(1), m.group(2)), "dims": parse_dims(m.group(4)),
                "comment": (m.group(5) or b"")[2:].strip().decode("latin-1")})
    return decls


def dim_value(text):
    try:
        return int(text, 0)
    except ValueError:
        return -1


def merge_dims(dim_lists, largest_first=False):
    """One dimension list for every declaration, or None when the dimensions disagree. With `largest_first` the
    first dimension may differ (the largest is kept: the smaller declarations saw only part of the array)."""
    depth = {len(d) for d in dim_lists}
    if len(depth) != 1:
        return None
    count = depth.pop()
    merged = []
    for k in range(count):
        values = {d[k] for d in dim_lists}
        sized = [v for v in values if v]
        if (k > 0 and len(values) > 1) or (len(set(sized)) > 1 and not (k == 0 and largest_first)):
            return None   # different sizes: which one is right is open (sizeof users would change)
        merged.append(max(sized, key=dim_value) if sized else "")
    return merged


def uses_sizeof(name, files):
    pattern = re.compile(rb"sizeof\s*\(?\s*" + name.encode() + rb"\b")
    return any(pattern.search(open(os.path.join(SRC, f), "rb").read()) for f in files)


# Types that may stand for each other in a conflict, the first listed wins on a tie.
SIGNEDNESS_GROUPS = [["char", "unsigned char", "signed char"], ["int", "unsigned int"],
                     ["short", "unsigned short"]]


def resolve_type(name, items):
    """One type for a global declared with different types, or None. A single typed pointer wins over int and void*
    (files that only cleared or compared it); a signedness conflict takes the type most files use."""
    types = [d["type"] for d in items]
    distinct = set(types)
    pointers = [t for t in distinct if t.endswith("*") and t != "void*"]
    if len(pointers) == 1 and distinct <= {pointers[0], "int", "void*"}:
        return pointers[0]
    for group in SIGNEDNESS_GROUPS:
        if distinct <= set(group):
            counts = collections.Counter(types)
            return max(distinct, key=lambda t: (counts[t], -group.index(t)))
    return None


def load_hold():
    """Globals a previous run found to cost a match when resolved (tools/merge_configs/globals_hold.txt, one name per line)."""
    path = os.path.join(ROOT, "tools", "merge_configs", "globals_hold.txt")
    return set(open(path).read().split()) if os.path.exists(path) else set()


def merge_comments(comments):
    comments = sorted({c for c in comments if c}, key=len, reverse=True)
    if not comments:
        return ""
    result = comments[0]
    for c in comments[1:]:
        if not set(re.findall(r"\w+", c.lower())) <= set(re.findall(r"\w+", result.lower())):
            result += "; " + c
    return result


def plan():
    owners = header_types()
    existing = header_globals()
    decls = collect()
    moves = {}      # name -> (header, type, dims, comment)
    held = {}       # name -> reason it stays per file
    hold = load_hold()
    RESOLVED.clear()
    for name, items in decls.items():
        files = [d["file"] for d in items]
        types = {d["type"] for d in items}
        ctype = types.pop() if len(types) == 1 else resolve_type(name, items)
        if ctype is None or (len(types) > 0 and name in hold):
            held[name] = "type conflict: " + " / ".join(sorted({d["type"] for d in items}))
            continue
        words = set(re.findall(r"\w+", ctype)) - BUILTIN
        unknown = [w for w in words if w not in owners]
        if unknown:
            held[name] = "file-local type " + ", ".join(unknown)
            continue
        dims = merge_dims([d["dims"] for d in items])
        if dims is None and name not in hold:
            smaller = [d["file"] for d in items if d["dims"] and d["dims"][0]]
            dims = merge_dims([d["dims"] for d in items], largest_first=True)
            if dims is not None and uses_sizeof(name, [f for f in smaller]):
                dims = None
        if dims is None:
            held[name] = "dimension conflict: " + " / ".join(sorted({"".join("[%s]" % x for x in d["dims"])
                                                                     for d in items}))
            continue
        differ = {d["file"] for d in items if d["type"] != ctype or (d["dims"] and d["dims"] != dims)}
        if differ:
            RESOLVED[name] = differ
        comment = merge_comments(d["comment"] for d in items)
        if name in existing:
            moves[name] = (existing[name], ctype, dims, comment, True)
            continue
        if dims and dims[0].lower() in ("0x80", "128", "char_slot_count"):
            header = CHAR_TABLES[0]
        else:
            weight = collections.Counter()
            for d in items:
                weight[subsystem_of(d["file"])] += GENERIC_FILES.get(d["file"], 1.0)
            order = [s[0] for s in SUBSYSTEMS]
            header = max(weight, key=lambda h: (weight[h], -order.index(h)))
        moves[name] = (header, ctype, dims, comment, False)
        for dim in dims:
            for word in re.findall(r"\b[A-Za-z_]\w*", dim):
                if word in ("CHAR_SLOT_COUNT",) or word in MACROS:
                    continue
                MACROS[word] = find_define(word, [d["file"] for d in items])
    return decls, moves, held, owners


# Globals whose type or size was settled by this run: name -> the files that declared it otherwise.
RESOLVED = {}

# Array-size constants used by moved declarations: name -> the `#define` line (with its comment) from a source file.
MACROS = {}


def find_define(word, files):
    for name in files:
        for line in open(os.path.join(SRC, name), "rb").read().split(b"\n"):
            line = line.rstrip(b"\r").decode("latin-1")
            if re.match(r"#define\s+%s\b" % word, line):
                return line
    raise SystemExit("no #define for array size %s in %s" % (word, files))


def address_key(name):
    m = re.search(r"([0-9a-fA-F]{8})$", name)
    return (0, int(m.group(1), 16)) if m else (1, name)


def generated_rows(header, skip):
    """The declarations already in a generated header (kept when a later run adds more), and its size constants."""
    path = os.path.join(INCLUDE, header)
    if not os.path.exists(path):
        return []
    rows = []
    for line in open(path, "rb").read().split(b"\n"):
        line = line.rstrip(b"\r")
        m = EXTERN_LINE.match(line)
        if m and m.group(3).decode() not in skip:
            rows.append((m.group(3).decode(), normalise_type(m.group(1), m.group(2)), parse_dims(m.group(4)),
                         (m.group(5) or b"")[2:].strip().decode("latin-1")))
        d = re.match(rb"#define\s+(\w+)\s", line)
        if d and not d.group(1).startswith(b"GURUMIN_") and d.group(1) != b"CHAR_SLOT_COUNT":
            MACROS.setdefault(d.group(1).decode(), line.decode("latin-1"))
    return rows


def write_headers(moves, owners):
    by_header = collections.defaultdict(list)
    for name, (header, ctype, dims, comment, existing) in moves.items():
        if not existing:
            by_header[header].append((name, ctype, dims, comment))
    descriptions = {h: d for h, d, _ in SUBSYSTEMS}
    descriptions[CHAR_TABLES[0]] = CHAR_TABLES[1]
    for header, rows in by_header.items():
        rows.extend(generated_rows(header, {r[0] for r in rows}))
        guard = "GURUMIN_" + header.replace(".", "_").upper()
        includes, forwards = set(), set()
        for name, ctype, dims, comment in rows:
            for word in set(re.findall(r"\w+", ctype)) - BUILTIN:
                if ctype.endswith("*"):
                    forwards.add(word)
                else:
                    includes.add(owners[word])
        out = ["// Globals of " + descriptions[header] + ".",
               "// Generated from the per-file extern declarations by tools/globals_driver.py; this is the one",
               "// declaration of each of them: include this header, never redeclare one. Comments say what is known.",
               "#ifndef " + guard, "#define " + guard, ""]
        if header == CHAR_TABLES[0]:
            out += ["// Number of character slots (also defined by char_state.h and char_slot.h).",
                    "#ifndef CHAR_SLOT_COUNT", "#define CHAR_SLOT_COUNT 0x80", "#endif", ""]
        used = [w for w in MACROS if any(re.search(r"\b%s\b" % w, "".join(r[2])) for r in rows)]
        for word in sorted(used):
            out += ["#ifndef " + word, MACROS[word], "#endif"]
        if used:
            out.append("")
        for inc in sorted(includes):
            out.append('#include "%s"' % inc)
        if includes:
            out.append("")
        for fwd in sorted(forwards - {w for w in forwards if owners[w] in includes}):
            kind = "struct"
            text = open(os.path.join(INCLUDE, owners[fwd]), encoding="latin-1").read()
            if re.search(r"^class\s+%s\b" % fwd, text, re.M):
                kind = "class"
            out.append("%s %s;   // %s" % (kind, fwd, owners[fwd]))
        if forwards:
            out.append("")
        for name, ctype, dims, comment in sorted(rows, key=lambda r: address_key(r[0])):
            dim_text = "".join("[%s]" % ("CHAR_SLOT_COUNT" if (k == 0 and header == CHAR_TABLES[0]) else d)
                               for k, d in enumerate(dims))
            stars = len(ctype) - len(ctype.rstrip("*"))
            spelled = ctype.rstrip("*") + " " + "*" * stars
            line = 'extern "C" %s%s%s;' % (spelled, name, dim_text)
            if comment:
                line = line.ljust(56) + "  // " + comment
            out.append(line)
        out += ["", "#endif", ""]
        open(os.path.join(INCLUDE, header), "wb").write("\n".join(out).encode("latin-1"))
    return sorted(by_header)


def rewrite_file(path, moves):
    """Removes the moved extern lines and includes their headers where the first one stood. Returns True if changed."""
    data = open(path, "rb").read()
    lines = data.split(b"\n")
    keep, needed, first = [], [], None
    for line in lines:
        bare = line.rstrip(b"\r")
        m = EXTERN_LINE.match(bare) if bare.startswith(b'extern "C"') and b"(" not in bare.split(b"//")[0] else None
        if m and m.group(3).decode() in moves:
            header = moves[m.group(3).decode()][0]
            if header not in needed:
                needed.append(header)
            if first is None:
                first = len(keep)
            continue
        keep.append(line)
    if first is None:
        return False
    moved_names = [m.group(3) for m in (EXTERN_LINE.match(l.rstrip(b"\r")) for l in lines) if m]
    moved_names = [n for n in moved_names if n.decode() in moves]
    used_at = [k for k, l in enumerate(keep[:first]) if not l.lstrip().startswith(b"//") and
               any(re.search(rb"\b" + n + rb"\b", l) for n in moved_names)]
    if used_at:
        # a moved global is used above its old extern line: include right after the last #include above that use
        includes_above = [k for k in range(used_at[0]) if keep[k].startswith(b"#include")]
        if includes_above:
            first = includes_above[-1] + 1
        else:
            first = 0
            while first < len(keep) and keep[first].startswith(b"//"):
                first += 1
    newline = b"\r\n" if b"\r\n" in data else b"\n"
    present = set(re.findall(rb'#include "([^"]+)"', data))
    add = [h for h in sorted(needed) if h.encode() not in present]
    cr = b"\r" if newline == b"\r\n" else b""
    block = [b'#include "%s"' % h.encode() + cr for h in add]
    if block and first < len(keep) and keep[first].strip() and not keep[first].startswith(b"#include"):
        block.append(cr)   # a blank line between the includes and the declarations that follow
    keep[first:first] = block
    open(path, "wb").write(b"\n".join(keep))
    return True


def matched_by_address(report):
    result = {}
    for unit in report["units"]:
        for f in unit.get("functions", []):
            address = f.get("metadata", {}).get("virtual_address")
            result[address or f["name"]] = (f["fuzzy_match_percent"], unit["metadata"].get("source_path", ""))
    return result


def main():
    args = sys.argv[1:]
    if not args or args[0] not in ("plan", "apply"):
        print(__doc__)
        return 2
    decls, moves, held, owners = plan()
    counts = collections.Counter(m[0] for m in moves.values())
    print("move %d globals, keep %d per file" % (len(moves), len(held)))
    for header, n in counts.most_common():
        print("  %-18s %d" % (header, n))
    if args[0] == "plan":
        for name, reason in sorted(held.items()):
            print("  held %s: %s" % (name, reason))
        return 0
    headers = write_headers(moves, owners)
    changed = []
    for path in sorted(glob.glob(os.path.join(SRC, "*.cpp"))):
        if os.path.basename(path) in SKIP_FILES:
            continue
        if rewrite_file(path, moves):
            changed.append(os.path.basename(path))
    print("headers:", " ".join(headers))
    print("files rewritten:", len(changed))
    json.dump({"held": held, "changed": changed}, open(os.path.join(ROOT, "build", "globals_driver.json"), "w"),
              indent=1)
    if "--nobuild" in args:
        return 0
    baseline = matched_by_address(json.loads(subprocess.check_output(
        ["git", "show", "HEAD:progress/jp/report.json"], cwd=os.path.join(ROOT, ".."))))
    reverted = set()
    for attempt in range(6):
        subprocess.call([sys.executable, os.path.join(ROOT, "tools", "build_all.py"), "--report", REPORT],
                        cwd=ROOT, stdout=subprocess.DEVNULL)
        now = matched_by_address(json.load(open(REPORT)))
        losers = set()
        for key, (percent, source) in baseline.items():
            if percent >= 100.0 and now.get(key, (0, ""))[0] < 100.0:
                losers.add(os.path.basename(source))
        losers &= set(changed) - reverted
        # a fuzzy drop of more than a point in a file that saw another type or size also blames that global
        dropped = {os.path.basename(source) for key, (percent, source) in baseline.items()
                   if percent < 100.0 and now.get(key, (0, ""))[0] < percent - 1.0}
        suspects = losers | (dropped & {f for files in RESOLVED.values() for f in files})
        print("build %d: %d files lost a match: %s" % (attempt + 1, len(losers), " ".join(sorted(losers))))
        if not suspects:
            break
        # a loser that saw a global with another type or size: hold those globals and start again from HEAD
        blamed = {name for name, files in RESOLVED.items() if files & suspects}
        if blamed and attempt < 5:
            hold = load_hold() | blamed
            open(os.path.join(ROOT, "tools", "merge_configs", "globals_hold.txt"), "w").write("\n".join(sorted(hold)) + "\n")
            print("  holding %d resolved globals: %s" % (len(blamed), " ".join(sorted(blamed))))
            subprocess.call(["git", "checkout", "--", "src", "include"], cwd=ROOT)
            for header in headers:
                if not subprocess.call(["git", "ls-files", "--error-unmatch", "include/" + header], cwd=ROOT,
                                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) == 0:
                    os.remove(os.path.join(INCLUDE, header))
            MACROS.clear()
            decls, moves, held, owners = plan()
            headers = write_headers(moves, owners)
            changed = [os.path.basename(p) for p in sorted(glob.glob(os.path.join(SRC, "*.cpp")))
                       if os.path.basename(p) not in SKIP_FILES and rewrite_file(p, moves)]
            continue
        if not losers:
            break
        for name in losers:
            subprocess.call(["git", "checkout", "--", "src/" + name], cwd=ROOT)
        reverted |= losers
    log = os.path.join(ROOT, "build", "globals_driver.log")
    open(log, "w").write("reverted\n" + "\n".join(sorted(reverted)) + "\n")
    print("reverted %d files (list in build/globals_driver.log)" % len(reverted))
    return 0


if __name__ == "__main__":
    sys.exit(main())
