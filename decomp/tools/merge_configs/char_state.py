"""Merge config for tools/merge_driver.py (done 2026-11-22; kept as an example).
CharState (include/char_state.h): the per-character behaviour state DAT_1020b430 and its 13 views.
Usage from decomp/: python tools/merge_driver.py tools/merge_configs/char_state.py <tag> <file.cpp>..."""
HEADER = "char_state.h"
CANON = "CharState"
GLOBAL = "DAT_1020b430"
EXTERN = r'extern "C" \w+ DAT_1020b430(?:\[[^\]]*\])+;[^\r\n]*\r?\n'
VIEWS = ["NpcState", "AiState", "ScriptState", "ZoneState", "CounterState", "SlotValues", "AttackState",
         "EnemyState", "ApproachState", "BlockState", "RouteState", "Shove", "CharState"]
FILE_VIEWS = {}
TYPES = {"NpcState": "CharState", "Shove": "Shove"}
NO_INCLUDE = {}
PRE_RAW = {}
WORLD = {"m_0000": "state", "m_0004": "values", "m_0028": "skipGroundProbe", "m_0038": "sawPlayer",
         "m_003c": "behaviour"}
COMMON = dict(WORLD)
RENAME = {
    "char_move.cpp": {"m_0010": "action", "m_0014": "alarmFlashes"},
    "enemy_chase.cpp": {"pause": "pausing"},
    "enemy_pendulum.cpp": {"backwards": "swingingBack"},
    "npc_facing.cpp": {"started": "idleStarted"},
    "route_carrier.cpp": {"backwards": "reversed", "pause": "pointPause", "point": "nextPoint"},
    "char_pit.cpp": {"frame": "wait"},
    "char_reset.cpp": {"mode": "behaviour"},
    "char_script.cpp": {"running": "scriptRunning", "step": "scriptStep", "value": "scriptValue",
                        "keep": "keepAfterScript", "result": "scriptResult"},
    "digit_counter.cpp": {"count": "counterValue", "phase": "counterPhase"},
    "npc_approach.cpp": {"started": "approachStarted", "code": "approachCode"},
    "route_walk.cpp": {"stopped": "routeStopped", "point": "targetPoint"},
    "map_unload.cpp": {"data": "words"},
    "map_enter_reset.cpp": {"m_0004": "wait", "m_0008": "talk", "m_000c": "range", "m_0010": "action", "m_0014": "alarmFlashes",
                            "m_0018": "fall", "m_001c": "rider", "m_0020": "pointPause", "m_0028": "fadeActions"},
}
IDX = r"(\[(?:[^\[\]]|\[[^\[\]]*\])*\])"
WORDS = [(r"\bDAT_1020b430" + IDX + r"\[", r"DAT_1020b430\1.words["),
         (r"\bDAT_1020b430" + IDX + r"(?![\[.\w])", r"DAT_1020b430\1.words")]
RAW = {f: WORDS for f in ["camera_follow.cpp", "char_kind_select.cpp", "companion_update.cpp", "route_follow.cpp",
                           "weapon_attach.cpp"]}
RAW["char_shove.cpp"] = [(r"\bDAT_1020b430" + IDX + r"(?![\[.\w])", r"DAT_1020b430\1.shoves")]
