# packs.dll

Small helper DLL loaded by APP.dll (`Packs_Load` 0x101b2670). Ghidra export: `re/run_ghidra_packs.bat`; exports that
Ghidra missed were decompiled with `DecompileAt.java` into `re/export/Packs.dll/extra.c`.

| Export | Address | What |
|---|---|---|
| ExecUnpack | 0x100070e0 | LZ77 decompressor (16-bit flag words, short/long offsets, RLE). Used for .it3 vertex/index arrays. Python: `tools/gpack.py` (verified on all 945 JP .it3 files). |
| ExecPack | 0x10006e90 | matching compressor (not ported; `tools/gpack.py` has no packer yet - write uncompressed-equivalent streams with literals only if needed) |
| BMPUnpack | 0x10004a70 | wavelet image decoder (3 channels as 16-bit fixed point, multi-level inverse transform 0x10002d20, entropy stage 0x10004830, dequant 0x10004120). Used by .iti types 666/667. Not ported. |
| BMPPack | 0x10004fb0 | its encoder |
| BMPLoad | 0x10005c00 | reads a .iti JPEG (types 888/889) into RGB / alpha buffers |
| BMPSave | 0x10005e50 | writes images |
| SaveJpeg | 0x10005470 | screenshot JPEG writer |
| MMV3 | 0x10005650 | movie (.mmv) frame decoder |
