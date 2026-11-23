r"""dialogue_tool.py - list and edit the game's text: dialogue and names in bin\map.itm, cutscene subtitles.

    python tools/dialogue_tool.py export <game folder | files...> <out.tsv>
    python tools/dialogue_tool.py import <game folder | files...> <edited.tsv> <out folder>
    python tools/dialogue_tool.py stats <game folder | files...>

Files may be game .itm files or their unscrambled copies (.itm.txt). import writes only the files whose text
changed, into <out folder>\bin\ and <out folder>\3ddata\ (the game's layout: install it like any mod folder).

Text found (the .itm files are plain Shift-JIS text once itm_tool.py has unscrambled them):
- bin\map.itm: Message(speaker, "line 1", "line 2", "line 3", "line 4")  named dialogue box (3,123 calls),
  Mes("text")  short message, SelText("choice", n)  menu choice, #StageName  stage names,
  #ItemTable  item names and descriptions, #StatusTable  character names.
  The speaker number is the row of #CharTable (0 = Parin, 20 = Hyperbolic, ...); 58-65 have no row.
- 3ddata\*.itm cutscene scripts (60 files): '##frame,ZIM,text,seconds,-1,x,y' subtitles (944), '\n' = next line.

The TSV has one row per entry: file, id, script, kind, speaker, speaker name, text1..text4. Change only the text
columns; import writes the rows whose text changed and checks that the original text is still at that place.

Game text rules (the text formatter of APP.dll 0x1014f590, which dialogue goes through):
- digits, '-' and '.' are drawn as their full-width glyphs; '_' is a 1-pixel space
- 'L' + 2 digits and 's' + 3 digits are control codes ('s080' moves the text to about x = 80 * 1.136)
- everything else is read two bytes at a time as Shift-JIS, so other half-width characters would show as garbage:
  game_text() turns them into their full-width forms (A -> Ａ). The full-width space is only 1 pixel wide in the
  game's font, so a space becomes '_____' (5 pixels) in dialogue.
"""
import math
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import itm_tool  # noqa: E402

TEXT_CALLS = {b"Message": 4, b"Mes": 1, b"BTMes": 1, b"SelText": 1, b"WMessage": 2}
TABLE_SECTIONS = (b"#StageName", b"#ItemTable", b"#StatusTable")
FORMATTER_DIGITS = {c: 0x824F + i for i, c in enumerate("0123456789")}
FORMATTER_DIGITS.update({"-": 0x817C, ".": 0x8144})
CONTROL = re.compile(r"s\d{3}|L\d{2}")
# The font's full-width space (0x8140) has no record (the game draws it 1 pixel wide), so a typed space becomes
# five '_' (each a 1-pixel gap in the formatter) in dialogue. Table names keep the full-width space.
DIALOGUE_SPACE = "_____"
# Full-width forms that Shift-JIS keeps outside the font's range (＇ ＂ are IBM extensions): use the JIS quotes.
HALF_TO_FONT = {"'": "’", '"': "”"}
SCENE_TEXT = b",ZIM,"
LINE_BREAK = b"\\n"     # written as backslash + n inside cutscene captions


def game_name(path):
    """The game file's name (the .itm scrambling key): map.itm for map.itm, map.itm.txt or a copy's source."""
    name = os.path.basename(path)
    return name[:-4] if name.lower().endswith(".itm.txt") else name


def load_plain(path):
    data = open(path, "rb").read()
    if path.lower().endswith(".itm"):
        return itm_tool.decode(data, path)[0]
    return data


def save_plain(plain, path, name=None, unknown=None):
    if path.lower().endswith(".itm"):
        data = itm_tool.encode(plain, name or os.path.basename(path), unknown)
    else:
        data = plain
    with open(path, "wb") as f:
        f.write(data)


def code_part(line):
    """End of the code on a line: the first // outside a string (Shift-JIS trail bytes are never '"' or '/')."""
    inside = False
    for i, c in enumerate(line):
        if c == 0x22:
            inside = not inside
        elif c == 0x2F and not inside and line[i + 1:i + 2] == b"/":
            return i
    return len(line)


def string_spans(line, start, end):
    spans, i = [], start
    while True:
        a = line.find(b'"', i, end)
        if a < 0:
            return spans
        b = line.find(b'"', a + 1, end)
        if b < 0:
            return spans
        spans.append((a + 1, b))
        i = b + 1


def char_table(plain):
    """speaker number -> (model base, Japanese name) from #CharTable."""
    rows, inside = {}, False
    for raw in plain.split(b"\n"):
        line = raw.strip()
        if line.startswith(b"#CharTable"):
            inside = True
        elif line.startswith(b"#EndCharTable"):
            break
        elif inside and line[:1].isdigit():
            parts = line.decode("cp932", "replace").split()
            for i in range(2, len(parts) - 5):
                if re.fullmatch(r"-?\d{4}", parts[i + 1]) and re.fullmatch(r"\d{6}", parts[i + 5]):
                    rows.setdefault(int(parts[0]), (parts[i].lower(), " ".join(parts[1:i])))
                    break
    return rows


def speaker_names(plain):
    """speaker number -> display name (English release name when known, else the Japanese one)."""
    try:
        from asset_groups import CHARACTER_NAMES, CHARACTER_ALIASES
    except ImportError:
        CHARACTER_NAMES, CHARACTER_ALIASES = {}, {}
    names = {}
    for number, (base, name) in char_table(plain).items():
        english = CHARACTER_NAMES.get(CHARACTER_ALIASES.get(base, base))
        names[number] = "%s (%s)" % (english, name) if english and english != name else name
    return names


def parse(plain):
    """Entries: dict(id, script, kind, speaker, spans=[(start, end) in plain], texts=[str])."""
    entries, script, section, offset = [], "", b"", 0
    for number, line in enumerate(plain.split(b"\n"), 1):
        start = offset
        offset += len(line) + 1
        stripped = line.strip()
        if stripped.startswith(b"#"):
            head = stripped.split()[0]
            if head.startswith(b"#Script:") or head.startswith(b"#BattleSc"):
                script, section = head.split(b":", 1)[1].decode("ascii", "replace"), b""
            elif head.startswith(b"#End"):
                section = b""
            else:
                section = head
            continue
        if stripped.startswith(b"//") or not stripped:
            continue
        end = code_part(line)
        if section in TABLE_SECTIONS:
            entries += table_entries(line, start, end, section, number)
            continue
        if section or b'"' not in line[:end]:
            continue
        call = re.search(rb"\b(" + b"|".join(TEXT_CALLS) + rb")\s*\(", line[:end])
        if not call:
            continue
        spans = string_spans(line, call.end(), end)[:TEXT_CALLS[call.group(1)]]
        if not spans:
            continue
        speaker = ""
        if call.group(1) == b"Message":
            arg = re.match(rb"\s*([^,]*),", line[call.end():end])
            speaker = arg.group(1).strip().decode("cp932", "replace") if arg else ""
        entries.append(dict(id="%s@%d" % (call.group(1).decode(), number), script=script,
                            kind=call.group(1).decode(), speaker=speaker,
                            spans=[(start + a, start + b) for a, b in spans],
                            texts=[line[a:b].decode("cp932", "replace") for a, b in spans]))
    return entries


def table_entries(line, start, end, section, number):
    tokens = [(m.start(), m.end()) for m in re.finditer(rb"[^ \t\r]+", line[:end])]
    if len(tokens) < 2 or not line[tokens[0][0]:tokens[0][1]].isdigit():
        return []
    kind = section[1:].decode()
    found = [(tokens[1], "name")]
    if section == b"#ItemTable":
        numeric = [i for i, (a, b) in enumerate(tokens) if re.fullmatch(rb"-?\d+", line[a:b])]
        tail = max(numeric) + 1 if numeric else len(tokens)
        if tail < len(tokens) and tail > 1:
            found.append(((tokens[tail][0], tokens[-1][1]), "description"))
    out = []
    for (a, b), what in found:
        out.append(dict(id="%s.%s@%d" % (kind, what, number), script=line[tokens[0][0]:tokens[0][1]].decode(),
                        kind="%s %s" % (kind, what), speaker="", spans=[(start + a, start + b)],
                        texts=[line[a:b].decode("cp932", "replace")], table=True))
    return out


def parse_scene(plain, name):
    """Cutscene captions in a 3ddata\\*.itm scene script: '##frame,ZIM,text,seconds,-1,x,y' (\\n = next line)."""
    entries, offset = [], 0
    stem = os.path.splitext(name)[0]
    for number, line in enumerate(plain.split(b"\n"), 1):
        start = offset
        offset += len(line) + 1
        body = line.rstrip(b"\r")
        fields = body.split(b",")
        if len(fields) < 7 or fields[1] != b"ZIM" or not fields[0].startswith(b"##"):
            continue
        a = len(fields[0]) + len(fields[1]) + 2
        b = len(body) - sum(len(f) + 1 for f in fields[-4:])
        texts = [t.decode("cp932", "replace") for t in body[a:b].split(LINE_BREAK)]
        speaker = re.match(r"(.+?)「", texts[0])
        entries.append(dict(id="ZIM@%d" % number, script="%s %s" % (stem, fields[0][2:].decode("ascii", "replace")),
                            kind="Subtitle", speaker=speaker.group(1) if speaker else "",
                            spans=[(start + a, start + b)], texts=texts, joined=True, max_lines=3))
    return entries


def is_scene(plain):
    return plain.startswith(b"##") and SCENE_TEXT in plain


def parse_file(plain, name):
    """Entries of map.itm or of a cutscene script, each tagged with its file name."""
    entries = parse(plain) if name.lower() == "map.itm" else parse_scene(plain, name)
    for entry in entries:
        entry["file"] = name
    return entries


def text_files(game_dir):
    """[(game name, path relative to the game folder)]: bin\\map.itm and the cutscene scripts with captions."""
    files = [("map.itm", os.path.join("bin", "map.itm"))]
    scene_dir = os.path.join(game_dir, "3ddata")
    for name in sorted(os.listdir(scene_dir)) if os.path.isdir(scene_dir) else []:
        if name.lower().endswith(".itm"):
            path = os.path.join(scene_dir, name)
            data = open(path, "rb").read()
            if itm_tool.decode(data[:2] + data[-12:], name)[0][:2] == b"##" and is_scene(load_plain(path)):
                files.append((name, os.path.join("3ddata", name)))
    return files


def game_text(text, table=False):
    """Text as the game wants it: half-width letters and signs become full-width (control codes stay)."""
    out, i = [], 0
    while i < len(text):
        control = CONTROL.match(text, i) if not table else None
        if control:
            out.append(control.group(0))
            i = control.end()
            continue
        c = text[i]
        if c in HALF_TO_FONT:
            c = HALF_TO_FONT[c]
        elif c == " ":
            c = "　" if table else DIALOGUE_SPACE
        elif "!" <= c <= "~" and (table or c not in "0123456789-._"):
            c = chr(ord(c) + 0xFEE0)
        out.append(c)
        i += 1
    return "".join(out)


def encode_text(text, table=False):
    """Bytes for the script; raises ValueError naming the characters Shift-JIS (and the font) does not have."""
    text = game_text(text, table)
    if "\n" in text or "\r" in text or (table and ("\t" in text)):
        raise ValueError("line breaks are not allowed inside one line")
    bad = sorted({c for c in text if not can_encode(c)})
    if bad:
        raise ValueError("not in the game's character set: " + " ".join(bad))
    return text.encode("cp932")


def can_encode(c):
    try:
        c.encode("cp932")
        return True
    except UnicodeEncodeError:
        return False


def apply(plain, changes):
    """changes: [(entry, [new texts])] -> new plain bytes. Checks the original text is still there."""
    edits = []
    for entry, texts in changes:
        if entry.get("joined"):
            (a, b), = entry["spans"]
            if plain[a:b] != LINE_BREAK.join(t.encode("cp932", "replace") for t in entry["texts"]):
                raise ValueError("%s: the script changed since it was read" % entry["id"])
            lines = list(texts)
            while len(lines) > 1 and not lines[-1]:
                lines.pop()
            if lines != entry["texts"]:
                edits.append((a, b, LINE_BREAK.join(encode_text(t) for t in lines)))
            continue
        for (a, b), old, new in zip(entry["spans"], entry["texts"], texts):
            if plain[a:b].decode("cp932", "replace") != old:
                raise ValueError("%s: the script changed since it was read" % entry["id"])
            if new != old:
                edits.append((a, b, encode_text(new, entry.get("table", False))))
    out, last = [], 0
    for a, b, data in sorted(edits):
        out += [plain[last:a], data]
        last = b
    out.append(plain[last:])
    return b"".join(out)


# ---- font and layout ---------------------------------------------------------------------------------------

class Font:
    """ddfont8.dat: 144-byte records per Shift-JIS code, 17 rows x 4 words of five 3-bit pixels; width in word 3."""

    def __init__(self, path):
        self.data = open(path, "rb").read()

    def record(self, code):
        index = (code - 0x8141 if code < 0xE040 else code - 0x4043) * 144
        if index < 0 or index + 144 > len(self.data):
            index = 0x1C0 * 2
        return struct.unpack_from("<72H", self.data, index)

    def width(self, code):
        return (self.record(code)[3] >> 9) & 0x1F

    def has(self, code):
        index = (code - 0x8141 if code < 0xE040 else code - 0x4043) * 144
        return 0 <= index and index + 144 <= len(self.data) and any(self.data[index:index + 144])

    def pixels(self, code):
        """17 rows of coverage values 0..7."""
        words, w = self.record(code), self.width(code)
        return [[(words[row * 4 + x // 5] >> ((x % 5) * 3)) & 7 for x in range(min(w, 18))] for row in range(17)]


def layout(text, font):
    """[(x, code)] glyph positions and the line width in pixels, following the game's formatter."""
    data = game_text(text).encode("cp932", "replace")
    glyphs, x, i = [], 0, 0
    while i < len(data):
        c = chr(data[i]) if data[i] < 0x80 else ""
        if c in FORMATTER_DIGITS:
            code, step = FORMATTER_DIGITS[c], 1
        elif c == "_":
            x, i = x + 1, i + 1
            continue
        elif c == "L" and data[i + 1:i + 3].isdigit():
            i += 3
            continue
        elif c == "s" and data[i + 1:i + 4].isdigit():
            x = max(x, int(math.ceil(int(data[i + 1:i + 4]) * 1.13636)))
            i += 4
            continue
        else:
            code, step = (data[i] << 8) | (data[i + 1] if i + 1 < len(data) else 0), 2
        glyphs.append((x, code))
        x += font.width(code) + 1
        i += step
    return glyphs, x


def missing_glyphs(text, font):
    return sorted({bytes([code >> 8, code & 0xFF]).decode("cp932", "replace")
                   for _x, code in layout(text, font)[0] if not font.has(code)})


def longest_line(entries, font, kind="Message"):
    return max((layout(t, font)[1] for e in entries if e["kind"] == kind for t in e["texts"]), default=0)


# ---- sources and TSV ---------------------------------------------------------------------------------------

HEADER = ["file", "id", "script", "kind", "speaker", "speaker_name", "text1", "text2", "text3", "text4"]


def load_sources(paths):
    """[(game name, path relative to a game folder, plain)] from a game folder or from single files."""
    if len(paths) == 1 and os.path.isdir(paths[0]):
        return [(name, rel, load_plain(os.path.join(paths[0], rel))) for name, rel in text_files(paths[0])]
    out = []
    for path in paths:
        name = game_name(path)
        out.append((name, os.path.join("bin" if name.lower() == "map.itm" else "3ddata", name), load_plain(path)))
    return out


def tsv_cell(text):
    return text.replace("\\", "\\\\").replace("\t", "\\t")


def tsv_value(cell):
    return re.sub(r"\\(.)", lambda m: "\t" if m.group(1) == "t" else m.group(1), cell)


def speaker_label(entry, names):
    speaker = entry["speaker"]
    return names.get(int(speaker), "") if re.fullmatch(r"-?\d+", speaker) else ""


def export_tsv(sources, path):
    names = {}
    for name, _rel, plain in sources:
        if name.lower() == "map.itm":
            names = speaker_names(plain)
    count = 0
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(HEADER) + "\n")
        for name, _rel, plain in sources:
            for e in parse_file(plain, name):
                texts = (e["texts"] + [""] * 4)[:4]
                f.write("\t".join([name, e["id"], e["script"], e["kind"], e["speaker"], speaker_label(e, names)]
                                  + [tsv_cell(t) for t in texts]) + "\n")
                count += 1
    return count


def import_tsv(sources, path):
    """{game name: (new plain, changed entries)} for the files whose text changed."""
    entries = {(name, e["id"]): e for name, _rel, plain in sources for e in parse_file(plain, name)}
    changes = {}
    with open(path, encoding="utf-8-sig") as f:
        header = f.readline().rstrip("\n").split("\t")
        for line in f:
            row = dict(zip(header, line.rstrip("\n").split("\t")))
            entry = entries.get((row.get("file", "map.itm"), row.get("id", "")))
            if not entry:
                continue
            count = max(len(entry["texts"]), entry.get("max_lines", 0))
            texts = [tsv_value(row.get("text%d" % (i + 1), "")) for i in range(count)]
            if entry.get("joined"):
                while len(texts) > 1 and not texts[-1]:
                    texts.pop()
            if texts != entry["texts"]:
                changes.setdefault(entry["file"], []).append((entry, texts))
    plains = {name: plain for name, _rel, plain in sources}
    return {name: (apply(plains[name], items), len(items)) for name, items in changes.items()}


def main():
    args = sys.argv[1:]
    if len(args) >= 3 and args[0] == "export":
        print("%d entries -> %s" % (export_tsv(load_sources(args[1:-1]), args[-1]), args[-1]))
    elif len(args) >= 4 and args[0] == "import":
        sources = load_sources(args[1:-2])
        rels = {name: rel for name, rel, _plain in sources}
        for name, (plain, count) in sorted(import_tsv(sources, args[-2]).items()):
            out = os.path.join(args[-1], rels[name])
            os.makedirs(os.path.dirname(out), exist_ok=True)
            save_plain(plain, out, name)
            print("%5d changed -> %s" % (count, out))
    elif len(args) >= 2 and args[0] == "stats":
        kinds = {}
        for name, _rel, plain in load_sources(args[1:]):
            for e in parse_file(plain, name):
                kinds[e["kind"]] = kinds.get(e["kind"], 0) + 1
        for kind, n in sorted(kinds.items()):
            print("%6d  %s" % (n, kind))
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
