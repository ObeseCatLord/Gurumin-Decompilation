// The spell-casting teleporter (a character kind; EN build, JP: decomp/src/enemy_cast_teleport.cpp,
// Enemy_CastAndTeleport). Declared once here: the CharState words it uses, its steps and its tuning.
#ifndef GURUMIN_EN_CAST_TELEPORT_ENEMY_H
#define GURUMIN_EN_CAST_TELEPORT_ENEMY_H

// CharState::values indices of the caster (CharState::state holds the step, a CastStep).
const int CAST_VALUE_WAIT = 0;             // frames in the current step (the spell counts its frames here)
const int CAST_VALUE_SET_UP = 2;           // 1 once its spine node was found and it stood on the ground
const int CAST_VALUE_SIGHT_COUNTDOWN = 3;  // frames it keeps hunting after losing sight of the player
const int CAST_VALUE_FADE_STEP = 4;        // step of the teleport fade (15 counts down to 1, then up to 15)
const int CAST_VALUE_MOTION = 5;           // motion to play this frame
const int CAST_VALUE_SIGHT = 6;            // 0 = player not seen, 1 = seen, 2 = seen with a clear line

// Steps of the caster (CharState::state).
enum CastStep
{
    CAST_STEP_WANDER = 0,       // turns to a random heading now and then
    CAST_STEP_KEEP_RANGE = 1,   // advances, backs off, faces the player and may start a spell
    CAST_STEP_SPELL = 2,        // bolt effect and sound, then a charge effect on its body
    CAST_STEP_FIND_SPOT = 5,    // looks for a spot near the player to teleport to
    CAST_STEP_FADE_OUT = 6,     // fades out
    CAST_STEP_SLIDE = 7,        // slides to the spot
    CAST_STEP_FADE_IN = 8       // fades in
};

// Motions it plays.
const int CAST_MOTION_RETREAT = 1;
const int CAST_MOTION_IDLE = 2;
const int CAST_MOTION_ADVANCE = 5;

const char CAST_SPINE_NAME[] = "Bip01 Spine";   // node its anchor point follows
const float CAST_PI = 3.1415f;
const float CAST_FULL_TURN = 6.283f;
const float CAST_SPINE_OFFSET_Y = -50.0f;       // anchor point relative to the spine, before turning
const float CAST_SPINE_OFFSET_Z = 10.0f;
const float CAST_GROUND_LIFT = 10.0f;           // the ground probe starts this far above the character
const int CAST_GROUND_PERIOD = 10;              // frames between ground and sight checks
const float CAST_SEE_RANGE = 1000.0f;
const float CAST_SEE_ANGLE = 0.2618f;           // bearing error allowed when it only looks ahead
const float CAST_SIGHT_HEIGHT = 75.0f;          // the sight ray ends this far above the player
const int CAST_BLIND_PERCENT = 10;              // percent of clear sights that are dropped
const int CAST_SIGHT_CLEAR = 2;
const int CAST_HUNT_FRAMES = 300;
const int CAST_WANDER_ROLL_RANGE = 5000;
const int CAST_WANDER_ROLL_LIMIT = 10;
const float CAST_NEAR_RANGE = 700.0f;
const float CAST_FAR_RANGE = 800.0f;
const int CAST_CAST_ROLL_RANGE = 10000;
const int CAST_CAST_ROLL_LIMIT = 1000;
const int CAST_SPELL_FRAMES = 0x4b;
const int CAST_CHARGE_FRAMES = 0x23;
const int CAST_BOLT_EFFECT = 0x673;
const int CAST_BOLT_SOUND = 0x2b6;
const int CAST_BOLT_AFTER_SOUND = 0x2b7;
const int CAST_BOLT_AFTER_DELAY = 0x1e;
const float CAST_BOLT_SCALE = 1.5f;
const char CAST_BOLT_MODE = 1;
const int CAST_CHARGE_EFFECT = 0x66d;
const unsigned char CAST_CHARGE_FOLLOW_OWNER = 2;   // EffectOptions::flags bit 1
const int CAST_SPOT_TRIES = 0x14;
const int CAST_SPOT_DEPTH_RANGE = 800;
const float CAST_SPOT_MIN_DEPTH = -100.0f;
const float CAST_SPOT_LIFT = 50.0f;
const float CAST_SPOT_FLOOR_DROP = 1000.0f;     // the ground below a spot is looked for this far down
const int CAST_FADE_STEPS = 15;
const int CAST_FADE_OFFSET = 10;
const float CAST_FADE_DIVISOR = 25.0f;
const float CAST_SLIDE_ALPHA = 0.25f;
const int CAST_DEGREES = 360;
const float CAST_DEG_HALF_TURN = 180.0f;
const float CAST_PI_FINE = 3.14159f;
const unsigned char CAST_MIN_LEVEL = 0xf;       // g_charHitLevel at or above this: no motion is played
const int CAST_MODE_PIT = 2;                    // CAST: AnimModel::mode while it falls into the pit

// One frame of the caster `index` (see enemy_cast_teleport.cpp; C++ linkage, as the original: its Matrix4 local needs an unwind frame). EN 0x665b70 (JP 0x1006e150).
int Enemy_CastAndTeleport(int index);

#endif
