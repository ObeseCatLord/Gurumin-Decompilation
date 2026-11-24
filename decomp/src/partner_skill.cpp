// The partner skills: the special moves of the characters in slots 6, 7, 11 and 16 (run once a frame from the
// enemy update 0x100e0320 and the chase 0x100bcce0). Slot 6 heals the player (it spins up a ring, then adds two
// hit points a frame), slot 7 makes the player invulnerable for 300 frames and then grabs the enemy in front of it
// (within 600 units and about 45 degrees) by every bone, slot 11 fires a bolt once its counter reaches 30 and
// slot 16 aims a bolt at the nearest enemy within 1000 units.

#include <math.h>

#include "last_position.h"
#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "dodge_game.h"
#include "game_vars.h"
#include "map_vars.h"
#include "world.h"

#include "partner_skill.h"

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1000e750(void);                                            // AntiTamper_StoreValues
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define SKILL_PI 3.1415f
#define SKILL_FULL_TURN 6.283f
#define SKILL_MAP_DODGE 0xd2
#define SKILL_DODGE_OVER 2
#define SKILL_PHASE_LOCKED 1000          // below this phase the skills do not run during scripts and cut-scenes
#define SKILL_ENEMY_FIRST 0x3a           // the enemy slots are 0x3a .. 0x5b
#define SKILL_ENEMY_END 0x5c
#define SKILL_MOTION_STAND 7
#define SKILL_MOTION_SHIELD_START 9
#define SKILL_MOTION_SHIELD_GRAB 8
#define SKILL_MOTION_NONE (-1)
#define SKILL_NO_TARGET (-1)
#define SKILL_NO_PARENT (-1)
#define SKILL_NO_DISTANCE 9000000.0f     // larger than any distance, so the first enemy always wins
#define SKILL_ALIVE_LIMIT (-20000.0f)    // enemies parked further out than this are gone
#define SKILL_NEAR_RANGE 1000.0f
#define SKILL_MOTION_SPEED_BOOST 15.0f
#define SKILL_BOOST_PERCENT 10

#define SKILL_AIM_FRAMES 2
#define SKILL_AIM_COOLDOWN 8
#define SKILL_AIM_MIN_DISTANCE 1.0f
#define SKILL_AIM_LIFT 80.0f
#define SKILL_AIM_EFFECT 0x785
#define SKILL_AIM_SOUND 0x122
#define SKILL_AIM_EFFECT_PARAM 0x2d
#define SKILL_AIM_SCALE 2.0f
#define SKILL_AIM_FLAGS 5

#define SKILL_BLAST_START_FRAME 30
#define SKILL_BLAST_BEHAVIOUR 0x4b
#define SKILL_BLAST_EFFECT 0x789
#define SKILL_BLAST_SOUND 0x123
#define SKILL_BLAST_SOUND_PERIOD 0xb
#define SKILL_BLAST_EFFECT_PARAM 0x19
#define SKILL_BLAST_SCALE 2.0f
#define SKILL_BLAST_LIFT 10.0f

#define SKILL_HEAL_RING_FRAMES 0x1c
#define SKILL_HEAL_RISE_FRAMES 0x2d
#define SKILL_HEAL_PULL_FRAMES 0x46
#define SKILL_HEAL_END 0x64
#define SKILL_HEAL_RESET 0x81
#define SKILL_HEAL_PLAYER_LIFT 132.5f
#define SKILL_HEAL_RING_HEIGHT 12.0f
#define SKILL_HEAL_RING_STEP 0.5f
#define SKILL_HEAL_RING_SCALE 1.5f
#define SKILL_HEAL_SINK 0.5f
#define SKILL_HEAL_POINTS 2
#define SKILL_HEAL_EFFECT 0x78c
#define SKILL_HEAL_PULL_EFFECT 0x2530
#define SKILL_HEAL_SOUND 0x126
#define SKILL_HEAL_OWNER 0x6a
#define SKILL_HEAL_MARK 0x4b

#define SHIELD_START_EFFECT_NECK 0x78e
#define SHIELD_START_EFFECT_SPINE 0x790
#define SHIELD_START_SOUND 0x124
#define SHIELD_OWNER 0x6b
#define SHIELD_MARK 0x4b
#define SHIELD_FRAMES 300
#define SHIELD_START_FRAMES 0x1e
#define SHIELD_RANGE_PHASE 0x14a
#define SHIELD_GRAB_PHASE 1000
#define SHIELD_WAIT_LIMIT 999
#define SHIELD_GRAB_END 0x406
#define SHIELD_GRAB_RESUME 0x1e
#define SHIELD_GRAB_FRAMES 31.0f
#define SHIELD_GRAB_LIFT (-37.0f)
#define SHIELD_GRAB_PAUSE 0x2d
#define SHIELD_GRAB_EFFECT 0x791
#define SHIELD_GRAB_SOUND 0x125
#define SHIELD_GRAB_TRAIL_EFFECT 0x664
#define SHIELD_GRAB_TRAIL_FRAMES 0x3c
#define SHIELD_GRAB_TRAIL_SCALE 2.0f
#define SHIELD_GRAB_TRAIL_FLAGS 5
#define SHIELD_REACH_HEIGHT 150.0f
#define SHIELD_REACH_RANGE 600.0f
#define SHIELD_REACH_ANGLE 0.7854f
#define SHIELD_COLOUR_VALUE 0x84
#define SHIELD_COLOUR_RELEASE 0x86
#define SHIELD_MASK_A 0x400
#define SHIELD_MASK_B 0x200

// Starts the grab effect on the bone `bone` of the shield partner (slot 7).
__forceinline void StartGrabEffect(const char *bone, const Vec4 &at)
{
    FUN_1001d8b0(PARTNER_SHIELD, SHIELD_GRAB_EFFECT, at, SKILL_NO_PARENT, SKILL_NO_PARENT,
                 &DAT_10db6ee8[PARTNER_SHIELD].root->Find(bone, 0)->worldMatrix, 0, 0, 0, 0);
}

// 0x10078060 (Partner_SpecialSkill). One frame of the skill of the character in slot `index` at `position`; any other
// slot returns -1. While scripts or cut-scenes run (or the map 0xd2 condition holds) a skill below phase 1000 does
// nothing. Slot 16 (specialPhase 0) looks for the nearest living enemy within 1000 units, then (phase 1) shoots an
// effect at it from node 1 and waits eight frames; it sometimes speeds its motion up. Slot 11 counts frames, sets
// behaviour 0x4b at the first and fires its bolt from the 30th on. Slot 6 (the healer) waits for the player to be
// hurt, turns to him and starts its effect (phase 1); the ring grows round it until frame 0x1c, runs away until
// 0x2d, the effect is pulled to the player (0x2d to 0x46), and from 0x46 on adds two hit points a frame (up to
// the maximum) until frame 0x64; it is ready again at 0x82. Slot 7 gives invulnerability for 300 frames (phase 1,
// motion 9), after 30 frames picks the enemy within 600 units, 150 above and 45 degrees in front of it, and
// (phase 1000 on) grabs it: effects on every bone, a lift curve in `steer`, ending at phase 0x406. Returns the
// motion to play, -1 = none.
int Partner_SpecialSkill(int index, Vec4 *position, int *found, Vec4 *steer)
{
    Vec4 at;            // the position of the enemy or of the player being looked at
    Vec4 spare1;        // three more vectors the original declares and never uses (their slots and constructors remain)
    Vec4 spare2;
    Vec4 spare3;
    int motion = SKILL_MOTION_NONE;
    float distance;     // slot 16: distance to the enemy; slot 7: squared distance of the best enemy so far
    if ((DAT_1032d69c == 0 || DAT_104bfe6c != 0 ||
         (DAT_10c62ed0 == SKILL_MAP_DODGE && DAT_10c655e0 != 0 && DAT_1020c2b8 == SKILL_DODGE_OVER &&
          DAT_10c64ed0 == 0)) &&
        DAT_1020b430[index].specialPhase < SKILL_PHASE_LOCKED)
        return SKILL_MOTION_NONE;
    if (index == PARTNER_HOMING)
    {
        if (DAT_10db6ee8[PARTNER_HOMING].m_238c != 0 && FUN_1013f880() % 100 < SKILL_BOOST_PERCENT)
            DAT_10db6ee8[PARTNER_HOMING].AddMotionSpeed(SKILL_MOTION_SPEED_BOOST);
        motion = SKILL_MOTION_STAND;
        if (DAT_1020b430[PARTNER_HOMING].specialPhase == 0)
        {
            int target = SKILL_NO_TARGET;
            float nearest = SKILL_NO_DISTANCE;
            for (int i = SKILL_ENEMY_FIRST; i < SKILL_ENEMY_END; i++)
            {
                if (DAT_1021f3c0[i] != 2 && DAT_10210d30[i] == 0 && DAT_10daaae8[i].stats[STAT_HEALTH] > 0 &&
                    !(DAT_10db6ee8[i].matrix.translation.x < SKILL_ALIVE_LIMIT))
                {
                    at.Copy(DAT_10db6ee8[i].matrix.GetTranslation());
                    distance = (at - *position).Length();
                    if (!(distance > SKILL_NEAR_RANGE) && (target == SKILL_NO_TARGET || distance < nearest))
                    {
                        target = i;
                        nearest = distance;
                    }
                }
            }
            if (target > SKILL_NO_TARGET)
            {
                at.Copy(DAT_10db6ee8[DAT_1020b430[PARTNER_HOMING].specialTarget].matrix.GetTranslation());   // the old target's position, never used
                DAT_1020b430[PARTNER_HOMING].specialTarget = target;
                DAT_1020b430[PARTNER_HOMING].specialPhase = 1;
            }
        }
        else if (DAT_1020b430[PARTNER_HOMING].specialPhase == 1)
        {
            EffectOptions options;
            Vec4 shotAt;
            Matrix4 shot;
            if (DAT_1020b430[PARTNER_HOMING].specialTarget > 0)
            {
                at.Copy(DAT_10db6ee8[DAT_1020b430[PARTNER_HOMING].specialTarget].matrix.GetTranslation());
                distance = (at - *position).Length();
                float heading = atan2f(at.x - position->x, at.y - position->y) + SKILL_PI;
                if (distance > SKILL_AIM_MIN_DISTANCE)
                {
                    if (at.z > position->z)
                        shot = RotationX(acosf((at - *position).Length2D() / distance)) * RotationZ(heading);
                    else
                        shot = RotationX(-acosf((at - *position).Length2D() / distance)) * RotationZ(heading);
                }
                else
                    shot = RotationZ(heading);
                shotAt.Copy(*position);
                shotAt.z += SKILL_AIM_LIFT;
                shot.SetTranslation(shotAt);
                shotAt.Fill(0.0f);
                options.m_0000 = 1;
                options.m_0004 = SKILL_AIM_EFFECT_PARAM;
                options.m_0008 = SKILL_AIM_SCALE;
                options.m_000e = SKILL_AIM_FLAGS;
                FUN_1001d8b0(PARTNER_HOMING, SKILL_AIM_EFFECT, shotAt, SKILL_NO_PARENT, SKILL_NO_PARENT, 0,
                             (int)&DAT_1021bdf8[PARTNER_HOMING][1]->worldMatrix, 0, &shot, &options);
                FUN_10065b10(PARTNER_HOMING, SKILL_AIM_SOUND, 1, 0);
            }
            DAT_1020b430[PARTNER_HOMING].specialPhase = SKILL_AIM_FRAMES;
        }
        else if (DAT_1020b430[PARTNER_HOMING].specialPhase < SKILL_AIM_COOLDOWN)
            DAT_1020b430[PARTNER_HOMING].specialPhase++;
        else
            DAT_1020b430[PARTNER_HOMING].specialPhase = 0;
    }
    else if (index == PARTNER_BLASTER)
    {
        DAT_1020b430[PARTNER_BLASTER].specialFrames++;
        if (DAT_1020b430[PARTNER_BLASTER].specialFrames < SKILL_BLAST_START_FRAME)
        {
            if (DAT_1020b430[PARTNER_BLASTER].specialFrames == 1)
                DAT_1020b430[PARTNER_BLASTER].behaviour = SKILL_BLAST_BEHAVIOUR;
        }
        else
        {
            Vec4 boltAt;
            Matrix4 bolt;
            EffectOptions options;
            options.m_0000 = 1;
            options.m_000e = 1;
            options.m_0004 = SKILL_BLAST_EFFECT_PARAM;
            options.m_0008 = SKILL_BLAST_SCALE;
            bolt = RotationZ(DAT_10db6ee8[PARTNER_BLASTER].heading);
            boltAt.Copy(DAT_1021bdf8[PARTNER_BLASTER][1]->worldMatrix.GetTranslation());
            boltAt.z += SKILL_BLAST_LIFT;
            bolt.SetTranslation(boltAt);
            boltAt.Fill(0.0f);
            FUN_1001d8b0(PARTNER_BLASTER, SKILL_BLAST_EFFECT, boltAt, SKILL_NO_PARENT, SKILL_NO_PARENT, 0, 0, 0,
                         &bolt, &options);
            if (DAT_104bf8a4 % SKILL_BLAST_SOUND_PERIOD == 0)
                FUN_10065b10(PARTNER_BLASTER, SKILL_BLAST_SOUND, 1, 0);
        }
    }
    else if (index == PARTNER_HEALER)
    {
        if (DAT_1020b430[PARTNER_HEALER].specialPhase > 1)
            DAT_1020b430[PARTNER_HEALER].specialPhase++;
        if (DAT_1020b430[PARTNER_HEALER].specialPhase == 0)
        {
            if (DAT_10daaae8[0].stats[STAT_HEALTH] < DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
                DAT_1020b430[PARTNER_HEALER].specialPhase = 1;
        }
        else if (DAT_1020b430[PARTNER_HEALER].specialPhase == 1)
        {
            motion = SKILL_MOTION_STAND;
            EffectOptions options;
            Vec4 ringAt;
            Matrix4 ring;
            at.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            DAT_10db6ee8[PARTNER_HEALER].targetHeading = atan2f(at.x - position->x, at.y - position->y) + SKILL_PI;
            DAT_10db6ee8[PARTNER_HEALER].heading = DAT_10db6ee8[PARTNER_HEALER].targetHeading;
            ring.SetIdentity();
            ringAt.Fill(0.0f);
            options.owner = SKILL_HEAL_OWNER;
            FUN_1001d8b0(PARTNER_HEALER, SKILL_HEAL_EFFECT, ringAt, SKILL_NO_PARENT, SKILL_NO_PARENT, &DAT_10869278, 0,
                         0, &ring, &options);
            DAT_1020b430[PARTNER_HEALER].specialPhase = 2;
            DAT_10869278.SetIdentity();
            DAT_10869278.SetTranslation(DAT_10db6ee8[PARTNER_HEALER].matrix.GetTranslation());
            DAT_1020b430[PARTNER_HEALER].specialMark = SKILL_HEAL_MARK;
            FUN_10065b10(PARTNER_HEALER, SKILL_HEAL_SOUND, 1, 0);
        }
        else if (DAT_1020b430[PARTNER_HEALER].specialPhase < SKILL_HEAL_END)
        {
            if (DAT_1020b430[PARTNER_HEALER].specialPhase < SKILL_HEAL_RING_FRAMES)
            {
                *found = 1;
                motion = SKILL_MOTION_STAND;
                steer->Fill(0.0f);
                DAT_10869278.translation.x = DAT_10db6ee8[PARTNER_HEALER].matrix.translation.x;
                DAT_10869278.translation.y = DAT_10db6ee8[PARTNER_HEALER].matrix.translation.y;
                DAT_10869278.translation.z =
                    (SKILL_HEAL_RING_HEIGHT -
                     (float)(DAT_1020b430[PARTNER_HEALER].specialPhase - 2) * SKILL_HEAL_RING_STEP) *
                        SKILL_HEAL_RING_SCALE +
                    DAT_10869278.translation.z;
            }
            else if (DAT_1020b430[PARTNER_HEALER].specialPhase < SKILL_HEAL_PULL_FRAMES)
            {
                if (DAT_1020b430[PARTNER_HEALER].specialPhase < SKILL_HEAL_RISE_FRAMES)
                {
                    motion = SKILL_MOTION_STAND;
                    *found = 1;
                    steer->Fill(0.0f);
                }
                at.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                at.z += SKILL_HEAL_PLAYER_LIFT;
                at.Copy((at - DAT_10869278.GetTranslation())
                            .Divided((float)(SKILL_HEAL_PULL_FRAMES - DAT_1020b430[PARTNER_HEALER].specialPhase)));
                DAT_10869278.SetTranslation(DAT_10869278.GetTranslation() + at);
            }
            else
            {
                if (DAT_1020b430[PARTNER_HEALER].specialPhase == SKILL_HEAL_PULL_FRAMES)
                {
                    Vec4 pullAt;
                    FUN_1001d8b0(PARTNER_HEALER, SKILL_HEAL_PULL_EFFECT, pullAt, SKILL_NO_PARENT, SKILL_NO_PARENT,
                                 &DAT_10db6ee8[0].root->Find("Bip01 Spine", 0)->worldMatrix, 0, 0, 0, 0);
                }
                DAT_10869278.translation.x = DAT_10db6ee8[0].matrix.translation.x;
                DAT_10869278.translation.y = DAT_10db6ee8[0].matrix.translation.y;
                DAT_10869278.translation.z =
                    DAT_10869278.translation.z -
                    (float)(DAT_1020b430[PARTNER_HEALER].specialPhase - SKILL_HEAL_PULL_FRAMES) * SKILL_HEAL_SINK;
                if (DAT_10daaae8[0].stats[STAT_HEALTH] < DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
                {
                    DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_HEALTH] + SKILL_HEAL_POINTS;
                    if (DAT_10daaae8[0].stats[STAT_HEALTH] > DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
                        DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
                    FUN_1000e750();
                }
            }
        }
        else if (DAT_1020b430[PARTNER_HEALER].specialPhase > SKILL_HEAL_RESET)
            DAT_1020b430[PARTNER_HEALER].specialPhase = 0;
    }
    else if (index == PARTNER_SHIELD)
    {
        if (DAT_1020b430[PARTNER_SHIELD].specialPhase == 0 && DAT_104c0398 == 0)
        {
            EffectOptions options;
            Vec4 shieldAt;
            Matrix4 shield;
            at.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            DAT_10db6ee8[PARTNER_SHIELD].targetHeading = atan2f(at.x - position->x, at.y - position->y) + SKILL_PI;
            DAT_10db6ee8[PARTNER_SHIELD].heading = DAT_10db6ee8[PARTNER_SHIELD].targetHeading;
            shieldAt.Fill(0.0f);
            options.owner = SHIELD_OWNER;
            FUN_1001d8b0(PARTNER_SHIELD, SHIELD_START_EFFECT_NECK, shieldAt, SKILL_NO_PARENT, SKILL_NO_PARENT,
                         &DAT_10db6ee8[0].root->Find("Bip01 Neck", 0)->worldMatrix, 0, 0, 0, &options);
            DAT_1020b430[PARTNER_SHIELD].specialPhase = 2;
            FUN_1001d8b0(PARTNER_SHIELD, SHIELD_START_EFFECT_SPINE, shieldAt, SKILL_NO_PARENT, SKILL_NO_PARENT,
                         &DAT_10db6ee8[PARTNER_SHIELD].root->Find("Bip01 Spine", 0)->worldMatrix, 0, 0, 0, 0);
            DAT_1020b430[PARTNER_SHIELD].specialMark = SHIELD_MARK;
            DAT_104c00b8 = 1;
            DAT_1020b430[PARTNER_SHIELD].specialPhase = 1;
            DAT_104c0398 = SHIELD_FRAMES;
            motion = SKILL_MOTION_SHIELD_START;
            FUN_10065b10(PARTNER_SHIELD, SHIELD_START_SOUND, 1, 0);
        }
        else if (DAT_1020b430[PARTNER_SHIELD].specialPhase < SHIELD_RANGE_PHASE)
        {
            DAT_1020b430[PARTNER_SHIELD].specialPhase++;
            if (DAT_1020b430[PARTNER_SHIELD].specialPhase < SHIELD_START_FRAMES)
            {
                *found = 1;
                motion = SKILL_MOTION_SHIELD_START;
                steer->Fill(0.0f);
            }
            if (DAT_1020b430[PARTNER_SHIELD].specialPause > 0 ||
                DAT_1020b430[PARTNER_SHIELD].specialPhase <= SHIELD_START_FRAMES)
                DAT_1020b430[PARTNER_SHIELD].specialPause--;
            else
            {
                int target = SKILL_NO_TARGET;
                for (int i = SKILL_ENEMY_FIRST; i < SKILL_ENEMY_END; i++)
                {
                    at.Copy(DAT_10db6ee8[i].matrix.GetTranslation());
                    if (DAT_10210d30[i] == 0 && DAT_10daaae8[i].m_0141 == 0 && DAT_10daaae8[i].held == 0 &&
                        !(floorf(at.z - position->z) > SHIELD_REACH_HEIGHT) &&
                        !((*position - at).Length2D() > SHIELD_REACH_RANGE))
                    {
                        float bearing = atan2f(at.x - position->x, at.y - position->y) + SKILL_PI;
                        float error = DAT_10db6ee8[PARTNER_SHIELD].heading - bearing;
                        while (error > SKILL_PI)
                            error = error - SKILL_FULL_TURN;
                        while (error < -SKILL_PI)
                            error = error + SKILL_FULL_TURN;
                        if (!(floorf(error) > SHIELD_REACH_ANGLE))
                        {
                            if (target == SKILL_NO_TARGET || (*position - at).LengthSquared3() < distance)
                            {
                                target = i;
                                distance = (*position - at).LengthSquared3();
                            }
                        }
                    }
                }
                if (target > 0)
                {
                    DAT_1020b430[PARTNER_SHIELD].specialTarget = target;
                    DAT_1020b430[PARTNER_SHIELD].specialResume = DAT_1020b430[PARTNER_SHIELD].specialPhase;
                    DAT_1020b430[PARTNER_SHIELD].specialPhase = SHIELD_GRAB_PHASE;
                    DAT_1020b430[PARTNER_SHIELD].specialPause = SHIELD_GRAB_PAUSE;
                    DAT_10db6ee8[PARTNER_SHIELD].root->M_10194730(SHIELD_COLOUR_VALUE, SHIELD_MASK_A);
                    DAT_10db6ee8[PARTNER_SHIELD].root->M_10194730(SHIELD_COLOUR_VALUE, SHIELD_MASK_B);
                    at.Copy(DAT_10db6ee8[target].matrix.GetTranslation());
                    DAT_102551d4 = atan2f(at.x - position->x, at.y - position->y) + SKILL_PI;
                    at.Fill(0.0f);
                    StartGrabEffect("Bip01 L Hand", at);
                    StartGrabEffect("Bip01 R Hand", at);
                    StartGrabEffect("Bip01 L Foot", at);
                    StartGrabEffect("Bip01 R Foot", at);
                    StartGrabEffect("Bip01 Spine", at);
                    StartGrabEffect("Bip01 Neck", at);
                    StartGrabEffect("Bip01 Head", at);
                    FUN_10065b10(PARTNER_SHIELD, SHIELD_GRAB_SOUND, 1, 0);
                }
            }
        }
        else
        {
            if (DAT_1020b430[PARTNER_SHIELD].specialPhase > SHIELD_WAIT_LIMIT)
            {
                DAT_1020b430[PARTNER_SHIELD].specialPhase++;
                float grabStep = (float)(DAT_1020b430[PARTNER_SHIELD].specialPhase - SHIELD_GRAB_PHASE);
                *found = 1;
                float t = grabStep / SHIELD_GRAB_FRAMES;
                motion = SKILL_MOTION_SHIELD_GRAB;
                DAT_10db6ee8[PARTNER_SHIELD].targetHeading = DAT_102551d4;
                steer->Fill(0.0f);
                steer->y = (1.0f - t * t) * SHIELD_GRAB_LIFT;
                steer->RotateZ(-DAT_10db6ee8[PARTNER_SHIELD].targetHeading);
                if (DAT_1020b430[PARTNER_SHIELD].specialPhase > SHIELD_GRAB_END)
                {
                    DAT_1020b430[PARTNER_SHIELD].specialPhase =
                        DAT_1020b430[PARTNER_SHIELD].specialResume + SHIELD_GRAB_RESUME;
                    DAT_10db6ee8[PARTNER_SHIELD].root->M_10194730(SHIELD_COLOUR_RELEASE, SHIELD_MASK_A);
                    DAT_10db6ee8[PARTNER_SHIELD].root->M_10194730(SHIELD_COLOUR_RELEASE, SHIELD_MASK_B);
                }
                EffectOptions trail;
                Vec4 trailAt;
                trail.m_0004 = SHIELD_GRAB_TRAIL_FRAMES;
                trail.m_0008 = SHIELD_GRAB_TRAIL_SCALE;
                trail.m_000e = SHIELD_GRAB_TRAIL_FLAGS;
                trail.m_0000 = 1;
                trailAt.Fill(0.0f);
                FUN_1001d8b0(PARTNER_SHIELD, SHIELD_GRAB_TRAIL_EFFECT, trailAt, SKILL_NO_PARENT, SKILL_NO_PARENT,
                             &DAT_10db6ee8[PARTNER_SHIELD].bipNode->worldMatrix, 0, 0, 0, &trail);
            }
            else
                DAT_1020b430[PARTNER_SHIELD].specialPhase = 0;
        }
    }
    return motion;
}
