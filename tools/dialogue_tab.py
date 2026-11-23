"""Dialogue tab of the modding GUI: browse, search and edit the game's text (dialogue_tool.py).

Covers bin\\map.itm (dialogue boxes, messages, menu choices, names) and the cutscene subtitles in 3ddata\\*.itm.
Edits go to the mod's editable copies (edit\\...\\<file>.itm.txt, the files the Mod tab exports), so the Mod tab's
status and repackaging work as for any other file. The preview draws each line with the game's own font
(ddfont8.dat) and the game's text rules, at 2x.
"""
import os
import re
import time
import tkinter as tk
import unicodedata
from tkinter import filedialog, messagebox, ttk

import dialogue_tool

BOX_BG = (28, 40, 64)
TEXT_RGB = (255, 255, 255)
LIMIT_RGB = (220, 60, 60)
PAD = 6
LIMIT_KINDS = ("Message", "Subtitle")


def fold(text):
    """Search key: full-width and half-width forms alike (Ａ = A), no case."""
    return unicodedata.normalize("NFKC", text).lower()


class DialogueTab:
    def __init__(self, app, kind):
        self.app = app
        self.kind = kind            # the GUI's .itm file kind (export/repack rules, edit file naming)
        self.sources = {}           # game name -> dict(rel, game_path, edit_file, plain)
        self.entries = []
        self.by_key = {}
        self.original = {}          # key -> texts in the game's own files
        self.pending = {}           # key -> texts changed here, not saved yet
        self.names = {}
        self.font = None
        self.limits = {}
        self.current = None
        self._build()

    # ---- layout ------------------------------------------------------------------------------------------

    def _build(self):
        app = self.app
        tab = app.new_tab("Dialogue", (
            "The game's text: dialogue boxes (4 lines and a speaker), short messages, menu choices, stage, item and "
            "character names (bin\\map.itm) and cutscene subtitles (3ddata\\*.itm). Pick a line, change it, press "
            "Apply, then Save (and repackage). The preview uses the game's own font. Half-width letters become "
            "full-width ones because the game cannot draw them; digits, '-' and '.' may stay half-width; a space "
            "becomes a 5-pixel gap. Codes such as s080 (move the text right) are kept. Translators: Export/Import TSV."))
        top = ttk.Frame(tab)
        top.pack(fill="x", padx=8)
        ttk.Button(top, text="Load", command=self.load).pack(side="left", padx=3)
        ttk.Button(top, text="Save", command=lambda: self.save(False)).pack(side="left", padx=3)
        ttk.Button(top, text="Save and repackage", command=lambda: self.save(True)).pack(side="left", padx=3)
        ttk.Button(top, text="Export TSV...", command=self.export_tsv).pack(side="left", padx=3)
        ttk.Button(top, text="Import TSV...", command=self.import_tsv).pack(side="left", padx=3)
        self.status = ttk.Label(top, text="Press Load (needs the game folder and a mod folder).")
        self.status.pack(side="left", padx=8)

        filters = ttk.Frame(tab)
        filters.pack(fill="x", padx=8, pady=4)
        ttk.Label(filters, text="Search").pack(side="left")
        self.search = tk.StringVar()
        self.search.trace_add("write", lambda *_: self.fill())
        ttk.Entry(filters, textvariable=self.search, width=26).pack(side="left", padx=4)
        ttk.Label(filters, text="Kind").pack(side="left")
        self.kind_var = tk.StringVar(value="All")
        self.kind_box = ttk.Combobox(filters, textvariable=self.kind_var, state="readonly", width=20)
        self.kind_box.pack(side="left", padx=4)
        self.kind_box.bind("<<ComboboxSelected>>", lambda _e: self.fill())
        ttk.Label(filters, text="Speaker").pack(side="left")
        self.speaker_var = tk.StringVar(value="All")
        self.speaker_box = ttk.Combobox(filters, textvariable=self.speaker_var, state="readonly", width=28)
        self.speaker_box.pack(side="left", padx=4)
        self.speaker_box.bind("<<ComboboxSelected>>", lambda _e: self.fill())
        self.changed_only = tk.BooleanVar(value=False)
        ttk.Checkbutton(filters, text="Changed only", variable=self.changed_only,
                        command=self.fill).pack(side="left", padx=6)

        body = ttk.PanedWindow(tab, orient="horizontal")
        body.pack(side="top", fill="both", expand=True, padx=8, pady=4)
        left = ttk.Frame(body)
        body.add(left, weight=1)
        self.tree = ttk.Treeview(left, columns=("script", "kind", "speaker", "text", "state"), show="headings",
                                 selectmode="browse", height=12)
        for col, title, width in (("script", "Script / scene", 80), ("kind", "Kind", 70),
                                  ("speaker", "Speaker", 120), ("text", "Text", 260), ("state", "", 64)):
            self.tree.heading(col, text=title)
            self.tree.column(col, width=width, stretch=(col == "text"))
        scroll = ttk.Scrollbar(left, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=scroll.set)
        self.tree.pack(side="left", fill="both", expand=True)
        scroll.pack(side="left", fill="y")
        self.tree.bind("<<TreeviewSelect>>", lambda _e: self.select())

        right = ttk.Frame(body)
        body.add(right, weight=1)
        self.title = ttk.Label(right, text="", font=("TkDefaultFont", 9, "bold"))
        self.title.pack(anchor="w")
        self.lines = []
        for _i in range(4):
            var = tk.StringVar()
            var.trace_add("write", lambda *_: self.preview())
            entry = ttk.Entry(right, textvariable=var, width=48)
            entry.pack(fill="x", pady=1)
            self.lines.append((var, entry))
        bar = ttk.Frame(right)
        bar.pack(fill="x", pady=3)
        ttk.Button(bar, text="Apply", command=self.apply_current).pack(side="left", padx=2)
        ttk.Button(bar, text="Undo changes", command=self.revert_current).pack(side="left", padx=2)
        ttk.Button(bar, text="Back to the game's text", command=self.original_current).pack(side="left", padx=2)
        self.info = ttk.Label(right, text="", wraplength=420, justify="left")
        self.info.pack(anchor="w", pady=4)
        preview = ttk.LabelFrame(tab, text="Preview (the game's font)")
        preview.pack(side="bottom", fill="x", padx=8, pady=(0, 4), before=body)
        self.canvas = ttk.Label(preview)
        self.canvas.pack(anchor="w", padx=4, pady=4)

    # ---- loading and saving ------------------------------------------------------------------------------

    def load(self):
        app = self.app
        if not app._mod_ready():
            return
        game = app.game_dir()
        if not os.path.isfile(os.path.join(game, "bin", "map.itm")):
            messagebox.showwarning("Dialogue", "bin\\map.itm not found in the game folder.")
            return
        self.sources, self.original, self.pending = {}, {}, {}
        originals = []
        for name, rel in dialogue_tool.text_files(game):
            game_path = os.path.join(game, rel)
            edit_file = app.edit_path_for(game_path, self.kind)
            original = dialogue_tool.load_plain(game_path)
            plain = open(edit_file, "rb").read() if os.path.isfile(edit_file) else original
            self.sources[name] = dict(rel=rel, game_path=game_path, edit_file=edit_file, plain=plain)
            for e in dialogue_tool.parse_file(original, name):
                self.original[self.key(e)] = e["texts"]
                originals.append(e)
            if name.lower() == "map.itm":
                self.names = dialogue_tool.speaker_names(original)
        self.font = self._load_font()
        self.limits = {k: dialogue_tool.longest_line(originals, self.font, k) for k in LIMIT_KINDS} if self.font else {}
        self._parse_all()
        self.kind_box["values"] = ["All"] + sorted({e["kind"] for e in self.entries})
        speakers = sorted({self.speaker_label(e) for e in self.entries if e["speaker"]},
                          key=lambda s: (not s[:1].isdigit(), len(s.split()[0]) if s[:1].isdigit() else 0, s))
        self.speaker_box["values"] = ["All"] + speakers
        self.fill()

    def _load_font(self):
        for path in (os.path.join(self.app.mod_dir(), "out", "3ddata", "ddfont8.dat"),
                     os.path.join(self.app.game_dir(), "3ddata", "ddfont8.dat")):
            if os.path.isfile(path):
                return dialogue_tool.Font(path)
        return None

    @staticmethod
    def key(entry):
        return "%s:%s" % (entry["file"], entry["id"])

    def _parse_all(self):
        self.entries = []
        for name, source in self.sources.items():
            self.entries += dialogue_tool.parse_file(source["plain"], name)
        self.by_key = {self.key(e): e for e in self.entries}
        changed = sum(1 for e in self.entries if self.original.get(self.key(e)) != e["texts"])
        self.status.config(text="%d entries in %d files, %d changed from the game." % (
            len(self.entries), len(self.sources), changed))

    def _write_edit_copy(self, name, plain):
        """Write a file's edited text into the mod's edit folder (exporting the original first if needed)."""
        source = self.sources[name]
        edit_file = source["edit_file"]
        if not os.path.isfile(edit_file):
            os.makedirs(os.path.dirname(edit_file), exist_ok=True)
            with open(edit_file, "wb") as f:
                f.write(dialogue_tool.load_plain(source["game_path"]))
            past = time.time() - 10
            os.utime(edit_file, (past, past))
            self.app._store_export_time(edit_file)
        with open(edit_file, "wb") as f:
            f.write(plain)
        source["plain"] = plain

    def save(self, repackage):
        if not self.sources:
            return
        self.apply_current(quiet=True)
        by_file = {}
        for key, texts in self.pending.items():
            entry = self.by_key[key]
            by_file.setdefault(entry["file"], []).append((entry, texts))
        try:
            new = {name: dialogue_tool.apply(self.sources[name]["plain"], items) for name, items in by_file.items()}
        except ValueError as exc:
            messagebox.showwarning("Dialogue", str(exc))
            return
        for name, plain in new.items():
            self._write_edit_copy(name, plain)
            self.app.log("saved %d change(s) to %s\n" % (len(by_file[name]), self.sources[name]["edit_file"]))
        self.pending = {}
        self._parse_all()
        self.fill()
        if repackage:
            for name, source in self.sources.items():
                if os.path.isfile(source["edit_file"]):
                    out = self.app.out_path_for(source["edit_file"], self.kind)
                    os.makedirs(os.path.dirname(out), exist_ok=True)
                    dialogue_tool.save_plain(source["plain"], out, name)
                    self.app.log("repackaged %s -> %s\n" % (source["edit_file"], out))
        self.app.refresh_mod_list()

    def _source_list(self):
        return [(name, s["rel"], s["plain"]) for name, s in self.sources.items()]

    def export_tsv(self):
        if not self.sources:
            return
        path = filedialog.asksaveasfilename(defaultextension=".tsv", initialfile="gurumin_text.tsv",
                                            filetypes=[("Tab-separated text", "*.tsv")])
        if path:
            count = dialogue_tool.export_tsv(self._source_list(), path)
            self.app.log("%d entries -> %s\n" % (count, path))

    def import_tsv(self):
        if not self.sources:
            return
        path = filedialog.askopenfilename(filetypes=[("Tab-separated text", "*.tsv")])
        if not path:
            return
        try:
            changed = dialogue_tool.import_tsv(self._source_list(), path)
        except ValueError as exc:
            messagebox.showwarning("Dialogue", str(exc))
            return
        for name, (plain, count) in changed.items():
            self._write_edit_copy(name, plain)
            self.app.log("imported %d changed entries into %s\n" % (count, self.sources[name]["edit_file"]))
        self.pending = {}
        self._parse_all()
        self.fill()

    # ---- list --------------------------------------------------------------------------------------------

    def speaker_label(self, entry):
        speaker = entry["speaker"]
        if re.fullmatch(r"-?\d+", speaker):
            name = self.names.get(int(speaker))
            return "%s %s" % (speaker, name) if name else "%s (unnamed)" % speaker
        return speaker

    def texts_of(self, entry):
        return self.pending.get(self.key(entry), entry["texts"])

    def state_of(self, entry):
        if self.key(entry) in self.pending:
            return "not saved"
        return "changed" if self.original.get(self.key(entry)) != entry["texts"] else ""

    def fill(self):
        self.tree.delete(*self.tree.get_children())
        needle = fold(self.search.get().strip())
        kind, speaker = self.kind_var.get(), self.speaker_var.get()
        shown = 0
        for entry in self.entries:
            if kind != "All" and entry["kind"] != kind:
                continue
            label = self.speaker_label(entry) if entry["speaker"] else ""
            if speaker != "All" and label != speaker:
                continue
            state = self.state_of(entry)
            if self.changed_only.get() and not state:
                continue
            text = " / ".join(t for t in self.texts_of(entry) if t)
            if needle and needle not in fold(text) and needle not in fold(label) and needle not in fold(entry["script"]):
                continue
            self.tree.insert("", "end", iid=self.key(entry),
                             values=(entry["script"], entry["kind"], label, text, state))
            shown += 1
            if shown >= 6000:
                break

    def select(self):
        selection = self.tree.selection()
        if not selection:
            return
        self.apply_current(quiet=True)
        self.current = self.by_key[selection[0]]
        self.set_lines(self.texts_of(self.current))

    def line_count(self, entry):
        return max(len(entry["texts"]), entry.get("max_lines", 0))

    def set_lines(self, texts):
        entry = self.current
        speaker = (" - " + self.speaker_label(entry)) if entry["speaker"] else ""
        self.title.config(text="%s, %s, %s%s" % (entry["kind"], entry["file"], entry["script"], speaker))
        count = self.line_count(entry)
        for i, (var, widget) in enumerate(self.lines):
            var.set(texts[i] if i < len(texts) else "")
            widget.configure(state="normal" if i < count else "disabled")
        self.preview()

    def edited_texts(self):
        texts = [self.lines[i][0].get() for i in range(self.line_count(self.current))]
        if self.current.get("joined"):
            while len(texts) > 1 and not texts[-1]:
                texts.pop()
        return texts

    def apply_current(self, quiet=False):
        entry = self.current
        if not entry:
            return
        texts = self.edited_texts()
        if texts == self.texts_of(entry):
            return
        try:
            for text in texts:
                dialogue_tool.encode_text(text, entry.get("table", False))
        except ValueError as exc:
            if not quiet:
                messagebox.showwarning("Dialogue", str(exc))
            return
        if texts == entry["texts"]:
            self.pending.pop(self.key(entry), None)
        else:
            self.pending[self.key(entry)] = texts
        self._update_row(entry)

    def _update_row(self, entry):
        key = self.key(entry)
        if self.tree.exists(key):
            self.tree.set(key, "text", " / ".join(t for t in self.texts_of(entry) if t))
            self.tree.set(key, "state", self.state_of(entry))

    def revert_current(self):
        if self.current:
            self.pending.pop(self.key(self.current), None)
            self.set_lines(self.current["texts"])
            self._update_row(self.current)

    def original_current(self):
        if self.current and self.key(self.current) in self.original:
            self.set_lines(self.original[self.key(self.current)])
            self.apply_current()

    # ---- preview -----------------------------------------------------------------------------------------

    def preview(self):
        entry = self.current
        if not entry or not self.font:
            return
        table = entry.get("table", False)
        texts = self.edited_texts()
        layouts = [dialogue_tool.layout(t if not table else dialogue_tool.game_text(t, True).replace("_", "＿"),
                                        self.font) for t in texts]
        limit = self.limits.get(entry["kind"], 0)
        widest = max([w for _g, w in layouts] + [limit, 40])
        width, height = widest + 2 * PAD, len(texts) * 20 - 3 + 2 * PAD
        rows = [[BOX_BG] * width for _ in range(height)]
        for n, (glyphs, _w) in enumerate(layouts):
            top = PAD + n * 20
            for x0, code in glyphs:
                for y, row in enumerate(self.font.pixels(code)):
                    for x, v in enumerate(row):
                        if v and 0 <= PAD + x0 + x < width:
                            a = v / 7.0
                            rows[top + y][PAD + x0 + x] = tuple(int(b + (t - b) * a) for b, t in zip(BOX_BG, TEXT_RGB))
        if limit:
            for y in range(height):
                rows[y][min(width - 1, PAD + limit)] = LIMIT_RGB
        image = tk.PhotoImage(width=width, height=height)
        image.put(" ".join("{" + " ".join("#%02x%02x%02x" % c for c in row) + "}" for row in rows))
        if width * 2 <= self.canvas.master.winfo_width() - 8:
            image = image.zoom(2)
        self.canvas.configure(image=image)
        self.canvas.image = image
        notes = []
        for n, (text, (_g, w)) in enumerate(zip(texts, layouts), 1):
            if limit and w > limit:
                notes.append("line %d is %d px wide; the longest %s line in the game is %d px"
                             % (n, w, entry["kind"].lower(), limit))
            try:
                dialogue_tool.encode_text(text, table)
            except ValueError as exc:
                notes.append("line %d: %s" % (n, exc))
                continue
            missing = dialogue_tool.missing_glyphs(text, self.font)
            if missing:
                notes.append("line %d: the font has no glyph for %s" % (n, " ".join(missing)))
        if limit:
            notes.append("Red line: the longest %s line in the game (%d px)." % (entry["kind"].lower(), limit))
        self.info.config(text="\n".join(notes))


def build(app, kind):
    return DialogueTab(app, kind)
