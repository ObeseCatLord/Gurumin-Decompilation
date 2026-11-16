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
| Models / animation | `3ddata\*.it3` | `tools/it3_tool.py info/obj`, `tools/it3_chunks.py` | Flat chunk list `tag[4] u32 size data`; each `INFO` (name[64], 4x4 matrix, 3 words) starts a node. Mesh chunk VPA2: packs of 0x70-byte vertices + u32 triangle indices, both compressed with packs.dll `ExecUnpack` (`tools/gpack.py`, Python port). All 945 JP files decode (2.4M vertices). Vertex (28 floats, from the initialiser 0x101acf10): 0-3 position, 4-6 normal, 8-11 RGBA, 12-13 UV, 24-27 blend weights; OBJ export writes v/vn/vt (UVs verified against the texture; V measured from the top) plus an MTL naming each node's MAT4 texture, and places meshes through the node hierarchy (CHID = parent, count, child names; row-vector matrices; Z-up). Skinning/animation not applied. Parser 0x101a58a0. |
| Movies | `*.mmv` | `tools/mmv_tool.py info/frames/gif` | Custom `MV` format, opened by 0x10160a90 (`.\bin\%s.MMV`), frames decoded by packs.dll `MMV3` and the hand-written RLE in 0x10160a20: 8-bit palettised frames (palette at player+0x4e) stored as deltas - `0xFC n` skips n pixels (keeps the previous frame), `0xFD n v` writes n copies of v, any other byte is a literal pixel. |
| Music | `bgm\*.de2` | `tools/de2_header.py` (header check; all 82 JP/EN files verified) | `#BGMTable` in map.txt gives file, extension and loop points; `Bgm_Play` 0x1003aef0 hands the file to soundarc.dll (decoding lives there) and separately checks the 16-byte header (four u32; the 4th xor the 2nd is a length) through 0x10001cb0 (sum and xor of a key built from the upper-cased file name - `de2_header.py expect <name>` prints them). **A missing or failing file feeds the anti-tamper table → the game closes itself**, so replacement music must keep a valid header. |
| Settings | `bin\gurumin.def` | text | `[CONFIG]`, `[KEYCONFIG]`; read once in `Game_Startup` 0x1013b700 via `Config_GetInt` 0x1015fe70. Key → global map: `docs/config_keys.tsv` (e.g. FULLSCREEN 0x10f65514, TEXD 0x1101d844 = skip the JPEG decoder, VSYNC 0x10f654a8). |
| Saves | `SaveData\gdataNN.sav` (NN = page×7 + slot) | `tools/save_tool.py` (7 real saves decode and re-encode byte-identically) | magic 9999 + key, body = game state (312,204 bytes) scrambled with the .itm cipher keyed by the file name shifted by the key byte, then body length + sum/xor. `system.sav` is 16 KB unscrambled flags. Reader 0x10011240, slot info 0x10010d10, writer 0x1005cdc0 → 0x1003b2c0. |

## Script system
- Scripts are C-like (`int main() { ... }`), compiled when the map is loaded (compiler 0x100ed9b0 / 0x10089a10).
- 118 built-ins: SCRIPT_API.md (with name glosses). `tools/builtin_cases.py` dumps each built-in's handler code from the
  decompiler output to re/export/builtins.txt (local only) to check what a call does. Game built-ins go through one switch (0x10138280), so a new built-in can be added by
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
  overwrites a random entry of the 16-int table at 0x104c0270 with `rand()`. **Consequence:** the main loop in
  Game_Startup posts WM_CLOSE whenever any entry is non-zero (checked on about 12 of every 570 frames), and
  Screenshot_Capture sets the quit flag - the game closes itself a few seconds later. A failed system.sav check feeds
  the same table. Patch 0x1000eb90 to return early (and zero the table) before debugging or editing system.sav.

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

## Frame loop and device loss (modern-hardware fixes)
- `Frame_PumpAndRecover` 0x1015c850 runs once per frame: Windows message pump (WM_QUIT sets 0x10f6552c), then spins while
  the window is inactive (0x10f65530 / 0x10f656a0 both 0) or `TestCooperativeLevel` returns D3DERR_DEVICELOST.
  On D3DERR_DEVICENOTRESET it calls 0x1014b2a0, `Texture_OnLostDevice` 0x101734d0, `IDirect3DDevice9::Reset` with the
  present parameters at **0x10f0ff84**, 0x1013fbb0, `Texture_OnResetDevice` 0x10173510, then `Render_BeginScene`.
- `Render_Present` 0x1015c960 ends the frame (EndScene + Present). Frame pacing: `timeGetTime()+16` deadlines at 0x10f654d8.
- Present parameters at 0x10f0ff84 are the place to change resolution, windowed mode, refresh rate or vsync before
  `Reset`/`CreateDevice`.

## Resolution and device creation
- `gurumin.def` `RESOLUTION` is read in `Game_Startup` (switch at 0x1013bc96): 0-2 = 640×480, 3-4 = 800×600,
  5-6 = 1280×960; the second number per pair sets 0x10f654ec (256/512/1024, a texture-size budget). Width/height land
  in **0x10202b24 / 0x10202b28** (copied to 0x10202b2c/30). Only 4:3 modes exist; widescreen needs these values
  patched or set before device creation, plus projection changes.
- `Render_CreateDevice` 0x10148b00: `Direct3DCreate9(32)`, adapter mode check, sizes the window so the client area is
  width×height, fills D3DPRESENT_PARAMETERS at 0x10f0ff84 (BackBufferWidth/Height = resolution, Windowed =
  !FULLSCREEN, SwapEffect = COPY, EnableAutoDepthStencil with D16, PresentationInterval = ONE, or IMMEDIATE when
  `VSYNC` is set), then `CreateDevice` (HAL; hardware vertex processing when `HDVS`, falling back to software).
- A "Trident Video Accelerator Cyber-XP4" adapter string disables HDVS (old driver workaround).

## Steam English game.exe
The Steam release (`game.exe`, VS2013, 2015) contains the same game code as APP.dll in one executable (image base
0x400000, not relocated). `tools/xref_en.py` maps JP functions to EN functions by the strings they reference:
`docs/symbols_en.tsv` (named functions; e.g. MapData_Load 0x607110, Script_RegisterGameBuiltins 0x644db0,
Game_Startup 0x609f70, Texture_LoadIti 0x736f60, It3_ParseChunks 0x755ef0) and `docs/jp_en_map.tsv` (every JP
function with strings: 121 of 420 mapped, 78 with ≥3 shared strings). Low-confidence rows share one string only. `tools/xref_propagate.py` grows the map through the call graph
(`docs/jp_en_map_full.tsv`: 129 pairs, 33 of them call-graph inferred - check those before relying on them; the EN
build inlines more, e.g. Texture_ReleaseAll → 0x740fe0 also does VRAM accounting).
Ghidra export: `re/run_ghidra_en.bat`.
- **Seasonal easter egg:** 0x1001d850 reads the PC clock and returns 1 on Christmas (Dec 25, or Dec 24 after 18:00) and 2 on January 1-3; hook it to force the seasonal content.

## Costumes (outfits)
`Costume_LoadPlayerModel` 0x1003b6e0 picks Parin's model:
- costume id: **0x10c655f0** (inside the script-globals block, so it is saved: body offset 0x302f4 + 0x2714 = 0x32a08).
- unlock bits: **0x104c0230**. Before loading, a costume whose bit is clear is reset to 0:
  1→bit 0x1, 2→0x2, 3→0x4, 4→0x8, 5→0x10, 6 needs 0x17 (all of 1,2,3,5), 7 only when `SeasonalEvent()` (0x1001d850)
  returns 2 (Jan 1-3), 8→0x40, 9→0x80, 10→0x20, 11→0x100, 12→0x200.
- model file: costume 0 = base name (with 0x10c66cc0 set: `<base>a.it3`), 11 = `<base>a1.it3`, 12 = `<base>a2.it3`,
  others `<base><N>.it3` (base name from 0x10daac4d, e.g. char02b).
- The unlock bits live in **record 0x80 of `system.sav`** (`SystemSav_ReadRecord` 0x101603c0 / `SystemSav_WriteRecord`
  0x1015fea0, read in Game_Startup). `system.sav` (16 KB) is bound to the PC: it is XOR-scrambled with a key built
  from the inverted computer name and the file's timestamps and carries a checksum at 0xff8. If the check fails the
  game zeroes the unlock bits, deletes and rewrites the file and drops a random value into the anti-tamper table
  0x104c0270. A save copied from another PC therefore loses its unlocks. Easiest unlock route for mods: set
  0x104c0230 in memory (or patch the check) rather than editing system.sav.
Adding a costume therefore needs: a model `<base><N>.it3` (+ its .iti textures), an id the checks above let through
(or a patched check), and a way to select it (the closet scripts in map.txt).
- Render state goes through three cached setters: `Render_SetRenderState` 0x1013ef30, `Render_SetTextureStageState`
  0x1013ef90 and `Render_SetSamplerState` 0x1013f000 (each skips the call when the cached value matches). Hooking the sampler
  setter is the simplest way to force anisotropic filtering or clamp/mip settings.
