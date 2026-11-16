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
| 0x3016c | 0x188 | 0x10daaae8 | player record: +0x04 HP, +0x08 max HP, +0xb8 money (Cash) - all three checked against two save thumbnails; +0x1c, +0xcc, +0xd0 also vary between saves (+0x1c 7..99, possibly drill energy) |
| 0x302f4 | 0x4000 | 0x10c62edc | script global variables (story flags; the script VM's global area). Costume id at +0x2714 (body 0x32a08; values 0/3/11 in the saves tested) |
| 0x342f4 | 0x80 | 0x103150c0 | |
| 0x34374 | 4 | 0x104c00dc | |
| 0x3437c | 16 | 0x10202464..70 | four ints |
| 0x3438c | 0x18000 | 0x1049f200 | large progress block (inventory/collection data suspected) |

Not yet mapped: 0x30018–0x3016b and 0x4c38c–end (312,204-byte bodies). Diffing two saves made a minute apart is the quickest way to pin further fields.

`save_tool.py set <save> money=N hp=N hp_max=N costume=N map=N` edits these fields in place (keeps a .bak). The file round-trips
and re-checksums correctly; the edited values have not been checked in the game yet.
