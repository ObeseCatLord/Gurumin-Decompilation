// Barrage enemies (EN build): behaviours that walk with the common enemy behaviour (Enemy_ChaseLungeAndSpit) and now
// and then stop to call down a barrage of effects on the player. JP twin: enemy_barrage.cpp.
#include <math.h>
#include <windows.h>

#include "anim_model.h"
#include "barrage_enemy.h"
#include "camera.h"
#include "char.h"
#include "char_inline.h"
#include "char_dispatch.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "engine.h"
#include "sound_effects.h"
#include "node.h"
#include "vec4.h"

// Walks with the common enemy behaviour; with a 2.5 % chance per frame (when motion layer 4 is not playing and no
// blend runs) it starts a barrage: it turns to the player (motion 0 while turning, 2 once facing within a degree),
// starts effect 0x3702 four times with sound 0x1c4, fires effect 0x429 from a random point above itself at the player
// every 10 frames from frame 60 to 110, and goes back to walking on frame 150. Returns 0. Called by the enemy
// updater (behaviour 15).
int Enemy_CallBarrage(int index)
{
    if (g_charState[index].state == BARRAGE_RUNNING)
    {
        Vec4 player;
        Vec4 self;
        player.Copy(g_characterModels[0].matrix.GetTranslation());
        self.Copy(g_characterModels[index].matrix.GetTranslation());
        (player - self).Length();  // computed and dropped
        // turn towards the player; walk while the heading is more than a degree off
        float turn = (float)atan2(player.x - self.x, player.y - self.y) + TURN_HALF;
        g_characterModels[index].targetHeading = turn;
        turn = g_characterModels[index].heading - turn;  // now the angle still to turn
        while (turn > TURN_HALF)
            turn -= TURN_FULL;
        while (turn < -TURN_HALF)
            turn += TURN_FULL;
        int motion = turn * turn < FACING_TOLERANCE_SQUARED ? BARRAGE_MOTION_IDLE : BARRAGE_MOTION_WALK;
        int frame = g_charState[index].values[0] + 1;
        g_charState[index].values[0] = frame;
        if (frame == 1)
        {
            // the charge: four copies of the effect following the enemy, three frames apart
            Vec4 offset;
            EffectOptions options;
            options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
            Char_StartEffectInline(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, &g_characterModels[index].matrix, 0, 0, 0, &options);
            Char_StartEffectInline(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, &g_characterModels[index].matrix, 0, BARRAGE_CHARGE_STEP, 0, &options);
            Char_StartEffectInline(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, &g_characterModels[index].matrix, 0, 2 * BARRAGE_CHARGE_STEP, 0, &options);
            Char_StartEffectInline(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, &g_characterModels[index].matrix, 0, 3 * BARRAGE_CHARGE_STEP, 0, &options);
            Character_PlaySoundInline(index, BARRAGE_CHARGE_SOUND, 0, 0);
        }
        else if (frame > BARRAGE_FIRST_SHOT - 1 && frame < BARRAGE_LAST_SHOT + 1 && frame % BARRAGE_SHOT_PERIOD == 0)
        {
            // one shot from a random point above the enemy, aimed at the player
            Vec4 position;
            Vec4 aim;
            EffectOptions options;
            Matrix4 shot;
            shot.SetIdentity();
            position.Copy(self);
            position.z = (float)(Random() % BARRAGE_SHOT_HEIGHT) + BARRAGE_SHOT_HEIGHT_MIN + position.z;
            position.x = (float)(Random() % BARRAGE_SHOT_SPREAD) - BARRAGE_SHOT_SPREAD_HALF + position.x;
            position.y = (float)(Random() % BARRAGE_SHOT_SPREAD) - BARRAGE_SHOT_SPREAD_HALF + position.y;
            aim.Copy(player);
            aim.z += BARRAGE_AIM_HEIGHT;
            shot = RotationZ(atan2f(aim.x - position.x, aim.y - position.y) + TURN_HALF);
            shot.SetTranslation(position);
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = BARRAGE_SHOT_SCALE;
            options.m_0000 = 0;
            options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
            position.Fill(0.0f);
            Char_StartEffect(index, BARRAGE_SHOT_EFFECT, position, -1, -1, 0, 0, 0, &shot, &options);
        }
        else if (frame == BARRAGE_FRAMES)
        {
            g_charState[index].state = 0;
            g_charState[index].values[0] = 0;
        }
        if (motion > BARRAGE_NO_MOTION)
            g_characterModels[index].PlayMotion(motion, 0, 0, 1.0f);
    }
    else
    {
        Enemy_ChaseLungeAndSpit(index, 0);
        if (g_characterModels[index].playback->layer != BARRAGE_BLOCKING_LAYER &&
            g_characterModels[index].playback->blending == 0 && Random() % BARRAGE_CHANCE_RANGE < BARRAGE_CHANCE)
        {
            g_charState[index].state = BARRAGE_RUNNING;
            g_charState[index].values[0] = 0;
        }
    }
    return 0;
}

// Walks with the common enemy behaviour; with a 2.5 % chance per frame (when motion layer 4 is not playing and no
// blend runs) it starts a spin: the enemy turns on the spot, speeding up over 30 frames and slowing down over the
// last 30 of 150 frames (running motion 1), starts effect 0x3701 from the "@firenull" node (else its head) when the
// spin starts (sound 0x1c3) and every 40 frames; effect 0x36ff fires 125 units above it every 10 frames from frame 30
// to 120 (sound 0x1c2). A model with neither node stops the spin at once. Returns 0. Called by the enemy updater
// (behaviour 14).
int Enemy_SpinFire(int index)
{
    if (g_charState[index].state == BARRAGE_RUNNING)
    {
        Vec4 player;
        Vec4 self;
        player.Copy(g_characterModels[0].matrix.GetTranslation());
        self.Copy(g_characterModels[index].matrix.GetTranslation());
        (player - self).Length();    // computed and dropped
        (player - self).Length2D();  // computed and dropped
        int starting = 0;
        if (g_charState[index].values[0] == 0)
        {
            starting = 1;
            Matrix4 effectMatrix;
            Vec4 offset;
            EffectOptions options;
            options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = SPIN_EFFECT_SCALE;
            offset.Fill(0.0f);
            effectMatrix.SetIdentity();
            options.owner = index + EFFECT_OWNER_BASE;
            Node *fire = g_characterModels[index].root->Find(SPIN_FIRE_NODE_NAME, 0);
            // the fire node, else the head node; a model with neither stops the spin
            if (fire != 0)
                Char_StartEffectInline(index, SPIN_START_EFFECT, offset, -1, -1, &fire->worldMatrix, 0, 0, &effectMatrix, &options);
            else if (g_characterModels[index].headNode != 0)
                Char_StartEffectInline(index, SPIN_START_EFFECT, offset, -1, -1, &g_characterModels[index].headNode->worldMatrix, 0, 0,
                                       &effectMatrix, &options);
            else
            {
                g_charState[index].state = 0;
                g_charState[index].values[0] = 0;
                return 0;
            }
            Character_PlaySoundInline(index, SPIN_START_SOUND, 0, 0);
        }
        int frame = g_charState[index].values[0] + 1;
        g_charState[index].values[0] = frame;
        // the speed ramps up over the first SPIN_RAMP_FRAMES frames and down over the last ones
        int ramp = min(frame > BARRAGE_FRAMES - frame ? BARRAGE_FRAMES - frame : frame, SPIN_RAMP_FRAMES);
        float heading = (float)ramp / SPIN_RAMP_FRAMES * SPIN_MAX_TURN + g_characterModels[index].targetHeading;
        g_characterModels[index].targetHeading = heading;
        g_characterModels[index].heading = heading;
        if (g_charState[index].values[0] > SPIN_FIRST_SHOT - 1 && g_charState[index].values[0] < SPIN_LAST_SHOT + 1 && g_charState[index].values[0] % SPIN_SHOT_PERIOD == 0)
        {
            Matrix4 effectMatrix;
            Vec4 offset;
            EffectOptions options;
            options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = SPIN_EFFECT_SCALE;
            offset.Fill(0.0f);
            effectMatrix.SetIdentity();
            Node *fire = g_characterModels[index].root->Find(SPIN_FIRE_NODE_NAME, 0);
            options.owner = index + EFFECT_OWNER_BASE;
            if (fire != 0)
            {
                effectMatrix.SetTranslation(fire->worldMatrix.GetTranslation());
                effectMatrix.m[MATRIX_TRANSLATION_Z] = self.z + SPIN_SHOT_HEIGHT;
                Char_StartEffect(index, SPIN_SHOT_EFFECT, offset, -1, -1, 0, 0, 0, &effectMatrix, &options);
            }
            else if (g_characterModels[index].headNode != 0)
            {
                effectMatrix.SetTranslation(g_characterModels[index].headNode->worldMatrix.GetTranslation());
                effectMatrix.m[MATRIX_TRANSLATION_Z] = self.z + SPIN_SHOT_HEIGHT;
                Char_StartEffect(index, SPIN_SHOT_EFFECT, offset, -1, -1, 0, 0, 0, &effectMatrix, &options);
            }
            Character_PlaySound(index, SPIN_SHOT_SOUND, 0, 0);
        }
        if (g_charState[index].values[0] % SPIN_FIRE_PERIOD == 0)
        {
            Matrix4 effectMatrix;
            Vec4 offset;
            EffectOptions options;
            options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = SPIN_EFFECT_SCALE;
            offset.Fill(0.0f);
            effectMatrix.SetIdentity();
            options.owner = index + EFFECT_OWNER_BASE;
            Node *fire = g_characterModels[index].root->Find(SPIN_FIRE_NODE_NAME, 0);
            if (fire != 0)
                Char_StartEffectInline(index, SPIN_START_EFFECT, offset, -1, -1, &fire->worldMatrix, 0, 0, &effectMatrix, &options);
            else if (g_characterModels[index].headNode != 0)
                Char_StartEffectInline(index, SPIN_START_EFFECT, offset, -1, -1, &g_characterModels[index].headNode->worldMatrix, 0, 0,
                                       &effectMatrix, &options);
        }
        if (g_charState[index].values[0] == BARRAGE_FRAMES)
        {
            g_charState[index].state = 0;
            g_charState[index].values[0] = 0;
        }
        g_characterModels[index].PlayMotion(SPIN_RUN_MOTION, starting, 5, 1.0f);
    }
    else
    {
        Enemy_ChaseLungeAndSpit(index, 0);
        if (g_characterModels[index].playback->layer != BARRAGE_BLOCKING_LAYER &&
            g_characterModels[index].playback->blending == 0 && Random() % BARRAGE_CHANCE_RANGE < BARRAGE_CHANCE)
        {
            g_charState[index].state = BARRAGE_RUNNING;
            g_charState[index].values[0] = 0;
        }
    }
    return 0;
}

// Walks with the common enemy behaviour and, with the same 2.5 % chance per frame as Enemy_CallBarrage, stops to call
// bursts: it faces the player, speeds its motion up and waits until its motion sets AnimModel::idleMotionEnabled; then
// effect 0x3705 bursts on the player's spine (else head, else position) at points around the enemy's facing: two 120
// units up, and with variant 1 four more at 45 and 87 units. It walks again 60 frames later. Returns 0. Called by the
// enemy updater (behaviours 16 and 17, variant 0 and 1).
int Enemy_CallBursts(int index, int variant)
{
    if (g_charState[index].state == BARRAGE_RUNNING)
    {
        Vec4 player;
        Vec4 self;
        player.Copy(g_characterModels[0].matrix.GetTranslation());
        self.Copy(g_characterModels[index].matrix.GetTranslation());
        if (++g_charState[index].values[0] == 1)
        {
            // the call starts: face the player
            g_characterModels[index].AddMotionSpeed(BURST_MOTION_SPEED);
            g_characterModels[index].idleMotionEnabled = 0;
            float dy = player.y - self.y;
            float dx = player.x - self.x;
            g_characterModels[index].targetHeading = (float)atan2((double)dx, (double)dy) + TURN_HALF;
        }
        if (g_charState[index].values[0] < BURST_FIRED)
        {
            if (g_characterModels[index].idleMotionEnabled == 1)
            {
                // the motion reached its call frame: fire the bursts
                g_charState[index].values[0] = BURST_FIRED;
                EffectOptions options;
                Matrix4 burst;
                Vec4 offset;
                const Matrix4 *target;
                if (g_characterModels[0].root->Find(BURST_NODE_NAME, 0) != 0)
                    target = &g_characterModels[0].root->Find(BURST_NODE_NAME, 0)->worldMatrix;
                else if (g_characterModels[0].headNode != 0)
                    target = &g_characterModels[0].headNode->worldMatrix;
                else
                    target = &g_characterModels[0].matrix;
                options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
                options.scale = BURST_SCALE;
                burst.SetIdentity();
                burst = RotationZ(g_characterModels[index].heading);
                if (variant != 0)
                {
                    offset.x = BURST_LOW_LEFT_X;
                    offset.z = BURST_LOW_Z;
                    offset.RotateZ(-g_characterModels[index].heading);
                    burst.SetTranslation(offset + self);
                    offset.Fill(0.0f);
                    Char_StartEffectInline(index, BURST_EFFECT, offset, -1, -1, target, 0, Random() % BURST_FRAME_SPREAD,
                                           &burst, &options);
                    offset.x = BURST_LOW_RIGHT_X;
                    offset.z = BURST_LOW_Z;
                    offset.RotateZ(-g_characterModels[index].heading);
                    burst.SetTranslation(offset + self);
                    offset.Fill(0.0f);
                    Char_StartEffectInline(index, BURST_EFFECT, offset, -1, -1, target, 0, Random() % BURST_FRAME_SPREAD,
                                           &burst, &options);
                    offset.x = BURST_MID_LEFT_X;
                    offset.z = BURST_MID_Z;
                    offset.RotateZ(-g_characterModels[index].heading);
                    burst.SetTranslation(offset + self);
                    offset.Fill(0.0f);
                    Char_StartEffectInline(index, BURST_EFFECT, offset, -1, -1, target, 0, Random() % BURST_FRAME_SPREAD,
                                           &burst, &options);
                    offset.x = BURST_MID_RIGHT_X;
                    offset.z = BURST_MID_Z;
                    offset.RotateZ(-g_characterModels[index].heading);
                    burst.SetTranslation(offset + self);
                    offset.Fill(0.0f);
                    Char_StartEffectInline(index, BURST_EFFECT, offset, -1, -1, target, 0, Random() % BURST_FRAME_SPREAD,
                                           &burst, &options);
                }
                offset.x = -BURST_HIGH_X;
                offset.z = BURST_HIGH_Z;
                offset.RotateZ(-g_characterModels[index].heading);
                burst.SetTranslation(offset + self);
                offset.Fill(0.0f);
                Char_StartEffectInline(index, BURST_EFFECT, offset, -1, -1, target, 0, Random() % BURST_FRAME_SPREAD,
                                       &burst, &options);
                offset.x = BURST_HIGH_X;
                offset.z = BURST_HIGH_Z;
                offset.RotateZ(-g_characterModels[index].heading);
                burst.SetTranslation(offset + self);
                offset.Fill(0.0f);
                Char_StartEffectInline(index, BURST_EFFECT, offset, -1, -1, target, 0, Random() % BURST_FRAME_SPREAD,
                                       &burst, &options);
            }
            g_characterModels[index].PlayMotion(BURST_MOTION_CALL, 0, BURST_MOTION_BLEND, 1.0f);
        }
        else if (g_charState[index].values[0] > BURST_END)
        {
            g_charState[index].state = 0;
            g_charState[index].values[0] = 0;
        }
    }
    else
    {
        Enemy_ChaseLungeAndSpit(index, 0);
        if (g_characterModels[index].playback->layer != BARRAGE_BLOCKING_LAYER &&
            g_characterModels[index].playback->blending == 0 && Random() % BARRAGE_CHANCE_RANGE < BARRAGE_CHANCE)
        {
            g_charState[index].state = BARRAGE_RUNNING;
            g_charState[index].values[0] = 0;
        }
    }
    return 0;
}
