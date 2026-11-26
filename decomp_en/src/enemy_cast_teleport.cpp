// The spell-casting teleporter (EN build; JP: decomp/src/enemy_cast_teleport.cpp, Enemy_CastAndTeleport). Its CharState
// words, steps and tuning are named in cast_teleport_enemy.h.
#include <math.h>

#include "anim_model.h"
#include "cast_teleport_enemy.h"
#include "char.h"
#include "char_dispatch.h"
#include "char_move.h"
#include "char_shots.h"
#include "collide_query.h"
#include "effect_options.h"
#include "engine.h"
#include "map_view.h"
#include "node.h"
#include "sound_delayed.h"
#include "swing_platform.h"
#include "vec4.h"
#include "visibility.h"

// Move factors and similar are written as literals where the original passes them by reference: a named constant would not
// compile to the same code (advance 4, retreat -8, slide 50, spot pull -50).

// One frame of the caster `index`. It looks up its spine node and stands on the ground on the first frame, then every ten
// frames; keeps its anchor point (the shot slot's position) at the spine; drops into the pit (mode 2) when it falls below
// the pit floor inside the pit area. Every ten frames it checks sight of the player (within 1000 units, along a ray from
// the spine to 75 units above the player; when it only looks ahead, within about 15 degrees) and the step falls back to
// wandering after 300 frames without sight. Wander turns to a random heading now and then; keep range advances closer than
// 700 units, backs off beyond 800, faces the player and may start a spell (bolt effect and sound, then a charge effect on
// its body every other frame for 35 frames, 75 frames in all); find spot tries 20 random spots around the player in a
// visible cell with room to stand, a clear line to the player and ground below, then fades out, slides to the spot 50 units a
// frame and fades in. Returns 1 while the character is busy (g_charPhase), else 0. Called by the character dispatch every
// frame.
int Enemy_CastAndTeleport(int index)
{
    if (g_charPhase[index] != 0)
        return 1;
    if (g_charAwake[index] == 0)
        return 0;
    if (g_charState[index].values[CAST_VALUE_SET_UP] == 0)
    {
        g_charState[index].values[CAST_VALUE_SET_UP] = 1;
        g_charNodes[index][0] = g_characterModels[index].root->Find(CAST_SPINE_NAME, 0);
        Char_PlaceOnGround(index, 0);
        g_characterModels[index].m_0012 = 2;
    }
    int phase = index % CAST_GROUND_PERIOD;
    if (g_frameCounter % CAST_GROUND_PERIOD == phase)
        Char_PlaceOnGround(index, 0);
    if (g_charToldToReact[index] != 0)
        g_charToldToReact[index] = 0;
    g_shotSlots[index].position.Fill(0.0f);
    g_shotSlots[index].position.y = CAST_SPINE_OFFSET_Y;
    g_shotSlots[index].position.z = CAST_SPINE_OFFSET_Z;
    g_shotSlots[index].position.RotateZ(-g_characterModels[index].heading);
    g_shotSlots[index].position.Copy(g_shotSlots[index].position +
                                     g_charNodes[index][0]->worldMatrix.GetTranslation());
    g_charClearedEachFrame[index] = 0;
    g_characterModels[index].locked = 0;
    g_charTalkPending[index] = 0;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    Vec4 move;
    EffectOptions options;
    Vec4 offset;
    Matrix4 shot;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    float distance = toPlayer.Length();
    toPlayer.Length2D();
    g_characterModels[index].mode = 1;
    if (self.z < g_mapBottomZ + CAST_GROUND_LIFT)
    {
        int y = (int)self.y;
        int x = (int)self.x;
        if (x >= (int)g_mapPitMin.x && y >= (int)g_mapPitMin.y && x <= (int)g_mapPitMax.x && y <= (int)g_mapPitMax.y &&
            g_mapBottomValid != 0)
        {
            if (self.z < g_mapBottomZ)
                g_characterModels[index].mode = CAST_MODE_PIT;
            else if (g_charRecords[index].stats[STAT_HEALTH] > 0)
                g_characterModels[index].mode = 0;
        }
    }
    int sight;
    if (g_frameCounter % CAST_GROUND_PERIOD == phase)
    {
        sight = 1;
        if (distance < CAST_SEE_RANGE)
        {
            if (g_wakeRadiusShort != 0 && g_charState[index].state == CAST_STEP_WANDER)
            {
                float error = CAST_PI - (float)atan2((double)toPlayer.x, (double)toPlayer.y);
                error = error - g_characterModels[index].heading;
                while (error > CAST_PI)
                    error = error - CAST_FULL_TURN;
                while (error < -CAST_PI)
                    error = error + CAST_FULL_TURN;
                if ((float)fabs(error) > CAST_SEE_ANGLE)
                    sight = 0;
            }
            Vec4 eye;
            Vec4 target;
            Vec4 hit;
            eye.Copy(g_charNodes[index][0]->worldMatrix.GetTranslation());
            target.Copy(player);
            target.z += CAST_SIGHT_HEIGHT;
            if (g_mapRootNode != 0 && g_mapRootNode->CastRay(0, 4, 0x4000000, &target, &eye, &hit, 0) != 0 && sight != 0)
            {
                if (g_wakeRadiusShort != 0 && g_charState[index].state == CAST_STEP_WANDER &&
                    Random() % 100 < CAST_BLIND_PERCENT)
                    sight = 0;
                else
                    sight = CAST_SIGHT_CLEAR;
            }
        }
        else
            sight = 0;
        g_charState[index].values[CAST_VALUE_SIGHT] = sight;
    }
    else
        sight = g_charState[index].values[CAST_VALUE_SIGHT];
    if (sight != 0)
        g_charState[index].values[CAST_VALUE_SIGHT_COUNTDOWN] = CAST_HUNT_FRAMES;
    else if (g_charState[index].values[CAST_VALUE_SIGHT_COUNTDOWN] != 0)
        g_charState[index].values[CAST_VALUE_SIGHT_COUNTDOWN]--;
    else
    {
        g_charState[index].state = CAST_STEP_WANDER;
        g_charState[index].values[CAST_VALUE_WAIT] = 1;
    }
    if (g_charState[index].state == CAST_STEP_WANDER && sight != 0)
    {
        g_charState[index].state = CAST_STEP_KEEP_RANGE;
        g_charState[index].values[CAST_VALUE_WAIT] = 0;
    }
    g_charState[index].values[CAST_VALUE_MOTION] = CAST_MOTION_IDLE;
    move.Fill(0.0f);
    if (g_charState[index].state == CAST_STEP_WANDER)
    {
        if (Random() % CAST_WANDER_ROLL_RANGE < CAST_WANDER_ROLL_LIMIT)
        {
            float angle = (float)(Random() % CAST_DEGREES);
            g_characterModels[index].targetHeading = angle / CAST_DEG_HALF_TURN * CAST_PI_FINE;
        }
    }
    else if (g_charState[index].state == CAST_STEP_KEEP_RANGE)
    {
        if (distance < CAST_NEAR_RANGE)
        {
            g_charState[index].values[CAST_VALUE_MOTION] = CAST_MOTION_ADVANCE;
            move.y = 4.0f;
            move.RotateZ(-g_characterModels[index].heading);
        }
        else if (distance > CAST_FAR_RANGE)
        {
            g_charState[index].values[CAST_VALUE_MOTION] = CAST_MOTION_RETREAT;
            move.y = -8.0f;
            move.RotateZ(-g_characterModels[index].heading);
        }
        else
            g_charState[index].values[CAST_VALUE_MOTION] = CAST_MOTION_IDLE;
        g_characterModels[index].targetHeading =
            atan2f(player.x - self.x, player.y - self.y) + CAST_PI;
        if (Random() % CAST_CAST_ROLL_RANGE < CAST_CAST_ROLL_LIMIT)
        {
            g_charState[index].state = CAST_STEP_SPELL;
            g_charState[index].values[CAST_VALUE_WAIT] = 0;
        }
    }
    else if (g_charState[index].state == CAST_STEP_SPELL)
    {
        g_characterModels[index].locked = 1;
        if (g_charRecords[index].stats[STAT_HEALTH] > 0)
            g_characterModels[index].mode = 0;
        if (g_charState[index].values[CAST_VALUE_WAIT] == 0)
        {
            offset.Fill(0.0f);
            options.m_0000 = 0;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = CAST_BOLT_SCALE;
            options.m_000e = CAST_BOLT_MODE;
            options.owner = index;
            offset.Copy(player);
            offset.z += CAST_SIGHT_HEIGHT;
            if (g_shotSlots[index].position.z < offset.z)
                shot = RotationX((float)acos((offset - g_shotSlots[index].position).Length2D() /
                                             (offset - g_shotSlots[index].position).Length())) *
                       RotationZ(g_characterModels[index].targetHeading);
            else if (g_shotSlots[index].position.z > offset.z)
                shot = RotationX(-(float)acos((offset - g_shotSlots[index].position).Length2D() /
                                              (offset - g_shotSlots[index].position).Length())) *
                       RotationZ(g_characterModels[index].targetHeading);
            else
                shot = RotationZ(g_characterModels[index].targetHeading);
            shot.SetTranslation(g_shotSlots[index].position);
            offset.Fill(0.0f);
            Char_StartEffect(index, CAST_BOLT_EFFECT, offset, -1, -1, 0, 0, 0, &shot, &options);
            Character_PlaySound(index, CAST_BOLT_SOUND, 0, 0);
            Sound_PlayDelayed(CAST_BOLT_AFTER_SOUND, CAST_BOLT_AFTER_DELAY,
                              &g_characterModels[index].matrix.GetTranslation());
        }
        if (g_charState[index].values[CAST_VALUE_WAIT] < CAST_CHARGE_FRAMES &&
            g_charState[index].values[CAST_VALUE_WAIT] % 2 != 0)
        {
            EffectOptions charge;
            charge.flags |= CAST_CHARGE_FOLLOW_OWNER;
            offset.Fill(0.0f);
            Char_StartEffect(index, CAST_CHARGE_EFFECT, offset, -1, -1, 0, 0, 0, &g_characterModels[index].matrix,
                             &charge);
        }
        if (++g_charState[index].values[CAST_VALUE_WAIT] > CAST_SPELL_FRAMES)
        {
            g_charState[index].state = CAST_STEP_FIND_SPOT;
            g_charState[index].values[CAST_VALUE_WAIT] = 0;
        }
    }
    else if (g_charState[index].state == CAST_STEP_FIND_SPOT)
    {
        Vec4 spot;
        Vec4 away;
        Vec4 spotCopy;
        Vec4 probeFrom;
        Vec4 standAt;   // declared by the original but never used (its slot and constructor remain)
        Vec4 hitPoint;
        int attempt = 1;
        do
        {
            spot.Fill(0.0f);
            spot.y = (float)-(Random() % CAST_SPOT_DEPTH_RANGE);
            if (!(spot.y > CAST_SPOT_MIN_DEPTH))
            {
                spot.RotateZ((float)(Random() % CAST_DEGREES) / CAST_DEG_HALF_TURN * CAST_PI_FINE);
                spot.Copy(spot + g_characterModels[0].matrix.GetTranslation());
                spot.z += CAST_SPOT_LIFT;
                if (Vis_IsVisible(spot) != 0)
                {
                    away.Copy(player - spot);
                    away.Normalized3();
                    away.Copy(away * -50.0f);
                    spotCopy.Copy(spot);
                    probeFrom.Copy(player);
                    probeFrom.z += CAST_SPOT_LIFT;
                    if (g_mapRootNode == 0 || g_mapRootNode->Collide(0, 4, &probeFrom, &spotCopy, &hitPoint) == 0)
                    {
                        away.Copy(spotCopy - self);
                        if (g_characterModels[index].IsClearOfMapLimited(&away) != 0)
                        {
                            probeFrom.Copy(spotCopy);
                            probeFrom.z -= CAST_SPOT_FLOOR_DROP;
                            if (g_mapRootNode != 0 &&
                                g_mapRootNode->Collide(0, 4, &probeFrom, &spotCopy, &hitPoint) != 0)
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
            g_charState[index].values[CAST_VALUE_WAIT] = 0;
            g_charState[index].state = CAST_STEP_KEEP_RANGE;
        }
        else
        {
            spot.z -= CAST_SPOT_LIFT;
            g_shotSlots[index].step.Copy(spot);
            g_charState[index].state = CAST_STEP_FADE_OUT;
            g_charState[index].values[CAST_VALUE_FADE_STEP] = CAST_FADE_STEPS;
        }
    }
    else if (g_charState[index].state == CAST_STEP_FADE_OUT)
    {
        g_charState[index].values[CAST_VALUE_FADE_STEP]--;
        g_characterModels[index].alpha =
            (float)(g_charState[index].values[CAST_VALUE_FADE_STEP] + CAST_FADE_OFFSET) / CAST_FADE_DIVISOR;
        if (g_charState[index].values[CAST_VALUE_FADE_STEP] == 1)
            g_charState[index].state = CAST_STEP_SLIDE;
        if (g_charRecords[index].stats[STAT_HEALTH] > 0)
            g_characterModels[index].mode = 0;
    }
    else if (g_charState[index].state == CAST_STEP_SLIDE)
    {
        if (g_charRecords[index].stats[STAT_HEALTH] > 0)
            g_characterModels[index].mode = 0;
        move.Copy(g_shotSlots[index].step - self);
        g_characterModels[index].alpha = CAST_SLIDE_ALPHA;
        if (move.Length() > CAST_SPOT_LIFT)
        {
            move.Normalized3();
            move.Copy(move * 50.0f);
        }
        else
            g_charState[index].state = CAST_STEP_FADE_IN;
        g_characterModels[index].SetPosition(move.x + self.x, move.y + self.y, move.z + self.z);
        move.Fill(0.0f);
    }
    else if (g_charState[index].state == CAST_STEP_FADE_IN)
    {
        if (g_charRecords[index].stats[STAT_HEALTH] > 0)
            g_characterModels[index].mode = 0;
        g_charState[index].values[CAST_VALUE_FADE_STEP]++;
        g_characterModels[index].alpha =
            (float)(g_charState[index].values[CAST_VALUE_FADE_STEP] + CAST_FADE_OFFSET) / CAST_FADE_DIVISOR;
        if (g_charState[index].values[CAST_VALUE_FADE_STEP] == CAST_FADE_STEPS)
        {
            g_characterModels[index].alpha = 1.0f;
            g_charState[index].state = CAST_STEP_KEEP_RANGE;
            g_charState[index].values[CAST_VALUE_WAIT] = 0;
            g_characterModels[index].mode = 1;
        }
        g_characterModels[index].targetHeading =
            atan2f(player.x - self.x, player.y - self.y) + CAST_PI;
    }
    if (g_characterModels[index].mode == CAST_MODE_PIT)
    {
        if (g_charRecords[index].stats[STAT_HEALTH] > 0)
            g_characterModels[index].mode = 0;
        move.z += CAST_GROUND_LIFT;
    }
    if (move.Length() != 0.0f)
        Character_SetMove(index, move.x, move.y, move.z);
    if (g_charHitLevel[index] < CAST_MIN_LEVEL)
        g_characterModels[index].PlayMotion(g_charState[index].values[CAST_VALUE_MOTION], 0, 5, 1.0f);
    return 0;
}
