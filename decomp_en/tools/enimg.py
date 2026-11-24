"""enimg.py : paths and the image reader for the English Steam exe (shared by the decomp_en tools).

"""
import csv, os

HERE = os.path.dirname(os.path.abspath(__file__))
EN = os.path.dirname(HERE)                      # decomp_en/
ROOT = os.path.dirname(EN)                      # repository root
from coff import PeImage

EXE = os.path.join(ROOT, "re", "bin", "game_en_steam.exe")
EXPORT = os.path.join(ROOT, "re", "export", "game_en_steam.exe")
FUNCS = os.path.join(EXPORT, "functions.tsv")
DECOMPILED = os.path.join(EXPORT, "decompiled.c")
CLASSIFY = os.path.join(EN, "classify.tsv")     # address, size, class (game/library), reason
OBJ_SYMBOLS = os.path.join(EN, "obj_symbols.tsv")
REPORT = os.path.join(ROOT, "progress", "en", "report.json")


class Image(PeImage):
    def __init__(self, path=EXE):
        super().__init__(path)


def inventory():
    """{address: (Ghidra name, size)} for every function of the EN exe"""
    inv = {}
    with open(FUNCS, encoding="utf-8", errors="replace") as f:
        next(f)
        for line in f:
            c = line.split("\t")
            try:
                inv[int(c[0], 16)] = (c[1], int(c[2]))
            except (ValueError, IndexError):
                pass
    return inv


def game_functions():
    """{address hex string: size} of the functions classified as game code (classify.tsv)"""
    out = {}
    for r in csv.DictReader(open(CLASSIFY, encoding="utf-8"), delimiter="\t"):
        if r["class"] == "game":
            out[r["address"]] = int(r["size"])
    return out
