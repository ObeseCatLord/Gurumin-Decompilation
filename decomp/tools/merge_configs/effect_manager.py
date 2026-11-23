"""Merge config for tools/merge_driver.py (done 2026-11-22; kept as an example).
EffectManager and EffectOptions (include/effect_manager.h): DAT_10daaa80, ModelState views.
Usage from decomp/: python tools/merge_driver.py tools/merge_configs/effect_manager.py <tag> <file.cpp>..."""
HEADER = "effect_manager.h"
CANON = "EffectManager"
GLOBAL = "DAT_10daaa80"
EXTERN = r'extern "C" EffectManager DAT_10daaa80;[^\r\n]*\r?\n'
VIEWS = ["EffectManager", "EffectOptions", "ModelState", "Colour4"]
FILE_VIEWS = {}
TYPES = {"ModelState": "EffectOptions", "Colour4": "Vec4", "EffectOptions": "EffectOptions"}
NO_INCLUDE = {}
PRE_RAW = {}
COMMON = {}
RENAME = {}
ROW_CLASS = {"ModelState": "EffectOptions"}
S = [(r"\bM_10172350\b", "Start"), (r"\bM_10184170\b", "Reset")]
RAW = {f: list(S) for f in ["camera_follow.cpp", "char_effects.cpp", "char_move.cpp", "effect_sprites.cpp",
                             "effect_start.cpp", "enemy_destroy.cpp", "enemy_spawn.cpp", "game_reset.cpp",
                             "map_special.cpp", "map_view.cpp", "npc_frozen.cpp", "player_damage.cpp",
                             "scene_start.cpp", "switch_effects.cpp"]}
RAW["char_effects.cpp"] += [
    (r"int EffectManager::Draw\(const Matrix4 &view, int unused, unsigned char pass\)",
     "int EffectManager::Draw(const Matrix4 &view, const Matrix4 *camera, unsigned char pass)"),
    (r"(DrawRecord\(i, view, |DrawLight\(i, view, )unused", r"\1camera"),
    (r"int FUN_1001d8b0\(int index, int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g\)",
     "int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner, int value, "
     "int frame, const Matrix4 *matrix, const EffectOptions *options)"),
    (r"Start\(effect, position, a, b, c, d, e, f, g, &DAT_10219770\[index\]\)",
     "Start(effect, position, parent, unused, owner, value, frame, matrix, options, &DAT_10219770[index])")]
RAW["effect_sprites.cpp"] += [(r"(int EffectManager::DrawSprite[AB]\(int index, const Matrix4 &view, )int unused\)",
                               r"\1const Matrix4 *camera)")]
RAW["effect_start.cpp"] += [(r"int unused, int matrixOwner, int value", "int unused, const Matrix4 *owner, int value"),
                            (r"\.matrixOwner = matrixOwner;", ".matrixOwner = owner;"),
                            (r"^    int matrixOwner;          ", "    const Matrix4 *matrixOwner;")]
RAW["switch_effects.cpp"] += [(r"\(int\)&", "&"), (r"state\.m_000d = 1;", "state.flags = 1;")]
RAW["npc_frozen.cpp"] += [(r"options\.m_002c\b", "options.owner")]
RAW["char_move.cpp"] += [(r"options\.m_002c\b", "options.owner")]
RAW["effect_start.cpp"] += [(r"options\.m_002c\b", "options.owner")]
RAW["char_effects.cpp"] += [(r"(Draw\w*\([^;]*?view, )unused", r"\1camera")]
RAW["kind4a_turret.cpp"] = [(r"shot\.m_002c\b", "shot.owner")]
