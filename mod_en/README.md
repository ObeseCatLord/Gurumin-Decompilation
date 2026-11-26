# Gurumin Enhanced Rendering (English Steam release)

A rendering, camera and VR mod for the English Steam release of **Gurumin: A Monstrous Adventure** (Steam version
1.4, the `game.exe` this repository calls `game_en_steam.exe`). It is a 32-bit `d3d9.dll` proxy loaded next to
`game.exe`, plus a small x64 OpenXR host (`GuruminVR\gurumin_vr_host.exe`) for VR mode. It changes nothing in the
game's data files.

This folder is the mod's source as of version 1.3.0, by ObeseCatLord under the MIT license (`LICENSE`; third-party
code in `THIRD_PARTY_NOTICES.md`). The original project, its releases and release notes are at
<https://github.com/ObeseCatLord/gurumin-enhanced-rendering>; its
[README](https://github.com/ObeseCatLord/gurumin-enhanced-rendering#readme) lists every setting. For the Japanese
PC release, see `../mod/` instead.

## Features
- Any internal resolution, including 3440x1440 and other ultrawide modes (Hor+ at a constant vertical field of view),
  with a proportional HUD, high frame rates and native VSync.
- Render interpolation for characters, attachments, camera, effects and shadows; gameplay keeps its 30 Hz simulation.
- FXAA / edge anti-aliasing, anisotropic filtering up to 16x, nearest filtering per texture category.
- Optional right-stick free or orbit camera with zoom (stick, keys or mouse wheel).
- Higher-resolution shadows.
- 6DoF VR on PC VR through OpenXR (SteamVR, Monado, WiVRn), life size or diorama scale, single-pass stereo.
- Linux / Proton and Steam Deck (`WINEDLLOVERRIDES="d3d9=n,b" %command%`).
- Steam Frame: runs standalone on the headset, in VR by default, with native Frame controller bindings.

## Build
Needs MinGW-w64 (`i686-w64-mingw32-g++` and `x86_64-w64-mingw32-g++`), and CMake and git for the OpenXR loader:

```sh
./build.sh                 # dist/d3d9.dll, dist/GuruminModern.ini, dist/GuruminVR/{gurumin_vr_host.exe,openxr_loader.dll}
./tests/run_tests.sh       # Python tests, portable C++ tests, and two Windows tests under Wine if installed
```

The first build clones OpenXR-SDK (pinned commit) into `build/openxr-sdk` and builds the Khronos loader
(`tools/build_openxr_loader.sh`; `REBUILD_OPENXR_LOADER=1` rebuilds it). The Wine tests create a D3D9 window, so run
them with a display (`xvfb-run -a ./tests/run_tests.sh` when headless). Tests that need your own game files are
skipped unless `GURUMIN_STOCK_DIR` points at the game folder. CI builds the same files on pushes to master and pull requests (artifact
`mod-en`, see `../.github/workflows/build.yml`). `tools/package.py` makes the release ZIP (it downloads the Windows
Python embeddable runtime).

## Install
Copy the contents of `dist/` (`d3d9.dll`, `GuruminModern.ini`, `GuruminModern-FXAA-LICENSE.txt` and the `GuruminVR`
folder; not the `.o` files) into the game folder next to `game.exe`, or run the checked installer from this folder:
`python3 tools/patch.py --game "/path/to/Gurumin A Monstrous Adventure"` (it only edits executables whose hash it
knows and can restore them with `--action restore`). On Linux / Steam Deck set the Steam launch option
`WINEDLLOVERRIDES="d3d9=n,b" %command%` (the installer can do this). Settings are in the game's launcher
(**Graphics / Audio** and **More settings**) and in `GuruminModern.ini`.

## EN addresses used by the mod and their decomp_en names
The mod addresses the game by RVA from `game.exe`'s base; the EN virtual address is `0x400000 + RVA`. Names come from
`../decomp_en/jp_en_map.tsv` (JP name carried over, with its confidence) and `../decomp_en/obj_symbols.tsv`.

| RVA | EN VA | Mod's hook (detour) | decomp_en name |
|---|---|---|---|
| 0x1a992a | 0x5a992a | return address of the camera view build, checked by the camera code | call site inside `Camera_Refresh` (0x5a90c0) |
| 0x312f90 | 0x712f90 | `cameraUpdateHook` | `Camera_Update` |
| 0x37aea0 | 0x77aea0 | `cameraViewHook` | `LookAt` |
| 0x379200 | 0x779200 | `cameraPitchHook` | `Vec4::RotateX` |
| 0x1d3360 | 0x5d3360 | `cameraWorldHook` | `Scene_DrawFrame` |
| 0x1c9750 | 0x5c9750 | `reflectionMatrixHook` (water reflection) | `Map_BuildTopView` |
| 0x31c5b0 | 0x71c5b0 | `shadowAllocateHook` | `Texture_CreateFromPixels` |
| 0x3a4bf0 | 0x7a4bf0 | `shadowBindHook` | `TextureObject::BeginRenderTo` (medium confidence) |
| 0x38d310 | 0x78d310 | `targetCopyHook` | unnamed |
| 0x390e70 | 0x790e70 | `focusCombineHook` | `TextureObject_DrawBlurred` |
| 0x37fbe0 | 0x77fbe0 | `projectedQuadHook` | `TextureObject::DrawQuad` |
| 0x375320 | 0x775320 | `vrCullHook` | `View_ContainsPoint` |
| 0x350a50 | 0x750a50 | `modelDrawHook` | `Node::QueueDraw` |
| 0x3528b0 | 0x7528b0 | `modelDrawThirdHook` | `Node::QueueDrawFading` |
| 0x351ee0 | 0x751ee0 | `shadowReceiverHook` | `Node::Draw` |
| 0x3a4360 | 0x7a4360 | `skinHook` | `MeshData::UploadBones` (obj_symbols: `SlotTable::UploadBones`) |
| 0x325f40 | 0x725f40 | `particleDrawHook` | `EffectManager::Draw` |
| 0x1defb0 | 0x5defb0 | `fadeHook` | `Fade_Draw` |
| 0x37a4a0 | 0x77a4a0 | `vectorHook` | `Project` |
