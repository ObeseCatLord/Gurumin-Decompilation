"""Merge config for tools/merge_driver.py (done 2026-11-22; kept as an example).
Game (include/game.h): the per-file Game views of DAT_104b7200 and DAT_10874848.
Usage from decomp/: python tools/merge_driver.py tools/merge_configs/game.py <tag> <file.cpp>..."""
HEADER = "game.h"
CANON = "Game"
GLOBAL = "DAT_104b7200"
EXTERN = r'extern "C" Game DAT_(?:104b7200|10874848);[^\r\n]*\r?\n'
VIEWS = ["Game"]
FILE_VIEWS = {}
TYPES = {}
NO_INCLUDE = {}
PRE_RAW = {}
COMMON = {}
RENAME = {}
RAW = {"char_move.cpp": [(r"\.RegisterFlag\(", ".RegisterEvent(")]}
