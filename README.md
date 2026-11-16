# Gurumin (PC, JP 2006) — reverse engineering, modding tools and matching decompilation

Work-in-progress study of `bin\APP.dll` from the Japanese PC release of Gurumin, aimed at making the game easier
to mod, with a byte-matching decompilation growing alongside. No game files are included; point the tools at
your own copy.

## Modding tools (`tools/`, Python 3; Pillow for images)
| Tool | Does |
|---|---|
| `itm_tool.py decode/encode` | `.itm` scripts/tables/BMPs (map.itm, 3ddata\*.itm) ↔ plain text. 2,511 files round-trip. |
| `iti_tool.py decode/encode/info` | `.iti` textures ↔ PNG (types 0/1/888/889; writes raw type 0/1). |
| `it3_tool.py info/obj` | `.it3` model node list; OBJ export with normals/UVs. All 945 JP models decode. |
| `gpack.py` | Python port of packs.dll `ExecUnpack` (used by .it3 meshes). |
| `save_tool.py decode/encode/info/thumb` | `SaveData\gdataNN.sav` ↔ raw state; thumbnail extract. |
| `script_api.py`, `gen_script_api_md.py` | script built-in list → `docs/SCRIPT_API.md`. |
| `config_keys.py`, `save_layout.py` | gurumin.def key map, save body map (from the decompiler output). |

## Documentation (`docs/`)
`MODDING_MAP.md` (start here), `ITM_FORMAT.md`, `SCRIPT_API.md`, `MAP_TABLES.md`, `SAVE_FORMAT.md`, `PACKS_DLL.md`,
`config_keys.tsv`, `save_layout.tsv`. Function names: `symbols.tsv` (apply to Ghidra with `re/apply_symbols.bat`).

## Decompilation (`decomp/`)
VS .NET 2003 (`decomp/env.bat`), flags `/O2 /Ob1 /Gy /GS`, byte compare against APP.dll:
`decomp\build.bat <file.cpp>`, `decomp\build_all.bat`. The original was built with VS .NET 2002 (cl 13.00.9466);
functions that 13.10 cannot reproduce are listed in `decomp/open.tsv`. The comparer reports MATCH (bytes identical,
relocations masked), EQUIV (identical instruction stream; only cl 13.00 encoding quirks such as `[esp+0]` with a disp8 differ) or DIFF. Notes and findings: `NOTES.md`.

## Ghidra
`re\run_ghidra_export.bat` (APP.dll), `re\run_ghidra_packs.bat` (packs.dll); exports land in `re\export\` (local only).
