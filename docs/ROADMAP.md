# Roadmap

The decompilation comes first: matching and naming APP.dll's functions is what makes the features below cheap to
build and safe to maintain. Everything under "Future goals" waits until the decompilation and naming are far enough
along; the notes record what is already known so the work can start from there.

## Now: decompilation and naming
- Match APP.dll function by function with the original compiler (VS .NET 2002, cl 13.00.9466, `/O2 /Ob1 /Op /Gy /GS /GX`).
- Give every function, global and structure field a readable name (`symbols.tsv`, `decomp/obj_symbols.tsv`, struct
  fields in `decomp/include/`).
- Keep `decomp/open.tsv` (functions that still differ, with the reason) and `decomp/QUEUE.md` (the work queue) current.
- Progress is published on every push: `progress/jp/report.json` (ByteMatch) and `progress/jp/progress.svg` (README).

## Future goals

### Rendering and modern hardware
The PC release targets 2006 hardware; modern machines can lift its limits.
- Higher framerates, wide and ultrawide resolutions, proportional HUD, FXAA, higher-resolution shadows. The MIT-licensed
  **Gurumin Enhanced Rendering** mod (ObeseCatLord) does this for the Steam EN build through a d3d9 proxy, with render
  interpolation over the game's 30 Hz simulation, FXAA, a free/orbit camera and OpenXR VR. Its source is now bundled
  in `mod_en/` (v1.3.0, MIT; CI builds it). Next: port its features onto the decompiled JP code, and expose its
  settings in our configurator. Keep its licence and credit.
  Started: its orbit camera and higher-resolution shadows are ported to the JP mod DLL, untested on the JP game
  ([CAMERA_SHADOWS_JP.md](CAMERA_SHADOWS_JP.md)).
- PSP-exclusive costumes: the Enhanced Rendering mod links the separate PSP Outfits mods (dinput8 proxy). Support
  loading them when the user supplies the files (never ship them).

### Costume transmog
Wear one costume's look while keeping another costume's abilities (e.g. Witch outfit look with the default outfit's
abilities, or with the Samurai outfit's abilities).
- Known so far (docs/MODDING_MAP.md "Costumes"): the costume id is the script global **0x10c655f0** (saved in the
  game save), unlock bits are **0x104c0230** (record 0x80 of `system.sav`), and `Costume_LoadPlayerModel`
  0x1003b6e0 turns the id into a model file name (`<base><N>.it3`, 11/12 use `a1`/`a2`).
- Plan: add an *appearance id* next to the costume id. Model loading (0x1003b6e0 and anything else that picks
  textures, voices or effects by costume) reads the appearance id; ability code keeps reading 0x10c655f0. To do:
  list every reader of 0x10c655f0 and sort it into "look" or "ability". Readers found so far: the scene start
  (0x100cc1d0, decomp/src/scene_start.cpp) sets world byte 0x10c68974 for costume 6; the face animation
  (0x1000f360, decomp/src/face_anim.cpp) loads the player's face textures with "14" in the name for costume 6.
- Only costumes the player has unlocked and can pick in the wardrobe may be chosen as an appearance: apply the same
  unlock-bit checks as 0x1003b6e0 (bit table in MODDING_MAP), plus the PSP outfits when their files are present.
- Expose it as a configurator option and, later, an in-game wardrobe option. Store the appearance id in the mod's
  own settings so the original save format stays unchanged.

### Text, fonts and languages
- **Unicode subtitle font.** The game draws text from a pre-rendered Shift-JIS bitmap font (`3ddata\ddfont8.dat`,
  17-pixel glyphs of 3-bit coverage; renderer near 0x10145380, font load 0x101455d0). `tools/ddfont_tool.py` already
  round-trips it to a PNG atlas. Goal: replace it with any TrueType/OpenType font at any size, rendered at the output
  resolution, with full Unicode text (UTF-8 script and subtitle files).
- **In-game language selection** for voice, text and subtitles, each chosen separately from an in-game menu. Needs:
  the text paths (script `map.txt`, menus, item and ability strings), the voice paths (soundarc.dll commands), and the
  subtitle data, each routed through a language setting.
- `mmv` movie editing for translations: `tools/mmv_tool.py` decodes frames today; add re-encoding so translated
  frames can be packed back.

### Tools
- **All-in-one tool (exe with a GUI)** that wraps the Python tools (archives, fonts, movies, items, saves, scripts,
  models) for user-friendly extraction, editing and repacking.
- Model viewer in the same tool: preview game models, import/load OBJ models and replace game models.
- **Texture glossary:** a list of every texture and what uses it (HUD icon, menu art, mesh texture, and for meshes
  the object type: tree, rock, character, ...).
- **`textures` override folder:** load custom textures from a loose folder at run time, without repacking the game
  archives (like the Typing of the Dead mod's texture packs).

### Configurator
- One configurator for all options above (rendering, transmog, fonts, languages, texture overrides).
