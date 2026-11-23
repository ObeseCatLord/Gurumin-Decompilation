# Modding GUI (`tools/gurumin_tools_gui.py`)

One window for all the modding tools in `tools/`. You don't need to type any commands: every button runs one of the
command-line tools (`itm_tool.py`, `iti_tool.py`, `it3_tool.py`, `mmv_tool.py`, `de2_tool.py`, `ddfont_tool.py`,
`save_tool.py`) and shows what the tool printed in the log at the bottom.

## Starting it
You need Python 3 (with Tkinter, which the normal Windows installer includes) and Pillow (`pip install pillow`),
like the tools themselves. Then either double-click `tools\gurumin_tools_gui.py` or run:

```
python tools\gurumin_tools_gui.py
```

`python tools\gurumin_tools_gui.py --print-commands` prints the command line behind every button, with placeholder
paths, without opening the window.

## First start: the Setup tab
- **Game folder**: your Gurumin installation (the folder with `bin\`, `3ddata\`, ...). The GUI only reads it and
  refuses to write into it.
- **Mod folder**: where your work goes. The default is `Documents\Gurumin_mod`; keep one folder per mod.
- **packs.dll** (optional): only needed for textures of type 666/667. Leave it empty and `iti_tool.py` finds the
  game's own copy in `bin\`.

The choices are saved in `.gurumin_tools_gui.json` in your user folder, so you set them only once. "List what the
game folder holds" counts the files of each type.

## Making a mod: the Mod tab
1. **Export.** Tick the file types you want and press *Export chosen files...* (pick files in the game folder),
   *Export a game sub-folder...* or *Export the whole game*. The editable files land in `<mod>\edit\` with the same
   sub-folders as the game.
2. **Edit** the files with any program: text editor for scripts, image editor for textures and the font, audio
   editor for music.
3. **Repackage.** *Repackage edited files* rebuilds only what you changed; *Repackage selected* and *Repackage all*
   do what they say. The rebuilt game files go to `<mod>\out\`, again with the game's sub-folders.
4. **Play.** Back up the game folder, then copy the contents of `<mod>\out\` over it.

The list shows every file in the mod with its status: *unchanged* (exported, not edited), *edited - needs
repackaging*, *repackaged* (the file in `out\` is newer than your edit) or *reference (view only)*. Select a row and
press *Preview selected* to look at an image or open the file in its usual program.

To continue a mod later, choose its folder as the mod folder on the Setup tab. The GUI finds everything again from
the file names, so you can also add files to `edit\` by hand if you follow the naming rule.

### Mod folder layout
| Folder | Holds | Example |
|---|---|---|
| `edit\` | editable files: the game file's name plus a new extension | `edit\3ddata\stage01.itm.txt` |
| `out\` | rebuilt game files, ready to copy into the game | `out\3ddata\stage01.itm` |
| `reference\` | things you can look at but not put back yet (models, movies, save thumbnails) | `reference\3ddata\pc.it3.obj` |
| `gurumin_mod.json` | file times recorded at export, used to tell which files you edited | |

Keep the game file's name in the editable file's name: several formats are scrambled with a key made from the
file name, and the GUI uses the name to know where each file belongs.

| Game file | Editable file | Back to the game? |
|---|---|---|
| `.itm` scripts and tables | `.itm.txt` (plain text) | yes |
| `.iti` textures | `.iti.png` | yes (written uncompressed, type 0 or 1) |
| `.de2` music | `.de2.wav` (44.1 kHz stereo 16-bit) | yes, with the original's sync block; untested in game |
| `ddfont8.dat`, `ddfont8s.dat` font | `.dat.png` + `.dat.tsv` | yes (keep the `.tsv` next to the PNG) |
| `.sav` saves | `.sav.bin` (raw game state) | yes, with the original save's key |
| `.it3` models | `.it3.obj` + `.mtl` | not yet |
| `.mmv` movies | `.mmv.gif` | not yet |

## The file-type tabs
Each type has its own tab for single files and whole folders, also outside a mod.

- **One file**: choose the input. The output box fills in by itself (into the mod folder if the input is in the
  game folder), and you can change it.
- **Whole folder**: converts every matching file under a folder, sub-folders included, and keeps the sub-folders in
  the output. The progress bar counts the files; *Cancel* stops after the current one. Rebuilding asks for the
  *Originals* folder (the game folder by default): music takes its sync block from there, the font needs its
  original file, and saves keep their key.

| Tab | Buttons |
|---|---|
| Scripts (.itm) | decode to text, encode back (the key comes from the output's name) |
| Textures (.iti) | decode to PNG, encode a PNG, preview, count texture types in a folder |
| Models (.it3) | node list, export OBJ (optionally in local coordinates). Decode the textures named in the `.mtl` on the Textures tab into the same folder to see the model textured. |
| Movies (.mmv) | size and frame count, animated GIF (frames per second), every Nth frame as PNG |
| Music (.de2) | info, decode to WAV, encode a WAV (with the original as template) |
| Font | export the atlas (64 glyphs per row, 20x17 cells), import an edited atlas |
| Saves | summary, decode, thumbnail, encode a `.bin` (key filled in from the original save), and change money/HP/costume/map on a **copy** of a save |
| Docs | regenerate `docs/script_api.tsv`, `SCRIPT_API.md`, `config_keys.tsv`, `save_layout.tsv` (needs the local Ghidra export; for contributors) |

## When something goes wrong
The log shows each tool's own messages; failures are in red with the full command, which you can run in a terminal
to see more. Common causes:
- *refused: ... is inside the game folder*: choose an output outside the game folder (the mod folder).
- *checksum MISMATCH* when decoding a script or save: the file was renamed; decode it under its original name.
- A 666/667 texture fails to decode: `tools/native/iti_unpack.exe` and the game's `packs.dll` are needed; set
  packs.dll on the Setup tab.
- Font import complains about the `.tsv`: keep the `.tsv` written by Export next to the PNG, with the same name.
