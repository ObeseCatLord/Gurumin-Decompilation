"""Merge config for tools/merge_driver.py (done 2026-11-22; kept as an example).
CharSlot (include/char_slot.h): the character slot records DAT_10daaaec. First pass (GLOBAL DAT_10daab84, views
CharSlot/SlotRecord/KindSlot) moved the files onto the header; the CharSlot views of DAT_10daab84 saw the records
0x98 bytes late, so their uses were rewritten to DAT_10daaaec[i] with the header's field names. This config is the
second pass: the ActorSlot / ActorFace views of DAT_10daaaec.
Usage from decomp/: python tools/merge_driver.py tools/merge_configs/char_slot.py <tag> <file.cpp>..."""
HEADER = "char_slot.h"
CANON = "CharSlot"
GLOBAL = "DAT_10daaaec"
EXTERN = r'extern "C" \w+ DAT_10daaaec\[[^\]]*\];[^\r\n]*\r?\n'
VIEWS = ["ActorSlot"]
FILE_VIEWS = {"face_anim.cpp": ["ActorFace"]}
TYPES = {}
NO_INCLUDE = {}
PRE_RAW = {}
COMMON = {"m_0000": "health", "m_0004": "maxHealth", "m_013e": "held"}
RENAME = {}
RAW = {}
