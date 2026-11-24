// A flying carrier that hovers beside the player and carries another character (0x10048050, behaviour 0x21 of
// Char_RunBehaviour 0x100dfcf0). The rider is the character `talk / 100` slots after the carrier. The carrier floats
// to the player's side with a smoothed velocity that is kept between the ground and the player's height, bounces off
// walls, and the rider is held above it; when the rider is gone the carrier is reset.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"

#include "char_vars.h"

#include "map_vars.h"

#include "world.h"

#define HOVER_PROBE_PERIOD 30                // frames between two probes of the ground under the player
#define HOVER_GROUND_PERIOD 15               // frames between two probes of the ground under the carrier
#define HOVER_GROUP_FIRST 0x3a               // group numbers of the characters 0x3a..0x5b are counted
#define HOVER_GROUP_END 0x5c
#define HOVER_GROUP_LIMIT 99
#define HOVER_PLAYER_PROBE_HEIGHT 75.0f
#define HOVER_PLAYER_PROBE_FLOOR -3500.0f
#define HOVER_GROUND_PROBE_DEPTH 10000.0f
#define HOVER_GROUND_PROBE_LIFT 25.0f
#define HOVER_NO_GROUND -5000
#define HOVER_GROUND_CLEARANCE 200.0f        // lowest height above the ground under the carrier
#define HOVER_PLAYER_CLEARANCE 100.0f        // lowest height above the player
#define HOVER_PLAYER_CEILING 0x1c2           // highest height above the ground under the player
#define HOVER_PLAYER_NEAR 500.0f
#define HOVER_PI 3.1415f
#define HOVER_VELOCITY_KEEP 0.95f            // part of the last velocity kept
#define HOVER_VELOCITY_TAKE 0.05f            // part of the new velocity taken
#define HOVER_SPEED 20.0f
#define HOVER_LOWERED_DROP 10.0f
#define HOVER_BOUNCE_SPEED_LIMIT 40.0f
#define HOVER_BOUNCE_SPREAD 180              // the bounce direction is turned by up to 90 degrees either way
#define HOVER_RIDER_LIFT 45.0f
#define HOVER_RIDER_RANGE_SQUARED 10000.0f
#define HOVER_RESET_DAMPING 0.9f
#define HOVER_LIMIT_PIN_X 1
#define HOVER_LIMIT_PIN_Y 2
#define HOVER_LIMIT_MAX_X 4
#define HOVER_LIMIT_MAX_Y 8
#define HOVER_LIMIT_MIN_X 0x10
#define HOVER_LIMIT_MIN_Y 0x20

extern "C" int FUN_10043cc0(int index);                    // Char_ResetState
extern "C" int FUN_100241e0(int index, float x, float y, float z);   // Character_SetMove
extern "C" int FUN_1013f880(void);                         // the game's random number
extern "C" int FUN_10024750(int index, Vec4 *ground, float depth, int fromHead, const Vec4 *offset, int testOnly,
                            int smallObjects);              // Char_ProbeGround

// 0x10048050 (Npc_HoverCarrier). One frame of the carrier `index`; its rider is slot `rider` (set in state 0 from the
// talk id: rider = index + talk / 100, limits = talk % 100). A carrier that was woken puts its rider 45 units above
// its spine; a carrier whose rider is flagged is put under it. State 0 picks the rider, a group number and the home
// position and finds the spine node. State 1 floats: every 30th frame it probes the ground under the player, every
// 15th the ground under itself, it flies towards the player (kept above the ground and the player and below the
// player's ceiling, tied to home by the limit bits), smooths its velocity and bounces off obstacles with a random
// turn. State 4 resets the rider and the carrier. It enters state 4 when its rider is dead or gone. Returns 0.
extern "C" int FUN_10048050(int index)
{
    Vec4 reference;
    Vec4 self;
    Vec4 selfCopy;
    reference.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    selfCopy.Copy(self);
    unsigned char level = DAT_102550e0[index];
    DAT_10db6ee8[index].mode = 0;
    DAT_10255a88[index] = 0;
    if (level != 0)
    {
        if (DAT_10210d30[index] == 0 && DAT_1020b430[index].rider != 0 &&
            DAT_10db6ee8[DAT_1020b430[index].rider].root != 0 && DAT_10210d30[DAT_1020b430[index].rider] == 0 &&
            DAT_10daaae8[DAT_1020b430[index].rider].stats[STAT_HEALTH] > 0)
        {
            Vec4 place;
            if (DAT_1021bdf8[index][0] != 0 &&
                (DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation())
                        .LengthSquared3() < HOVER_RIDER_RANGE_SQUARED)
                place.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
            else
                place.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            DAT_10db6ee8[DAT_1020b430[index].rider].SetPosition(place.x, place.y, place.z + HOVER_RIDER_LIFT);
        }
    }
    else
    {
        if (DAT_10210b30[index] != 0)
        {
            DAT_10210b30[index] = 0;
            DAT_10db6ee8[index].alpha = 1.0f;
        }
        if (DAT_10315350[index] == 0)
        {
            if (DAT_1020b430[index].rider != 0 && DAT_10315350[DAT_1020b430[index].rider] != 0)
            {
                reference.Copy(self - DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                reference.Copy(DAT_10db6ee8[DAT_1020b430[index].rider].matrix.GetTranslation() + reference);
                reference.z -= HOVER_RIDER_LIFT;
                DAT_10db6ee8[index].SetPosition(reference.x, reference.y, reference.z);
                return 0;
            }
            if (DAT_104c0380 % HOVER_PROBE_PERIOD == index % HOVER_PROBE_PERIOD)
            {
                Vec4 from;
                Vec4 to;
                Vec4 hit;
                from.Copy(reference);
                to.Copy(reference);
                to.z += HOVER_PLAYER_PROBE_HEIGHT;
                from.z = HOVER_PLAYER_PROBE_FLOOR;
                if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment(0, 4, &from, &to, &hit, 0) != 0)
                    DAT_1020b430[index].playerGround = (int)hit.z;
            }
            if (DAT_1020b430[index].state == 0)
            {
                int talk = DAT_1020b430[index].talk;
                int rider = talk / 100 + index;
                int group = 1;
                DAT_1020b430[index].rider = rider;
                DAT_1020b430[index].hoverLimits = talk % 100;
                DAT_1020b430[index].state = 1;
                int i = HOVER_GROUP_FIRST;
                do
                {
                    if (group <= DAT_10255f10[i] && DAT_10255f10[i] < HOVER_GROUP_LIMIT)
                        group = DAT_10255f10[i] + 1;
                    if (group <= DAT_10255f10[i + 1] && DAT_10255f10[i + 1] < HOVER_GROUP_LIMIT)
                        group = DAT_10255f10[i + 1] + 1;
                    i += 2;
                } while (i < HOVER_GROUP_END);
                DAT_10255f10[index] = group;
                DAT_10255f10[rider] = group;
                DAT_10802bf8[index].step.Copy(self);
                DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find("@Bip01 Spine", 0);
            }
            else if (DAT_1020b430[index].state == 1)
            {
                DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
                Vec4 offset;
                offset.Copy(reference - self);
                if (DAT_104c0380 % HOVER_GROUND_PERIOD == index % HOVER_GROUND_PERIOD)
                {
                    Vec4 ground;
                    Vec4 lift;
                    lift.z = HOVER_GROUND_PROBE_LIFT;
                    if (FUN_10024750(index, &ground, HOVER_GROUND_PROBE_DEPTH, 0, &lift, 0, 0) != 0)
                        DAT_1020b430[index].hoverGround = (int)ground.z;
                    else
                        DAT_1020b430[index].hoverGround = HOVER_NO_GROUND;
                }
                if (self.z + offset.z < (float)DAT_1020b430[index].hoverGround + HOVER_GROUND_CLEARANCE)
                    offset.z = (float)DAT_1020b430[index].hoverGround + HOVER_GROUND_CLEARANCE - self.z;
                if (self.z + offset.z < reference.z + HOVER_PLAYER_CLEARANCE)
                    offset.z = reference.z + HOVER_PLAYER_CLEARANCE - self.z;
                if (self.z + offset.z > (float)(DAT_1020b430[index].playerGround + HOVER_PLAYER_CEILING))
                    offset.z = (float)(DAT_1020b430[index].playerGround + HOVER_PLAYER_CEILING) - self.z;
                DAT_10db6ee8[index].targetHeading = atan2f(offset.x, offset.y) + HOVER_PI;
                if (offset.Length() < HOVER_PLAYER_NEAR)
                {
                    float height = offset.z;
                    offset.Copy(offset * -1.0f);
                    offset.z = height;
                }
                int limits = DAT_1020b430[index].hoverLimits;
                if ((limits & HOVER_LIMIT_PIN_X) != 0)
                    offset.x = DAT_10802bf8[index].step.x - self.x;
                if ((limits & HOVER_LIMIT_PIN_Y) != 0)
                    offset.y = DAT_10802bf8[index].step.y - self.y;
                if ((limits & HOVER_LIMIT_MAX_X) != 0 && self.x + offset.x > DAT_10802bf8[index].step.x)
                    offset.x = DAT_10802bf8[index].step.x - self.x;
                if ((limits & HOVER_LIMIT_MAX_Y) != 0 && self.y + offset.y > DAT_10802bf8[index].step.y)
                    offset.y = DAT_10802bf8[index].step.y - self.y;
                if ((limits & HOVER_LIMIT_MIN_X) != 0 && self.x + offset.x < DAT_10802bf8[index].step.x)
                    offset.x = DAT_10802bf8[index].step.x - self.x;
                if ((limits & HOVER_LIMIT_MIN_Y) != 0 && self.y + offset.y < DAT_10802bf8[index].step.y)
                    offset.y = DAT_10802bf8[index].step.y - self.y;
                offset.Normalized3();
                offset.z *= 2.0f;
                offset.Copy(offset * HOVER_SPEED);
                if (DAT_1020b430[index].hoverLowered != 0)
                    offset.z -= HOVER_LOWERED_DROP;
                offset.Copy(offset * HOVER_VELOCITY_TAKE + DAT_10802bf8[index].position * HOVER_VELOCITY_KEEP);
                DAT_10802bf8[index].position.Copy(offset);
                FUN_100241e0(index, offset.x, offset.y, offset.z);
                if (DAT_10db6ee8[index].IsClearOfMapLimited(&DAT_10802bf8[index].position) == 0)
                {
                    float pitch;
                    if (DAT_10802bf8[index].position.Length() > 1.0f)
                    {
                        if (DAT_10802bf8[index].position.z > 0.0f)
                            pitch = acosf(DAT_10802bf8[index].position.Length2D() / DAT_10802bf8[index].position.Length());
                        else
                            pitch = -acosf(DAT_10802bf8[index].position.Length2D() / DAT_10802bf8[index].position.Length());
                    }
                    else
                        pitch = 0.0f;
                    float bounce;
                    float heading = atan2f(DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y);
                    if (DAT_10802bf8[index].position.Length() * 2.0f < HOVER_BOUNCE_SPEED_LIMIT)
                        bounce = DAT_10802bf8[index].position.Length() * 2.0f;
                    else
                        bounce = HOVER_BOUNCE_SPEED_LIMIT;
                    DAT_10802bf8[index].position.x = 0.0f;
                    DAT_10802bf8[index].position.z = 0.0f;
                    DAT_10802bf8[index].position.y = -bounce;
                    pitch += (float)(FUN_1013f880() % HOVER_BOUNCE_SPREAD - HOVER_BOUNCE_SPREAD / 2) * HOVER_PI / 360.0f;
                    heading += (float)(FUN_1013f880() % HOVER_BOUNCE_SPREAD - HOVER_BOUNCE_SPREAD / 2) * HOVER_PI / 360.0f;
                    DAT_10802bf8[index].position.RotateX(-pitch);
                    DAT_10802bf8[index].position.RotateZ(-heading);
                }
            }
            else if (DAT_1020b430[index].state == 4)
            {
                int rider = DAT_1020b430[index].rider;
                if (rider != 0 && DAT_10db6ee8[rider].root != 0 && DAT_10210d30[rider] == 0 &&
                    DAT_10daaae8[rider].stats[STAT_HEALTH] > 0)
                    FUN_10043cc0(rider);
                FUN_10043cc0(index);
                Vec4 place;
                DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * HOVER_RESET_DAMPING);
                place.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + DAT_10802bf8[index].position);
                DAT_10db6ee8[index].SetPosition(place.x, place.y, place.z);
            }
            if (DAT_10210d30[index] == 0 && DAT_1020b430[index].rider != 0 &&
                DAT_10db6ee8[DAT_1020b430[index].rider].root != 0 && DAT_10210d30[DAT_1020b430[index].rider] == 0 &&
                DAT_10daaae8[DAT_1020b430[index].rider].stats[STAT_HEALTH] > 0)
            {
                Vec4 place;
                if (DAT_1021bdf8[index][0] != 0 &&
                    (DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() -
                     DAT_10db6ee8[index].matrix.GetTranslation()).LengthSquared3() < HOVER_RIDER_RANGE_SQUARED)
                    place.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation());
                else
                    place.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
                DAT_10db6ee8[DAT_1020b430[index].rider].SetPosition(place.x, place.y, place.z + HOVER_RIDER_LIFT);
                DAT_10254598[DAT_1020b430[index].rider] = 0;
                return 0;
            }
            if (DAT_10210d30[DAT_1020b430[index].rider] == 0 ||
                DAT_10daaae8[DAT_1020b430[index].rider].stats[STAT_HEALTH] != 0)
                DAT_1020b430[index].state = 4;
        }
    }
    return 0;
}
