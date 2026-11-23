"""Merge config for tools/merge_driver.py (done 2026-11-22; kept as an example).
TextureObject (include/texture_object.h): the texture slots DAT_118d0ba8.
Usage from decomp/: python tools/merge_driver.py tools/merge_configs/texture_object.py <tag> <file.cpp>..."""
HEADER = "texture_object.h"
CANON = "TextureObject"
GLOBAL = "DAT_118d0ba8"
EXTERN = r'extern "C" \w+ DAT_118d0ba8\[[^\]]*\];[^\r\n]*\r?\n'
VIEWS = ["TextureObject", "RenderItem", "TextureSlot"]
FILE_VIEWS = {}
TYPES = {"RenderItem": "TextureObject", "TextureSlot": "TextureObject"}
NO_INCLUDE = {}
PRE_RAW = {}
COMMON = {}
RENAME = {"texture_create.cpp": {"alpha": "sourceAlpha", "pixels": "sourcePixels", "width": "sourceWidth",
                                 "height": "sourceHeight"},
          "texture_mgr.cpp": {"texture": "loaded"}}
S = [(r"\bM_1014e5d0\b", "DrawFrameAltStretched"), (r"\bM_1014bb70\b", "DrawTriangles"), (r"\bM_10169df0\b", "Recreate"),
     (r"\bM_10169e10\b", "Restore"), (r"\bM_10169e40\b", "Unload"), (r"\bM_1014b670\b", "BindStage0"),
     (r"\bM_1014b6e0\b", "BindStage1")]
import glob, os
RAW = {os.path.basename(f): S for f in glob.glob("src/*.cpp")}
