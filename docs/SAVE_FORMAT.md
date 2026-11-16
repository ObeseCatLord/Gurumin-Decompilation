# Save files (SaveData\gdataNN.sav)

Container: see `tools/save_tool.py` (magic 9999, key, scrambled body, body length, sum, xor). Verified on 7 saves.
Loader: `Save_Load` APP.dll 0x1003b2c0 copies the body into these globals (`tools/save_layout.py` regenerates
`docs/save_layout.tsv` from the decompiler output):

| Body offset | Size | Goes to | What |
|---|---|---|---|
| 0x00000 | 0x30000 | (menu only) | 256×256 RGB thumbnail, top-down (`save_tool.py thumb`) |
| 0x30000 | 4 | 0x1030b660 → 0x10c62ed0 | current map id |
| 0x30004 | 12 | 0x108029b8..c0 | player position x, y, z (floats); passed to 0x10162420 |
| 0x30010 | 4 | — | float, not read by the loader |
| 0x30014 | 4 | 0x104bfd18 | |
| 0x3016c | 0x188 | 0x10daaae8 | one 0x188-byte character record (the player's) |
| 0x302f4 | 0x4000 | 0x10c62edc | script global variables (story flags; the script VM's global area, see Script_RegisterGameBuiltins) |
| 0x342f4 | 0x80 | 0x103150c0 | |
| 0x34374 | 4 | 0x104c00dc | |
| 0x3437c | 16 | 0x10202464..70 | four ints |
| 0x3438c | 0x18000 | 0x1049f200 | large progress block (inventory/collection data suspected) |

Not yet mapped: 0x30018–0x3016b and 0x4c38c–end (312,204-byte bodies). The money/HP shown on the thumbnail are
somewhere in these blocks; diffing two saves made a minute apart is the quickest way to pin each field.
