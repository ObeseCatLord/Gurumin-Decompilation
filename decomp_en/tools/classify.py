"""classify.py : sort every function of game_en_steam.exe into game code and library code -> decomp_en/classify.tsv

Evidence (NOTES.md "EN: game code vs library code" has the details):
- Rich header: the game objects (41 C++, 67 C) were compiled with cl 18.00.31101 without LTCG; MFC and the CRT come
  from prebuilt libraries (/GS on: 928 library prologues load the security cookie, none in the game span).
- Ghidra's function-ID pass names MFC/CRT functions; every FID-named function is library code.
- Address spans (the linker keeps each object's functions together):
    0x401000-0x59ffff  MFC (static, Feature Pack); no game strings
    0x5a0000-0x7e1a3b  game code, with two third-party libraries compiled into the project:
        0x6f15b0-0x709c9f  IJG libjpeg 6b (jpeg_CreateDecompress checks version 62; "JPEGMEM"); in JP it is part of
                           Packs.dll. Called only from the Packs loader 0x7be0c0-0x7be690.
        0x7c6220-0x7e1a3b  Xiph libogg/libvorbis ("Xiph.Org libVorbis I 20120203"); in JP part of sound3d.dll.
    0x7e1a3c-end       CRT (eh vector iterators, string functions, libm, ...)
- Thunks (5-byte jmp stubs Ghidra names thunk_FUN_...) are linker/incremental artefacts: class "thunk".

Classes: game, library, thunk. Areas: game, launcher (EN-only MFC launcher/config dialog inside the game span,
0x6e5d60-0x6f15af), mfc, libjpeg, libvorbis, crt, fid (library function named by Ghidra inside the game span).
"""
import csv, os
from enimg import CLASSIFY, inventory

MFC_END = 0x5A0000
LAUNCHER = (0x6E5D60, 0x6F15B0)
THIRD_PARTY = [(0x6F15B0, 0x709CA0, "libjpeg"), (0x7C6220, 0x7E1A3C, "libvorbis")]
CRT_START = 0x7E1A3C
# library functions inside the game span, named by Ghidra's function ID in the first export (MFC/ATL inline
# helpers instantiated in the launcher, IsEqualGUID, a catch funclet). Kept as a list so that applying JP names to
# the EN project (ghidra/ApplyJpNames.java) cannot turn game functions into "library" on the next run.
FID_IN_GAME_SPAN = {0x6E5E90, 0x6E5EB0, 0x6E6440, 0x6E6750, 0x6E6817, 0x6E6F70, 0x6E7200, 0x6E9030, 0x6EA7C0,
                    0x6ED880, 0x6EDA10, 0x6F0370, 0x6F0390, 0x6F0450, 0x6F06D0, 0x773BA0, 0x7AEEE0, 0x7AEF10}


def classify(address, name, size):
    if address < MFC_END:
        return "library", "mfc", "below 0x5a0000 (static MFC)"
    if address >= CRT_START:
        return "library", "crt", "CRT span"
    for lo, hi, lib in THIRD_PARTY:
        if lo <= address < hi:
            return "library", lib, "third-party library compiled into the project"
    if address in FID_IN_GAME_SPAN:
        return "library", "fid", "named by Ghidra function ID (%s)" % name
    if name.startswith("thunk_") or size <= 5:  # 5-byte jmp stubs
        return "thunk", "game", "jump stub"
    if LAUNCHER[0] <= address < LAUNCHER[1]:
        return "game", "launcher", "EN-only launcher/config code (MFC dialogs)"
    return "game", "game", "game span"


def main():
    inv = inventory()
    counts = {}
    rows = []
    for a in sorted(inv):
        name, size = inv[a]
        cls, area, reason = classify(a, name, size)
        rows.append(("%08x" % a, size, cls, area, reason))
        c = counts.setdefault((cls, area), [0, 0])
        c[0] += 1
        c[1] += size
    with open(CLASSIFY, "w", encoding="utf-8", newline="\n") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["address", "size", "class", "area", "reason"])
        w.writerows(rows)
    for (cls, area), (n, b) in sorted(counts.items()):
        print("%-8s %-10s %6d functions %9d bytes" % (cls, area, n, b))
    game = [r for r in rows if r[2] == "game"]
    print("game total: %d functions, %d bytes; library: %d functions" % (
        len(game), sum(r[1] for r in game), sum(1 for r in rows if r[2] == "library")))


if __name__ == "__main__":
    main()
