r"""asset_groups.py - sort Gurumin game files into folders by character or type (used by the GUI's export).

    python tools/asset_groups.py <game folder> [--list]     prints "group <TAB> file" for every game file

Characters, enemies and bosses come from the game's own character table (`#CharTable` in bin\map.itm: name, model
base name, behaviour, type), read from the user's game files. English names are the ones of the English release's
table (CHARACTER_NAMES below; names only, no game data). A file belongs to a character when its name starts with
the character's model base name (Parin = char02b...), when it is a face texture of that character (cf02_* = Parin)
or when one of the character's models uses it as a texture (the .it3 MAT4 material names). Everything else is
grouped by type from its name prefix (mp_ = maps, O_ = objects, it_ = items, ...).

No two files in the game share a name, so a sorted export can always be put back by name.
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Model base name (lower case) -> English name, from the English release's #CharTable.
CHARACTER_NAMES = {
    "char02b": "Parin", "g_010": "Digby", "g_006": "Doug", "char06": "Fan", "g_011": "Pecky", "char07": "Disk",
    "g_008": "Pierre", "g_009": "Rocko", "char12": "Cream", "char02c": "Fake Parin", "g_013": "Chucky",
    "char11": "Prince", "char08": "Pamela", "g_012": "Doc", "char09": "Cylinder", "g_003": "Poco", "g_005": "Puku",
    "g_004": "Pino", "g_007": "Motoro", "char10": "Hyperbolic", "en_015": "Tree", "o_056": "Spikey Shield",
    "en_013": "King Fish", "en_012": "Wizard", "en_011": "Blackbean", "en_001": "Slime", "bs_001": "Boss Sap",
    "bs_001b": "Armored Boss Sap", "bs_002": "Roger", "bs_005": "Giga", "en_010": "Spinyspine",
    "en_016": "Armor Slime", "bs_006": "Tokaron", "en_017": "Jarhead", "bs_003": "Tokaron", "bs_004": "Mosby",
    "en_018": "Merman", "e_010c": "Rockrock", "o_057": "Spiny Ball", "en_014": "Shield", "en_008": "Winged",
    "en_023": "Bat", "en_009": "Horned", "en_007": "Turtle", "en_006b": "Soccer Ball", "char14": "Popon",
    "en_005": "Fish", "en_019": "Box", "en_020": "Blacky", "en_021": "Bug", "en_030": "Naycorn",
    "en_025": "Pumpkin", "en_024": "Try Flap", "en_029": "I.C.", "en_028": "Gap Lurker", "en_027": "Salaman",
    "en_026": "Bee", "en_022": "Gashroom",
}

# Extra base names for characters that the table lists under another model (lower case).
CHARACTER_ALIASES = {"char02": "char02b", "charpj": "char02b"}

# Name prefix (lower case, longest first wins) -> folder, for files that belong to no character.
TYPE_PREFIXES = [
    ("mp", "Maps"), ("o_", "Objects"), ("door", "Objects"), ("e_", "Objects"), ("tekiata", "Objects"),
    ("ksk", "Furniture"), ("h_", "Objects"),
    ("it_", "Items"), ("eq_", "Items"), ("eeq", "Items"), ("eqq", "Items"), ("kane", "Items"), ("okane", "Items"),
    ("drill", "Items"), ("betyo", "Items"),
    ("ef", "Effects"), ("celef", "Effects"), ("zanzo", "Effects"), ("kemu", "Effects"), ("zanri", "Effects"),
    ("ripple", "Effects"), ("awa", "Effects"), ("honoo", "Effects"), ("inazu", "Effects"), ("fire", "Effects"),
    ("lens", "Effects"), ("stars", "Effects"), ("galaxy", "Effects"), ("zapa", "Effects"), ("kumo", "Effects"),
    ("mt_", "Characters/Motions"), ("motion", "Characters/Motions"),
    ("menu", "Interface"), ("waku", "Interface"), ("fukida", "Interface"), ("ico", "Interface"),
    ("tit", "Interface"), ("tute", "Interface"), ("maruf", "Interface"), ("home", "Interface"),
    ("open", "Interface"), ("kb", "Interface"), ("ep_", "Interface"), ("gf", "Interface"), ("dparts", "Interface"),
    ("bdend", "Interface"), ("ms", "Messages"),
    ("bs", "Bosses"), ("en", "Enemies"), ("char", "Characters"), ("g_", "Characters"), ("cf", "Characters"),
]

EXTENSION_FOLDERS = {".mmv": "Movies", ".de2": "Music", ".sav": "Saves", ".efx": "Effects", ".wav": "Sounds",
                     ".avi": "Movies"}


def safe_folder_name(name):
    name = re.sub(r'[<>:"/\\|?*\x00-\x1f]', "", name.replace("　", " ")).strip(" .")
    return name or "unnamed"


def read_char_table(game_dir):
    """[(base, name, type)] from bin\\map.itm's #CharTable (type 2 = NPC, 3 = enemy), [] when unreadable."""
    path = os.path.join(game_dir, "bin", "map.itm")
    if not os.path.isfile(path):
        return []
    try:
        import itm_tool
        plain = itm_tool.decode(open(path, "rb").read(), path)[0]
    except Exception:
        return []
    rows, inside = [], False
    for raw in plain.split(b"\n"):
        line = raw.decode("cp932", "replace").strip()
        if line.startswith("#CharTable"):
            inside = True
        elif line.startswith("#EndCharTable"):
            break
        elif inside and line and not line.startswith("//"):
            parts = line.split()
            # "0000 <name...> <file> <behaviour> <speed> <link> <fc> <type> ..." - the name may contain spaces
            for i in range(2, len(parts) - 5):
                if re.fullmatch(r"-?\d{4}", parts[i + 1]) and re.fullmatch(r"\d{6}", parts[i + 5] if i + 5 < len(parts) else ""):
                    rows.append((parts[i].lower(), " ".join(parts[1:i]), int(parts[i + 5])))
                    break
    return rows


def it3_texture_names(path):
    """Texture names used by an .it3 model's materials (MAT4 chunks), lower case without extension."""
    try:
        data = open(path, "rb").read()
        import gpack
    except Exception:
        return set()
    names, off = set(), 0
    while off + 8 <= len(data):
        tag = data[off:off + 4]
        size, = struct.unpack_from("<I", data, off + 4)
        if off + 8 + size > len(data):
            break
        if tag == b"MAT4":
            count, packed = struct.unpack_from("<II", data, off + 8)
            if count:
                try:
                    raw = gpack.unpack(data[off + 16:off + 16 + packed])
                except Exception:
                    raw = b""
                for i in range(count):
                    name = raw[i * 0x180 + 0x40:i * 0x180 + 0x80].split(b"\0")[0].decode("cp932", "replace")
                    if name:
                        names.add(os.path.splitext(os.path.basename(name.replace("\\", "/")))[0].lower())
        off += 8 + size
    return names


class AssetGroups:
    """group_for(file name) -> folder such as "Characters/Parin", "Enemies/Slime", "Maps" or "Other/xyz"."""

    def __init__(self, game_dir, link_textures=True):
        self.game_dir = game_dir
        self.characters = {}     # base -> (folder, name)
        for base, name, kind in read_char_table(game_dir):
            english = CHARACTER_NAMES.get(base)
            top = "Bosses" if base.startswith("bs_") else ("Enemies" if kind == 3 else "Characters")
            self.characters.setdefault(base, "%s/%s" % (top, safe_folder_name(english or name or base)))
        if not self.characters:  # no map.itm: the English names alone
            for base, english in CHARACTER_NAMES.items():
                top = "Bosses" if base.startswith("bs_") else (
                    "Characters" if base.startswith(("char", "g_")) else "Enemies")
                self.characters[base] = "%s/%s" % (top, safe_folder_name(english))
        for alias, base in CHARACTER_ALIASES.items():
            if base in self.characters:
                self.characters.setdefault(alias, self.characters[base])
        self.bases = sorted(self.characters, key=len, reverse=True)
        self.texture_owner = self._link_textures() if link_textures else {}
        # wav\*.itm are scrambled WAV sounds (voice lines and sound effects), not scripts
        wav_dir = os.path.join(game_dir, "wav")
        self.sound_names = {n.lower() for n in os.listdir(wav_dir)} if os.path.isdir(wav_dir) else set()

    def _character_for(self, stem):
        for base in self.bases:
            if stem.startswith(base):
                rest = stem[len(base):]
                # "en_015c" belongs to en_015, but "en_0151" would be another number; "char02b1" is Parin's
                if not rest or not (base[-1].isdigit() and rest[0].isdigit()):
                    return self.characters[base]
        face = re.match(r"cf(\d\d)", stem)    # face textures: cf02_* = char02 (Parin)
        if face:
            return self._character_for("char" + face.group(1))
        return None

    def _link_textures(self):
        """texture stem -> character folder, for textures that the models of exactly one character use."""
        users = {}
        data_dir = os.path.join(self.game_dir, "3ddata")
        if not os.path.isdir(data_dir):
            return {}
        for name in sorted(os.listdir(data_dir)):
            stem, ext = os.path.splitext(name.lower())
            if ext != ".it3":
                continue
            folder = self._character_for(stem)
            if folder:
                for texture in it3_texture_names(os.path.join(data_dir, name)):
                    users.setdefault(texture, set()).add(folder)
        return {texture: folders.pop() for texture, folders in users.items() if len(folders) == 1}

    def group_for(self, file_name):
        stem, ext = os.path.splitext(os.path.basename(file_name).lower())
        if ext in EXTENSION_FOLDERS:
            return EXTENSION_FOLDERS[ext]
        if os.path.basename(file_name).lower() in self.sound_names:
            return "Sounds/Effects" if stem.startswith(("gse", "vse", "mse")) else "Sounds/Voices"
        if stem.startswith("ddfont"):
            return "Fonts"
        folder = self._character_for(stem)
        if folder:
            return folder
        if ext == ".iti" and stem in self.texture_owner:
            return self.texture_owner[stem]
        for prefix, folder in sorted(TYPE_PREFIXES, key=lambda p: len(p[0]), reverse=True):
            if stem.startswith(prefix):
                return folder
        if ext == ".itm":
            letters = re.match(r"[a-z]+", stem)
            return "Scripts/" + (letters.group(0) if letters else "other")
        if ext == ".iti":
            return "Textures"
        if ext == ".it3":
            return "Models"
        return "Other"


def game_file_index(game_dir):
    """lower-case file name -> path relative to the game folder (every file; names are unique in the game)."""
    index = {}
    for folder, _dirs, files in os.walk(game_dir):
        rel_dir = os.path.relpath(folder, game_dir)
        if rel_dir.split(os.sep)[0].lower() in ("mod", "savedata", "capture"):
            continue
        for name in files:
            index.setdefault(name.lower(), os.path.normpath(os.path.join(rel_dir, name)))
    return index


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    groups = AssetGroups(sys.argv[1])
    counts = {}
    for name, rel in sorted(game_file_index(sys.argv[1]).items()):
        if not name.endswith((".iti", ".it3", ".itm", ".mmv", ".de2", ".dat")):
            continue
        group = groups.group_for(name)
        counts[group] = counts.get(group, 0) + 1
        if "--list" in sys.argv:
            print("%s\t%s" % (group, rel))
    if "--list" not in sys.argv:
        for group, n in sorted(counts.items()):
            print("%5d  %s" % (n, group))


if __name__ == "__main__":
    main()
