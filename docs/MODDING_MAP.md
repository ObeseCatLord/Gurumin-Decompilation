# Gurumin modding map

Where things live in the game and how to change them. Addresses are for the Japanese PC release's
`bin\APP.dll` (image base 0x10000000; the DLL may be relocated at run time: use `module + (va - 0x10000000)`).
The Steam English `game.exe` is a 2015 VS2013 rebuild with different addresses.

## Data you can edit without code

| What | File | Tool | Notes |
|---|---|---|---|
| Scripts and game tables | `bin\map.itm` | `tools/itm_tool.py decode/encode` | Text: `#VERSION`, ~25 `#...Table` sections, ~820 `#Script:NNNNN` blocks. See ITM_FORMAT.md. |
| Per-area scripts and dialogue | `3ddata\*.itm` (text ones) | `tools/itm_tool.py` | Same scrambling, keyed by file name. |
| Some textures | `3ddata\*.itm` (BMP ones), `images\*.itm` | `tools/itm_tool.py` | Scrambled BMP; the texture loader (0x10173550) also accepts plain `.bmp`. |
| Textures | `3ddata\*.iti` | `tools/iti_tool.py decode/encode` | `u32 type, width, height`. Type 0 raw RGB, 1 raw RGB + grey alpha, 888 JPEG + raw RGB copy, 889 JPEG colour + JPEG alpha + raw RGB + raw alpha, 666/667 use packs.dll `BMPUnpack`, a custom wavelet codec (not decoded yet). 1,022/1,097 JP files decode; encoding writes raw type 0/1, which the loader reads without packs.dll. Search order: every directory in the slot table 0x10202b40, then `./NAME.itp`, `./data/NAME.itp`. |
| Models / animation | `3ddata\*.it3` | `tools/it3_tool.py info/obj`, `tools/it3_chunks.py` | Flat chunk list `tag[4] u32 size data`; each `INFO` (name[64], 4x4 matrix, 3 words) starts a node. Mesh chunk VPA2: packs of 0x70-byte vertices + u32 triangle indices, both compressed with packs.dll `ExecUnpack` (`tools/gpack.py`, Python port). All 945 JP files decode (2.4M vertices). Vertex (28 floats, from the initialiser 0x101acf10): 0-3 position, 4-6 normal, 8-11 RGBA, 12-13 UV, 24-27 blend weights; OBJ export writes v/vn/vt. Node transforms and skinning are not applied yet. Parser 0x101a58a0. |
| Movies | `*.mmv` | none yet | Custom "MV" format, opened by 0x10160a90 (`.\bin\%s.MMV`). |
| Music | `bgm\*.de2`, ogg | none yet | `#BGMTable` in map.txt gives file, extension and loop points; played by 0x1003aef0. |
| Settings | `bin\gurumin.def` | text | `[CONFIG]`, `[KEYCONFIG]`; read once in `Game_Startup` 0x1013b700 via `Config_GetInt` 0x1015fe70. Key → global map: `docs/config_keys.tsv` (e.g. FULLSCREEN 0x10f65514, TEXD 0x1101d844 = skip the JPEG decoder, VSYNC 0x10f654a8). |
| Saves | `SaveData\gdata%02d.sav` | none yet | Menus at 0x10064630 / 0x10112140; save code 0x1013b700. |

## Script system
- Scripts are C-like (`int main() { ... }`), compiled when the map is loaded (compiler 0x100ed9b0 / 0x10089a10).
- 118 built-ins: SCRIPT_API.md. Game built-ins go through one switch (0x10138280), so a new built-in can be added by
  hooking `Script_RegisterGameBuiltins` (0x1013ad90) to register another signature with a new id and hooking the switch.

## Code map (names in `symbols.tsv`, applied to Ghidra by `re/apply_symbols.bat`)
- Startup: `GuruMain` (export, 0x1013dfb0) ← `game.exe`.
- Data load: `Game_LoadData` 0x100cb0c0 → `MapData_Load` 0x10002440 → `ItmDescramble` 0x101615d0.
- Files: `File_Open` 0x101b2310 (all loaders), `MakeDataPath` 0x1013f910.
- Graphics: Direct3D 9 only (`Direct3DCreate9`); textures 0x10173550 (bmp/itm) and 0x10174ce0 (iti); models 0x101abe30.
- Sound: external `soundarc.dll` controlled with `SendMessage(wnd, 0x400, code, value)` (0x10001570..0x10001f30;
  codes 0x201, 0x212, 0x221, 0x224, 0x225, 0x227, 0x230); BGM 0x1003aef0.
- Input: DirectInput 8; mouse helper object (0x10001090 cursor, 0x10001110 state, 0x10001450 moved-flag).
- Fonts: 0x101455d0 (`ddfont8.dat`, MS P Gothic).
- Screenshots: 0x10137b10 writes `capture\capture%05d.jpg`.
- **Anti-debug:** 0x1000eb90 calls `IsDebuggerPresent` (looked up at run time) and, when a debugger is attached,
  overwrites a random entry of the 16-int table at 0x104c0270 with `rand()`. Patch it to return early before debugging.

## Globals worth knowing
| Address | What |
|---|---|
| 0x104b7200 | the main `Game` object (thiscall `this` for most 0x1000xxxx methods; flag array at +0x3ee694, named entries at +0x95e14) |
| 0x10f1fcf0 | `IDirect3DDevice9*` used for all rendering (BeginScene/EndScene wrappers 0x1013ed20/0x1013ed60) |
| 0x10f6547c | "scene open" flag |
| 0x10f654d8 | frame timer deadline (`timeGetTime()+16`) |
| 0x10f627d0 | data directory (set by 0x1013f950) |
| 0x10db6ed8 | 128 characters × 0x249c bytes (id at +0, model object at +0x10, position Vec3 at +0x2368); slots 0x5c..0x7f are enemies |
| 0x10daab84 | per-character records × 0x188 |
| 0x1020b430 | per-character state × 0x40 |
| 0x10860df0 | 64 effect slots × 0xfc |

## packs.dll
`Packs_Load` (0x101b2670) loads `./bin/packs.dll` (else `packs.dll`) and resolves `ExecPack`, `ExecUnpack`, `BMPUnpack`,
`BMPPack`, `BMPSave`, `BMPLoad` (JPEG .iti decoding), `SaveJpeg` (screenshots) and `MMV3` (movies) into 0x11b1bb68..0x11b1bb84.
A replacement packs.dll with the same exports is a clean place to add PNG textures or modern video.
