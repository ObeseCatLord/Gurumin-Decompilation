# Gurumin (PC, JP 2006) — reverse engineering, modding tools and matching decompilation

Work-in-progress study of `bin\APP.dll` from the Japanese PC release of Gurumin, aimed at making the game easier
to mod, with a byte-matching decompilation growing alongside. No game files are included; point the tools at
your own copy.

## Modding tools (`tools/`, Python 3; Pillow for images)
| Tool | Does |
|---|---|
| `itm_tool.py decode/encode` | `.itm` scripts/tables/BMPs (map.itm, 3ddata\*.itm) ↔ plain text. 2,511 files round-trip. |
| `iti_tool.py decode/encode/info` | `.iti` textures ↔ PNG (all types; 666/667 via your packs.dll and tools/native/iti_unpack.c; writes raw type 0/1). |
| `it3_tool.py info/obj` | `.it3` model node list; OBJ export with normals/UVs. All 945 JP models decode. |
| `gpack.py` | Python port of packs.dll `ExecUnpack` (used by .it3 meshes). |
| `mmv_tool.py info/frames/gif` | `.mmv` movies → PNG frames or GIF. |
| `de2_tool.py info/decode/encode` | `.de2` music ↔ WAV (MS ADPCM; encoding untested in game). |
| `ddfont_tool.py export/import` | in-game font ↔ PNG atlas (byte-identical round trip). |
| `save_tool.py decode/encode/info/thumb` | `SaveData\gdataNN.sav` ↔ raw state; thumbnail extract. |
| `script_api.py`, `gen_script_api_md.py` | script built-in list → `docs/SCRIPT_API.md`. |
| `config_keys.py`, `save_layout.py` | gurumin.def key map, save body map (from the decompiler output). |

## Documentation (`docs/`)
`MODDING_MAP.md` (start here), `ITM_FORMAT.md`, `SCRIPT_API.md`, `MAP_TABLES.md`, `SAVE_FORMAT.md`, `PACKS_DLL.md`,
`config_keys.tsv`, `save_layout.tsv`. Function names: `symbols.tsv` (apply to Ghidra with `re/apply_symbols.bat`).

## Decompilation (`decomp/`)
VS .NET 2002, cl 13.00.9466: the exact compiler the original was built with (`decomp/env.bat`; set `GURUMIN_VC` to
another `Vc7` folder to override). Flags `/O2 /Ob1 /Op /Gy /GS /GX`; extra flags for experiments go in `GURUMIN_CFLAGS`.
`decomp\build.bat <file.cpp>`, `decomp\build_all.bat`, byte compare against APP.dll. Functions that still differ are
listed in `decomp/open.tsv` with the reason. The comparer reports MATCH (bytes identical, relocations masked) or DIFF.
Notes and findings: `NOTES.md`.

## Ghidra
`re\run_ghidra_export.bat` (APP.dll), `re\run_ghidra_packs.bat` (packs.dll); exports land in `re\export\` (local only).

**Progress (2026-11-17):** 366 of 1,212 game functions in APP.dll match (30.2 %). By size that is 32,144 of
1,720,206 code bytes (1.9 %): the matched functions are the small ones, and a few huge functions (the model loader at
0x1017cc90 alone is 29,886 bytes) hold most of the code. `python decomp/tools/build_all.py` prints
the current figures; `decomp/tools/todo.py` lists what is left.
