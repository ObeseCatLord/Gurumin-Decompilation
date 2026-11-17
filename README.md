<p align="center"><img src="docs/media/logo_header.png" alt="Gurumin: A Monstrous Adventure" width="720"></p>

# Gurumin decompilation and modding

A matching decompilation of **Gurumin** (Japanese PC release, 2006) together with tools and documentation for
modding it. The target is the game code in `bin\APP.dll`. The goals, in order:

1. **Moddability:** readable source, named functions and documented file formats, so changes are easy to make.
2. **Modern hardware:** lift the 2006-era limits (frame rate, resolution, text rendering, input) on today's PCs.
3. **A byte-matching rebuild:** every function compiles back to the original bytes with the original compiler.

No game files are included. Point the tools at your own copy of the game.

## Progress

<p align="center"><img src="progress/jp/progress.svg" alt="Decompilation progress" width="600"></p>

The meter is regenerated with every progress report and refreshed by CI on each push to `master`; the same data is
published as the `jp_report` artifact for ByteMatch. Matched functions are the smaller ones
so far: a few very large functions (the model loader at 0x1017cc90 alone is 29,886 bytes) hold most of the code, so
the byte share lags the function share. `python decomp/tools/build_all.py` prints the current figures and
`decomp/tools/todo.py` lists what is left.

## What you can do today

### Modding tools (`tools/`, Python 3; Pillow for images)
| Tool | Does |
|---|---|
| `itm_tool.py decode/encode` | `.itm` scripts, tables and BMPs (map.itm, 3ddata\*.itm) ↔ plain text. 2,511 files round-trip. |
| `iti_tool.py decode/encode/info` | `.iti` textures ↔ PNG (all types; 666/667 via your packs.dll and tools/native/iti_unpack.c; writes raw type 0/1). |
| `it3_tool.py info/obj` | `.it3` model node list; OBJ export with normals and UVs. All 945 JP models decode. |
| `gpack.py` | Python port of packs.dll `ExecUnpack` (used by .it3 meshes). |
| `mmv_tool.py info/frames/gif` | `.mmv` movies → PNG frames or GIF. |
| `de2_tool.py info/decode/encode` | `.de2` music ↔ WAV (MS ADPCM; encoding untested in game). |
| `ddfont_tool.py export/import` | in-game font ↔ PNG atlas (byte-identical round trip). |
| `save_tool.py decode/encode/info/thumb` | `SaveData\gdataNN.sav` ↔ raw state; thumbnail extract. |
| `script_api.py`, `gen_script_api_md.py` | script built-in list → `docs/SCRIPT_API.md`. |
| `config_keys.py`, `save_layout.py` | gurumin.def key map, save body map (from the decompiler output). |

### Documentation (`docs/`)
`MODDING_MAP.md` (start here: costumes, anti-cheat guards, render states, sound, menu images), `ITM_FORMAT.md`,
`SCRIPT_API.md`, `MAP_TABLES.md`, `SAVE_FORMAT.md`, `PACKS_DLL.md`, `config_keys.tsv`, `save_layout.tsv`.
Function names: `symbols.tsv` (apply to Ghidra with `re/apply_symbols.bat`).

## Decompilation (`decomp/`)
- Compiler: Visual Studio .NET 2002, cl 13.00.9466, the exact build in APP.dll's Rich header (`decomp/env.bat`; set
  `GURUMIN_VC` to another `Vc7` folder to override). Flags `/O2 /Ob1 /Op /Gy /GS /GX`; experiment flags go in
  `GURUMIN_CFLAGS`.
- Build and compare: `decomp\build.bat <file.cpp>` compiles one file and byte-compares every function with APP.dll
  (relocations masked); `decomp\build_all.bat` or `python decomp/tools/build_all.py --report progress/jp/report.json`
  does everything and writes the report and the meter.
- Helpers: `tools/variants.py` builds `#if V == k` variants of a file, `tools/pick.py` keeps one, `tools/sbs.py` shows
  our code and the original side by side, `tools/asm.py` / `tools/fn.py` show the original's disassembly / Ghidra output.
- Source style: readable names and plenty of comments (what each function does, its fields and constants).
- Functions that still differ are listed in `decomp/open.tsv` with the reason; the work queue is `decomp/QUEUE.md`;
  findings and compiler habits are in `NOTES.md`.
- Ghidra: `re\run_ghidra_export.bat` (APP.dll), `re\run_ghidra_packs.bat` (packs.dll); exports land in `re\export\`
  (local only).

## Roadmap
Decompilation and naming come first. Planned once they are far enough along (details in
[`docs/ROADMAP.md`](docs/ROADMAP.md)):
- Higher frame rates, wide and ultrawide resolutions and other rendering upgrades (building on or bundling the
  MIT-licensed Gurumin Enhanced Rendering mod), and support for the PSP-exclusive costumes when you supply the files.
- **Costume transmog:** wear any unlocked costume's look while keeping another costume's abilities.
- A Unicode subtitle and text font from any TrueType/OpenType font, and in-game selection of voice, text and subtitle
  languages.
- An all-in-one GUI tool (exe) for the file tools, with a model viewer and OBJ import/replacement.
- `mmv` movie re-encoding for translations, a texture glossary, and a `textures` folder for loose texture overrides.
