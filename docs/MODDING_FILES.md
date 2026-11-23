# Replacement files (`mod\` folder)

The mod DLL (`mod/build/d3d9.dll` next to `game.exe`, Japanese PC version) loads files from a `mod\` folder in the
game folder in place of the game's own files. Nothing in the game folder is overwritten, and removing the folder
gives the original game back.

## How it works
- The DLL hooks the C runtime `fopen` inside `APP.dll` (VA 0x101b3304, `_fsopen(path, mode, 0x40)`). Every file the
  game reads goes through it, and the game always passes absolute paths.
- At start the DLL lists every file under the configured folders (sub-folders included). When the game opens a
  file for reading whose name matches a listed file, the listed file is opened instead.
- Matching uses the file name only, without case. No two of the game's 4,379 files share a name, so the
  sub-folders in `mod\` are free for sorting (`mod\My Mod\Characters\Parin\char02b2.iti` works).
- When the replacement cannot be opened, the game gets its own file.

Measured in game (log lines in `gurumin_mod.log`): "files: 2 replacement file(s) in ...\mod", "replacement loader
on", then "char02b2.iti.png -> mod\.cache\char02b2.iti (1024x1024)" when Parin's texture loaded. The name-entry
screen showed Parin with the recoloured 4x texture.

## Textures as PNG
A PNG (or JPEG) named `<texture>.png` or `<texture>.iti.png` replaces `<texture>.iti`. The DLL converts it with
Windows Imaging Component into `mod\.cache\<texture>.iti` (type 1 = RGB plus alpha) the first time, and again
whenever the PNG is newer than the cached file.

- **Any size up to 2048x2048.** The game's texture builder (FUN_10169c60) refuses larger textures. An upscaled
  texture keeps the original's proportions so that the model's texture coordinates still fit: a 256x256 texture
  can be replaced by 1024x1024 or 2048x2048.
- The exported `<texture>.iti.png` files from the GUI or `iti_tool.py` can be copied into `mod\` as they are.
- A real `.iti` file in the same folder wins over a PNG of the same texture.

## Checked files
Game_Startup checksums `bin\map.itm` and `3ddata\test.dat` (and the program files) and closes the game when one
differs from the values stored in `soundarc.dll` ([`TEXT.md`](TEXT.md#integrity-check)). The loader therefore gives
that check (File_Checksum, 0x101602d0) the game's own file and every other reader the replacement: an edited
`map.itm` works from `mod\`, but not copied over the game's file. With `log=1`, `gurumin_files.log` names the
function that opened each file (`called from 0x...`).

## Other files
Any game file can be replaced by a file of the same name: `.itm` scripts and tables, `.it3` models, `.mmv` movies
(HD movies too, see [`MOVIES_HD.md`](MOVIES_HD.md)), `.de2` music, fonts. Make them with the tools or the GUI's
*Repackage*, which writes them into `<mod>\out\`.

Files and folders starting with a dot and `.txt`, `.ini` and `.md` files are ignored, so notes can sit next to
the replacements.

## Settings (`gurumin_mod.ini`, section `[files]`)
| Key | Default | Meaning |
|---|---|---|
| `enabled` | 1 | 0 = load only the game's files |
| `folders` | `mod` | folders next to `game.exe` or full paths, separated by commas; the first one wins when two have the same file |
| `log` | 0 | 1 = write `gurumin_files.log` with every file the game opens, and the replacement used (`path <- replacement`) |

The files log is the way to find which name to replace: start the game with `log=1`, go to the scene, close the
game and read the log. The hook is only installed when there are replacement files or logging is on.

## From the GUI
1. **Export** on the Mod tab sorts the files into folders by character or type: `edit\Characters\Parin\`,
   `edit\Enemies\Slime\`, `edit\Bosses\Giga\`, `edit\Maps\`, `edit\Items\`, `edit\Interface\`,
   `edit\Cutscenes\` (scene scripts with the subtitles), `edit\Sounds\Voices\` and `edit\Sounds\Effects\` (the
   `wav\*.itm` sounds), `edit\Scripts\<name>\`, `edit\Movies\`, and so on (`tools/asset_groups.py`). Untick *Sort exports* to mirror the
   game's own folders instead.
2. **Edit** the files. Textures may be larger than the originals.
3. **Repackage** writes the game files into `<mod>\out\` with the game's folder layout.
4. **Install into the game** copies `out\` to `<game>\mod\<mod folder name>\` and adds `d3d9.dll` and
   `gurumin_mod.ini` next to `game.exe` when they are missing (a foreign `d3d9.dll` is kept as `d3d9.dll.orig`).
   **Remove from the game** deletes `<game>\mod\<mod folder name>\` again. The same from the command line:

```
python tools/mod_install.py install <mod>\out <game folder> <mod name>
python tools/mod_install.py remove <game folder> <mod name>
```

`mod\uninstall.bat <game folder>` removes the DLL.

## How files are sorted
Characters, enemies and bosses come from the `#CharTable` in the game's `bin\map.itm` (model base name and type;
type 3 = enemy). The folder names are the English release's names. A file belongs to a character when its name
starts with the character's model base name (`char02b*` = Parin), when it is one of the character's face textures
(`cf02_*` = Parin) or when only that character's models use it as a texture (the `.it3` materials). Everything
else is sorted by its name prefix (`mp*` maps, `o_` objects, `it_` items, `ef*` effects, `menu*` interface, ...)
or by its type. `python tools/asset_groups.py <game folder>` prints the folder sizes, `--list` every file.
