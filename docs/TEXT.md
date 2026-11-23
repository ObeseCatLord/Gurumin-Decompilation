# The game's text (dialogue, subtitles, names)

All of the game's text is plain Shift-JIS inside `.itm` files, drawn at run time with the pre-rendered font
`3ddata\ddfont8.dat` (the PNG atlas of `ddfont_tool.py` is only the font). `tools/dialogue_tool.py` and the GUI's
**Dialogue** tab list and edit it; nothing is drawn into images.

## Where it is
| Text | File | Form | Count |
|---|---|---|---|
| Dialogue box with speaker name and face | `bin\map.itm` | `Message(speaker, "line 1", "line 2", "line 3", "line 4");` | 3,123 |
| Short message (items found, hints) | `bin\map.itm` | `Mes("text");` | 140 |
| Menu choice | `bin\map.itm` | `SelText("text", value);` | 441 |
| Stage names | `bin\map.itm` | `#StageName` rows, second column | 41 |
| Item names and descriptions | `bin\map.itm` | `#ItemTable` rows, second and last column | 95 + 95 |
| Character names (status screen) | `bin\map.itm` | `#StatusTable` rows, second column | 58 |
| Cutscene subtitles | `3ddata\*.itm` (58 scene scripts) | `##frame,ZIM,text,seconds,-1,x,y` | 944 |

The `Message` speaker is the row number of `#CharTable` (0 = Parin, 20 = Hyperbolic, ...). Numbers 58 to 65 are
used (232 times for 58) but have no row there; their names are not known yet. In subtitles, `\n` (backslash, n)
starts the second line. Menu pictures (title screen, buttons) are textures, not text.

## Text rules
Dialogue and subtitles go through the text formatter at APP.dll 0x1014f590 (seen in game for both):
- `0`-`9`, `-` and `.` are drawn with their full-width glyphs; `_` is a 1-pixel gap.
- `L` + 2 digits and `s` + 3 digits are control codes (`s080` moves the text to x = 80 x 1.136; the game uses it to
  line up text such as `s080怪盗Ｍ」`).
- Everything else is read two bytes at a time as Shift-JIS. Half-width letters would turn into wrong characters, so
  the tools store their full-width forms (`A` becomes `Ａ`); `'` and `"` become `’` and `”` (the full-width `＇` and
  `＂` are outside the font).
- The font has no glyph for the full-width space (0x8140): the game draws it 1 pixel wide. The tools store a typed
  space as `_____` (5 pixels).
- Each glyph advances by its width + 1 (Font_GlyphWidth 0x10145380). The longest dialogue line in the game is 429
  pixels and the longest subtitle line 407; the GUI's preview marks that width with a red line.

Names in tables keep a full-width space for a space, because table columns are separated by spaces.

## Integrity check
Game_Startup (0x1013b700) checksums `bin\app.dll`, `bin\app8.dll`, `bin\map.itm`, `bin\sound3d.dll`,
`3ddata\test.dat` and `game.exe` with File_Checksum (0x101602d0: sum and xor of the file's dwords, xor seeded with
0xcc82) and compares each with a pair stored in `bin\soundarc.dll` at 0x17000 (FUN_10160360). A mismatch feeds the
anti-tamper table and the game closes itself about 9 seconds after start. An edited `map.itm` copied over the game's
file therefore does not work. The mod DLL's replacement loader gives File_Checksum the game's own file and the
loader of the game the replacement (docs/MODDING_FILES.md), so an edited `map.itm` must be installed through the mod
folder.

The trailer of an `.itm` file starts with a content type (0 = BMP, 1 = text, 3 = WAV, docs/ITM_FORMAT.md); a text
file stored with 0 is refused too.

## Tested in game
With the mod DLL and an edited `map.itm` in `mod\`: the first line of every `Message` set to `Hello, Parin! Test
123 - ok?` showed in the dialogue boxes of the first scene at home, spaces as gaps. With an edited
`3ddata\open1_1.itm`: English subtitles (`Parin: So this is the house.`, a two-line subtitle) showed in the opening
cutscene.

## Command line
```
python tools/dialogue_tool.py export "C:\Games\Gurumin" text.tsv
python tools/dialogue_tool.py import "C:\Games\Gurumin" text.tsv my_translation
python tools/mod_install.py install my_translation "C:\Games\Gurumin" my_translation
```
The TSV (UTF-8) has one row per entry: file, id, script, kind, speaker, speaker name, text1 to text4. Change only the
text columns. `import` writes only the files whose text changed, in the game's layout (`bin\`, `3ddata\`).
