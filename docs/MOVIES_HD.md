# HD movies

The game's `.mmv` movies are small: 480×120 (the map backgrounds), 320×240, 400×320 and so on. The game scales them
up on screen, so at 1280×960 every movie pixel becomes a 2×2 or larger block. An HD movie is the same movie made
at 2× or 4× the width and height. With the mod DLL the game shows it at the original size on screen, with the
extra detail.

## Making one

Make the frames at exactly 2× or 4× the original movie's width and height. For example, `logo.mmv` is 256×160, so
its HD ×2 frames are 512×320. Then encode them with the original movie as the template:

    python tools/mmv_tool.py encode my_frames\ out\3ddata\logo.mmv --template GAME\3ddata\logo.mmv

The encoder compares the frame size with the template. At the original size it writes a normal movie. At exactly 2×
or 4× it writes an HD movie. `--scale 2` or `--scale 4` sets it without a template. `mmv_tool.py info` shows
whether a movie is HD.

In the GUI (`tools/gurumin_tools_gui.py`) nothing changes. Put the larger frames into the `.mmv.frames` folder (or
use a larger GIF) and repackage. When a repackage writes an HD movie, the GUI also copies `d3d9.dll` and
`gurumin_mod.ini` into the mod's `out\` folder, because the game needs them to show the movie at the right size. The
DLL has to be built first (`mod/build.sh`).

Limits:

- 2× and 4× only. The movie's texture is 1024×512 at the original scale, so a movie must stay within 1024×512 at
  its original size (all of the game's movies do).
- Larger movies take more memory and disk: the game loads each movie whole.

## Installing

Copy `d3d9.dll` and `gurumin_mod.ini` next to `game.exe`, or run `mod\install.bat "C:\path\to\Gurumin"`. Without
the DLL the game plays HD movies too: it draws them 2× or 4× too large and cuts off what does not fit into its
movie texture.

`[movies] hd=0` in `gurumin_mod.ini` turns the support off.

## How it works

An HD movie is a normal `.mmv`. Its palette's unused fourth bytes of entries 0, 1 and 2 hold `H`, `D` and the
scale. The game ignores those bytes, and no original movie uses them.

The game (addresses in `APP.dll`; source in `decomp/src/mmv_texreg.cpp`):

- `MmvPlayer::Open` (0x10160a90) loads the whole movie and takes its size from the header (player +0x40, +0x42).
- `MmvPlayer::M_10160960` copies the decoded frame into the texture slot `videotex`, which the game creates
  1024×512. The copy is clipped to the slot's width and height (+0x08, +0x0c).
- The drawing code takes the movie size from the player for the size on screen. It divides that size by 1024 and
  512 for the texture coordinates.

The DLL (`mod/src/movies.c`) hooks `Open`, the upload, `Close` and the DirectShow upload (0x1015c6a0). For an HD
movie:

1. After `Open`, the player reports the original size (header size divided by the scale). Every screen keeps its
   layout, and scripts get the original size too (script built-in 0x5dc).
2. While the movie plays, the `videotex` slot holds a texture `scale` times larger (2048×1024 for 2×). The game's
   texture coordinates (original size / 1024) then cover the whole HD frame.
3. The upload runs with the real HD size.

Normal movies and the DirectShow video (`3ddata\op.avi`) get the original texture back before their upload.
