// The four drill claws ("@tume01" attachments, DAT_102559f4) that the player gets at drill level 4 with a full
// gauge of 400 (drill_claws.cpp, 0x1008fea0). Docked, they ride on the player's own claw node. In the strike phase
// they fly to an enemy and circle it, and fly back when it is out of reach or dead.
// This is the one declaration of the claw constants and helpers: include this header, never redeclare them.
#ifndef GURUMIN_DRILL_CLAWS_H
#define GURUMIN_DRILL_CLAWS_H

#include "vec4.h"
#include "matrix4.h"
#include "effect_manager.h"
#include "char_slot.h"
#include "char_vars.h"
#include "anim_model.h"

#define CLAW_COUNT 4
#define CLAW_NODE_NAME "@tume01"
#define CLAW_HIDDEN_FLAG 0x20            // Node flag: not drawn
#define CLAW_LEVEL 3                     // DAT_10c64ad4 while the claws exist
#define CLAW_GAUGE_STRIKE 400            // DAT_10c64ad8 at which they strike (300 raises the aura only)
#define CLAW_GAUGE_AURA 300              // aura gauge without the bank bonus
#define CLAW_MAP_NO_CLAWS 1000           // map where the claws never show

// DAT_104c0144: the claws' target, a character slot (1..0x7f) or one of these.
#define CLAW_TARGET_NONE 0
#define CLAW_TARGET_SETTLED 0x7f         // roaming after the free phase
#define CLAW_TARGET_FREE 0x80            // circling the rally point DAT_10f08d0c, looking for an enemy
#define CLAW_FREE_TICKS 30               // ticks (DAT_10256210) of the free phase
#define CLAW_RETARGET_TICKS 0x50         // ticks before the selected enemy replaces the current one
#define CLAW_FORWARD_DISTANCE 600.0f     // the rally point lies this far in front of the player
#define CLAW_RALLY_HEIGHT 50.0f
#define CLAW_TARGET_HEIGHT 30.0f         // the claws aim this far above the enemy's position
#define CLAW_LOST_DISTANCE 1200.0f       // the enemy is let go when it is this far from the player
#define CLAW_SEEK_RANGE 450              // nearest-enemy search radius for the free phase
#define CLAW_SEEK_FIRST_SLOT 58          // enemy slots 58..91
#define CLAW_SEEK_END_SLOT 92
#define CLAW_SEEK_REACH 400              // plus the player's collider reach: an enemy this near is taken
#define CLAW_KIND_ENEMY_A 'c'
#define CLAW_KIND_ENEMY_B 3

// DAT_1020aeec[i]: what claw i does. In the docked phase non-zero = flying back to the player.
#define CLAW_STATE_ORBIT 0               // on the circle around the target
#define CLAW_STATE_SWEEP 1
#define CLAW_STATE_RISE 2
#define CLAW_STATE_HOLD 3
#define CLAW_STATE_WAIT_1 5              // waiting to launch: ticks 1, 6, 11 and 16
#define CLAW_STATE_WAIT_2 6
#define CLAW_STATE_WAIT_3 7
#define CLAW_STATE_WAIT_4 8
#define CLAW_STATE_BURST 10
#define CLAW_WAIT_TICKS_1 1
#define CLAW_WAIT_TICKS_2 6
#define CLAW_WAIT_TICKS_3 11
#define CLAW_WAIT_TICKS_4 16

#define CLAW_ORBIT_RADIUS 200.0f
#define CLAW_ORBIT_MARGIN 15.0f          // inside orbit +- margin the claw is on the circle
#define CLAW_SWEEP_TURN 0.1f             // turn per frame about the target
#define CLAW_CLIMB_HEIGHT 20.0f
#define CLAW_BURST_STEP 20.0f
#define CLAW_BURST_FRAMES 5
#define CLAW_APPROACH_SCALE 0.1
#define CLAW_SPEED_MIN 10.0f
#define CLAW_SPEED_MAX_FAR 40.0f
#define CLAW_QUADRANT_OFFSET_X 100.0f
#define CLAW_QUADRANT_OFFSET_Z 20.0f
#define CLAW_QUADRANT_TURN 3.1415f
#define CLAW_SWEEP_ODDS 20               // one frame in 20 a sweeping claw rises
#define CLAW_LAUNCH_ODDS 5
#define CLAW_LAUNCH_MIN_TICKS 25
#define CLAW_TICKS_MAX 100
#define CLAW_FACING_TICKS 15             // DAT_1032d678 above this: the claw turns to face its target

// Docked phase.
#define CLAW_DOCK_DISTANCE 15.0f
#define CLAW_RETURN_RATE 0.02            // speed per tick since the recall began
#define CLAW_RETURN_BASE 0.1
#define CLAW_FACING_DISTANCE 80.0f       // nearer than this the claw takes the dock's matrix as it is
#define CLAW_RECALL_SOUND 0x12b
#define CLAW_DOCK_SOUND 0x129
#define CLAW_DOCK_OFFSET_Y -10.0f        // placement of the docked claws on the player's claw node
#define CLAW_DOCK_OFFSET_Z 35.0f
#define CLAW_DOCK_TURN_HALF 3.1415f
#define CLAW_DOCK_TURN_QUARTER (CLAW_DOCK_TURN_HALF / 2)

// Aura effects (script ids) started while the gauge allows the strike, and the burst effect of a launching claw.
#define CLAW_EFFECT_AURA 2
#define CLAW_EFFECT_AURA_POPON 0xd
#define CLAW_EFFECT_BURST 9
#define CLAW_EFFECT_OWNER 0x4d55
#define CLAW_EFFECT_LIST 5
#define CLAW_EFFECT_SETTING_AREA_OWNER 5
#define CLAW_EFFECT_SCALE_DEFAULT 4.0f
#define CLAW_EFFECT_SCALE_POPON_ALT 20.0f
#define CLAW_EFFECT_SCALE_COSTUME_0B 12.0f
#define CLAW_EFFECT_SCALE_COSTUME_08 10.0f
#define CLAW_EFFECT_SCALE_HELMET_400 8.0f
#define CLAW_EFFECT_SCALE_HELMET_300 6.8f
#define CLAW_EFFECT_SCALE_HELMET_200 6.0f
#define CLAW_EFFECT_SCALE_HELMET_100 4.8f
#define CLAW_COSTUME_POPON 6
#define CLAW_COSTUME_0C 0xc
#define CLAW_COSTUME_0B 0xb
#define CLAW_COSTUME_08 8
#define CLAW_HELMET_400 0x400
#define CLAW_HELMET_300 0x300
#define CLAW_HELMET_200 0x200
#define CLAW_HELMET_100 0x100

// 0x101b1360: translation matrix of (x, y, z); 0x101b1f30: the matrix that turns the z axis into `direction`.
Matrix4 TranslationMatrix(float x, float y, float z);
Matrix4 DirectionMatrix(Vec4 direction);

extern "C" Vec4 DAT_10f08d0c;   // the point the claws circle in the free phase, and the previous position of their target

// The claw effect of the current costume and helmet: scale picked from the costume (0xc, 0xb, 8) and the helmet's face
// bits, strength from the player's effect parameter. Drill 5 uses 1 and 1.0.
__forceinline void StartClawEffect(int effect, const Matrix4 *matrix)
{
    Vec4 position;
    position.Fill(0.0f);
    EffectOptions options;
    options.m_0004 = -DAT_10daaae8[0].stats[STAT_EFFECT_PARAM];
    options.m_0008 = CLAW_EFFECT_SCALE_DEFAULT;
    if (DAT_10c655f0 == CLAW_COSTUME_0C)
        options.m_0008 = CLAW_EFFECT_SCALE_POPON_ALT;
    else if (DAT_10c655f0 == CLAW_COSTUME_0B)
        options.m_0008 = CLAW_EFFECT_SCALE_COSTUME_0B;
    else if (DAT_10c655f0 == CLAW_COSTUME_08)
        options.m_0008 = CLAW_EFFECT_SCALE_COSTUME_08;
    else if ((DAT_104c02c8 & CLAW_HELMET_400) != 0)
        options.m_0008 = CLAW_EFFECT_SCALE_HELMET_400;
    else if ((DAT_104c02c8 & CLAW_HELMET_300) == CLAW_HELMET_300)
        options.m_0008 = CLAW_EFFECT_SCALE_HELMET_300;
    else if ((DAT_104c02c8 & CLAW_HELMET_200) != 0)
        options.m_0008 = CLAW_EFFECT_SCALE_HELMET_200;
    else if ((DAT_104c02c8 & CLAW_HELMET_100) != 0)
        options.m_0008 = CLAW_EFFECT_SCALE_HELMET_100;
    if (DAT_10c6566c != 0)
    {
        options.m_0004 = 1;
        options.m_0008 = 1.0f;
    }
    options.owner = CLAW_EFFECT_OWNER;
    options.m_0000 = 1;
    options.m_000e = CLAW_EFFECT_LIST;
    options.settings[CLAW_EFFECT_SETTING_AREA_OWNER] = 0;
    DAT_10daaa80.Start(effect, position, -1, 0, 0, 0, 0, matrix, &options, 0);
}

// 0x1008fea0 (a C++ function: it has an EH frame for its matrices).
int DrillClaws_Update(void);

#endif
