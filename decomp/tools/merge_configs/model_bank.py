"""Merge config for tools/merge_driver.py (done 2026-11-22; kept as an example).
ModelBank (include/model_bank.h): DAT_10804bf8.
Usage from decomp/: python tools/merge_driver.py tools/merge_configs/model_bank.py <tag> <file.cpp>..."""
HEADER = "model_bank.h"
CANON = "ModelBank"
GLOBAL = "DAT_10804bf8"
EXTERN = r'extern "C" ModelBank DAT_10804bf8;[^\r\n]*\r?\n'
VIEWS = ["ModelBank"]
FILE_VIEWS = {}
TYPES = {}
NO_INCLUDE = {}
PRE_RAW = {}
COMMON = {}
RENAME = {}
RAW = {f: [(r"\bM_10164c10\b", "LightDirection"), (r"\bM_10164aa0\b", "Reset")]
       for f in ["char_ground.cpp", "game_reset.cpp", "map_view.cpp", "npc_dispatch.cpp", "overlay_draw.cpp"]}
