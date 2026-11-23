r"""gurumin_tools_gui.py - one window for all of Gurumin's modding tools (Python 3 + Tkinter, standard library only).

    python tools/gurumin_tools_gui.py                 open the window
    python tools/gurumin_tools_gui.py --print-commands  print the command line behind every button (for checking)

The GUI adds no format logic of its own: every button runs one of the command-line tools next to this file
(itm_tool.py, iti_tool.py, it3_tool.py, mmv_tool.py, de2_tool.py, ddfont_tool.py, save_tool.py and the doc
generators) as a separate process and shows what the tool prints in the log pane.

Mod folders use one simple naming rule, so a mod can be reopened at any time without a project file:
    <mod>\edit\<path in the game>.<editable extension>   e.g. edit\3ddata\stage01.itm.txt, edit\3ddata\ddfont8.dat.png
    <mod>\out\<path in the game>                        the repackaged game file,  e.g. out\3ddata\stage01.itm
    <mod>\edit\<path in the game>.frames\              movies: one PNG per frame (frame_0000.png, ...)
    <mod>\reference\<path in the game>.<ext>            export-only views (models as OBJ, save thumbnails)
The game folder is only ever read. Nothing is written inside it; copy <mod>\out over a backup of the game yourself.
See docs/GUI.md for a walkthrough.
"""
import json
import os
import queue
import shutil
import subprocess
import sys
import threading

import asset_groups
import dialogue_tab

FROZEN = getattr(sys, "frozen", False)     # True in the standalone exe (tools/build_exe.py)
TOOLS_DIR = os.path.join(sys._MEIPASS, "tools") if FROZEN else os.path.dirname(os.path.abspath(__file__))
if FROZEN:
    sys.path.insert(0, TOOLS_DIR)
REPO_DIR = os.path.dirname(TOOLS_DIR)
SETTINGS_PATH = os.path.join(os.path.expanduser("~"), ".gurumin_tools_gui.json")
MOD_STATE_NAME = "gurumin_mod.json"      # per-mod file: modification times recorded at export, to spot edited files
DEFAULT_SAVE_KEY = 0x273                  # save_tool.py's own default for --key
MOD_DLL = os.path.join(REPO_DIR, "mod", "build", "d3d9.dll")   # HD movie support (mod/, docs/MOVIES_HD.md)
MOD_INI = os.path.join(REPO_DIR, "mod", "gurumin_mod.ini")


# ----------------------------------------------------------------------------------------------------------------
# Command builders. Each returns the argument list for one tool run; the GUI never builds a command anywhere else,
# so this block is the single place to check that every button calls the right tool with the right arguments.
# ----------------------------------------------------------------------------------------------------------------

def tool_command(script, *args):
    """Run tools/<script> with the current Python interpreter."""
    return [sys.executable, os.path.join(TOOLS_DIR, script)] + [str(a) for a in args]


def cmd_itm_decode(src_itm, dst_txt):
    return tool_command("itm_tool.py", "decode", src_itm, dst_txt)


def cmd_itm_encode(src_txt, dst_itm, game_name=None):
    # The scramble key comes from the file's base name. Without --name the tool would use the .txt name
    # ("stage01.itm.txt" -> "STAGE01.ITM"), so the GUI always names the game file explicitly.
    # Pass dst as an absolute path: itm_tool.py drops any positional argument equal to the --name value.
    game_name = game_name or os.path.basename(dst_itm)
    return tool_command("itm_tool.py", "encode", src_txt, os.path.abspath(dst_itm), "--name", game_name)


def cmd_iti_decode(src_iti, dst_png, packs_dll=None):
    args = ["decode", src_iti, dst_png]
    if packs_dll:
        args += ["--packs", packs_dll]   # only needed for types 666/667 (otherwise looked up in ..\bin\)
    return tool_command("iti_tool.py", *args)


def cmd_iti_encode(src_png, dst_iti):
    return tool_command("iti_tool.py", "encode", src_png, dst_iti)


def cmd_iti_info(folder):
    return tool_command("iti_tool.py", "info", folder)


def cmd_it3_info(src_it3):
    return tool_command("it3_tool.py", "info", src_it3)


def cmd_it3_obj(src_it3, dst_obj, local=False):
    args = ["obj", src_it3, dst_obj]
    if local:
        args.append("--local")
    return tool_command("it3_tool.py", *args)


def cmd_mmv_info(src_mmv):
    return tool_command("mmv_tool.py", "info", src_mmv)


def cmd_mmv_frames(src_mmv, out_dir, every=1):
    args = ["frames", src_mmv, out_dir]
    if int(every) != 1:
        args += ["--every", int(every)]
    return tool_command("mmv_tool.py", *args)


def cmd_mmv_gif(src_mmv, dst_gif, fps=15):
    return tool_command("mmv_tool.py", "gif", src_mmv, dst_gif, "--fps", fps)


def cmd_mmv_encode(src_frames_or_gif, dst_mmv, template_mmv=None, dither=False, original_frame1=False):
    # src: a folder of PNG frames (sorted by name) or an animated GIF.
    args = ["encode", src_frames_or_gif, dst_mmv]
    if template_mmv:
        args += ["--template", template_mmv]   # palette for frames that are not already indexed; size check
    if dither:
        args.append("--dither")
    if original_frame1:
        args.append("--original-frame1")       # byte-identical rebuilds; loop-safe frame 1 otherwise
    return tool_command("mmv_tool.py", *args)


def cmd_de2_info(src_de2):
    return tool_command("de2_tool.py", "info", src_de2)


def cmd_de2_decode(src_de2, dst_wav):
    return tool_command("de2_tool.py", "decode", src_de2, dst_wav)


def cmd_de2_encode(src_wav, dst_de2, template_de2=None):
    # The header is keyed by dst's base name, so dst must carry the game's file name.
    args = ["encode", src_wav, dst_de2]
    if template_de2:
        args += ["--template", template_de2]   # copies the original's sync block
    return tool_command("de2_tool.py", *args)


def cmd_font_export(src_dat, dst_png):
    # Also writes <dst without .png>.tsv, which import needs next to the PNG.
    return tool_command("ddfont_tool.py", "export", src_dat, dst_png)


def cmd_font_import(original_dat, src_png, dst_dat):
    return tool_command("ddfont_tool.py", "import", original_dat, src_png, dst_dat)


def cmd_save_decode(src_sav, dst_bin):
    return tool_command("save_tool.py", "decode", src_sav, dst_bin)


def cmd_save_encode(src_bin, dst_sav, key=DEFAULT_SAVE_KEY):
    # save_tool.py reads --key with int(), so it must be decimal. The scramble is keyed by dst's base name.
    return tool_command("save_tool.py", "encode", src_bin, dst_sav, "--key", int(key))


def cmd_save_info(src_sav):
    return tool_command("save_tool.py", "info", src_sav)


def cmd_save_thumb(src_sav, dst_png):
    return tool_command("save_tool.py", "thumb", src_sav, dst_png)


def cmd_save_set(sav_copy, fields):
    # Writes in place (and a .bak); the GUI only ever runs it on a copy outside the game folder.
    return tool_command("save_tool.py", "set", sav_copy, *["%s=%s" % (k, v) for k, v in fields.items()])


SAVE_FIELDS = ("money", "hp", "hp_max", "costume", "map")   # the names save_tool.py set accepts

DOC_GENERATORS = [
    # (button text, script, what it needs, what it writes)
    ("Script built-ins list", "script_api.py", "re/export/APP.dll/decompiled.c (Ghidra export)", "docs/script_api.tsv"),
    ("Script API page", "gen_script_api_md.py", "docs/script_api.tsv", "docs/SCRIPT_API.md"),
    ("gurumin.def key map", "config_keys.py", "the Ghidra export and decomp/tools", "docs/config_keys.tsv"),
    ("Save body map", "save_layout.py", "the Ghidra export and decomp/tools", "docs/save_layout.tsv"),
]


def cmd_doc_generator(script):
    return tool_command(script)


# ----------------------------------------------------------------------------------------------------------------
# File kinds: which tool turns a game file into an editable file and back. Used by the batch buttons and the mod
# workflow. "edit_ext" is appended to the game file's name ("stage01.itm" -> "stage01.itm.txt").
# ----------------------------------------------------------------------------------------------------------------

class FileKind:
    def __init__(self, key, title, game_ext, edit_ext, export, repack=None, reference=False, extra_edit_files=(),
                 folder=False):
        self.key = key                    # short id
        self.title = title                # shown in the GUI
        self.game_ext = game_ext          # extension of the game file (lower case, with dot)
        self.edit_ext = edit_ext          # extension of the exported file
        self.export = export              # function(app, game_file, edit_file) -> command
        self.repack = repack              # function(app, edit_file, out_file, original_game_file) -> command, or None
        self.reference = reference        # True: export-only (goes to <mod>\reference)
        self.extra_edit_files = extra_edit_files   # sidecar extensions written next to the edit file (font .tsv)
        self.folder = folder              # True: the editable item is a folder (movie frames), not one file

    def edit_outputs(self, edit_path):
        """Job outputs for an export: a trailing separator makes the runner create the folder itself."""
        return [edit_path + os.sep] if self.folder else [edit_path]

    def matches(self, filename):
        name = filename.lower()
        if self.key == "font":
            return name.startswith("ddfont") and name.endswith(".dat")
        return name.endswith(self.game_ext)


def is_hd_movie(path):
    """True for an .mmv whose palette carries the HD marker (see mmv_tool.py)."""
    try:
        with open(path, "rb") as f:
            head = f.read(18)
    except OSError:
        return False
    return len(head) == 18 and head[:2] == b"MV" and head[9:10] == b"H" and head[13:14] == b"D" and head[17] in (2, 4)


def add_hd_movie_support(out_root):
    """Copies the mod DLL (and its settings, if not there yet) to where game.exe goes; returns (log line, tag)."""
    if not os.path.isfile(MOD_DLL):
        return ("HD movie: the mod DLL is not built yet (run mod/build.sh), so %s has no d3d9.dll. Without it the "
                "game draws HD movies 2x or 4x too big.\n" % out_root, "error")
    os.makedirs(out_root, exist_ok=True)
    dll = os.path.join(out_root, "d3d9.dll")
    if not os.path.isfile(dll) or open(dll, "rb").read() != open(MOD_DLL, "rb").read():
        shutil.copyfile(MOD_DLL, dll)
    ini = os.path.join(out_root, "gurumin_mod.ini")
    if not os.path.isfile(ini) and os.path.isfile(MOD_INI):
        shutil.copyfile(MOD_INI, ini)
    return ("HD movie: added d3d9.dll and gurumin_mod.ini to %s (they go next to game.exe)\n" % out_root, "log")


def repack_save(app, edit_bin, out_sav, original_sav):
    return cmd_save_encode(edit_bin, out_sav, read_save_key(original_sav))


def read_save_key(sav_path):
    """The key word stored right after the magic (save_tool.py keeps it when it rewrites a save in place).
    Re-encoding with the original key keeps the repacked save as close to the original as possible."""
    try:
        with open(sav_path, "rb") as f:
            head = f.read(8)
        return int.from_bytes(head[4:8], "little") if len(head) == 8 else DEFAULT_SAVE_KEY
    except OSError:
        return DEFAULT_SAVE_KEY


FILE_KINDS = [
    FileKind("itm", "Scripts and tables (.itm)", ".itm", ".txt",
             lambda app, g, e: cmd_itm_decode(g, e),
             lambda app, e, o, g: cmd_itm_encode(e, o, os.path.basename(o))),
    FileKind("iti", "Textures (.iti)", ".iti", ".png",
             lambda app, g, e: cmd_iti_decode(g, e, app.settings.get("packs_dll") or None),
             lambda app, e, o, g: cmd_iti_encode(e, o)),
    FileKind("de2", "Music (.de2)", ".de2", ".wav",
             lambda app, g, e: cmd_de2_decode(g, e),
             lambda app, e, o, g: cmd_de2_encode(e, o, g if os.path.isfile(g) else None)),
    FileKind("font", "Font (ddfont*.dat)", ".dat", ".png",
             lambda app, g, e: cmd_font_export(g, e),
             lambda app, e, o, g: cmd_font_import(g, e, o), extra_edit_files=(".tsv",)),
    FileKind("sav", "Saves (.sav)", ".sav", ".bin",
             lambda app, g, e: cmd_save_decode(g, e), repack_save),
    FileKind("it3", "Models (.it3)", ".it3", ".obj",
             lambda app, g, e: cmd_it3_obj(g, e), reference=True),
    FileKind("mmv", "Movies (.mmv)", ".mmv", ".frames",
             lambda app, g, e: cmd_mmv_frames(g, e),
             lambda app, e, o, g: cmd_mmv_encode(e, o, g if os.path.isfile(g) else None), folder=True),
]
KIND_BY_KEY = {k.key: k for k in FILE_KINDS}


def kind_for_game_file(path):
    for kind in FILE_KINDS:
        if kind.matches(os.path.basename(path)):
            return kind
    return None


def kind_for_edit_file(path):
    """'stage01.itm.txt' -> (itm kind, 'stage01.itm'); None when the name doesn't follow the mod naming rule."""
    name = os.path.basename(path)
    for kind in FILE_KINDS:
        if kind.reference or not name.lower().endswith(kind.edit_ext):
            continue
        game_name = name[:-len(kind.edit_ext)]
        if kind.matches(game_name):
            return kind, game_name
    return None


def game_name_for_edit_file(edit_name, kind):
    """'stage01.itm.txt' -> 'stage01.itm'; a plain 'stage01.txt' -> 'stage01.itm'; 'intro.mmv.gif' -> 'intro.mmv'."""
    if edit_name.lower().endswith(kind.edit_ext):
        stem = edit_name[:-len(kind.edit_ext)]
    else:
        stem = os.path.splitext(edit_name)[0]
    return stem if kind.matches(stem) else stem + kind.game_ext


def is_inside(path, folder):
    if not path or not folder:
        return False
    try:
        path, folder = os.path.normcase(os.path.abspath(path)), os.path.normcase(os.path.abspath(folder))
        return os.path.commonpath([path, folder]) == folder
    except ValueError:      # different drives
        return False


def find_files(folder, predicate, frame_folders=False):
    """Every file under folder (recursive) whose name passes predicate, sorted. Movie frame folders (*.frames) are
    never searched inside; with frame_folders=True they are returned themselves when their name passes."""
    hits = []
    for root, dirs, files in os.walk(folder):
        for name in [d for d in dirs if d.lower().endswith(".frames")]:
            dirs.remove(name)
            if frame_folders and predicate(name):
                hits.append(os.path.join(root, name))
        for name in files:
            if predicate(name):
                hits.append(os.path.join(root, name))
    return sorted(hits)


def edit_mtime(path):
    """Modification time of an editable item; for a frames folder the newest frame (editing a PNG doesn't touch
    the folder's own time)."""
    if os.path.isdir(path):
        times = [os.path.getmtime(os.path.join(path, n)) for n in os.listdir(path)]
        return max(times + [os.path.getmtime(path)])
    return os.path.getmtime(path)


# ----------------------------------------------------------------------------------------------------------------
# Settings and per-mod state
# ----------------------------------------------------------------------------------------------------------------

class Settings(dict):
    """game_dir, mod_dir, packs_dll - saved as JSON in the user's home folder."""
    def __init__(self):
        super().__init__(game_dir="", mod_dir=os.path.join(os.path.expanduser("~"), "Documents", "Gurumin_mod"),
                         packs_dll="", sort_exports=True)
        try:
            with open(SETTINGS_PATH, encoding="utf-8") as f:
                self.update(json.load(f))
        except (OSError, ValueError):
            pass

    def save(self):
        try:
            with open(SETTINGS_PATH, "w", encoding="utf-8") as f:
                json.dump(dict(self), f, indent=2)
        except OSError:
            pass


def load_mod_state(mod_dir):
    try:
        with open(os.path.join(mod_dir, MOD_STATE_NAME), encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return {"exported": {}}


def save_mod_state(mod_dir, state):
    os.makedirs(mod_dir, exist_ok=True)
    with open(os.path.join(mod_dir, MOD_STATE_NAME), "w", encoding="utf-8") as f:
        json.dump(state, f, indent=2, sort_keys=True)


# ----------------------------------------------------------------------------------------------------------------
# Jobs: one tool run each. A worker thread runs a list of jobs and reports through a queue to the window.
# ----------------------------------------------------------------------------------------------------------------

class Job:
    def __init__(self, label, command, outputs=(), before=None, after=None):
        self.label = label          # one line shown in the log
        self.command = command      # argument list
        self.outputs = list(outputs)  # files/folders the tool writes: parents are created; game folder is refused
        self.before = before        # optional callable run first in the worker (e.g. copying a save)
        self.after = after          # optional callable(ok) run after the tool finished


class JobRunner:
    def __init__(self, post):
        self.post = post            # post(kind, payload): thread-safe message to the window
        self.cancel_requested = False
        self.thread = None

    def busy(self):
        return self.thread is not None and self.thread.is_alive()

    def start(self, jobs, game_dir):
        self.cancel_requested = False
        self.thread = threading.Thread(target=self._run, args=(jobs, game_dir), daemon=True)
        self.thread.start()

    def _run(self, jobs, game_dir):
        failures = 0
        self.post("progress", (0, len(jobs)))
        for n, job in enumerate(jobs):
            if self.cancel_requested:
                self.post("log", "-- cancelled, %d of %d done\n" % (n, len(jobs)))
                break
            failures += 0 if self._run_one(job, game_dir) else 1
            self.post("progress", (n + 1, len(jobs)))
        else:
            self.post("log", "-- finished: %d run(s), %d failed\n" % (len(jobs), failures))
        self.post("done", failures)

    def _run_one(self, job, game_dir):
        self.post("log", "> %s\n" % job.label)
        for out in job.outputs:
            if is_inside(out, game_dir):
                self.post("error", "  refused: %s is inside the game folder (the GUI never writes there)\n" % out)
                return False
        try:
            for out in job.outputs:
                parent = out if out.endswith(os.sep) else os.path.dirname(out)
                if parent:
                    os.makedirs(parent, exist_ok=True)
            if job.before:
                job.before()
            if FROZEN and job.command[0] == sys.executable:
                ok = self._run_in_process(job.command[1:])
                if not ok:
                    self.post("error", "  failed: %s\n" % subprocess.list2cmdline(job.command[1:]))
                if job.after:
                    job.after(ok)
                return ok
            env = dict(os.environ, PYTHONIOENCODING="utf-8")
            flags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
            proc = subprocess.Popen(job.command, cwd=TOOLS_DIR, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    text=True, encoding="utf-8", errors="replace", env=env, creationflags=flags)
            for line in proc.stdout:
                self.post("log", "  " + line)
            ok = proc.wait() == 0
        except Exception as exc:        # missing file, permission error, ... - report it and carry on
            self.post("error", "  error: %s\n" % exc)
            ok = False
        if not ok:
            self.post("error", "  failed: %s\n" % subprocess.list2cmdline(job.command))
        if job.after:
            job.after(ok)
        return ok


    def _run_in_process(self, argv):
        """The exe has no Python to start, so it runs the tool's script in this thread with its output captured."""
        import runpy, traceback

        class LogWriter:
            def __init__(self, post):
                self.post, self.pending = post, ""

            def write(self, text):
                self.pending += text
                *lines, self.pending = self.pending.split("\n")
                for line in lines:
                    self.post("log", "  " + line + "\n")
                return len(text)

            def flush(self):
                pass

        writer = LogWriter(self.post)
        saved = sys.argv, sys.stdout, sys.stderr
        sys.argv, sys.stdout, sys.stderr = list(argv), writer, writer
        try:
            runpy.run_path(argv[0], run_name="__main__")
            ok = True
        except SystemExit as exc:
            ok = exc.code in (None, 0)
            if isinstance(exc.code, str):
                writer.write(exc.code + "\n")
        except Exception:
            traceback.print_exc(file=writer)
            ok = False
        finally:
            if writer.pending:
                writer.write("\n")
            sys.argv, sys.stdout, sys.stderr = saved
        return ok


# ----------------------------------------------------------------------------------------------------------------
# The window
# ----------------------------------------------------------------------------------------------------------------

def build_gui():
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox

    class PathRow:
        """Label + entry + Browse button. mode: 'open', 'save' or 'dir'."""
        def __init__(self, parent, row, label, mode="open", filetypes=None, on_change=None, width=70):
            self.var = tk.StringVar()
            self.mode, self.filetypes = mode, filetypes or [("All files", "*.*")]
            ttk.Label(parent, text=label).grid(row=row, column=0, sticky="w", padx=4, pady=2)
            ttk.Entry(parent, textvariable=self.var, width=width).grid(row=row, column=1, sticky="ew", padx=4, pady=2)
            ttk.Button(parent, text="Browse...", command=self.browse).grid(row=row, column=2, padx=4, pady=2)
            self.on_change = on_change

        def browse(self):
            start = os.path.dirname(self.var.get()) if self.var.get() else app.start_folder()
            if self.mode == "dir":
                path = filedialog.askdirectory(initialdir=self.var.get() or start)
            elif self.mode == "save":
                path = filedialog.asksaveasfilename(initialdir=start, filetypes=self.filetypes,
                                                    initialfile=os.path.basename(self.var.get()))
            else:
                path = filedialog.askopenfilename(initialdir=start, filetypes=self.filetypes)
            if path:
                self.set(os.path.normpath(path))
                if self.on_change:
                    self.on_change(self.get())

        def get(self):
            return self.var.get().strip().strip('"')

        def set(self, value):
            self.var.set(value)

    class App:
        def __init__(self, root):
            self.root = root
            self.settings = Settings()
            self.messages = queue.Queue()
            self.runner = JobRunner(lambda kind, payload: self.messages.put((kind, payload)))
            root.title("Gurumin modding tools")
            root.geometry("1000x760")
            root.minsize(820, 560)

            paned = ttk.PanedWindow(root, orient="vertical")
            paned.pack(fill="both", expand=True)
            self.notebook = ttk.Notebook(paned)
            paned.add(self.notebook, weight=3)
            paned.add(self._build_log(paned), weight=2)

            self._build_setup_tab()
            self._build_mod_tab()
            self._build_itm_tab()
            self.dialogue = dialogue_tab.build(self, KIND_BY_KEY["itm"])
            self._build_iti_tab()
            self._build_it3_tab()
            self._build_mmv_tab()
            self._build_de2_tab()
            self._build_font_tab()
            self._build_save_tab()
            self._build_docs_tab()
            root.after(100, self._poll_messages)
            root.protocol("WM_DELETE_WINDOW", self._on_close)
            if not self.settings["game_dir"]:
                self.log("Start on the Setup tab: choose your Gurumin folder and a mod folder.\n")
            else:
                self.refresh_mod_list()

        # ---- shared helpers ---------------------------------------------------------------------------------

        def game_dir(self):
            return self.settings.get("game_dir", "")

        def mod_dir(self):
            return self.settings.get("mod_dir", "")

        def start_folder(self):
            return self.game_dir() or self.mod_dir() or REPO_DIR

        def file_groups(self):
            """Sorts game files into character/type folders (built once per game folder: reads map.itm and models)."""
            if getattr(self, "_groups_for", None) != self.game_dir():
                self._groups = asset_groups.AssetGroups(self.game_dir())
                self._groups_for = self.game_dir()
            return self._groups

        def game_rel_for_name(self, game_name):
            """Path of a game file inside the game folder, found by its name (names are unique in the game)."""
            if getattr(self, "_index_for", None) != self.game_dir():
                self._index = asset_groups.game_file_index(self.game_dir()) if self.game_dir() else {}
                self._index_for = self.game_dir()
            return self._index.get(game_name.lower(), "")

        def edit_path_for(self, game_file, kind, ext=None, sub=None):
            """Default export target: <mod>/edit (or /reference), sorted into character/type folders or mirroring
            the game's folders; next to the file when no mod folder is set."""
            ext = ext or (kind.edit_ext if kind else ".out")
            if self.mod_dir() and is_inside(game_file, self.game_dir()):
                rel = os.path.relpath(game_file, self.game_dir())
                sub = sub or ("reference" if kind and kind.reference else "edit")
                if self.settings.get("sort_exports", True):
                    group = self.file_groups().group_for(os.path.basename(game_file))
                    rel = os.path.join(*group.split("/"), os.path.basename(game_file))
                return os.path.join(self.mod_dir(), sub, rel + ext)
            if is_inside(game_file, self.game_dir()):
                return ""
            return game_file + ext

        def game_rel_for_edit(self, edit_file, game_name):
            """Game path of an edited file: by name from the game folder (sorted exports), else by its position in
            <mod>/edit (exports that mirror the game's folders)."""
            edit_root = os.path.join(self.mod_dir(), "edit")
            return self.game_rel_for_name(game_name) or os.path.relpath(
                os.path.join(os.path.dirname(edit_file), game_name), edit_root)

        def out_path_for(self, edit_file, kind):
            """Default import target: <mod>/out/<game path> for files in <mod>/edit; next to the file otherwise."""
            game_name = game_name_for_edit_file(os.path.basename(edit_file), kind)
            edit_root = os.path.join(self.mod_dir(), "edit")
            if self.mod_dir() and is_inside(edit_file, edit_root):
                return os.path.join(self.mod_dir(), "out", self.game_rel_for_edit(edit_file, game_name))
            return os.path.join(os.path.dirname(edit_file), game_name)

        def original_for_edit(self, edit_file, kind):
            """The game file an edit file came from (template for music, original font, key for saves)."""
            edit_root = os.path.join(self.mod_dir(), "edit")
            if self.game_dir() and self.mod_dir() and is_inside(edit_file, edit_root):
                game_name = game_name_for_edit_file(os.path.basename(edit_file), kind)
                return os.path.join(self.game_dir(), self.game_rel_for_edit(edit_file, game_name))
            return ""

        def run(self, jobs):
            if not jobs:
                self.log("Nothing to do.\n")
                return
            if self.runner.busy():
                messagebox.showinfo("Busy", "A run is still going. Wait for it or press Cancel.")
                return
            self.progress.configure(maximum=len(jobs), value=0)
            self.cancel_button.state(["!disabled"])
            self.runner.start(jobs, self.game_dir())

        def run_one(self, label, command, outputs=(), before=None, after=None):
            self.run([Job(label, command, outputs, before, after)])

        def require(self, *pairs):
            """require(('input file', value), ...) -> True when every value is filled in."""
            missing = [name for name, value in pairs if not value]
            if missing:
                messagebox.showwarning("Missing", "Please choose: " + ", ".join(missing))
                return False
            return True

        # ---- log pane ---------------------------------------------------------------------------------------

        def _build_log(self, parent):
            frame = ttk.Frame(parent)
            bar = ttk.Frame(frame)
            bar.pack(fill="x")
            ttk.Label(bar, text="Log").pack(side="left", padx=4)
            self.progress = ttk.Progressbar(bar, length=300, mode="determinate")
            self.progress.pack(side="left", padx=8, pady=2)
            self.progress_text = ttk.Label(bar, text="")
            self.progress_text.pack(side="left")
            self.cancel_button = ttk.Button(bar, text="Cancel", command=self._cancel)
            self.cancel_button.pack(side="right", padx=4)
            self.cancel_button.state(["disabled"])
            ttk.Button(bar, text="Clear log", command=lambda: self.log_text.delete("1.0", "end")).pack(side="right")
            self.log_text = tk.Text(frame, height=12, wrap="word", font=("Consolas", 9))
            scroll = ttk.Scrollbar(frame, command=self.log_text.yview)
            self.log_text.configure(yscrollcommand=scroll.set)
            scroll.pack(side="right", fill="y")
            self.log_text.pack(fill="both", expand=True)
            self.log_text.tag_configure("error", foreground="#b00000")
            return frame

        def log(self, text, tag=None):
            self.log_text.insert("end", text, tag)
            self.log_text.see("end")

        def _cancel(self):
            self.runner.cancel_requested = True
            self.log("-- cancelling after the current file...\n")

        def _poll_messages(self):
            try:
                while True:
                    kind, payload = self.messages.get_nowait()
                    if kind == "log":
                        self.log(payload)
                    elif kind == "error":
                        self.log(payload, "error")
                    elif kind == "progress":
                        done, total = payload
                        self.progress.configure(maximum=max(total, 1), value=done)
                        self.progress_text.configure(text="%d / %d" % (done, total))
                    elif kind == "done":
                        self.cancel_button.state(["disabled"])
                        self.refresh_mod_list()
                    elif kind == "call":
                        payload()
            except queue.Empty:
                pass
            self.root.after(100, self._poll_messages)

        def _on_close(self):
            self.settings.save()
            self.root.destroy()

        # ---- helpers to build tabs --------------------------------------------------------------------------

        def new_tab(self, title, intro):
            outer = ttk.Frame(self.notebook)
            self.notebook.add(outer, text=title)
            ttk.Label(outer, text=intro, wraplength=940, justify="left").pack(fill="x", padx=8, pady=6)
            return outer

        def section(self, tab, title):
            box = ttk.LabelFrame(tab, text=title)
            box.pack(fill="x", padx=8, pady=4)
            box.columnconfigure(1, weight=1)
            return box

        def buttons(self, box, row, *specs):
            bar = ttk.Frame(box)
            bar.grid(row=row, column=0, columnspan=3, sticky="w", padx=4, pady=4)
            for text, command in specs:
                ttk.Button(bar, text=text, command=command).pack(side="left", padx=3)
            return bar

        def single_file_rows(self, box, kind, in_label, in_types, out_label, out_types, out_mode="save",
                             default_out=None):
            """Input + output pickers; choosing the input fills in a default output."""
            out_row = PathRow(box, 1, out_label, out_mode, out_types)

            def fill_default(path):
                out_row.set(default_out(path) if default_out else self.edit_path_for(path, kind))
            in_row = PathRow(box, 0, in_label, "open", in_types, on_change=fill_default)
            return in_row, out_row

        def batch_rows(self, box, first_row, src_label, out_label, out_default):
            src = PathRow(box, first_row, src_label, "dir")
            out = PathRow(box, first_row + 1, out_label, "dir")
            src.on_change = lambda path: out.set(out.get() or out_default())
            return src, out

        def batch_export_jobs(self, kind, src_dir, out_dir, export=None):
            """One job per matching file under src_dir, written to out_dir with the same sub-folders."""
            jobs = []
            for game_file in find_files(src_dir, kind.matches):
                edit_file = os.path.join(out_dir, os.path.relpath(game_file, src_dir) + kind.edit_ext)
                command = (export or kind.export)(self, game_file, edit_file)
                jobs.append(Job("%s -> %s" % (game_file, edit_file), command,
                                kind.edit_outputs(edit_file), after=self._record_export(edit_file)))
            return jobs

        def batch_repack_jobs(self, kind, src_dir, out_dir, originals_dir=""):
            """One job per edited file under src_dir (any name ending in the kind's edit extension)."""
            jobs = []
            for edit_file in find_files(src_dir, lambda n: n.lower().endswith(kind.edit_ext), kind.folder):
                named = kind_for_edit_file(edit_file)
                if named and named[0] is not kind:
                    continue    # e.g. ddfont8.dat.png is the font, not a texture
                if kind.key == "font" and not os.path.isfile(os.path.splitext(edit_file)[0] + ".tsv"):
                    continue    # a font atlas always has its .tsv; other PNGs in the folder are not fonts
                rel_dir = os.path.relpath(os.path.dirname(edit_file), src_dir)
                game_name = game_name_for_edit_file(os.path.basename(edit_file), kind)
                out_file = os.path.normpath(os.path.join(out_dir, rel_dir, game_name))
                original = os.path.normpath(os.path.join(originals_dir, rel_dir, game_name)) if originals_dir else ""
                if kind.key == "font" and not os.path.isfile(original):
                    self.log("skipped %s: the original %s is needed to rebuild the font\n" % (edit_file, original),
                             "error")
                    continue
                jobs.append(Job("%s -> %s" % (edit_file, out_file), kind.repack(self, edit_file, out_file, original),
                                [out_file], after=self._hd_movie_check(kind, out_file, out_dir)))
            return jobs

        def _hd_movie_check(self, kind, out_file, out_root):
            """After a movie is built: an HD movie needs the mod DLL, so put it into the output's root folder."""
            if kind.key != "mmv":
                return None

            def after(ok):
                if not ok or not is_hd_movie(out_file):
                    return
                if out_root:
                    text, tag = add_hd_movie_support(out_root)
                else:
                    text, tag = ("HD movie: %s needs the mod DLL next to game.exe (mod\\install.bat, "
                                 "docs/MOVIES_HD.md)\n" % out_file, "log")
                self.messages.put((tag, text))
            return after

        def _record_export(self, edit_file):
            """After an export into the mod's edit folder, remember the file's time so 'changed only' works."""
            def after(ok):
                if ok and self.mod_dir() and is_inside(edit_file, self.mod_dir()):
                    self.messages.put(("call", lambda: self._store_export_time(edit_file)))
            return after

        def _store_export_time(self, edit_file):
            state = load_mod_state(self.mod_dir())
            if os.path.exists(edit_file):
                rel = os.path.relpath(edit_file, self.mod_dir()).replace(os.sep, "/")
                state.setdefault("exported", {})[rel] = edit_mtime(edit_file)
                save_mod_state(self.mod_dir(), state)

        def preview_image(self, path):
            """Show a PNG/GIF in a small window (Tk reads both; GIFs show their first frame)."""
            if not os.path.isfile(path):
                messagebox.showwarning("Preview", "File not found:\n" + path)
                return
            try:
                image = tk.PhotoImage(file=path)
            except tk.TclError as exc:
                messagebox.showwarning("Preview", "Tk cannot show this image: %s" % exc)
                return
            factor = max(1, (max(image.width(), image.height()) + 799) // 800)
            if factor > 1:
                image = image.subsample(factor)
            win = tk.Toplevel(self.root)
            win.title("%s (%dx%d)" % (os.path.basename(path), image.width() * factor, image.height() * factor))
            label = ttk.Label(win, image=image)
            label.image = image
            label.pack()

        def open_folder(self, path):
            if not path:
                return
            os.makedirs(path, exist_ok=True)
            if hasattr(os, "startfile"):
                os.startfile(path)
            else:
                subprocess.Popen(["xdg-open" if sys.platform != "darwin" else "open", path])

        # ---- Setup ------------------------------------------------------------------------------------------

        def _build_setup_tab(self):
            tab = self.new_tab("Setup", "Choose your Gurumin folder once (it is only read, never changed) and a mod "
                               "folder where exports and repackaged files go. Settings are saved in %s." % SETTINGS_PATH)
            box = self.section(tab, "Folders")
            self.game_row = PathRow(box, 0, "Game folder", "dir", on_change=lambda p: self._save_setting("game_dir", p))
            self.mod_row = PathRow(box, 1, "Mod folder", "dir", on_change=lambda p: self._save_setting("mod_dir", p))
            self.packs_row = PathRow(box, 2, "packs.dll (optional)", "open", [("packs.dll", "*.dll")],
                                     on_change=lambda p: self._save_setting("packs_dll", p))
            self.game_row.set(self.game_dir())
            self.mod_row.set(self.mod_dir())
            self.packs_row.set(self.settings.get("packs_dll", ""))
            self.buttons(box, 3, ("Save settings", self._save_all_settings),
                         ("Open game folder", lambda: self.open_folder(self.game_dir())),
                         ("Open mod folder", lambda: self.open_folder(self.mod_dir())))
            ttk.Label(box, wraplength=900, justify="left", text=(
                "packs.dll is only needed for textures of type 666/667; by default iti_tool.py looks for it in the "
                "game's bin folder next to the texture. Edits typed into the boxes are saved with 'Save settings' "
                "or when the window closes.")).grid(row=4, column=0, columnspan=3, sticky="w", padx=4, pady=4)
            check = self.section(tab, "Check")
            self.buttons(check, 0, ("Count texture types in the game folder", lambda: self.game_dir() and self.run_one(
                "texture types in " + self.game_dir(), cmd_iti_info(self.game_dir()))),
                ("List what the game folder holds", self._summarise_game_folder))

        def _save_setting(self, key, value):
            self.settings[key] = value
            self.settings.save()
            if key in ("game_dir", "mod_dir"):
                self.refresh_mod_list()

        def _save_all_settings(self):
            self.settings.update(game_dir=self.game_row.get(), mod_dir=self.mod_row.get(),
                                 packs_dll=self.packs_row.get())
            self.settings.save()
            self.refresh_mod_list()
            self.log("Settings saved to %s\n" % SETTINGS_PATH)

        def _on_close_sync(self):
            self.settings.update(game_dir=self.game_row.get(), mod_dir=self.mod_row.get(),
                                 packs_dll=self.packs_row.get())

        def _summarise_game_folder(self):
            if not self.require(("game folder", self.game_dir())):
                return
            for kind in FILE_KINDS:
                self.log("%-28s %5d file(s)\n" % (kind.title, len(find_files(self.game_dir(), kind.matches))))

        # ---- Mod workflow -----------------------------------------------------------------------------------

        def _build_mod_tab(self):
            tab = self.new_tab("Mod", "1. Export game files into the mod folder (edit\\ for editable files and "
                               "movie frame folders, reference\\ for models), sorted into folders by character "
                               "(Characters\\Parin, Enemies\\Slime, ...) or type (Maps, Items, ...).  2. Edit them with "
                               "any editor; textures may be larger than the originals (up to 2048x2048).  3. Repackage: "
                               "the game files are rebuilt in the mod's out\\ folder with the game's folder layout.  "
                               "4. Install into the game: the mod DLL loads them from the game's mod\\ folder, nothing "
                               "of the game is overwritten. Reopening a mod folder later picks up where you left off.")
            top = self.section(tab, "Export from the game")
            self.mod_kind_vars = {}
            kinds_bar = ttk.Frame(top)
            kinds_bar.grid(row=0, column=0, columnspan=3, sticky="w", padx=4)
            for kind in FILE_KINDS:
                var = tk.BooleanVar(value=not kind.reference)
                self.mod_kind_vars[kind.key] = var
                ttk.Checkbutton(kinds_bar, text=kind.title, variable=var).pack(side="left", padx=3)
            self.buttons(top, 1, ("Export chosen files...", self._mod_export_files),
                         ("Export a game sub-folder (ticked types)...", self._mod_export_folder),
                         ("Export the whole game (ticked types)", lambda: self._mod_export_tree(self.game_dir())))
            self.sort_var = tk.BooleanVar(value=self.settings.get("sort_exports", True))
            ttk.Checkbutton(top, text="Sort exports into folders by character or type (Characters\\Parin, Maps, "
                            "Items, ...); off = the game's own folders", variable=self.sort_var,
                            command=lambda: self.settings.__setitem__("sort_exports", self.sort_var.get())).grid(
                row=2, column=0, columnspan=3, sticky="w", padx=4)

            box = self.section(tab, "Files in the mod")
            columns = ("game", "edit", "status")
            self.mod_tree = ttk.Treeview(box, columns=columns, show="headings", height=10, selectmode="extended")
            for col, text, width in (("game", "Game file", 300), ("edit", "Editable file", 330), ("status", "Status", 200)):
                self.mod_tree.heading(col, text=text)
                self.mod_tree.column(col, width=width, anchor="w")
            scroll = ttk.Scrollbar(box, command=self.mod_tree.yview)
            self.mod_tree.configure(yscrollcommand=scroll.set)
            self.mod_tree.grid(row=0, column=0, columnspan=2, sticky="nsew", padx=4, pady=4)
            scroll.grid(row=0, column=2, sticky="ns")
            box.columnconfigure(0, weight=1)
            self.buttons(box, 1, ("Refresh", self.refresh_mod_list),
                         ("Repackage edited files", lambda: self._mod_repack("changed")),
                         ("Repackage selected", lambda: self._mod_repack("selected")),
                         ("Repackage all", lambda: self._mod_repack("all")),
                         ("Preview selected", self._mod_preview),
                         ("Open edit folder", lambda: self.open_folder(os.path.join(self.mod_dir(), "edit"))),
                         ("Open out folder", lambda: self.open_folder(os.path.join(self.mod_dir(), "out"))))
            self.mod_items = {}

            play = self.section(tab, "Play the mod")
            ttk.Label(play, wraplength=900, justify="left", text=(
                "Install copies out\\ to the game's mod\\<mod name>\\ folder and adds the mod DLL (d3d9.dll and "
                "gurumin_mod.ini) next to game.exe. The DLL loads every file in mod\\ in place of the game's file of "
                "the same name; the game's own files are not touched. Remove takes this mod's folder out again "
                "(mod\\uninstall.bat removes the DLL).")).grid(row=0, column=0, columnspan=3, sticky="w", padx=4)
            self.buttons(play, 1, ("Install into the game", self._mod_install),
                         ("Remove from the game", self._mod_uninstall),
                         ("Open the game's mod folder", lambda: self.open_folder(os.path.join(self.game_dir(), "mod"))))

        def _mod_name(self):
            return os.path.basename(os.path.normpath(self.mod_dir())) or "mod"

        def _mod_install(self):
            if not self._mod_ready():
                return
            out_dir = os.path.join(self.mod_dir(), "out")
            self.run([Job("install %s into %s\\mod" % (self._mod_name(), self.game_dir()),
                          tool_command("mod_install.py", "install", out_dir, self.game_dir(), self._mod_name()))])

        def _mod_uninstall(self):
            if not self._mod_ready():
                return
            self.run([Job("remove %s from %s\\mod" % (self._mod_name(), self.game_dir()),
                          tool_command("mod_install.py", "remove", self.game_dir(), self._mod_name()))])

        def _ticked_kinds(self):
            return [k for k in FILE_KINDS if self.mod_kind_vars[k.key].get()]

        def _mod_export_job(self, game_file):
            kind = kind_for_game_file(game_file)
            if not kind:
                return None
            edit_file = self.edit_path_for(game_file, kind)
            return Job("%s -> %s" % (os.path.relpath(game_file, self.game_dir()), edit_file),
                       kind.export(self, game_file, edit_file), kind.edit_outputs(edit_file),
                       after=self._record_export(edit_file))

        def _mod_ready(self):
            if not self.require(("game folder", self.game_dir()), ("mod folder", self.mod_dir())):
                return False
            if is_inside(self.mod_dir(), self.game_dir()):
                messagebox.showwarning("Mod folder", "Choose a mod folder outside the game folder.")
                return False
            return True

        def _mod_export_files(self):
            if not self._mod_ready():
                return
            files = filedialog.askopenfilenames(initialdir=self.game_dir(), title="Game files to export")
            jobs, skipped = [], []
            for path in files:
                path = os.path.normpath(path)
                job = self._mod_export_job(path) if is_inside(path, self.game_dir()) else None
                (jobs.append(job) if job else skipped.append(path))
            for path in skipped:
                self.log("skipped %s (not a known type inside the game folder)\n" % path, "error")
            self.run(jobs)

        def _mod_export_folder(self):
            if not self._mod_ready():
                return
            folder = filedialog.askdirectory(initialdir=self.game_dir(), title="Game sub-folder to export")
            if folder and is_inside(folder, self.game_dir()):
                self._mod_export_tree(os.path.normpath(folder))
            elif folder:
                self.log("The folder must be inside the game folder.\n", "error")

        def _mod_export_tree(self, folder):
            if not self._mod_ready():
                return
            kinds = self._ticked_kinds()
            files = find_files(folder, lambda n: any(k.matches(n) for k in kinds))
            self.run([job for job in (self._mod_export_job(f) for f in files) if job])

        def scan_mod(self):
            """[(edit_file, kind, game_rel, status)] for every file in <mod>/edit that follows the naming rule."""
            mod = self.mod_dir()
            edit_root = os.path.join(mod, "edit")
            if not mod or not os.path.isdir(edit_root):
                return []
            exported = load_mod_state(mod).get("exported", {})
            rows = []
            for edit_file in find_files(edit_root, lambda n: kind_for_edit_file(n) is not None, frame_folders=True):
                kind, game_name = kind_for_edit_file(edit_file)
                game_rel = self.game_rel_for_edit(edit_file, game_name)
                out_file = os.path.join(mod, "out", game_rel)
                mtime = edit_mtime(edit_file)
                rec = exported.get(os.path.relpath(edit_file, mod).replace(os.sep, "/"))
                if os.path.isfile(out_file) and os.path.getmtime(out_file) >= mtime:
                    status = "repackaged"
                elif rec is not None and abs(rec - mtime) < 1e-3:
                    status = "unchanged"
                else:
                    status = "edited - needs repackaging"
                rows.append((edit_file, kind, game_rel, status))
            ref_root = os.path.join(mod, "reference")
            if os.path.isdir(ref_root):
                for ref in find_files(ref_root, lambda n: n.lower().endswith((".obj", ".gif", ".png"))):
                    rows.append((ref, None, os.path.splitext(os.path.relpath(ref, ref_root))[0], "reference (view only)"))
            return rows

        def refresh_mod_list(self):
            if not hasattr(self, "mod_tree"):
                return
            self.mod_tree.delete(*self.mod_tree.get_children())
            self.mod_items = {}
            for edit_file, kind, game_rel, status in self.scan_mod():
                item = self.mod_tree.insert("", "end", values=(game_rel, os.path.relpath(edit_file, self.mod_dir()), status))
                self.mod_items[item] = (edit_file, kind, game_rel, status)

        def _mod_repack(self, which):
            if not self._mod_ready():
                return
            rows = self.scan_mod()
            if which == "selected":
                chosen = {self.mod_items[i][0] for i in self.mod_tree.selection() if i in self.mod_items}
                rows = [r for r in rows if r[0] in chosen]
            elif which == "changed":
                rows = [r for r in rows if r[3].startswith("edited")]
            jobs = []
            for edit_file, kind, game_rel, _status in rows:
                if kind is None or kind.repack is None:
                    continue
                out_file = os.path.join(self.mod_dir(), "out", game_rel)
                original = os.path.join(self.game_dir(), game_rel)
                if kind.key == "font" and not os.path.isfile(original):
                    self.log("skipped %s: original font %s not found\n" % (edit_file, original), "error")
                    continue
                jobs.append(Job("%s -> out\\%s" % (os.path.relpath(edit_file, self.mod_dir()), game_rel),
                                kind.repack(self, edit_file, out_file, original), [out_file],
                                after=self._hd_movie_check(kind, out_file, os.path.join(self.mod_dir(), "out"))))
            self.run(jobs)

        def _mod_preview(self):
            for item in self.mod_tree.selection():
                edit_file = self.mod_items[item][0]
                if edit_file.lower().endswith((".png", ".gif")):
                    self.preview_image(edit_file)
                elif os.path.isdir(edit_file):
                    self.open_folder(edit_file)    # movie frames
                elif hasattr(os, "startfile"):
                    os.startfile(edit_file)    # text, WAV, OBJ, BIN: open with the system's default program

        # ---- Scripts (.itm) ---------------------------------------------------------------------------------

        def _build_itm_tab(self):
            kind = KIND_BY_KEY["itm"]
            tab = self.new_tab("Scripts (.itm)", "map.itm and 3ddata\\*.itm hold the game's scripts, tables and "
                               "some BMPs, scrambled. Decode gives plain text; encode scrambles it again with the key "
                               "of the game file's name (so keep the name, e.g. stage01.itm.txt -> stage01.itm).")
            box = self.section(tab, "One file")
            src, dst = self.single_file_rows(box, kind, ".itm file", [("Scripts", "*.itm")], "Text output",
                                             [("Text", "*.txt")])
            self.buttons(box, 2, ("Decode", lambda: self.require(("input", src.get()), ("output", dst.get()))
                                  and self.run_one("decode " + src.get(), cmd_itm_decode(src.get(), dst.get()),
                                                   [dst.get()], after=self._record_export(dst.get()))))
            enc = self.section(tab, "Encode one text file")
            esrc, edst = self.single_file_rows(enc, kind, "Text file", [("Text", "*.txt"), ("All", "*.*")],
                                               ".itm output", [("Scripts", "*.itm")],
                                               default_out=lambda p: self.out_path_for(p, kind))
            self.buttons(enc, 2, ("Encode", lambda: self.require(("input", esrc.get()), ("output", edst.get()))
                                  and self.run_one("encode " + esrc.get(), cmd_itm_encode(esrc.get(), edst.get()),
                                                   [edst.get()])))
            self._batch_section(tab, kind)

        def _batch_section(self, tab, kind, export=None):
            """Whole-folder decode and encode for one kind."""
            box = self.section(tab, "Whole folder (sub-folders included)")
            src, out = self.batch_rows(box, 0, "Folder with %s" % kind.game_ext, "Export to",
                                       lambda: os.path.join(self.mod_dir(), "edit") if self.mod_dir() else "")
            self.buttons(box, 2, ("Export every %s file" % kind.game_ext, lambda: self.require(
                ("source folder", src.get()), ("output folder", out.get())) and self.run(
                self.batch_export_jobs(kind, src.get(), out.get(), export))))
            if kind.repack:
                esrc, eout = self.batch_rows(box, 3, "Folder with edited *%s%s" % (
                    kind.edit_ext, " folders" if kind.folder else ""), "Rebuild into",
                                             lambda: os.path.join(self.mod_dir(), "out") if self.mod_dir() else "")
                orig = PathRow(box, 5, "Originals (game folder)", "dir")
                orig.set(self.game_dir())
                originals = lambda: orig.get() or self.game_dir()
                ttk.Label(box, text="Originals: the game folder the files came from (music copies its sync block, "
                          "the font needs its original, saves keep their key).", wraplength=900).grid(
                    row=6, column=0, columnspan=3, sticky="w", padx=4)
                self.buttons(box, 7, ("Rebuild every *%s" % kind.edit_ext, lambda: self.require(
                    ("edited folder", esrc.get()), ("output folder", eout.get())) and self.run(
                    self.batch_repack_jobs(kind, esrc.get(), eout.get(), originals()))))

        # ---- Textures (.iti) --------------------------------------------------------------------------------

        def _build_iti_tab(self):
            kind = KIND_BY_KEY["iti"]
            tab = self.new_tab("Textures (.iti)", "Textures to PNG and back. Encoding writes an uncompressed texture "
                               "(type 0, or type 1 when the PNG has transparency), which the game loads like the "
                               "packed ones.")
            box = self.section(tab, "One file")
            src, dst = self.single_file_rows(box, kind, ".iti file", [("Textures", "*.iti")], "PNG output",
                                             [("PNG", "*.png")])
            packs = lambda: self.settings.get("packs_dll") or None
            self.buttons(box, 2,
                         ("Decode", lambda: self.require(("input", src.get()), ("output", dst.get())) and self.run_one(
                             "decode " + src.get(), cmd_iti_decode(src.get(), dst.get(), packs()), [dst.get()],
                             after=self._record_export(dst.get()))),
                         ("Preview PNG", lambda: self.preview_image(dst.get())))
            enc = self.section(tab, "Encode one PNG")
            esrc, edst = self.single_file_rows(enc, kind, "PNG file", [("PNG", "*.png")], ".iti output",
                                               [("Textures", "*.iti")], default_out=lambda p: self.out_path_for(p, kind))
            self.buttons(enc, 2,
                         ("Encode", lambda: self.require(("input", esrc.get()), ("output", edst.get())) and self.run_one(
                             "encode " + esrc.get(), cmd_iti_encode(esrc.get(), edst.get()), [edst.get()])),
                         ("Preview PNG", lambda: self.preview_image(esrc.get())))
            info = self.section(tab, "Info")
            folder = PathRow(info, 0, "Folder", "dir")
            folder.set(self.game_dir())
            self.buttons(info, 1, ("Count texture types", lambda: self.require(("folder", folder.get())) and self.run_one(
                "texture types in " + folder.get(), cmd_iti_info(folder.get()))))
            self._batch_section(tab, kind)

        # ---- Models (.it3) ----------------------------------------------------------------------------------

        def _build_it3_tab(self):
            kind = KIND_BY_KEY["it3"]
            tab = self.new_tab("Models (.it3)", "Models: list the nodes or export the geometry as OBJ (+ MTL). The "
                               "MTL names the textures as <name>.png: decode those .iti files on the Textures tab into "
                               "the same folder to see them textured. There is no OBJ -> .it3 import yet.")
            box = self.section(tab, "One file")
            src, dst = self.single_file_rows(box, kind, ".it3 file", [("Models", "*.it3")], "OBJ output",
                                             [("Wavefront OBJ", "*.obj")])
            local = tk.BooleanVar(value=False)
            ttk.Checkbutton(box, text="Local coordinates (no node transforms)", variable=local).grid(
                row=2, column=1, sticky="w", padx=4)
            self.buttons(box, 3,
                         ("Info (node list)", lambda: self.require(("input", src.get())) and self.run_one(
                             "info " + src.get(), cmd_it3_info(src.get()))),
                         ("Export OBJ", lambda: self.require(("input", src.get()), ("output", dst.get())) and self.run_one(
                             "obj " + src.get(), cmd_it3_obj(src.get(), dst.get(), local.get()), [dst.get()])))
            self._batch_section(tab, kind, export=lambda app, g, e: cmd_it3_obj(g, e, local.get()))

        # ---- Movies (.mmv) ----------------------------------------------------------------------------------

        def _build_mmv_tab(self):
            kind = KIND_BY_KEY["mmv"]
            tab = self.new_tab("Movies (.mmv)", "Movies: show the size and frame count, write every frame as PNG, make "
                               "an animated GIF, or build a movie from PNG frames or a GIF. To change an existing movie, "
                               "edit its PNG frames: they keep the movie's own colours exactly. A GIF is fine for "
                               "viewing or for a new animation; its colours are matched to the template's palette.")
            box = self.section(tab, "Export one movie")
            src, dst = self.single_file_rows(box, kind, ".mmv file", [("Movies", "*.mmv")], "GIF output",
                                             [("GIF", "*.gif")])
            frames_row = PathRow(box, 2, "Frames folder", "dir")
            src.on_change = lambda p: (dst.set(self.edit_path_for(p, kind, ".gif", "reference")),
                                       frames_row.set(self.edit_path_for(p, kind)))
            opts = ttk.Frame(box)
            opts.grid(row=3, column=1, sticky="w", padx=4)
            fps, every = tk.StringVar(value="15"), tk.StringVar(value="1")
            ttk.Label(opts, text="GIF frames per second").pack(side="left")
            ttk.Spinbox(opts, from_=1, to=60, textvariable=fps, width=5).pack(side="left", padx=4)
            ttk.Label(opts, text="   write every Nth frame (1 = all; only all frames can be rebuilt)").pack(side="left")
            ttk.Spinbox(opts, from_=1, to=100, textvariable=every, width=5).pack(side="left", padx=4)
            self.buttons(box, 4,
                         ("Info", lambda: self.require(("input", src.get())) and self.run_one(
                             "info " + src.get(), cmd_mmv_info(src.get()))),
                         ("Write PNG frames", lambda: self.require(("input", src.get()), ("frames folder", frames_row.get()))
                          and self.run_one("frames " + src.get(), cmd_mmv_frames(src.get(), frames_row.get(), every.get()),
                                           [frames_row.get() + os.sep], after=self._record_export(frames_row.get()))),
                         ("Make GIF", lambda: self.require(("input", src.get()), ("output", dst.get())) and self.run_one(
                             "gif " + src.get(), cmd_mmv_gif(src.get(), dst.get(), fps.get()), [dst.get()])),
                         ("Preview GIF (first frame)", lambda: self.preview_image(dst.get())))

            enc = self.section(tab, "Build a movie")
            frames_in = PathRow(enc, 0, "PNG frames folder", "dir")
            gif_in = PathRow(enc, 1, "or an animated GIF", "open", [("GIF", "*.gif")])
            template = PathRow(enc, 2, "Template (original .mmv)", "open", [("Movies", "*.mmv")])
            out = PathRow(enc, 3, ".mmv output", "save", [("Movies", "*.mmv")])

            def fill(path):
                out.set(self.out_path_for(path, kind))
                template.set(self.original_for_edit(path, kind) or template.get())
            frames_in.on_change = lambda p: (gif_in.set(""), fill(p))
            gif_in.on_change = lambda p: (frames_in.set(""), fill(p))
            dither, original_frame1 = tk.BooleanVar(value=False), tk.BooleanVar(value=False)
            flags = ttk.Frame(enc)
            flags.grid(row=4, column=1, sticky="w", padx=4)
            ttk.Checkbutton(flags, text="Dither (smoother gradients, larger file)", variable=dither).pack(side="left")
            ttk.Checkbutton(flags, text="Frame 1 as in the original files (byte-identical rebuild of unedited frames)",
                            variable=original_frame1).pack(side="left", padx=12)
            ttk.Label(enc, wraplength=900, justify="left", text=(
                "The template gives the palette when the frames are not already indexed, and its size decides the kind "
                "of movie: frames at the original size make a normal movie; frames exactly 2x or 4x as wide and high "
                "make an HD movie (sharper, same size on screen; it needs the mod DLL, which 'Repackage' adds to "
                "out\\). Frames written by 'Write PNG frames' keep their palette. Index 255 is not used (the game "
                "replaces that colour).")).grid(
                row=5, column=0, columnspan=3, sticky="w", padx=4)

            def build():
                source = frames_in.get() or gif_in.get()
                if not self.require(("frames folder or GIF", source), ("output", out.get())):
                    return
                self.run_one("encode " + source, cmd_mmv_encode(source, out.get(), template.get() or None,
                                                                dither.get(), original_frame1.get()), [out.get()],
                             after=self._hd_movie_check(kind, out.get(), None))
            self.buttons(enc, 6, ("Build .mmv", build))
            self._batch_section(tab, kind)

        # ---- Music (.de2) -----------------------------------------------------------------------------------

        def _build_de2_tab(self):
            kind = KIND_BY_KEY["de2"]
            tab = self.new_tab("Music (.de2)", "Music to WAV and back. The WAV to encode must be 44.1 kHz, stereo, "
                               "16-bit PCM. Give the original .de2 as template so its sync block is kept. The output "
                               "must keep the game file's name (the header is keyed by it). Encoding is untested in "
                               "game.")
            box = self.section(tab, "One file")
            src, dst = self.single_file_rows(box, kind, ".de2 file", [("Music", "*.de2")], "WAV output",
                                             [("WAV", "*.wav")])
            self.buttons(box, 2,
                         ("Info", lambda: self.require(("input", src.get())) and self.run_one(
                             "info " + src.get(), cmd_de2_info(src.get()))),
                         ("Decode", lambda: self.require(("input", src.get()), ("output", dst.get())) and self.run_one(
                             "decode " + src.get(), cmd_de2_decode(src.get(), dst.get()), [dst.get()],
                             after=self._record_export(dst.get()))))
            enc = self.section(tab, "Encode one WAV")
            template = PathRow(enc, 2, "Template (original .de2)", "open", [("Music", "*.de2")])

            def fill(path):
                edst.set(self.out_path_for(path, kind))
                template.set(self.original_for_edit(path, kind) or template.get())
            esrc, edst = self.single_file_rows(enc, kind, "WAV file", [("WAV", "*.wav")], ".de2 output",
                                               [("Music", "*.de2")])
            esrc.on_change = fill
            self.buttons(enc, 3, ("Encode", lambda: self.require(("input", esrc.get()), ("output", edst.get()))
                                  and self.run_one("encode " + esrc.get(),
                                                   cmd_de2_encode(esrc.get(), edst.get(), template.get() or None),
                                                   [edst.get()])))
            self._batch_section(tab, kind)

        # ---- Font -------------------------------------------------------------------------------------------

        def _build_font_tab(self):
            kind = KIND_BY_KEY["font"]
            tab = self.new_tab("Font", "The in-game font (3ddata\\ddfont8.dat, ddfont8s.dat) as a PNG atlas: 64 glyphs "
                               "per row in 20x17 cells, grey levels = coverage. Export also writes a .tsv (code and "
                               "width per glyph) next to the PNG; keep it there, import reads it. Import needs the "
                               "original .dat as a base.")
            box = self.section(tab, "Export")
            src, dst = self.single_file_rows(box, kind, "Font .dat", [("Font", "ddfont*.dat"), ("All", "*.*")],
                                             "PNG atlas", [("PNG", "*.png")])
            self.buttons(box, 2,
                         ("Export", lambda: self.require(("input", src.get()), ("output", dst.get())) and self.run_one(
                             "export " + src.get(), cmd_font_export(src.get(), dst.get()),
                             [dst.get(), os.path.splitext(dst.get())[0] + ".tsv"], after=self._record_export(dst.get()))),
                         ("Preview atlas", lambda: self.preview_image(dst.get())))
            imp = self.section(tab, "Import")
            original = PathRow(imp, 2, "Original font .dat", "open", [("Font", "ddfont*.dat"), ("All", "*.*")])

            def fill(path):
                out.set(self.out_path_for(path, kind))
                original.set(self.original_for_edit(path, kind) or original.get())
            png, out = self.single_file_rows(imp, kind, "Edited PNG atlas", [("PNG", "*.png")], "New font .dat",
                                             [("Font", "*.dat")])
            png.on_change = fill

            def do_import():
                if not self.require(("edited PNG", png.get()), ("original font", original.get()), ("output", out.get())):
                    return
                if not os.path.isfile(os.path.splitext(png.get())[0] + ".tsv"):
                    messagebox.showwarning("Font", "The .tsv written by Export must sit next to the PNG:\n" +
                                           os.path.splitext(png.get())[0] + ".tsv")
                    return
                self.run_one("import " + png.get(), cmd_font_import(original.get(), png.get(), out.get()), [out.get()])
            self.buttons(imp, 3, ("Import", do_import))

        # ---- Saves ------------------------------------------------------------------------------------------

        def _build_save_tab(self):
            kind = KIND_BY_KEY["sav"]
            tab = self.new_tab("Saves", "Save files (SaveData\\gdataNN.sav): decode to the raw game state, encode back, "
                               "show a summary or the load-menu thumbnail, or change a few values. The output save "
                               "must have the slot's file name (the scramble is keyed by it). Values are changed on a "
                               "copy, never on the original.")
            box = self.section(tab, "One save")
            src, dst = self.single_file_rows(box, kind, "Save file", [("Saves", "*.sav")], "Decoded .bin",
                                             [("Raw state", "*.bin")])
            thumb = PathRow(box, 2, "Thumbnail PNG", "save", [("PNG", "*.png")])
            src.on_change = lambda p: (dst.set(self.edit_path_for(p, kind)),
                                       thumb.set(self.edit_path_for(p, kind, "_thumb.png", "reference")))
            self.buttons(box, 3,
                         ("Info", lambda: self.require(("save", src.get())) and self.run_one(
                             "info " + src.get(), cmd_save_info(src.get()))),
                         ("Decode", lambda: self.require(("save", src.get()), ("output", dst.get())) and self.run_one(
                             "decode " + src.get(), cmd_save_decode(src.get(), dst.get()), [dst.get()],
                             after=self._record_export(dst.get()))),
                         ("Thumbnail", lambda: self.require(("save", src.get()), ("thumbnail", thumb.get())) and self.run_one(
                             "thumb " + src.get(), cmd_save_thumb(src.get(), thumb.get()), [thumb.get()],
                             after=lambda ok: ok and self.messages.put(("call", lambda: self.preview_image(thumb.get()))))))

            enc = self.section(tab, "Encode a decoded .bin")
            key_var = tk.StringVar(value=str(DEFAULT_SAVE_KEY))

            def fill(path):
                edst.set(self.out_path_for(path, kind))
                original = self.original_for_edit(path, kind)
                if original and os.path.isfile(original):
                    key_var.set(str(read_save_key(original)))
            esrc, edst = self.single_file_rows(enc, kind, "Decoded .bin", [("Raw state", "*.bin")], "Save output",
                                               [("Saves", "*.sav")])
            esrc.on_change = fill
            keys = ttk.Frame(enc)
            keys.grid(row=2, column=1, sticky="w", padx=4)
            ttk.Label(keys, text="Key (decimal; filled in from the original save when known)").pack(side="left")
            ttk.Entry(keys, textvariable=key_var, width=8).pack(side="left", padx=4)
            self.buttons(enc, 3, ("Encode", lambda: self.require(("input", esrc.get()), ("output", edst.get()))
                                  and self._check_int(key_var.get()) and self.run_one(
                                      "encode " + esrc.get(), cmd_save_encode(esrc.get(), edst.get(), int(key_var.get())),
                                      [edst.get()])))

            edit = self.section(tab, "Change values (on a copy)")
            sav_in = PathRow(edit, 0, "Save file", "open", [("Saves", "*.sav")])
            sav_out = PathRow(edit, 1, "Write the changed copy to", "save", [("Saves", "*.sav")])
            sav_in.on_change = lambda p: sav_out.set(self._save_copy_default(p))
            fields_bar = ttk.Frame(edit)
            fields_bar.grid(row=2, column=0, columnspan=3, sticky="w", padx=4)
            field_vars = {}
            for name in SAVE_FIELDS:
                ttk.Label(fields_bar, text=name).pack(side="left", padx=(8, 2))
                field_vars[name] = tk.StringVar()
                ttk.Entry(fields_bar, textvariable=field_vars[name], width=8).pack(side="left")
            ttk.Label(edit, text="Leave a box empty to keep that value. The copy keeps the slot's file name.").grid(
                row=3, column=0, columnspan=3, sticky="w", padx=4)

            def do_set():
                fields = {k: v.get().strip() for k, v in field_vars.items() if v.get().strip()}
                if not self.require(("save", sav_in.get()), ("copy", sav_out.get()), ("a value", fields)):
                    return
                if not all(self._check_int(v) for v in fields.values()):
                    return
                if os.path.basename(sav_in.get()).lower() != os.path.basename(sav_out.get()).lower():
                    messagebox.showwarning("Save", "The copy must have the same file name as the save (gdataNN.sav).")
                    return
                if os.path.abspath(sav_in.get()) == os.path.abspath(sav_out.get()):
                    messagebox.showwarning("Save", "Choose a different folder for the copy.")
                    return
                source, copy = sav_in.get(), sav_out.get()
                self.run_one("set %s on a copy: %s" % (fields, copy), cmd_save_set(copy, fields), [copy],
                             before=lambda: shutil.copyfile(source, copy))
            self.buttons(edit, 4, ("Copy and change", do_set))
            self._batch_section(tab, kind)

        def _save_copy_default(self, sav):
            if self.mod_dir() and is_inside(sav, self.game_dir()):
                return os.path.join(self.mod_dir(), "out", os.path.relpath(sav, self.game_dir()))
            return os.path.join(os.path.dirname(sav), "changed", os.path.basename(sav))

        def _check_int(self, text):
            try:
                int(str(text), 0)
                return True
            except ValueError:
                messagebox.showwarning("Number", "Not a whole number: %r" % text)
                return False

        # ---- Doc generators ---------------------------------------------------------------------------------

        def _build_docs_tab(self):
            tab = self.new_tab("Docs", "Regenerate the reference documents in docs/ from the decompiler output. These "
                               "need the Ghidra export (re/export, local only) and are for contributors; modders do "
                               "not need them.")
            box = self.section(tab, "Generators")
            for row, (text, script, needs, writes) in enumerate(DOC_GENERATORS):
                ttk.Button(box, text=text, width=24, command=lambda s=script, w=writes: self.run_one(
                    "%s -> %s" % (s, w), cmd_doc_generator(s))).grid(row=row, column=0, sticky="w", padx=4, pady=2)
                ttk.Label(box, text="%s  (needs %s, writes %s)" % (script, needs, writes)).grid(
                    row=row, column=1, sticky="w", padx=4)
            self.buttons(box, len(DOC_GENERATORS), ("Open docs folder", lambda: self.open_folder(
                os.path.join(REPO_DIR, "docs"))))

    root = tk.Tk()
    app = App(root)

    def close():
        app._on_close_sync()
        app._on_close()
    root.protocol("WM_DELETE_WINDOW", close)
    return root, app


# ----------------------------------------------------------------------------------------------------------------
# --print-commands: show the command behind every button with placeholder paths (no files are touched)
# ----------------------------------------------------------------------------------------------------------------

def print_commands():
    g, m = os.path.join("GAME", "3ddata"), os.path.join("MOD", "edit", "3ddata")
    o = os.path.join("MOD", "out", "3ddata")
    samples = [
        cmd_itm_decode(os.path.join(g, "stage01.itm"), os.path.join(m, "stage01.itm.txt")),
        cmd_itm_encode(os.path.join(m, "stage01.itm.txt"), os.path.join(o, "stage01.itm")),
        cmd_iti_decode(os.path.join(g, "tex.iti"), os.path.join(m, "tex.iti.png"), "GAME\\bin\\Packs.dll"),
        cmd_iti_encode(os.path.join(m, "tex.iti.png"), os.path.join(o, "tex.iti")),
        cmd_iti_info("GAME"),
        cmd_it3_info(os.path.join(g, "model.it3")),
        cmd_it3_obj(os.path.join(g, "model.it3"), "MOD\\reference\\3ddata\\model.it3.obj", local=True),
        cmd_mmv_info("GAME\\movie.mmv"),
        cmd_mmv_frames("GAME\\3ddata\\logo.mmv", "MOD\\edit\\3ddata\\logo.mmv.frames"),
        cmd_mmv_gif("GAME\\3ddata\\logo.mmv", "MOD\\reference\\3ddata\\logo.mmv.gif", fps=15),
        cmd_mmv_encode("MOD\\edit\\3ddata\\logo.mmv.frames", "MOD\\out\\3ddata\\logo.mmv",
                       "GAME\\3ddata\\logo.mmv"),
        cmd_mmv_encode("new.gif", "MOD\\out\\3ddata\\logo.mmv", "GAME\\3ddata\\logo.mmv", dither=True,
                       original_frame1=True),
        cmd_de2_info("GAME\\bgm01.de2"),
        cmd_de2_decode("GAME\\bgm01.de2", "MOD\\edit\\bgm01.de2.wav"),
        cmd_de2_encode("MOD\\edit\\bgm01.de2.wav", "MOD\\out\\bgm01.de2", "GAME\\bgm01.de2"),
        cmd_font_export(os.path.join(g, "ddfont8.dat"), os.path.join(m, "ddfont8.dat.png")),
        cmd_font_import(os.path.join(g, "ddfont8.dat"), os.path.join(m, "ddfont8.dat.png"),
                        os.path.join(o, "ddfont8.dat")),
        cmd_save_info("GAME\\SaveData\\gdata01.sav"),
        cmd_save_decode("GAME\\SaveData\\gdata01.sav", "MOD\\edit\\SaveData\\gdata01.sav.bin"),
        cmd_save_encode("MOD\\edit\\SaveData\\gdata01.sav.bin", "MOD\\out\\SaveData\\gdata01.sav", 627),
        cmd_save_thumb("GAME\\SaveData\\gdata01.sav", "MOD\\reference\\SaveData\\gdata01.sav_thumb.png"),
        cmd_save_set("MOD\\out\\SaveData\\gdata01.sav", {"money": 9999, "hp": 8}),
    ] + [cmd_doc_generator(script) for _t, script, _n, _w in DOC_GENERATORS]
    for command in samples:
        print(subprocess.list2cmdline(command[1:]))
    print("\nmod naming:")
    for name in ("stage01.itm.txt", "tex.iti.png", "bgm01.de2.wav", "ddfont8.dat.png", "gdata01.sav.bin", "logo.mmv.frames",
                 "notes.txt"):
        hit = kind_for_edit_file(name)
        print("  %-18s -> %s" % (name, "%s (%s)" % (hit[1], hit[0].key) if hit else "not part of the mod"))


def run_tool_from_command_line(argv):
    """GuruminModTools.exe --run-tool itm_tool.py decode a.itm a.txt: runs one bundled tool without the window."""
    if sys.stdout is None:          # the exe has no console of its own; borrow the one it was started from
        try:
            import ctypes
            ctypes.windll.kernel32.AttachConsole(-1)
            sys.stdout = sys.stderr = open("CONOUT$", "w", encoding="utf-8", errors="replace")
        except OSError:
            sys.stdout = sys.stderr = open(os.devnull, "w")
    out = sys.stdout

    def post(kind, text):
        if kind in ("log", "error"):
            out.write(text[2:] if text.startswith("  ") else text)
            out.flush()

    script = os.path.join(TOOLS_DIR, os.path.basename(argv[0]))
    return 0 if JobRunner(post)._run_in_process([script] + argv[1:]) else 1


def main():
    if "--run-tool" in sys.argv[1:2]:
        sys.exit(run_tool_from_command_line(sys.argv[2:]))
    if "--print-commands" in sys.argv:
        print_commands()
        return
    root, _app = build_gui()
    root.mainloop()


if __name__ == "__main__":
    main()
