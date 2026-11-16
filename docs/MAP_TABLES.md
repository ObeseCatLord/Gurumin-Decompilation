# map.txt tables in memory

`bin\map.itm` (decrypt with `tools/itm_tool.py`) is parsed once by `MapData_Load` (APP.dll 0x10002440) into the
main `Game` object at **0x104b7200**. Each data line is split into whitespace tokens; numbers go through `atol`,
strings are copied as-is. The first token is usually the record id and picks the slot. Memory address of a
field = 0x104b7200 + Game offset (APP.dll at its preferred base; add the relocation delta otherwise).

Derived from the parser code only; column meanings beyond "int"/"string" still need in-game checks.

| Section | Record (Game offset, size) | Columns after the id |
|---|---|---|
| `#StageName` | name +0x600 + id×0x40 (64 B); movie +0x1600 + id×0x20 (32 B) | name, movie file (`.mmv`) |
| `#BGMTable` | +0x66b10 + id×0x30 | file name (16 B at +0), extension (16 B at +0x10), 4 ints at +0x20..+0x2c (loop start, loop end, then two more) |
| `#ItemTable` | +0x19e08 + id×0xc0 | name (32 B at +0), 22 ints at +0x60..+0xbc, description (64 B at +0x20, last column) |
| `#SkillTable` | +0x32210 + id×0xcb4 | 25 ints from +0, then strings from +0x70 (the quoted node name, e.g. `"scop-pt"`) |
| `#AbTable` | +0x81e74 + id×0x240 | 15 ints from +0, string at +0x60 |
| `#StatusTable` | ints at +0xb2efc + id×0xc0 | name first, then 17 ints |
| `#CharTable` | +0xa4618 + n×0x174, sequential (count at +0xb2e98) | id at +0, name (64 B, +4), model (64 B, +0x44), ints at +0x84, +0x88, +0x8c, +0x94, an int array at +0x98, a string at +0xc4 |
| `#LVTable` | id×0x20 + 16 ints | (base offset still unclear) |
| `#MoveTable` | — | first column ignored, then ints |
| `#Script:NNNNN` | compiled at load | C-like source, see SCRIPT_API.md |

Other sections (`#MapTable`, `#MapEventTable`, `#EventTrigger`, `#MapEventTrigger`, `#CCTable`, `#SE_Table`,
`#FaceTable`, `#FaceFrameTable`, `#CharMotTable`, `#BtMotTable`, `#CharPos`, `#CharMonsPos`, `#BattleSc2`, `#USDG`,
`#ObjAnimeTable`, `#BtMapTable`) are parsed by the same function. Their layouts are still to be written down.

To change item prices, stats, skill damage and so on, edit these tables in the decrypted text and re-encode with
`itm_tool.py encode` (the file name must stay `map.itm`, it keys the scrambling).
