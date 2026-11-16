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
| Textures | `3ddata\*.iti` | (todo: `tools/iti_tool.py`) | 16-byte header `type, width, height, size` + JPEG; type 888/666 add a raw RGB block, 889 a second JPEG (alpha), 0/1 raw. |
| Models / animation | `3ddata\*.it3` | `tools/it3_chunks.py` (lister) | Flat list of tagged chunks `tag[4] size[u32] data`; nodes start with `INFO` (140 bytes). Tags: INFO RTY2 LIG3 INFS CHID KAN3 MAT4 VPA2 MAN2 ... parsed by 0x101a58a0. |
| Movies | `*.mmv` | none yet | Custom "MV" format, opened by 0x10160a90 (`.\bin\%s.MMV`). |
| Music | `bgm\*.de2`, ogg | none yet | `#BGMTable` in map.txt gives file, extension and loop points; played by 0x1003aef0. |
| Settings | `bin\gurumin.def` | text | `[CONFIG]`, `[KEYCONFIG]`. |
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
