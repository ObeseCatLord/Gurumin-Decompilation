// Partner_Follow: the per-frame update of a partner (a helper the player picks up: slots 6, 7, 0xb, 0x10 and the show
// partner 0x19) in the party line (enemy_queue.h). Enemy_IsBusy calls it for every partner in the line with the
// character it follows: the player for the front one, else the partner ahead.

#include <string.h>

#include "last_position.h"
#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_tables.h"

#include "char_vars.h"

#include "map_vars.h"

#include "game_vars.h"

#include "camera_vars.h"

#include "enemy_chase_hop.h"

#include "partner_skill.h"

#include "enemy_queue.h"

#include "hikari_button.h"
#include "doppler_loops.h"
#include "char_hikari.h"
#include "mole_game.h"
#include "partner_follow.h"

#define STEP_APPEAR 0              // not arrived yet: appears at the entry point and fades in
#define STEP_GATHER 1              // far from the leader: heads for it
#define STEP_FOLLOW 2              // follows the leader (Enemy_ChaseAndHop)
#define STEP_FADE_OUT 3            // fades out, then looks for a new place near the leader
#define STEP_FADE_IN 4             // fades in
#define STEP_FIND_PLACE 5          // looks for a free place near the leader
#define STEP_SKILL 6               // uses its special skill, riding the leader's node
#define STEP_LEAVE 7               // says goodbye, then leaves the line
#define FADE_FRAMES 15             // frames of a fade
#define FADE_FRAMES_F 15.0f
#define GATHER_DISTANCE 400.0f     // a partner further than this from its leader hurries
#define UNSET_X -20000.0f          // a leader at or below this x is not on the map yet
#define MAP_NO_PLACE 0x191         // no placing on these maps (and map 0xc, and the first five)
#define MAP_NO_PLACE2 0xc
#define MAP_FIRST_PLACED 5
#define PLACE_ATTEMPTS 5
#define PARKED_X -200000.0f        // where a partner with no place is put
#define STAND_MOTION 2
#define STAND_BLEND 5
#define SPOT_NEAR_SKILL 100.0f     // distance from the leader of a new place while the skill is busy
#define SPOT_NEAR 300.0f           // and otherwise
#define PROBE_FAR_SKILL 200.0f     // distance of the far end of the wall probe
#define PROBE_FAR 400.0f
#define PROBE_LIFT 50.0f
#define PROBE_START_LIFT 25.0f
#define PROBE_DEPTH 500.0f
#define PROBE_FLOOR_LIFT 5.0f
#define HIT_BACKOFF 100.0f
#define SPAWN_BEHIND -10.0f
#define PI_APPROX 3.1415f
#define SKILL_NODE_LIFT 10.0f
#define FAREWELL_FRAMES 60
#define OWNER_SHOW 0x7d            // effect owner of the show partner's entrance
#define SHOW_ENTRY_EFFECT 0x7bf
#define SHOW_VOICE 0x18e
#define SHOW_CAMERA_ZOOM 2.0f
#define RING_COUNT 6
#define RING_STEP 1.0471975f       // pi / 3
#define EXIT_BURST_FRAME 0x55      // frame of the exit show with the big burst
#define EXIT_RING_END 0x69         // rings are thrown every EXIT_RING_PERIOD frames until here
#define EXIT_RING_PERIOD 15
#define EXIT_DIM_FRAME 0x37        // frame of the leaving show that dims the screen
#define EXIT_BRIGHT_FRAME 0x2d     // and the frame that flashes it white

// The messages the partners say when they go (Shift-JIS) and the show partner's entrance line and name.
#define HEALER_LEAVE_A1 "\x82\xbb\x82\xeb\x82\xbb\x82\xeb\x8e\x9e\x8a\xd4\x82\xcc\x82\xe6\x82\xa4\x82\xc5\x82\xb7\x81" "B"
#define HEALER_LEAVE_A2 "\x83\x8f\x83^\x83N\x83V\x82\xcd\x8b" "A\x82\xe7\x82\xb9\x82\xc4\x82\xa2\x82\xbd\x82\xbe\x82\xab\x82\xdc\x82\xb7"
#define HEALER_LEAVE_B1 "\x83\x8f\x83^\x83N\x83V\x82\xcd\x82\xe0\x82\xa4\x94\xe6\x82\xea\x89\xca\x82\xc4\x82\xdc\x82\xb5\x82\xbd\x81" "B"
#define HEALER_LEAVE_B2 "\x8c\xe3\x82\xcd\x8a\xe6\x92\xa3\x82\xc1\x82\xc4\x82\xad\x82\xbe\x82\xb3\x82\xa2"
#define SHIELD_LEAVE_A1 "\x8b" "A\x82\xe9\x8e\x9e\x8a\xd4\x82\xcc\x82\xe6\x82\xa4\x82\xbe\x81" "B"
#define SHIELD_LEAVE_A2 "\x8a\xe6\x92\xa3\x82\xea"
#define SHIELD_LEAVE_B1 "\x97\xac\x90\xce\x82\xc9\x94\xe6\x82\xea\x82\xbd\x82\xc8\x81" "B"
#define SHIELD_LEAVE_B2 "\x82\xe0\x82\xa4\x8b" "A\x82\xe9\x82\xe6"
#define BLASTER_LEAVE_A1 "\x82\xbb\x82\xeb\x82\xbb\x82\xeb\x8b" "A\x82\xe7\x82\xc8\x82\xad\x82\xbf\x82\xe1\x81" "B"
#define BLASTER_LEAVE_A2 "\x8a\xe6\x92\xa3\x82\xc1\x82\xc4\x82\xcb"
#define BLASTER_LEAVE_B1 "\x82\xa8\x82\xc8\x82\xa9\x82\xaa\x82\xb7\x82\xa2\x82\xbd\x82\xa9\x82\xe7\x8b" "A\x82\xe9\x82\xe6"
#define HOMING_LEAVE_A1 "\x8c\xe3\x82\xcd\x94" "C\x82\xb9\x82\xbd\x82\xdb"
#define HOMING_LEAVE_B1 "\x82\xe0\x82\xa4\x94\xe6\x82\xea\x82\xbd\x82\xdb\x81" "B"
#define HOMING_LEAVE_B2 "\x8b" "A\x82\xe9\x82\xdb"
#define GALAXY_LEAVE "\x82\xb3\x82\xe7\x82\xce\x82\xbe"
#define SHOW_LINE "\x8c\xa9\x82\xb9\x82\xc4\x82\xa0\x82\xb0\x82\xe9\x82\xe6\x81" "B"
#define SHOW_NAME "\x83l\x83I\x81" "E\x83M\x83\x83\x83\x89\x83N\x83V\x81[\x82r"

// 0x100e0320 (Partner_Follow, a C++ function: its frame has EH registration for the effect matrices). One frame of the
// partner in slot `index`, following character `leader`. The first partner to be updated while nobody is registered
// clears its state and registers as DAT_10c66cd8. A partner told to leave (CharState::fadeOut) goes to step 7 once
// its farewell is over. Steps: 0 puts it at the entry point and starts the fade in; 1 and 2 follow the leader (2
// through the chase); 3 and 4 fade out and in; 5 tries up to five random places around the leader (probing the map
// for walls and ground) and fades in at the first free one; 6 runs the special skill at the leader and rides its
// node; 7 reads the farewell (60 frames), then empties the line. Slot 0x19 dims the fog and runs an exit show with
// rings of effects. Returns 0.
int Partner_Follow(int index, int leader)
{
    if (DAT_10c66cd8 == 0)
    {
        memset(&DAT_1020b430[index], 0, sizeof(CharState));
        DAT_10c66cd8 = index;
    }
    if (DAT_104bf888 == index + QUEUE_KEEP_TARGET)
    {
        DAT_104bf888 = 0;
        DAT_104bfd80 = 0;
    }
    Vec4 leaderPos;
    Vec4 ownPos;
    leaderPos.Copy(DAT_10db6ee8[leader].matrix.GetTranslation());
    ownPos.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    if (DAT_1020b430[index].fadeOut != 0 && DAT_1020b430[index].farewellFrames <= 0 &&
        (index != PARTNER_HEALER || DAT_1020b5d4 > 99 || DAT_1020b5d4 == 0))
    {
        DAT_1020b430[index].state = STEP_LEAVE;
        DAT_1020b430[index].fadeOut = 0;
    }
    int *state = &DAT_1020b430[index].state;
    int step = *state;
    if (step == STEP_APPEAR)
    {
        if (leaderPos.x > UNSET_X && DAT_10c62ed0 != MAP_NO_PLACE && DAT_10c62ed0 > MAP_FIRST_PLACED &&
            DAT_10c62ed0 != MAP_NO_PLACE2)
        {
            DAT_10db6ee8[index].SetPosition((float)DAT_10c66cdc[0], (float)DAT_10c66cdc[1], (float)DAT_10c66cdc[2]);
            DAT_10802bf8[index].position.Fill(0.0f);
            DAT_10255160[index][0] = 0.0f;
            *state = STEP_FOLLOW;
            DAT_1020b430[index].wait = 0;
            DAT_10db6ee8[index].root->M_10194630(index, QUEUE_ROOT_KIND_MASK);
            DAT_10db6ee8[index].root->M_101946b0(0xfe, QUEUE_ROOT_KIND_MASK);
            DAT_10db6ee8[index].root->M_10194730(0x86, QUEUE_ROOT_KIND_MASK);
            DAT_10db6ee8[index].root->M_10194730(0x86, QUEUE_ROOT_KIND_MASK2);
            DAT_10db6ee8[index].m_2390 = 2;
            DAT_10db6ee8[index].alpha = 0.0f;
            *state = STEP_FADE_IN;
            DAT_1020b430[index].partnerFade = 0;
            if (index == PARTNER_HOMING)
            {
                FUN_10065b10(index, 0x52, 1, 1);
                DAT_1021c03c = DAT_10db6ee8[PARTNER_HOMING].root->Find("Bip01 Footsteps", 0);
                DAT_1020b85c = 0x4b;
                DAT_1020b430[index].sawPlayer = 1;
                return 0;
            }
            if (index == PARTNER_HEALER)
            {
                FUN_10065b10(index, 0x53, 1, 1);
                DAT_1021bed4 = DAT_10db6ee8[PARTNER_HEALER].root->Find("Bip01", 0);
                DAT_1020b430[index].sawPlayer = 1;
                return 0;
            }
            if (index == PARTNER_BLASTER)
            {
                FUN_10065b10(index, 0x51, 1, 1);
                DAT_1021bf88 = DAT_10db6ee8[PARTNER_BLASTER].root->Find("Bip01 Head", 0);
                DAT_1021bf8c = DAT_10db6ee8[leader].root->Find("@Dummy05", 0);
                DAT_1020b430[index].sawPlayer = 1;
                return 0;
            }
            if (index == PARTNER_SHIELD)
            {
                FUN_10065b10(index, 0x50, 1, 1);
                DAT_1020b430[index].sawPlayer = 1;
                return 0;
            }
            if (index == PARTNER_GALAXY)
            {
                EffectOptions options;
                options.owner = OWNER_SHOW;
                Vec4 position;
                DAT_1086adb8.SetIdentity();
                DAT_1086adb8.SetTranslation(DAT_118e6cb8);
                FUN_1001d8b0(index, SHOW_ENTRY_EFFECT, position, -1, -1, &DAT_1086adb8, 0, 0, 0, &options);
                if (DAT_10255f08 != 0 && _stricmp(DAT_10255f08->name, "fogset") == 0)
                {
                    DAT_108689b8.x = DAT_10255f08->fogColour.x;
                    DAT_108689b8.y = DAT_10255f08->fogColour.y;
                    DAT_108689b8.z = DAT_10255f08->fogColour.z;
                    DAT_108689b8.w = DAT_10255f08->fogColour.w;
                    DAT_10255f08->fogColour.Fill(0.0f);
                }
                if (DAT_10255f0c != 0 && _stricmp(DAT_10255f0c->name, "fogset") == 0)
                {
                    DAT_108689c8.x = DAT_10255f0c->fogColour.x;
                    DAT_108689c8.y = DAT_10255f0c->fogColour.y;
                    DAT_108689c8.z = DAT_10255f0c->fogColour.z;
                    DAT_108689c8.w = DAT_10255f0c->fogColour.w;
                    DAT_10255f0c->fogColour.Fill(0.0f);
                }
                DAT_102552f8 = DAT_1020344c;
                DAT_1020344c = SHOW_CAMERA_ZOOM;
                FUN_10056260(PARTNER_GALAXY, SHOW_LINE, SHOW_NAME, "", "");
                FUN_1003ad20(SHOW_VOICE, 0, 1.0f);
            }
            DAT_1020b430[index].sawPlayer = 1;
            return 0;
        }
    }
    else if (step == STEP_GATHER)
    {
        if ((leaderPos - ownPos).Length() > GATHER_DISTANCE)
            *state = STEP_FOLLOW;
        DAT_10db6ee8[index].PlayMotion(STAND_MOTION, 0, STAND_BLEND, 1.0f);
    }
    else if (step == STEP_FOLLOW)
        FUN_100bcce0(index, leader, &leaderPos, &ownPos);
    else if (step == STEP_FADE_OUT)
    {
        int frame = ++DAT_1020b430[index].partnerFade;
        DAT_10db6ee8[index].alpha = (float)(FADE_FRAMES - frame) / FADE_FRAMES_F;
        if (frame == FADE_FRAMES)
        {
            *state = STEP_FIND_PLACE;
            DAT_1020b430[index].partnerFade = 0;
            DAT_10db6ee8[index].alpha = 0.0f;
        }
    }
    else if (step == STEP_FADE_IN)
    {
        int frame = ++DAT_1020b430[index].partnerFade;
        DAT_10db6ee8[index].alpha = (float)frame / FADE_FRAMES_F;
        if (frame == FADE_FRAMES)
        {
            *state = STEP_FOLLOW;
            DAT_1020b430[index].partnerFade = 0;
            DAT_10db6ee8[index].alpha = 1.0f;
            if (index == PARTNER_BLASTER)
                DAT_1020b6f0 = 6;
        }
    }
    else if (step == STEP_FIND_PLACE)
    {
        if (leaderPos.x > UNSET_X && DAT_10c62ed0 != MAP_NO_PLACE && DAT_10c62ed0 > MAP_FIRST_PLACED &&
            DAT_10c62ed0 != MAP_NO_PLACE2)
        {
            int attempt = 0;
            do
            {
                attempt++;
                Vec4 spawn;
                Vec4 direction;
                spawn.y = SPAWN_BEHIND;
                spawn.RotateZ((float)(FUN_1013f880() % 360) / 180.0f * PI_APPROX);
                float leaderZ = leaderPos.z;
                spawn.z = 0.0f;
                direction.Copy(spawn.Normalized3());
                spawn.z = leaderZ;
                if (DAT_1020b430[index].specialPhase != 0)
                    ownPos.Copy(leaderPos + direction * SPOT_NEAR_SKILL);
                else
                    ownPos.Copy(leaderPos + direction * SPOT_NEAR);
                ownPos.z = spawn.z;
                Vec4 farPoint;
                Vec4 nearPoint;
                Vec4 hitPoint;
                if (DAT_1020b430[index].specialPhase != 0)
                    farPoint.Copy(leaderPos + direction * PROBE_FAR_SKILL);
                else
                    farPoint.Copy(leaderPos + direction * PROBE_FAR);
                farPoint.z = spawn.z + PROBE_LIFT;
                nearPoint.Copy(leaderPos);
                nearPoint.z += PROBE_LIFT;
                if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment(0, 4, &nearPoint, &farPoint, &hitPoint, 0))
                {
                    ownPos.Copy(hitPoint - direction * HIT_BACKOFF);
                    farPoint.Copy(ownPos);
                    ownPos.z -= PROBE_LIFT;
                }
                nearPoint.Copy(farPoint);
                nearPoint.z += PROBE_START_LIFT;
                farPoint.z -= PROBE_DEPTH;
                if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment(0, 4, &farPoint, &nearPoint, &hitPoint, 0))
                {
                    farPoint.z = hitPoint.z + PROBE_FLOOR_LIFT;
                    if (DAT_10255f08->CastSegment(0, 4, &farPoint, &nearPoint, &hitPoint, 0))
                        ownPos.z = hitPoint.z + PROBE_FLOOR_LIFT;
                    else
                        ownPos.z = farPoint.z;
                }
                else if (DAT_1020b430[index].specialPhase == 0)
                    continue;
                DAT_10db6ee8[index].SetPosition(ownPos.x, ownPos.y, ownPos.z);
                if (DAT_10db6ee8[index].IsClearOfMapLimited(0) && DAT_10db6ee8[index].IsClearAny(0))
                {
                    DAT_10802bf8[index].position.Fill(0.0f);
                    DAT_10255160[index][0] = 0.0f;
                    *state = STEP_FOLLOW;
                    DAT_1020b430[index].wait = 0;
                    DAT_10db6ee8[index].root->M_10194630(index, QUEUE_ROOT_KIND_MASK);
                    DAT_10db6ee8[index].root->M_101946b0(0xfe, QUEUE_ROOT_KIND_MASK);
                    DAT_10db6ee8[index].root->M_10194730(0x86, QUEUE_ROOT_KIND_MASK);
                    DAT_10db6ee8[index].root->M_10194730(0x86, QUEUE_ROOT_KIND_MASK2);
                    float alpha = DAT_10db6ee8[index].alpha;
                    DAT_10db6ee8[index].m_2390 = 2;
                    if (alpha < 1.0f)
                    {
                        *state = STEP_FADE_IN;
                        DAT_1020b430[index].partnerFade = 0;
                    }
                    return 0;
                }
                DAT_10db6ee8[index].SetPosition(PARKED_X, 0, 0);
            } while (attempt < PLACE_ATTEMPTS);
        }
    }
    else if (step == STEP_SKILL)
    {
        Vec4 steer;
        int busy;
        FUN_10078060(index, &leaderPos, &busy, &steer);
        ownPos.Copy(DAT_1021bdf8[index][2]->worldMatrix.GetTranslation());
        ownPos.z += SKILL_NODE_LIFT;
        DAT_10db6ee8[index].SetPosition(ownPos.x, ownPos.y, ownPos.z);
        DAT_10db6ee8[index].heading = DAT_10db6ee8[leader].heading;
        DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[leader].targetHeading;
        DAT_10db6ee8[index].mode = 0;
    }
    else if (step == STEP_LEAVE)
    {
        DAT_10db6ee8[index].PlayMotion(STAND_MOTION, 0, STAND_BLEND, 1.0f);
        if (index == PARTNER_GALAXY && DAT_1020ba9c > 0)
        {
            if (DAT_1020ba9c < FADE_FRAMES)
            {
                if (DAT_10255f08 != 0 && _stricmp(DAT_10255f08->name, "fogset") == 0)
                {
                    DAT_10255f08->fogColour.x = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689b8.x / FADE_FRAMES_F;
                    DAT_10255f08->fogColour.y = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689b8.y / FADE_FRAMES_F;
                    DAT_10255f08->fogColour.z = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689b8.z / FADE_FRAMES_F;
                    DAT_10255f08->fogColour.w = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689b8.w / FADE_FRAMES_F;
                }
                if (DAT_10255f0c != 0 && _stricmp(DAT_10255f0c->name, "fogset") == 0)
                {
                    DAT_10255f0c->fogColour.x = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689c8.x / FADE_FRAMES_F;
                    DAT_10255f0c->fogColour.y = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689c8.y / FADE_FRAMES_F;
                    DAT_10255f0c->fogColour.z = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689c8.z / FADE_FRAMES_F;
                    DAT_10255f0c->fogColour.w = (float)(FADE_FRAMES - DAT_1020ba9c) * DAT_108689c8.w / FADE_FRAMES_F;
                }
            }
            if (DAT_1020ba9c == EXIT_DIM_FRAME)
                FUN_1000ec70(10, 0.4f, 0.4f, 0.4f, 1.0f);
            if (DAT_1020ba9c == EXIT_BRIGHT_FRAME)
                FUN_1000ec70(PARTNER_GALAXY, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        if (DAT_1020b430[index].farewellFrames != 0)
        {
            if (DAT_1020b430[index].farewellFrames == 1)
            {
                if (index == PARTNER_GALAXY)
                {
                    DAT_1020344c = DAT_102552f8;
                    DAT_10daaa80.RemoveOwnedBy(OWNER_SHOW);
                    if (DAT_10255f08 != 0 && _stricmp(DAT_10255f08->name, "fogset") == 0)
                    {
                        DAT_10255f08->fogColour.x = DAT_108689b8.x;
                        DAT_10255f08->fogColour.y = DAT_108689b8.y;
                        DAT_10255f08->fogColour.z = DAT_108689b8.z;
                        DAT_10255f08->fogColour.w = DAT_108689b8.w;
                    }
                    if (DAT_10255f0c != 0 && _stricmp(DAT_10255f0c->name, "fogset") == 0)
                    {
                        DAT_10255f0c->fogColour.x = DAT_108689c8.x;
                        DAT_10255f0c->fogColour.y = DAT_108689c8.y;
                        DAT_10255f0c->fogColour.z = DAT_108689c8.z;
                        DAT_10255f0c->fogColour.w = DAT_108689c8.w;
                    }
                    DAT_10da90d0.Set(1.0f, 1.0f, 1.0f, 1.0f);
                    DAT_1042da68 = 0;
                }
                DAT_10c66cf4 = 0;
                memset(state, 0, sizeof(CharState));
                DAT_10c66cd4 = 0;
            }
        }
        else
        {
            switch (index)
            {
            case PARTNER_HOMING:
                if (FUN_1013f880() % 2 != 0)
                    FUN_10056260(index, HOMING_LEAVE_B1, HOMING_LEAVE_B2, "", "");
                else
                    FUN_10056260(index, HOMING_LEAVE_A1, "", "", "");
                DAT_1020b430[index].farewellFrames = FAREWELL_FRAMES;
                break;
            case PARTNER_BLASTER:
                if (FUN_1013f880() % 2 != 0)
                    FUN_10056260(index, BLASTER_LEAVE_B1, "", "", "");
                else
                    FUN_10056260(index, BLASTER_LEAVE_A1, BLASTER_LEAVE_A2, "", "");
                DAT_1020b430[index].farewellFrames = FAREWELL_FRAMES;
                break;
            case PARTNER_SHIELD:
                if (FUN_1013f880() % 2 != 0)
                    FUN_10056260(index, SHIELD_LEAVE_B1, SHIELD_LEAVE_B2, "", "");
                else
                    FUN_10056260(index, SHIELD_LEAVE_A1, SHIELD_LEAVE_A2, "", "");
                DAT_1020b430[index].farewellFrames = FAREWELL_FRAMES;
                break;
            case PARTNER_HEALER:
                if (FUN_1013f880() % 2 != 0)
                    FUN_10056260(index, HEALER_LEAVE_B1, HEALER_LEAVE_B2, "", "");
                else
                    FUN_10056260(index, HEALER_LEAVE_A1, HEALER_LEAVE_A2, "", "");
                DAT_1020b430[index].farewellFrames = FAREWELL_FRAMES;
                break;
            case PARTNER_GALAXY:
                DAT_1020b430[index].farewellFrames = FAREWELL_FRAMES;
                DAT_1020b430[index].sawPlayer = 0;
                FUN_10056260(index, GALAXY_LEAVE, "", "", "");
                break;
            }
        }
    }
    DAT_10802bf8[index].position.Copy(DAT_10db6ee8[index].move);
    DAT_10255160[index][0] = DAT_10802bf8[index].position.Length();
    if (index == PARTNER_GALAXY && DAT_1020baa8 > 0)
    {
        int fade = FADE_FRAMES - DAT_1020baa8;
        FUN_1000ec70(fade < 0 ? 0 : fade, 0.4f, 0.4f, 0.4f, 0.1f);
        DAT_1020baa8++;
        DAT_1086adb8.SetTranslation(DAT_118e6cb8);
        if (DAT_1020baa8 == EXIT_BURST_FRAME)
        {
            EffectOptions options;
            Matrix4 matrix;
            Vec4 position;
            matrix.SetIdentity();
            matrix.SetTranslation(DAT_10db6ee8[0].matrix.GetTranslation());
            options.m_0000 = 1;
            options.m_0004 = 100;
            options.m_0008 = 2.0f;
            options.m_000e = 1;
            DAT_10daaa80.Start(0x538, position, -1, -1, 0, 0, 0, &matrix, &options, &DAT_10219770[PARTNER_GALAXY]);
            FUN_10054550(0x232, 0x78, 0);
            options.m_0000 = 0;
            options.m_0004 = 0x37;
            options.m_0008 = 1.0f;
            DAT_10daaa80.Start(0x544, position, -1, -1, 0, 0, 0x1b, &matrix, &options, &DAT_10219770[PARTNER_GALAXY]);
        }
        if (DAT_1020baa8 % EXIT_RING_PERIOD == 0 && DAT_1020baa8 < EXIT_RING_END)
        {
            Vec4 position;
            EffectOptions options;
            Matrix4 matrix;
            float spin = (float)(FUN_1013f880() % 360) * PI_APPROX / 180.0f;
            matrix.SetIdentity();
            position.y = (float)(FUN_1013f880() % 900);
            position.RotateZ((float)(FUN_1013f880() % 360) * PI_APPROX / 180.0f);
            position.z = (float)(FUN_1013f880() % 600) - 300.0f;
            position.Copy(position + DAT_10db6ee8[0].matrix.GetTranslation());
            matrix.SetTranslation(position);
            position.Fill(0.0f);
            options.m_0000 = 1;
            options.m_0004 = 100;
            options.m_0008 = 1.0f;
            DAT_10daaa80.Start(0x534, position, -1, -1, 0, 0, 0, &matrix, &options, &DAT_10219770[PARTNER_GALAXY]);
            int pass;
            for (pass = 0; pass < 2; pass++)
            {
                spin += PI_APPROX;
                int ring;
                for (ring = 0; ring < RING_COUNT; ring++)
                {
                    position.Copy(matrix.GetTranslation());
                    if (pass == 1)
                        matrix = RotationZ((float)ring * RING_STEP + spin) * Scaling(0.9f, 0.9f, 0.9f);
                    else
                        matrix = RotationZ((float)ring * RING_STEP + spin);
                    matrix.SetTranslation(position);
                    position.Fill(0.0f);
                    FUN_1001d8b0(PARTNER_GALAXY, 0x535, position, -1, -1, 0, 0, 0, &matrix, &options);
                }
                for (ring = 0; ring < RING_COUNT; ring++)
                {
                    position.Copy(matrix.GetTranslation());
                    if (pass == 1)
                        matrix = RotationZ((float)ring * RING_STEP + spin) * Scaling(-0.9f, -0.9f, -0.9f);
                    else
                        matrix = RotationZ((float)ring * RING_STEP + spin) * Scaling(-1.0f, -1.0f, -1.0f);
                    matrix.SetTranslation(position);
                    position.Fill(0.0f);
                    FUN_1001d8b0(PARTNER_GALAXY, 0x535, position, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
        }
    }
    if (DAT_1020b430[index].farewellFrames > 0)
        DAT_1020b430[index].farewellFrames--;
    return 0;
}
