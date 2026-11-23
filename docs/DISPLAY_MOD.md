# Display mod (any resolution, widescreen) for the Japanese PC version

**Parked (2026-11-22), off by default.** The same DLL carries the HD movie support ([MOVIES_HD.md](MOVIES_HD.md));
the display options below only run with `[display] enabled=1` in `gurumin_mod.ini`.

`mod/` builds a `d3d9.dll` that goes next to `game.exe`. The game loads it instead of Windows' own `d3d9.dll`. The
mod changes a few values in `bin\APP.dll` before the game creates its Direct3D device, then passes everything on to
the real `d3d9.dll`. No game file is changed.

What it does:

- **Any resolution.** The game normally offers only a short list (640×480 to 1280×960). Set `width` and `height`
  in `gurumin_mod.ini`, or 0 for the desktop size.
- **Hor+ widescreen.** On a screen wider than 4:3 you see more at the sides. The vertical view stays the same.
- **Centred 4:3 HUD.** Menus, text and the HUD keep their shape in the middle of the screen. Anything the game
  draws across the whole 4:3 width (fades, cutscene letterbox bars) is stretched to the full width. Markers that
  follow 3D objects (names, icons above characters) follow them into the wider view.
- **Window size.** In windowed mode the client area is exactly the chosen resolution.
- **Effect texture.** The texture used for full-screen effects grows with the resolution.

It only works with the known Japanese `APP.dll` (it checks the file's build stamp). With any other build it does
nothing and writes so in its log.

## Install

1. Build it (`mod/build.sh`, needs `pip install ziglang`) or take `d3d9.dll` from a release.
2. Run `mod\install.bat "C:\path\to\Gurumin"`. It copies `d3d9.dll` and `gurumin_mod.ini` into the game folder.
   If a different `d3d9.dll` is already there, it is kept as `d3d9.dll.orig`.
3. Start `game.exe` as usual. The resolution choice in the launcher no longer matters. The Windowモード box still
   picks windowed or fullscreen.

`mod\uninstall.bat "C:\path\to\Gurumin"` removes the files again.

## Settings (`gurumin_mod.ini`)

| Key | Default | Meaning |
|---|---|---|
| `enabled` | 0 | 1 turns the display options on (parked: see Known problems). |
| `width`, `height` | 1280, 720 | Resolution. 0 = desktop size. |
| `hor_plus` | 1 | 1 = wider screens see more; 0 = stretch the 4:3 picture. |
| `hud` | center | `center` = 4:3 HUD in the middle; `stretch` = HUD stretched to the full width. |
| `stretch_fullscreen` | 1 | With `hud=center`: 2D drawn across the whole 4:3 width still covers the screen. |
| `effect_texture` | 0 | Size of the full-screen effect texture; 0 = the next power of two above the resolution. |
| `log` (section `[mod]`) | 1 | Writes `gurumin_mod.log` next to the ini (settings applied, crashes with their address). |

## How it works

Addresses are Ghidra addresses in `APP.dll` (image base 0x10000000). The DLL relocates, so the mod adds the loaded
base. Function names are in `symbols.tsv`.

| Address | What | Change |
|---|---|---|
| 0x10202b24 / 0x10202b28 | Screen width and height, set from `RESOLUTION` in `gurumin.def` | Set to the chosen size before the device is created. |
| 0x10202b34 / 0x10202b38 | 2D canvas size. The game sets it to the viewport size and scales its 640×480 2D coordinates by canvas/640 | Set to H×4/3 × H after each full-screen viewport (`IDirect3DDevice9::SetViewport` hook). |
| 0x10f654ec | Effect texture size (256/512/1024) | Next power of two above the resolution. |
| 6 projection sites (0x1000bc96, 0x10037e5c, 0x1013cfd1, 0x1013d06e, 0x1013d161, 0x1019439f) | `ScaleMatrix(1, 1.333, 1)` after the π/4 perspective: pushes of the floats 1.0, 1.333 | X scale becomes 1.333 / (W/H). |
| 0x101ae060 `Project` | World-to-screen for 2D markers (56 calls in 16 functions) | Calls redirected to a wrapper that widens the x result by W/(H×4/3). |

2D drawing uses `DrawPrimitiveUP` and `DrawIndexedPrimitiveUP` with pre-transformed (XYZRHW) vertices. When the
viewport is the whole back buffer, the mod copies the vertices and shifts them right by the side gap. Draws that
cover the whole 4:3 width are stretched instead. Draws that already cover the whole back buffer (copies of
full-screen render targets, such as the title screen) are left alone.

In windowed mode the game sizes its window as 2×W minus the current client width, which is only right when the
window starts W wide. The mod sets the client size itself just before `CreateDevice`.

## Tested

Windowed 1280×720: title, main menu, name entry, the opening cutscene and the first room. The cutscene letterbox
bars cover the full width. Fullscreen and other resolutions are untested.

Known problems (work parked on 2026-11-22):

- Graphical glitches in the extra side areas. In one scene change, a white line crossed the screen with a white
  block below it, and the picture showed a seam near the right edge. Suspect: the full-screen effect texture
  (2048 with the mod; the game's own maximum is 1024) or effects that copy the screen with 4:3 texture
  coordinates.

## Testing tools

- `mod/tools/run_game.py <game folder> <out prefix> --windowed --shots 25,40 --keys 24:click640x400,35:enter`
  starts the game through its launcher, takes screenshots and presses keys or clicks at the given times, then closes
  the game and prints the log. The title and menus need mouse clicks.
- `mod/tools/capture_window.py game.exe out.png` takes one screenshot of a running game.
