# Orbit camera and high-resolution shadows for the Japanese PC version

**Untested on JP hardware. Both features are off by default.** No one has run them on the Japanese game yet: this
port was written without a Japanese `APP.dll` at hand, from the evidence in this repository only. Each address
below comes from the decompiled source, and the mod checks it at run time before it patches anything.

Both features are ported from **Gurumin Enhanced Rendering** (MIT licence, ObeseCatLord), a d3d9 proxy for the
English Steam `game.exe` (`src/camera.hpp`, `src/shadows.hpp`, `src/settings.hpp`). The EN addresses do not carry
over. The EN game was built with VS2013 and the JP `APP.dll` with VS .NET 2002, so code and data sit in different
places. What carries over is what each EN hook does. This port finds the JP function or global that does the same job
in the decompilation and hooks it at a call whose bytes the decompilation shows.

The code is in `mod/src/camera.c`/`.h`, `mod/src/camera_math.h` and `mod/src/shadows.c`/`.h`, inside the same `d3d9.dll` as
the file replacement, HD movie and display options ([DISPLAY_MOD.md](DISPLAY_MOD.md)).

## What they do

**Orbit camera (`[camera] enabled=1`).**
- The right stick of any XInput pad turns the camera around Gurumin, or the mouse while a chosen button is held.
- Stick up and down tilts the camera up to 80 degrees above or below the target.
- PageUp/PageDown, the mouse wheel and optionally the stick zoom from 60 % to 250 % of the game's distance. The
  distance is saved in `gurumin_mod.ini`.
- A tilted or more distant camera stops in front of walls.
- It works only while the game's own free follow camera runs. Events, scripted cameras, path cameras, demos and
  queued scripts keep the game's camera.

**High-resolution shadows (`[shadows] enabled=1`, `size=2048`).** The game draws each of up to ten nearby
characters from its light into a 256x256 texture and lays that texture on the ground as its shadow. The mod makes these
textures `size` squared: 512, 1024 or 2048, limited by the graphics card. That gives sharper character shadows.
If anything needed is missing, the shadows stay at 256: an unexpected call site, too little memory or too small a
depth buffer.

## How it hooks (no entry detours)

Entry detours need the first bytes of the hooked function. Every function this port would detour is only partly
matched in the decompilation (Camera_Update 34 %, Texture_CreateFromPixels 16 %, BeginRenderTo 77 %), so those bytes are
not known. The mod therefore redirects `call rel32` (E8) instructions inside functions that **are** matched byte for
byte, and wraps the callee:

1. It scans the matched caller for E8 instructions whose target is exactly the callee's address.
2. It patches only if the number found equals the number of calls in the decompiled source. It finds every call
   site of a feature before it patches any.
3. It writes only the 4-byte call offset at each site and logs every site.

The same technique already redirects the 56 `Project` calls of the display mod. On top of this, the DLL does nothing
unless `APP.dll` has the known build stamp: timestamp 0x44604392, image size 0x1b5c000.

The matched build compares code with relocations masked. A call's target is a relocation, so the match alone does not
prove the target. The mod therefore checks the target itself at run time: an E8 counts only when it lands exactly
on the expected callee. The repository has no committed object files or listings (`git ls-files` shows no
`.obj`/`.asm`/`.lst`), so the mod cannot compare whole byte sequences.

## JP address table

Addresses are Ghidra addresses in `APP.dll` (image base 0x10000000). The DLL is relocated at run time, so the mod
adds the actual base. "Site" is the confidence that the address is right. "Behaviour" is the confidence that the
change does what is intended; a behaviour that depends on draft (unmatched) source is graded lower.

### Camera

| JP address | What | Evidence (JP) | EN counterpart (`decomp_en/jp_en_map.tsv`) | Site | Behaviour |
|---|---|---|---|---|---|
| 0x10015960, 2968 bytes | `Camera_Refresh`: the one call to `Camera_Update` is redirected (exactly 1 expected) | `decomp/src/camera_refresh.cpp`, matched 100 % (`progress/jp/report.json`); `?Camera_Refresh@@YAHXZ` in `decomp/obj_symbols.tsv` | (caller of EN 0x712f90) | high | high |
| 0x1016c3e0 | `Camera_Update(Vec4 *eye, Vec4 *target)`, cdecl (`YA`), wrapped | `decomp/src/camera_update.cpp` (draft, 34 %); `?Camera_Update@@YAHPAUVec4@@0@Z` | EN 0x712f90, `Camera_Update`, 0.69 high (EN's `nativeCamera` hook) | high | medium |
| 0x11af3e28 / 0x118e6c78 | camera position and target (Vec4) | `camera_vars.h`; `camera_orbit.cpp`; the game's own wheel and key turn in `game_frame.cpp` rotates 0x11af3e28 about 0x118e6c78 | EN 0x1d7b5b0 / 0x1ae4900 (EN's `orbitYaw` on the non-authored camera) | high | medium |
| 0x1101de30 | set to 2 = turned by hand this frame (skips the wall peek), as the game's wheel/key turn does | `game_frame.cpp` (wheel and camera keys), `camera_update.cpp` | none (EN state differs) | high | medium |
| 0x1101ddf4, 0x1101de24, 0x10ff1df0, 0x1101ddf0, 0x1101d890 | free follow camera test: the negation of `FUN_1016c300` | `decomp/src/camera.cpp` FUN_1016c300 (the test `game_frame.cpp` makes before its own camera turns); same order as the `Camera_Update` branches | EN `orbitCameraOwned` / `fixedGameplayOrbit` (different globals) | high | medium |
| 0x1032d69c | 0 while a queued script runs: no input then | `char_vars.h`; gates the game's camera keys in `game_frame.cpp` | (EN `manualBookActive`, HUD checks) | high | medium |
| 0x1065cb90 | non-zero while a demo records or plays: no input then | `game_main.cpp` / `map_setup_entry.cpp` set it with 0x1065cb94/98 | none | high | medium |
| 0x104c010f | `char`, config.ini `WROT`: the game turns the camera with the wheel | `game_main.cpp` `READ_CONFIG("WROT")`; `game_main.h`; gates the wheel turn in `game_frame.cpp` | EN reads `WROT` from `%APPDATA%\FALCOM\GURUMIN\config.ini` | high | high |
| 0x101aaa20 | `Node::CastSegment(name, mask, from, to, hit, t)`, thiscall | `node.h`; `?CastSegment@Node@@QAEHPBDIPAUVec4@@11PAM@Z`; called with this exact shape by the **matched** `Camera_PullToItemStand` 0x10018870 (`camera_item_stand_pull.cpp`) | EN 0x761d30 (EN `cameraSegment`, 6 arguments); the map row pairs it with `Node::RayCast` 0x101a9670 (7 arguments, medium), which does not fit, so it is not used | high | medium (CastSegment itself is 21 % matched) |
| 0x10255f08 | `Node *`, the map's root node: world collision | `map_vars.h`; the collision `Camera_PullToItemStand` casts against | EN `actor + 0x231c` geometry | high | medium |

Turn direction: the game's "turns the camera right" key (DAT_1065cb42, `game_frame.h`) turns the position by +0.08
through `Project(v, RotationZ(a))`. With the layout of `Matrix4::SetRotationZ` (`vec_math.cpp`: m[1] = -sin,
m[4] = sin) and `Project`'s row-vector product (`vec4.cpp`), a positive angle turns clockwise. The mod's
`camera_orbit_yaw` turns counter-clockwise, so the stick value is negated: stick right should turn like that key.
The EN mod reaches the same result with its `invertX = true` default. One caveat: `RotationZ` 0x101b1b40 itself is
not decompiled, and this assumes it uses `SetRotationZ`'s layout. `invert_x=1` reverses the turn.

The wall test copies `Camera_PullToItemStand`. It casts from 25 units beyond the eye to the target against "CA"
(mask 4), then "CS" (mask 0). On a hit it stops 25 units short, or at 3/4 of the distance when the hit is nearer
than 50. It runs only when the camera is tilted or farther than the game's own distance; the game's own follow
camera already avoids walls at its own distance.

### Shadows

| JP address | What | Evidence (JP) | EN counterpart | Site | Behaviour |
|---|---|---|---|---|---|
| 0x1017c160, 1037 bytes | `ModelBank::CreateTextures`: its 10 calls to `Texture_CreateFromPixels` are redirected (exactly 10 expected: one in the shadow loop, 3 crossfade, 6 mirror/save/minimap); only `shadow0`-`shadow9` at 256x256, alpha 1, flags 0 change | `decomp/src/model_bank.cpp`, matched 100 %; `?CreateTextures@ModelBank@@QAEHXZ` | EN 0x71c5b0 allocator, called from EN RVA 0x334403 | high | high |
| 0x10175c90 | `Texture_CreateFromPixels(name, w, h, pixels, alpha, flags)`, cdecl: returns the existing slot for a loaded name, -1 on failure (no name registered) | `decomp/src/texture_create.cpp` (16 %, draft) | EN 0x71c5b0, 0.90 high | high | medium |
| 0x10169c60 | `TextureObject::Create`: rejects sides over 0x800; alpha 1 = generated alpha, render target, size kept in `sourceWidth/Height` (+0x2c/+0x30), rebuilt after a device reset | `decomp/src/mesh_cache.cpp`, `texture_object.h` | - | (not called) | medium |
| 0x1017c160 (same function) | its 1 call to `TextureObject::UpdateImage` (the shadow clear loop) is redirected | `model_bank.cpp` | - | high | medium |
| 0x101a24e0, 1267 bytes | `Node::RenderShadowTexture`: its 1 call to `UpdateImage` is redirected | `decomp/src/shadow_texture.cpp`, matched 100 %; `?RenderShadowTexture@Node@@...` | (EN hooks the binder instead) | high | medium |
| 0x10169f70 | `TextureObject::UpdateImage(value)`, thiscall: `BeginRenderTo(sourceWidth, sourceHeight, value)` | `decomp/src/mesh_cache.cpp`, matched 100 % | - | high | high |
| 0x10143870 | `TextureObject::BeginRenderTo`: clips the viewport to 0x10202b24/28, binds depth 0x10f08d38 (not hooked; the wrapper raises the two globals around the call) | `decomp/src/texture_bind.cpp` (77 %, draft) | EN 0x7a4bf0 binder, 0.59 medium (EN's `shadowBindHook` raises its canvas globals the same way) | (not hooked) | medium |
| 0x10202b24 / 0x10202b28 | back-buffer width/height (int) | `render_vars.h`; the display mod writes the same globals | EN 0x508b0c/10 (canvas) | high | medium |
| 0x118d0ba8 | `TextureObject DAT_118d0ba8[1600]`, 0x38 bytes each | `texture_object.h` (layout checked by `_Static_assert` in `shadows.c`) | EN 0x198f6a0 | high | high |
| 0x10f08d38 | `IDirect3DSurface9 *` depth buffer for off-screen targets, created `DAT_10202b18` squared; recognised in `CreateDepthStencilSurface` by its output pointer | `render_vars.h`; `render_on_reset.cpp`; `render_reset.cpp` | EN identifies its call by return RVA 0x3a4158 | high | medium |

Texture memory: ten 2048x2048 R5G6B5 targets take about 80 MB, plus a 2048x2048 D16 depth buffer. The game's own
budget counter DAT_10f0ff80 (`render_on_reset.cpp`, `buffer_pool.cpp`) counts the depth buffer at its native
`DAT_10202b18` size. Render-target textures are not counted (`TextureObject::Upload`), so the game does not shrink
anything else to make room. On a card without that much spare memory, creating a target fails and that shadow stays
at 256.

The EN mod also turns the dynamic-shadow gate off when a shadow target fails. In JP that gate is DAT_10203434, and
`RenderShadowTexture` already clears it when `UpdateImage` returns 0. So if the depth buffer is ever re-created smaller
than a large shadow target (after a device reset), the wrapper returns 0 and the game falls back to its round
`kage.bmp` shadows by itself. The mod never writes the gate.

No texel or projection change is needed. `RenderShadowTexture` and `Node::DrawShadow` build the shadow camera in clip
space and move it into 0..1 texture space with a translation of 0.5. Nothing in that path uses the pixel size: there
is no 1/256 constant. The EN mod's extra work (saved canvas sizes, viewport checks) fixes up its own canvas
bookkeeping. JP's `BeginRenderTo` records the target size it was given, as it does at 256.

## Left out, and why

- **EN input hook** (EN 0x7ae330, which reads the game's XInput state and zeroes the right-stick axes). The map pairs
  it with `sound3d.dll` at low confidence, and the JP game reads input through DirectInput. So the mod reads XInput
  itself and **cannot stop the game from also seeing the right stick**. If your pad's right stick is bound to
  something in the game's key configuration, it does both.
- **EN pitch hook** (EN 0x779200, `Vec4::RotateX`, which changes the pitch only for five return addresses inside
  EN's camera function). The JP return addresses would lie inside `Camera_Update`, which is unmatched. In JP, the
  pitch is applied to the eye that `Camera_Update` returns instead.
- **EN view hook** (EN 0x77aea0, `LookAt`, which swaps the eye and target passed by value). JP `LookAt` 0x101b1c20
  is unmatched. The JP wrapper changes the returned eye before `Camera_Refresh` builds the view from it, which needs
  no hook there.
- **EN world hook** (EN 0x5d3360, scene draw, which republishes eye/target for map cell culling while zoomed).
  `Camera_Refresh` publishes the eye it got from `Camera_Update` (DAT_10f65688), so the drawn eye is already the
  published one. Whether map cells are picked from another copy has not been checked; a far zoom may show missing map
  pieces at the edges.
- **EN movement hook** (EN 0x72cb60, camera-relative movement for the authored camera). The map has no JP pair for it.
  It is not needed for the turn, because the turn moves the game's own camera position and the game's
  `Camera_TurnInput` 0x1016b400 reads that. The tilt and zoom are drawing-only and do not change the heading.
- **Orbiting the authored (fixed) cameras.** EN can also orbit cameras placed by the map. JP has no evidenced place
  for that, so the orbit works only with the free follow camera.
- **Town-only exclusion, interior detection, HUD-based gating.** These use EN scene numbers and map names with no
  evidenced JP addresses. JP uses the game's own camera gates instead.
- **VR.** Out of scope (dozens of mid-function hooks).

## How a JP owner can test it

1. Build (`sh mod/build.sh`, needs `pip install ziglang`) and install (`mod\install.bat "C:\path\to\Gurumin"`).
2. In `gurumin_mod.ini` set `[camera] enabled=1` and/or `[shadows] enabled=1`, keep `[mod] log=1`.
3. Start the game. In `gurumin_mod.log` (next to the ini) look for:
   - `APP.dll at ...` (the build stamp matched; otherwise nothing is patched);
   - `camera: Camera_Refresh 10015960 (2968 bytes) has 1 call(s) to Camera_Update 1016c3e0 (expected 1)`, then
     `camera: call at 1001xxxx redirected`;
   - `shadows: ... 10 call(s) to Texture_CreateFromPixels ... 1 to TextureObject::UpdateImage ...`,
     `Node::RenderShadowTexture ...: 1 call(s)`, `create call at ... redirected` (10 lines);
   - after the device is created: `shadows: device allows ...`, `CreateDepthStencilSurface (vtable 29) hooked`,
     `off-screen depth buffer NxN -> 2048x2048: 00000000`;
   - after starting or loading a game: `shadows: shadow0 slot S is 2048x2048` (ten lines) and
     `first 2048 target drawn with the full viewport`;
   - while playing: `camera: pitch ..., distance ... %` every 1800 frames.
      `camera: free follow test ...` lists the globals the camera reads.
   Any `expected` count that differs, or `kept at 256` / `could not be made`, means that part stayed off.
4. Camera: in a field, move the right stick. The camera should turn around Gurumin and keep turning smoothly. Walking
   should still follow the stick relative to the new view. Tilt up and down, zoom with PageUp/PageDown. Check that
   events and cutscenes keep their own camera and that the camera does not pass through walls when tilted or zoomed
   out. Tilt the camera fully down and check that it does not go under the ground. It is not known whether the "CA"
   and "CS" shapes include floors. If stick right turns the wrong way, set `invert_x=1`.
5. Shadows: stand near a light so the character casts a dynamic (shaped, not round) shadow, and compare with
   `enabled=0`. The edge should be sharper. Watch for a shadow cut off on one side (viewport), a black or missing
   shadow (depth buffer) or a fall back to round shadows. Try a resolution change or Alt+Tab in fullscreen (device
   reset).
6. Report the log and what you saw (an issue on the repository).
