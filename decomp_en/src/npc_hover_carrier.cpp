// A flying carrier that hovers beside the player and carries another character (EN build; JP: decomp/src/npc_hover_carrier.cpp,
// behaviour 0x21). The rider is the character `talk / 100` slots after the carrier. The carrier floats to the player's side
// with a smoothed velocity that is kept between the ground and the player's height, bounces off walls, and the rider is held
// above it; when the rider is gone the carrier is reset.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_move.h"
#include "char_shots.h"
#include "collide_query.h"
#include "engine.h"
#include "node.h"
#include "npc_hover_carrier.h"
#include "vec4.h"
#include "visibility.h"

// CharState::values indices the carrier keeps its state in.
const int HOVER_VALUE_TALK = 1;           // talk id: rider = index + talk / 100, limits = talk % 100
const int HOVER_VALUE_LIMITS = 2;         // talk % 100 = bits that tie the carrier to its home position
const int HOVER_VALUE_GROUND = 3;         // height of the ground under the carrier (HOVER_NO_GROUND = none)
const int HOVER_VALUE_LOWERED = 4;        // non-zero = the carrier sinks while it floats
const int HOVER_VALUE_RIDER = 6;          // slot of the rider, 0 = none
const int HOVER_VALUE_PLAYER_GROUND = 7;  // height of the ground under the player

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


// Npc_HoverCarrier. One frame of the carrier `index`; its rider is slot `rider` (set in state 0 from the
// talk id: rider = index + talk / 100, limits = talk % 100). A carrier that was woken puts its rider 45 units above
// its spine; a carrier whose rider is flagged is put under it. State 0 picks the rider, a group number and the home
// position and finds the spine node. State 1 floats: every 30th frame it probes the ground under the player, every
// 15th the ground under itself, it flies towards the player (kept above the ground and the player and below the
// player's ceiling, tied to home by the limit bits), smooths its velocity and bounces off obstacles with a random
// turn. State 4 resets the rider and the carrier. It enters state 4 when its rider is dead or gone. Returns 0.
int Npc_HoverCarrier(int index)
{
    Vec4 reference;
    Vec4 self;
    Vec4 selfCopy;
    reference.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    selfCopy.Copy(self);
    unsigned char level = g_charHitLevel[index];
    g_characterModels[index].mode = 0;
    g_charTalkPending[index] = 0;
    if (level != 0)
    {
        if (g_charPhase[index] == 0 && g_charState[index].values[HOVER_VALUE_RIDER] != 0 &&
            g_characterModels[g_charState[index].values[HOVER_VALUE_RIDER]].root != 0 && g_charPhase[g_charState[index].values[HOVER_VALUE_RIDER]] == 0 &&
            g_charRecords[g_charState[index].values[HOVER_VALUE_RIDER]].stats[STAT_HEALTH] > 0)
        {
            Vec4 place;
            if (g_charNodes[index][0] != 0 &&
                (g_charNodes[index][0]->worldMatrix.GetTranslation() - g_characterModels[index].matrix.GetTranslation())
                        .LengthSquared3() < HOVER_RIDER_RANGE_SQUARED)
                place.Copy(g_charNodes[index][0]->worldMatrix.GetTranslation());
            else
                place.Copy(g_characterModels[index].matrix.GetTranslation());
            g_characterModels[g_charState[index].values[HOVER_VALUE_RIDER]].SetPosition(place.x, place.y, place.z + HOVER_RIDER_LIFT);
        }
    }
    else
    {
        if (g_charToldToReact[index] != 0)
        {
            g_charToldToReact[index] = 0;
            g_characterModels[index].alpha = 1.0f;
        }
        if (g_charSpawnFlag[index] == 0)
        {
            if (g_charState[index].values[HOVER_VALUE_RIDER] != 0 && g_charSpawnFlag[g_charState[index].values[HOVER_VALUE_RIDER]] != 0)
            {
                reference.Copy(self - g_charNodes[index][0]->worldMatrix.GetTranslation());
                reference.Copy(g_characterModels[g_charState[index].values[HOVER_VALUE_RIDER]].matrix.GetTranslation() + reference);
                reference.z -= HOVER_RIDER_LIFT;
                g_characterModels[index].SetPosition(reference.x, reference.y, reference.z);
                return 0;
            }
            if (g_frameCounter % HOVER_PROBE_PERIOD == index % HOVER_PROBE_PERIOD)
            {
                Vec4 from;
                Vec4 to;
                Vec4 hit;
                from.Copy(reference);
                to.Copy(reference);
                to.z += HOVER_PLAYER_PROBE_HEIGHT;
                from.z = HOVER_PLAYER_PROBE_FLOOR;
                if (g_mapRootNode != 0 && g_mapRootNode->CastSegment(0, 4, &from, &to, &hit, 0) != 0)
                    g_charState[index].values[HOVER_VALUE_PLAYER_GROUND] = (int)hit.z;
            }
            if (g_charState[index].state == 0)
            {
                int talk = g_charState[index].values[HOVER_VALUE_TALK];
                int rider = talk / 100 + index;
                int group = 1;
                g_charState[index].values[HOVER_VALUE_RIDER] = rider;
                g_charState[index].values[HOVER_VALUE_LIMITS] = talk % 100;
                g_charState[index].state = 1;
                int i = HOVER_GROUP_FIRST;
                do
                {
                    if (group <= g_charGroup[i] && g_charGroup[i] < HOVER_GROUP_LIMIT)
                        group = g_charGroup[i] + 1;
                    if (group <= g_charGroup[i + 1] && g_charGroup[i + 1] < HOVER_GROUP_LIMIT)
                        group = g_charGroup[i + 1] + 1;
                    i += 2;
                } while (i < HOVER_GROUP_END);
                g_charGroup[index] = group;
                g_charGroup[rider] = group;
                g_shotSlots[index].step.Copy(self);
                g_charNodes[index][0] = g_characterModels[index].root->Find("@Bip01 Spine", 0);
            }
            else if (g_charState[index].state == 1)
            {
                g_characterModels[index].PlayMotion(2, 0, 5, 1.0f);
                Vec4 offset;
                offset.Copy(reference - self);
                if (g_frameCounter % HOVER_GROUND_PERIOD == index % HOVER_GROUND_PERIOD)
                {
                    Vec4 ground;
                    Vec4 lift;
                    lift.z = HOVER_GROUND_PROBE_LIFT;
                    if (Char_CastRayDown(index, &ground, HOVER_GROUND_PROBE_DEPTH, 0, &lift, 0, 0) != 0)
                        g_charState[index].values[HOVER_VALUE_GROUND] = (int)ground.z;
                    else
                        g_charState[index].values[HOVER_VALUE_GROUND] = HOVER_NO_GROUND;
                }
                if (self.z + offset.z < (float)g_charState[index].values[HOVER_VALUE_GROUND] + HOVER_GROUND_CLEARANCE)
                    offset.z = (float)g_charState[index].values[HOVER_VALUE_GROUND] + HOVER_GROUND_CLEARANCE - self.z;
                if (self.z + offset.z < reference.z + HOVER_PLAYER_CLEARANCE)
                    offset.z = reference.z + HOVER_PLAYER_CLEARANCE - self.z;
                if (self.z + offset.z > (float)(g_charState[index].values[HOVER_VALUE_PLAYER_GROUND] + HOVER_PLAYER_CEILING))
                    offset.z = (float)(g_charState[index].values[HOVER_VALUE_PLAYER_GROUND] + HOVER_PLAYER_CEILING) - self.z;
                g_characterModels[index].targetHeading = atan2f(offset.x, offset.y) + HOVER_PI;
                if (offset.Length() < HOVER_PLAYER_NEAR)
                {
                    float height = offset.z;
                    offset.Copy(offset * -1.0f);
                    offset.z = height;
                }
                int limits = g_charState[index].values[HOVER_VALUE_LIMITS];
                if ((limits & HOVER_LIMIT_PIN_X) != 0)
                    offset.x = g_shotSlots[index].step.x - self.x;
                if ((limits & HOVER_LIMIT_PIN_Y) != 0)
                    offset.y = g_shotSlots[index].step.y - self.y;
                if ((limits & HOVER_LIMIT_MAX_X) != 0 && self.x + offset.x > g_shotSlots[index].step.x)
                    offset.x = g_shotSlots[index].step.x - self.x;
                if ((limits & HOVER_LIMIT_MAX_Y) != 0 && self.y + offset.y > g_shotSlots[index].step.y)
                    offset.y = g_shotSlots[index].step.y - self.y;
                if ((limits & HOVER_LIMIT_MIN_X) != 0 && self.x + offset.x < g_shotSlots[index].step.x)
                    offset.x = g_shotSlots[index].step.x - self.x;
                if ((limits & HOVER_LIMIT_MIN_Y) != 0 && self.y + offset.y < g_shotSlots[index].step.y)
                    offset.y = g_shotSlots[index].step.y - self.y;
                offset.Normalized3();
                offset.z *= 2.0f;
                offset.Copy(offset * HOVER_SPEED);
                if (g_charState[index].values[HOVER_VALUE_LOWERED] != 0)
                    offset.z -= HOVER_LOWERED_DROP;
                offset.Copy(offset * HOVER_VELOCITY_TAKE + g_shotSlots[index].position * HOVER_VELOCITY_KEEP);
                g_shotSlots[index].position.Copy(offset);
                Character_SetMove(index, offset.x, offset.y, offset.z);
                if (g_characterModels[index].IsClearOfMapLimited(&g_shotSlots[index].position) == 0)
                {
                    float pitch;
                    if (g_shotSlots[index].position.Length() > 1.0f)
                    {
                        if (g_shotSlots[index].position.z > 0.0f)
                            pitch = acosf(g_shotSlots[index].position.Length2D() / g_shotSlots[index].position.Length());
                        else
                            pitch = -acosf(g_shotSlots[index].position.Length2D() / g_shotSlots[index].position.Length());
                    }
                    else
                        pitch = 0.0f;
                    float bounce;
                    float heading = atan2f(g_shotSlots[index].position.x, g_shotSlots[index].position.y);
                    if (g_shotSlots[index].position.Length() * 2.0f < HOVER_BOUNCE_SPEED_LIMIT)
                        bounce = g_shotSlots[index].position.Length() * 2.0f;
                    else
                        bounce = HOVER_BOUNCE_SPEED_LIMIT;
                    g_shotSlots[index].position.x = 0.0f;
                    g_shotSlots[index].position.z = 0.0f;
                    g_shotSlots[index].position.y = -bounce;
                    pitch += (float)(Random() % HOVER_BOUNCE_SPREAD - HOVER_BOUNCE_SPREAD / 2) * HOVER_PI / 360.0f;
                    heading += (float)(Random() % HOVER_BOUNCE_SPREAD - HOVER_BOUNCE_SPREAD / 2) * HOVER_PI / 360.0f;
                    g_shotSlots[index].position.RotateX(-pitch);
                    g_shotSlots[index].position.RotateZ(-heading);
                }
            }
            else if (g_charState[index].state == 4)
            {
                int rider = g_charState[index].values[HOVER_VALUE_RIDER];
                if (rider != 0 && g_characterModels[rider].root != 0 && g_charPhase[rider] == 0 &&
                    g_charRecords[rider].stats[STAT_HEALTH] > 0)
                    Char_ResetState(rider);
                Char_ResetState(index);
                Vec4 place;
                g_shotSlots[index].position.Copy(g_shotSlots[index].position * HOVER_RESET_DAMPING);
                place.Copy(g_characterModels[index].matrix.GetTranslation() + g_shotSlots[index].position);
                g_characterModels[index].SetPosition(place.x, place.y, place.z);
            }
            if (g_charPhase[index] == 0 && g_charState[index].values[HOVER_VALUE_RIDER] != 0 &&
                g_characterModels[g_charState[index].values[HOVER_VALUE_RIDER]].root != 0 && g_charPhase[g_charState[index].values[HOVER_VALUE_RIDER]] == 0 &&
                g_charRecords[g_charState[index].values[HOVER_VALUE_RIDER]].stats[STAT_HEALTH] > 0)
            {
                Vec4 place;
                if (g_charNodes[index][0] != 0 &&
                    (g_charNodes[index][0]->worldMatrix.GetTranslation() -
                     g_characterModels[index].matrix.GetTranslation()).LengthSquared3() < HOVER_RIDER_RANGE_SQUARED)
                    place.Copy(g_charNodes[index][0]->worldMatrix.GetTranslation());
                else
                    place.Copy(g_characterModels[index].matrix.GetTranslation());
                g_characterModels[g_charState[index].values[HOVER_VALUE_RIDER]].SetPosition(place.x, place.y, place.z + HOVER_RIDER_LIFT);
                g_charAwakeTimer[g_charState[index].values[HOVER_VALUE_RIDER]] = 0;
                return 0;
            }
            if (g_charPhase[g_charState[index].values[HOVER_VALUE_RIDER]] == 0 ||
                g_charRecords[g_charState[index].values[HOVER_VALUE_RIDER]].stats[STAT_HEALTH] != 0)
                g_charState[index].state = 4;
        }
    }
    return 0;
}
