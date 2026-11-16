# .itm files (scripts, tables and some textures)

Tool: `tools/itm_tool.py decode|encode` (no dependencies). Verified on 2,511 files from the JP and EN PC releases and
the Gurumin-Script-Fixes repo: every one decodes with a valid checksum and re-encodes byte-identically.
`sound.itm` (4 KB) is a different format and is not covered.

## Layout
```
body[n]      scrambled plain text (or data)
int32        unknown (0 in all files seen; read into a local and ignored)
int32        sum of all plain bytes (mod 2^32)
int32        0xDE2F xor every plain byte
```
The loader (APP.dll 0x10002440 for `bin\map.itm`) always replaces the extension with `itm`, reads `size-12` bytes,
then the three trailer words, and calls the descrambler 0x101615d0 (`ItmDescramble(buf, len, name, sum, xor)`).
The file is rejected (returns 0, nothing loaded) if either checksum differs.

## Descrambling
```
key   = "GURUguruMinmIN_"                       (15 bytes)
table = 64 bytes from APP.dll 0x101e8c98, with bytes 8..15 replaced by the first 8 characters of the
        upper-cased file base name (no directory, no extension; zero-padded)
plain[i] = ((key[i % 15] + (i & 7)) * table[i & 63]) & 0xFF  XOR  ~body[i]
```
The keystream repeats every 960 bytes for a given name, so a file renamed must be re-encoded.

## Contents
`bin\map.itm` → `map.txt`: `#VERSION`, then sections `#StageName`, `#BGMTable`, `#MapTable`, `#MapEventTable`,
`#EventTrigger`, `#MapEventTrigger`, `#CCTable`, `#SE_Table`, `#FaceTable`, `#FaceFrameTable`, `#CharMotTable:NNNN`,
`#BtMotTable`, `#CharTable`, `#StatusTable`, `#CharPos`, `#CharMonsPos`, `#AbTable`, `#ItemTable`, `#SkillTable`,
`#MoveTable`, `#BattleSc2:NNNNN`, `#USDG`, `#LVTable`, `#ObjAnimeTable`, `#BtMapTable` and ~820 `#Script:NNNNN`
blocks (C-like script source, compiled at load time; see SCRIPT_API.md). Each section ends with `#End<Name>`.
All parsed by 0x10002440 (`MapData::Load`).
