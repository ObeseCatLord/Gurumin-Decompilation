// The scene commands (ids 30000..30129) of Script_SceneCommand (scene_command_ids.cpp, 0x1009e380) and the few
// globals and functions only it uses. Script_ValueCommand (value_command.cpp) hands every id from 30000 on to it.
#ifndef GURUMIN_SCENE_COMMAND_IDS_H
#define GURUMIN_SCENE_COMMAND_IDS_H

#include "vec4.h"
#include "matrix4.h"

struct Node;

// Command ids of Script_SceneCommand; `value` is the command's argument, the script flags 0x1a4.. carry more.
enum SceneCommandId
{
    SCENE_ID_CLEAR_SCRATCH = 0x7530,
    SCENE_ID_HIDE_GIMMICK_AT_MODEL = 0x7531,
    SCENE_ID_TOGGLE_LEVER = 0x7532,
    SCENE_ID_SET_TURNING = 0x7533,
    SCENE_ID_SET_VARIANT_AND_MODE = 0x7534,
    SCENE_ID_READ_PLAYER_POSITION = 0x7535,
    SCENE_ID_MIRROR_CAMERA = 0x7536,
    SCENE_ID_PLAYER_SET_HEIGHT = 0x7537,
    SCENE_ID_PLAYER_JUMP = 0x7538,
    SCENE_ID_CAMERA_JUMP_TO_REGISTERS = 0x753a,
    SCENE_ID_HIDE_GIMMICK_AT_REGISTERS = 0x753b,
    SCENE_ID_MOVE_PEOPLE_AWAY = 0x753c,
    SCENE_ID_ADD_TO_EFFECT_HOLDS = 0x753d,
    SCENE_ID_AIM_STEP_AT_MODEL = 0x753e,
    SCENE_ID_PEOPLE_ARRIVE = 0x753f,
    SCENE_ID_PEOPLE_ARRIVE_SIMPLE = 0x7540,
    SCENE_ID_SET_ARRIVAL_ARGS = 0x7541,
    SCENE_ID_CLEAR_PERSON = 0x7542,
    SCENE_ID_SET_CAMERA_MODE = 0x7543,
    SCENE_ID_READ_PLAYER_REGISTERS_B = 0x7544,
    SCENE_ID_RENAME_COLLISION_NODES = 0x7545,
    SCENE_ID_PEOPLE_ARRIVE_FAR = 0x7546,
    SCENE_ID_PEOPLE_RETURN = 0x7547,
    SCENE_ID_PEOPLE_ACTIVATE = 0x7548,
    SCENE_ID_FIND_PEOPLE_BY_ROUTE = 0x7549,
    SCENE_ID_FIND_OBJECT_SLOT = 0x754a,
    SCENE_ID_RESET_CHARACTER_BY_ROUTE = 0x754b,
    SCENE_ID_ACTIVATE_PERSON = 0x754c,
    SCENE_ID_REVEAL_OBJECT = 0x754d,
    SCENE_ID_FACE_AWAY_FROM_MODEL = 0x754e,
    SCENE_ID_SET_MOTION_SPEED = 0x754f,
    SCENE_ID_SET_FOLLOW_ANGLE = 0x7550,
    SCENE_ID_SET_CAMERA_SCALE = 0x7551,
    SCENE_ID_FIND_NTH_OBJECT_OF_KIND = 0x7552,
    SCENE_ID_PLAYER_TAKE_DAMAGE = 0x7553,
    SCENE_ID_SHOW_KEY_PROMPT = 0x7554,
    SCENE_ID_PLAYER_MOTION_AND_SOUND = 0x7555,
    SCENE_ID_PLAYER_STOP_MOTION = 0x7556,
    SCENE_ID_STORE_CARRIED_ITEM = 0x7557,
    SCENE_ID_FIND_NEAR_ENEMIES = 0x7558,
    SCENE_ID_SET_MODE_FROM_FLAG = 0x7559,
    SCENE_ID_RECORD_SCRIPT_EVENT = 0x755a,
    SCENE_ID_PLACE_CART = 0x755b,
    SCENE_ID_SET_FOLLOW_COUNT = 0x755c,
    SCENE_ID_START_EFFECT_AT_FLAGS = 0x755d,
    SCENE_ID_COLOUR_FADE = 0x755e,
    SCENE_ID_WAKE_HIDDEN_OBJECT = 0x755f,
    SCENE_ID_FIND_FREE_SPOT = 0x7560,
    SCENE_ID_PEOPLE_ARRIVE_RANGE = 0x7561,
    SCENE_ID_BRING_OBJECT_BACK = 0x7562,
    SCENE_ID_SET_COUNTER_FROM_FLAG = 0x7563,
    SCENE_ID_PLACE_JUMP_MARKS = 0x7564,
    SCENE_ID_SHOW_KEY_PROMPT_FOR = 0x7565,
    SCENE_ID_CLEAR_CARRIERS = 0x7566,
    SCENE_ID_ADD_JUMP_NODE = 0x7567,
    SCENE_ID_SELECT_CAMERA_PATH = 0x7568,
    SCENE_ID_PLACE_CAMERA_BEHIND_PLAYER = 0x7569,
    SCENE_ID_CHASE_PLAYER_STEP = 0x756a,
    SCENE_ID_SET_CAMERA_NOT_SMOOTHED = 0x756b,
    SCENE_ID_FOLLOW_PLAYER_STEP = 0x756c,
    SCENE_ID_FOLLOW_PLAYER_BEGIN = 0x756d,
    SCENE_ID_FREEZE_CAMERA = 0x756e,
    SCENE_ID_SET_FIRST_CHAR_STATE = 0x756f,
    SCENE_ID_SET_GROUP_TIMER = 0x7570,
    SCENE_ID_WAIT_CAMERA_SCRIPT = 0x7571,
    SCENE_ID_SET_CAMERA_FOLLOW = 0x7572,
    SCENE_ID_PLAYER_WALK_TO = 0x7573,
    SCENE_ID_READ_CAMERA_GOAL = 0x7574,
    SCENE_ID_START_EFFECT_ON_SPARKLE = 0x7575,
    SCENE_ID_FACE_POINT = 0x7576,
    SCENE_ID_REMOVE_EFFECTS_OF = 0x7577,
    SCENE_ID_SET_GUIDE_SPEED = 0x7578,
    SCENE_ID_NOP_7579 = 0x7579,
    SCENE_ID_SET_REGISTER_Y = 0x757a,
    SCENE_ID_START_FRAME_BLEND = 0x757b,
    SCENE_ID_FIND_SAFE_SPOT = 0x757c,
    SCENE_ID_SET_FREE_MODE = 0x757d,
    SCENE_ID_SET_SCRIPT_MODE = 0x757e,
    SCENE_ID_SET_TURNING_SPEED = 0x757f,
    SCENE_ID_SHOW_ROCKS = 0x7580,
    SCENE_ID_READ_CAMERA_OFFSET = 0x7581,
    SCENE_ID_SET_EFFECT_HOLD = 0x7582,
    SCENE_ID_FIND_FLOATING_ENEMY = 0x7583,
    SCENE_ID_PLACE_DISPLAY_STAND = 0x7584,
    SCENE_ID_RANDOM_PLAYER_MOTION = 0x7585,
    SCENE_ID_HIDE_FUTA = 0x7586,
    SCENE_ID_HIDE_MAP_NODES = 0x7587,
    SCENE_ID_SIGHT_TEST = 0x7588,
    SCENE_ID_SET_AREA_BASE = 0x7589,
    SCENE_ID_PREPARE_CHARACTER = 0x758a,
    SCENE_ID_TURN_SWITCH_ON = 0x758b,
    SCENE_ID_RESET_ENEMY = 0x758c,
    SCENE_ID_SET_CAMERA_SHIFT_A = 0x758d,
    SCENE_ID_SET_CAMERA_SHIFT_B = 0x758e,
    SCENE_ID_SET_NODE_COLOURS = 0x758f,
    SCENE_ID_SET_FREE_PERSON_MODEL = 0x7590,
    SCENE_ID_SET_FOLLOWER_FLAG = 0x7591,
    SCENE_ID_SET_GREETING_COUNTDOWN = 0x7592,
    SCENE_ID_TEST_MODEL_IN_VIEW = 0x7593,
    SCENE_ID_PLAY_SOUND_AT_MODEL = 0x7594,
    SCENE_ID_READ_MODEL_IDLE = 0x7595,
    SCENE_ID_SET_CAMERA_SHIFT_FRAMES = 0x7596,
    SCENE_ID_SET_MODEL_ALPHA = 0x7597,
    SCENE_ID_SET_MODEL_HOLDING = 0x7598,
    SCENE_ID_READ_PLAYER_REGISTERS_C = 0x7599,
    SCENE_ID_READ_LEVER_VALUE = 0x759a,
    SCENE_ID_READ_BEHIND_MODEL = 0x759b,
    SCENE_ID_WAKE_ENEMIES_NEAR = 0x759c,
    SCENE_ID_SET_ROUTE_RECORD = 0x759d,
    SCENE_ID_FIND_NTH_OF_KIND = 0x759e,
    SCENE_ID_START_SPIRIT_EFFECT = 0x759f,
    SCENE_ID_READ_DISTANCE_TO_POINT = 0x75a0,
    SCENE_ID_SET_PERSON_SPEED = 0x75a1,
    SCENE_ID_DEFEAT_ENEMY = 0x75a2,
    SCENE_ID_MAKE_ENEMY = 0x75a3,
    SCENE_ID_SET_FLAG_1021F9C0 = 0x75a4,
    SCENE_ID_SET_FLAG_10255A88 = 0x75a5,
    SCENE_ID_SET_LISTED_PARTS = 0x75a6,
    SCENE_ID_THROW_FROM_MODEL = 0x75a7,
    SCENE_ID_SET_HEALTH = 0x75a8,
    SCENE_ID_DESTROY_ENEMY = 0x75a9,
    SCENE_ID_READ_ROAMER_STILL = 0x75aa,
    SCENE_ID_SPAWN_ENEMY_AT_REGISTERS = 0x75ab,
    SCENE_ID_FIND_OBJECT_OF_KIND_P = 0x75ac,
    SCENE_ID_WAIT_FOR_GAME_STATE = 0x75ad,
    SCENE_ID_PLAY_DELAYED_SOUND = 0x75ae,
    SCENE_ID_CLEAR_THREE_VECTORS = 0x75af,
    SCENE_ID_PLAY_SOUND_OPTIONAL = 0x75b0,
    SCENE_ID_HIDE_CAMERA_POSITIONS_NEAR = 0x75b1
};

extern "C" int DAT_10c635b4;    // scratch register cleared by the first scene command
extern "C" int DAT_10c635b8;    // scratch register cleared by the first scene command
extern "C" int DAT_10c635e0;    // colour variant of the last placed spirit (1..6)
extern "C" Vec4 DAT_108025f8[0x80][4];   // per character: vectors the reset command clears (role unknown)

extern "C" int FUN_10017b80(void);                          // number of gimmick records in use (app_0011.cpp)
extern "C" int FUN_100143c0(int index);                     // Camera_AddTrackedPosition
extern "C" int FUN_100158b0(Node *node, int reverse);       // UiAnim_Play
extern "C" int FUN_100165e0(int on);                        // Player_SetHeadTracking
extern "C" int FUN_1001b670(int index, float range, int count);   // Enemies_WakeNearby
extern "C" int FUN_1001ba40(int unused, const Vec4 *point); // Char_NearKind5c
extern "C" int FUN_10043cc0(int index);                     // Char_ResetState
extern "C" int FUN_1005dbf0(int damage);                    // Player_TakeDamage
extern "C" int FUN_1005f530(int index);                     // Switch_TurnOn
extern "C" int FUN_100627d0(int index);                     // Enemy_Defeat
extern "C" int FUN_1016a8d0(void);                          // Camera_SetMode2
extern "C" int FUN_1016a8e0(void);                          // Camera_SetModeFollow
extern "C" int FUN_1016b050(int id);                        // Camera_SelectPath

// 0x1009e380 (Script_SceneCommand; C++ linkage). Runs scene command `id` with `value`. Returns 0.
int Script_SceneCommand(int id, int value);

#endif
