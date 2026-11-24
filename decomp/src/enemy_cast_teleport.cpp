// A caster that keeps its distance from the player, shoots, and teleports (character kind with behaviour 0xc, the
// per-kind behaviour 0x1006e150 of 0x100f9670). It wanders and turns on the spot until it sees the player, then
// keeps 700 to 800 units away (backing off when closer, advancing when farther), now and then casts a spell
// that lasts 75 frames (a charge effect on its body every other frame, a bolt at the first frame), then picks a
// random spot in sight around the player, fades out, slides there and fades in again.

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
#include "map_vars.h"
#include "world.h"
#include "model_bank.h"

#include "enemy_cast_teleport.h"
#include "shadow_ground.h"

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_10054550(int sound, int delay, const Vec4 &position);       // queued sound
extern "C" int FUN_100241e0(int index, float x, float y, float z);             // Character_SetMove
extern "C" int FUN_10015860(Vec4 pos);                                         // Vis_IsVisible
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define CAST_PI 3.1415f
#define CAST_FULL_TURN 6.283f
#define CAST_SPINE_NAME "Bip01 Spine"
#define CAST_SPINE_OFFSET_Y (-50.0f)    // where its anchor point lies relative to the spine, before turning
#define CAST_SPINE_OFFSET_Z 10.0f
#define CAST_GROUND_LIFT 10.0f          // the ground probe starts this far above the character
#define CAST_GROUND_PERIOD 10           // frames between ground and sight checks
#define CAST_SEE_RANGE 1000.0f
#define CAST_SEE_ANGLE 0.2618f          // the player must be within this bearing error when it only looks ahead
#define CAST_SIGHT_HEIGHT 75.0f
#define CAST_BLIND_PERCENT 10
#define CAST_SIGHT_CLEAR 2
#define CAST_HUNT_FRAMES 300
#define CAST_WANDER_ROLL_RANGE 5000
#define CAST_WANDER_ROLL_LIMIT 10
#define CAST_NEAR_RANGE 700.0f
#define CAST_FAR_RANGE 800.0f
#define CAST_ADVANCE_SPEED 4.0f
#define CAST_RETREAT_SPEED (-8.0f)
#define CAST_MOTION_RETREAT 1
#define CAST_MOTION_IDLE 2
#define CAST_MOTION_ADVANCE 5
#define CAST_CAST_ROLL_RANGE 10000
#define CAST_CAST_ROLL_LIMIT 1000
#define CAST_SPELL_FRAMES 0x4b
#define CAST_CHARGE_FRAMES 0x23
#define CAST_BOLT_EFFECT 0x673
#define CAST_BOLT_SOUND 0x2b6
#define CAST_BOLT_AFTER_SOUND 0x2b7
#define CAST_BOLT_AFTER_DELAY 0x1e
#define CAST_BOLT_SCALE 1.5f
#define CAST_CHARGE_EFFECT 0x66d
#define CAST_TARGET_HEIGHT 75.0f
#define CAST_SPOT_TRIES 0x14
#define CAST_SPOT_DEPTH_RANGE 800
#define CAST_SPOT_MIN_DEPTH (-100.0f)
#define CAST_SPOT_LIFT 50.0f
#define CAST_SPOT_PULL (-50.0f)
#define CAST_SPOT_FLOOR_DROP 1000.0f
#define CAST_FADE_STEPS 15
#define CAST_FADE_DIVISOR 25.0f
#define CAST_SLIDE_SPEED 50.0f
#define CAST_SLIDE_ALPHA 0.25f
#define CAST_STATE_WANDER 0
#define CAST_STATE_KEEP_RANGE 1
#define CAST_STATE_SPELL 2
#define CAST_STATE_FIND_SPOT 5
#define CAST_STATE_FADE_OUT 6
#define CAST_STATE_SLIDE 7
#define CAST_STATE_FADE_IN 8
#define CAST_DEG_HALF_TURN 180.0f
#define CAST_PI_FINE 3.14159f
#define CAST_MIN_LEVEL 0xf

// 0x1006e150 (Enemy_CastAndTeleport). Looks up its spine node and stands on the ground on the first frame, then every
// ten frames; keeps its anchor point (DAT_10802bf8 position) at the spine; drops into the pit (mode 2) when
// it falls below the pit floor inside the pit area. Every ten frames it checks sight of the player (within 1000
// units, along a ray from the spine to 75 units above the player; when it only looks ahead, within about 15
// degrees): sight 1 = seen, 2 = seen with the line clear, 0 = not seen (a tenth of the clear ones are dropped); the
// state falls back to 0 after 300 frames without sight. Wander (0): turns to a random heading now and then.
// Keep range (1): advances (motion 5) closer than 700 units, backs off (motion 1) beyond 800, faces the player
// and starts a spell with a one in ten chance a frame. Spell (2): the bolt effect and sound at the first frame, a
// charge effect on its body every other frame for 35 frames, 75 frames in all. Find spot (5): tries 20 random
// spots 100 to 800 units in front of the player's height (turned about the player), in a visible cell, with room to
// stand, a clear line to the player and ground below; fades out (6), slides to the spot 50 units a frame (7) and
// fades in (8). Returns 1 while DAT_10210d30 is set, else 0.
int Enemy_CastAndTeleport(int index)
{
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_104483e8[index] == 0)
        return 0;
    if (DAT_1020b430[index].castSetUp == 0)
    {
        DAT_1020b430[index].castSetUp = 1;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(CAST_SPINE_NAME, 0);
        if (DAT_102022b8 != 0)
            PlaceShadowOnGround(index);
        DAT_10db6ee8[index].m_0012 = 2;
    }
    int phase = index % CAST_GROUND_PERIOD;
    if ((int)DAT_104c0380 % CAST_GROUND_PERIOD == phase && DAT_102022b8 != 0)
        PlaceShadowOnGround(index);
    if (DAT_10210b30[index] != 0)
        DAT_10210b30[index] = 0;
    DAT_10802bf8[index].position.Fill(0.0f);
    DAT_10802bf8[index].position.y = CAST_SPINE_OFFSET_Y;
    DAT_10802bf8[index].position.z = CAST_SPINE_OFFSET_Z;
    DAT_10802bf8[index].position.RotateZ(-DAT_10db6ee8[index].heading);
    DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position +
                                      DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
    DAT_10218fc0[index] = 0;
    DAT_10db6ee8[index].locked = 0;
    DAT_10255a88[index] = 0;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    Vec4 move;
    EffectOptions options;
    Vec4 offset;
    Matrix4 shot;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    float distance = toPlayer.Length();
    toPlayer.Length2D();
    DAT_10db6ee8[index].mode = 1;
    if (self.z < DAT_104bfe24 + CAST_GROUND_LIFT)
    {
        int y = (int)self.y;
        int x = (int)self.x;
        if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x &&
            y <= (int)DAT_104d29f8.y && DAT_104bfe28 != 0)
        {
            if (self.z < DAT_104bfe24)
                DAT_10db6ee8[index].mode = 2;
            else if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
                DAT_10db6ee8[index].mode = 0;
        }
    }
    int sight;
    if ((int)DAT_104c0380 % CAST_GROUND_PERIOD == phase)
    {
        sight = 1;
        if (distance < CAST_SEE_RANGE)
        {
            if (DAT_10c66710 != 0 && DAT_1020b430[index].state == 0)
            {
                float error = CAST_PI - atan2f(toPlayer.x, toPlayer.y);
                error = error - DAT_10db6ee8[index].heading;
                while (error > CAST_PI)
                    error = error - CAST_FULL_TURN;
                while (error < -CAST_PI)
                    error = error + CAST_FULL_TURN;
                if (floorf(error) > CAST_SEE_ANGLE)
                    sight = 0;
            }
            Vec4 eye;
            Vec4 target;
            Vec4 hit;
            eye.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
            target.Copy(player);
            target.z += CAST_SIGHT_HEIGHT;
            if (DAT_10255f08 != 0 && DAT_10255f08->RayCast(0, 4, 0x4000000, &target, &eye, &hit, 0) != 0 &&
                sight != 0)
            {
                if (DAT_10c66710 != 0 && DAT_1020b430[index].state == 0 &&
                    FUN_1013f880() % 100 < CAST_BLIND_PERCENT)
                    sight = 0;
                else
                    sight = CAST_SIGHT_CLEAR;
            }
        }
        else
            sight = 0;
        DAT_1020b430[index].sightState = sight;
    }
    else
        sight = DAT_1020b430[index].sightState;
    if (sight != 0)
        DAT_1020b430[index].sightCountdown = CAST_HUNT_FRAMES;
    else if (DAT_1020b430[index].sightCountdown != 0)
        DAT_1020b430[index].sightCountdown--;
    else
    {
        DAT_1020b430[index].state = CAST_STATE_WANDER;
        DAT_1020b430[index].wait = 1;
    }
    if (DAT_1020b430[index].state == CAST_STATE_WANDER && sight != 0)
    {
        DAT_1020b430[index].state = CAST_STATE_KEEP_RANGE;
        DAT_1020b430[index].wait = 0;
    }
    DAT_1020b430[index].castMotion = CAST_MOTION_IDLE;
    move.Fill(0.0f);
    if (DAT_1020b430[index].state == CAST_STATE_WANDER)
    {
        if (FUN_1013f880() % CAST_WANDER_ROLL_RANGE < CAST_WANDER_ROLL_LIMIT)
        {
            float angle = (float)(FUN_1013f880() % 360);
            DAT_10db6ee8[index].targetHeading = angle / CAST_DEG_HALF_TURN * CAST_PI_FINE;
        }
    }
    else if (DAT_1020b430[index].state == CAST_STATE_KEEP_RANGE)
    {
        if (distance < CAST_NEAR_RANGE)
        {
            DAT_1020b430[index].castMotion = CAST_MOTION_ADVANCE;
            move.y = CAST_ADVANCE_SPEED;
            move.RotateZ(-DAT_10db6ee8[index].heading);
        }
        else if (distance > CAST_FAR_RANGE)
        {
            DAT_1020b430[index].castMotion = CAST_MOTION_RETREAT;
            move.y = CAST_RETREAT_SPEED;
            move.RotateZ(-DAT_10db6ee8[index].heading);
        }
        else
            DAT_1020b430[index].castMotion = CAST_MOTION_IDLE;
        DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CAST_PI;
        if (FUN_1013f880() % CAST_CAST_ROLL_RANGE < CAST_CAST_ROLL_LIMIT)
        {
            DAT_1020b430[index].state = CAST_STATE_SPELL;
            DAT_1020b430[index].wait = 0;
        }
    }
    else if (DAT_1020b430[index].state == CAST_STATE_SPELL)
    {
        DAT_10db6ee8[index].locked = 1;
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
        if (DAT_1020b430[index].wait == 0)
        {
            offset.Fill(0.0f);
            options.m_0000 = 0;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = CAST_BOLT_SCALE;
            options.m_000e = 1;
            options.owner = index;
            offset.Copy(player);
            offset.z += CAST_TARGET_HEIGHT;
            if (DAT_10802bf8[index].position.z < offset.z)
                shot = RotationX(acosf((offset - DAT_10802bf8[index].position).Length2D() /
                                       (offset - DAT_10802bf8[index].position).Length())) *
                       RotationZ(DAT_10db6ee8[index].targetHeading);
            else if (DAT_10802bf8[index].position.z > offset.z)
                shot = RotationX(-acosf((offset - DAT_10802bf8[index].position).Length2D() /
                                       (offset - DAT_10802bf8[index].position).Length())) *
                       RotationZ(DAT_10db6ee8[index].targetHeading);
            else
                shot = RotationZ(DAT_10db6ee8[index].targetHeading);
            shot.SetTranslation(DAT_10802bf8[index].position);
            offset.Fill(0.0f);
            FUN_1001d8b0(index, CAST_BOLT_EFFECT, offset, -1, -1, 0, 0, 0, &shot, &options);
            FUN_10065b10(index, CAST_BOLT_SOUND, 0, 0);
            FUN_10054550(CAST_BOLT_AFTER_SOUND, CAST_BOLT_AFTER_DELAY, DAT_10db6ee8[index].matrix.GetTranslation());
        }
        if (DAT_1020b430[index].wait < CAST_CHARGE_FRAMES && DAT_1020b430[index].wait % 2 != 0)
        {
            EffectOptions charge;
            charge.flags |= 2;
            offset.Fill(0.0f);
            FUN_1001d8b0(index, CAST_CHARGE_EFFECT, offset, -1, -1, 0, 0, 0, &DAT_10db6ee8[index].matrix, &charge);
        }
        if (++DAT_1020b430[index].wait > CAST_SPELL_FRAMES)
        {
            DAT_1020b430[index].state = CAST_STATE_FIND_SPOT;
            DAT_1020b430[index].wait = 0;
        }
    }
    else if (DAT_1020b430[index].state == CAST_STATE_FIND_SPOT)
    {
        Vec4 spot;
        Vec4 away;
        Vec4 spotCopy;
        Vec4 probeFrom;
        Vec4 standAt;             // declared by the original but never used (its slot and constructor remain)
        Vec4 hitPoint;
        int attempt = 1;
        do
        {
            spot.Fill(0.0f);
            spot.y = (float)-(FUN_1013f880() % CAST_SPOT_DEPTH_RANGE);
            if (!(spot.y > CAST_SPOT_MIN_DEPTH))
            {
                spot.RotateZ((float)(FUN_1013f880() % 360) / CAST_DEG_HALF_TURN * CAST_PI_FINE);
                spot.Copy(spot + DAT_10db6ee8[0].matrix.GetTranslation());
                spot.z += CAST_SPOT_LIFT;
                if (FUN_10015860(spot) != 0)
                {
                    away.Copy(player - spot);
                    away.Normalized3();
                    away.Copy(away * CAST_SPOT_PULL);
                    spotCopy.Copy(spot);
                    probeFrom.Copy(player);
                    probeFrom.z += CAST_SPOT_LIFT;
                    if (DAT_10255f08 == 0 || DAT_10255f08->Collide(0, 4, &probeFrom, &spotCopy, &hitPoint) == 0)
                    {
                        away.Copy(spotCopy - self);
                        if (DAT_10db6ee8[index].IsClearOfMapLimited(&away) != 0)
                        {
                            probeFrom.Copy(spotCopy);
                            probeFrom.z -= CAST_SPOT_FLOOR_DROP;
                            if (DAT_10255f08 != 0 &&
                                DAT_10255f08->Collide(0, 4, &probeFrom, &spotCopy, &hitPoint) != 0)
                                break;
                        }
                    }
                }
            }
            attempt++;
        } while (attempt != CAST_SPOT_TRIES);
        if (attempt == CAST_SPOT_TRIES)
        {
            move.Fill(0.0f);
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = CAST_STATE_KEEP_RANGE;
        }
        else
        {
            spot.z -= CAST_SPOT_LIFT;
            DAT_10802bf8[index].step.Copy(spot);
            DAT_1020b430[index].state = CAST_STATE_FADE_OUT;
            DAT_1020b430[index].fadeStep = CAST_FADE_STEPS;
        }
    }
    else if (DAT_1020b430[index].state == CAST_STATE_FADE_OUT)
    {
        DAT_1020b430[index].fadeStep--;
        DAT_10db6ee8[index].alpha = (float)(DAT_1020b430[index].fadeStep + 10) / CAST_FADE_DIVISOR;
        if (DAT_1020b430[index].fadeStep == 1)
            DAT_1020b430[index].state = CAST_STATE_SLIDE;
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
    }
    else if (DAT_1020b430[index].state == CAST_STATE_SLIDE)
    {
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
        move.Copy(DAT_10802bf8[index].step - self);
        DAT_10db6ee8[index].alpha = CAST_SLIDE_ALPHA;
        if (move.Length() > CAST_SPOT_LIFT)
        {
            move.Normalized3();
            move.Copy(move * CAST_SLIDE_SPEED);
        }
        else
            DAT_1020b430[index].state = CAST_STATE_FADE_IN;
        DAT_10db6ee8[index].SetPosition(move.x + self.x, move.y + self.y, move.z + self.z);
        move.Fill(0.0f);
    }
    else if (DAT_1020b430[index].state == CAST_STATE_FADE_IN)
    {
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
        DAT_1020b430[index].fadeStep++;
        DAT_10db6ee8[index].alpha = (float)(DAT_1020b430[index].fadeStep + 10) / CAST_FADE_DIVISOR;
        if (DAT_1020b430[index].fadeStep == CAST_FADE_STEPS)
        {
            DAT_10db6ee8[index].alpha = 1.0f;
            DAT_1020b430[index].state = CAST_STATE_KEEP_RANGE;
            DAT_1020b430[index].wait = 0;
            DAT_10db6ee8[index].mode = 1;
        }
        DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CAST_PI;
    }
    if (DAT_10db6ee8[index].mode == 2)
    {
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
        move.z += CAST_GROUND_LIFT;
    }
    if (move.Length() != 0.0f)
        FUN_100241e0(index, move.x, move.y, move.z);
    if (DAT_102550e0[index] < CAST_MIN_LEVEL)
        DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].castMotion, 0, 5, 1.0f);
    return 0;
}
