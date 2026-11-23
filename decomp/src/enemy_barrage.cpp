// Barrage enemies: behaviour numbers of Enemy_UpdateBehaviour 0x100bcb30 (char_dispatch.cpp) that walk with the
// common enemy behaviour (FUN_10076790) and now and then stop to call down a barrage of effects on the player.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "effect_manager.h"

#include "char_slot.h"

#include "char_state.h"

#include "barrage_enemy.h"

#include "strafe_enemy.h"

extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect
extern "C" int FUN_10076790(int index, int variant);                        // the common enemy behaviour
Matrix4 RotationZ(float angle);                                             // 0x101b1b40

#define FULL_TURN 6.283f               // two pi as the game writes it

// 0x100a77f0 (Enemy_CallBarrage, behaviour 15 of Enemy_UpdateBehaviour). Walks with the common enemy behaviour;
// with a 2.5 % chance per frame (when motion layer 4 is not playing and no blend runs) it starts a barrage: it turns
// to the player (motion 0 while turning, 2 once facing within a degree), starts effect 0x3702 four times with sound
// 0x1c4, fires effect 0x429 from a random point above itself at the player every 10 frames from frame 60 to 110, and
// goes back to walking on frame 150. Returns 0.
int Enemy_CallBarrage(int index)
{
    if (DAT_1020b430[index].state == BARRAGE_RUNNING)
    {
        Vec4 player;
        Vec4 self;
        player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        Matrix4 *matrix = &DAT_10db6ee8[index].matrix;
        self.Copy(matrix->GetTranslation());
        (player - self).Length();  // computed and dropped
        // turn towards the player; walk while the heading is more than a degree off
        float dy = player.y - self.y;
        float dx = player.x - self.x;
        float turn = (float)atan2((double)dx, (double)dy) + PI_APPROX;
        DAT_10db6ee8[index].targetHeading = turn;
        turn = DAT_10db6ee8[index].heading - turn;  // now the angle still to turn
        while (turn > PI_APPROX)
            turn -= FULL_TURN;
        while (turn < -PI_APPROX)
            turn += FULL_TURN;
        int motion = turn * turn < FACING_TOLERANCE_SQUARED ? 2 : 0;  // idle once facing the player
        int frame = ++DAT_1020b430[index].wait;
        if (frame == 1)
        {
            // the charge: four copies of the effect following the enemy, three frames apart
            Vec4 offset;
            EffectOptions options;
            options.followOwner = 1;
            FUN_1001d8b0(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, matrix, 0, 0, 0, &options);
            FUN_1001d8b0(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, matrix, 0, 3, 0, &options);
            FUN_1001d8b0(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, matrix, 0, 6, 0, &options);
            FUN_1001d8b0(index, BARRAGE_CHARGE_EFFECT, offset, -1, -1, matrix, 0, 9, 0, &options);
            FUN_10065b10(index, BARRAGE_CHARGE_SOUND, 0, 0);
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
            position.z = (float)(FUN_1013f880() % BARRAGE_SHOT_HEIGHT) + position.z + 50.0f;
            position.x = (float)(FUN_1013f880() % BARRAGE_SHOT_SPREAD) - 150.0f + position.x;
            position.y = (float)(FUN_1013f880() % BARRAGE_SHOT_SPREAD) - 150.0f + position.y;
            aim.Copy(player);
            aim.z += BARRAGE_AIM_HEIGHT;
            shot = RotationZ(atan2f(aim.x - position.x, aim.y - position.y) + PI_APPROX);
            shot.SetTranslation(position);
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = BARRAGE_SHOT_SCALE;
            options.m_0000 = 0;
            options.followOwner = 1;
            position.Fill(0.0f);
            FUN_1001d8b0(index, BARRAGE_SHOT_EFFECT, position, -1, -1, 0, 0, 0, &shot, &options);
        }
        else if (frame == BARRAGE_FRAMES)
        {
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].wait = 0;
        }
        if (motion > -1)
            DAT_10db6ee8[index].PlayMotion(motion, 0, 0, 1.0f);
    }
    else
    {
        FUN_10076790(index, 0);
        if (DAT_10db6ee8[index].motion->layer != BARRAGE_BLOCKING_LAYER &&
            DAT_10db6ee8[index].motion->blending == 0 && FUN_1013f880() % 10000 < BARRAGE_CHANCE)
        {
            DAT_1020b430[index].state = BARRAGE_RUNNING;
            DAT_1020b430[index].wait = 0;
        }
    }
    return 0;
}

#define min(a, b) ((a) < (b) ? (a) : (b))

// 0x100a7db0 (Enemy_SpinFire, behaviour 14 of Enemy_UpdateBehaviour). Walks with the common enemy behaviour; with a
// 2.5 % chance per frame (as Enemy_CallBarrage) it spins in place for 150 frames, speeding up to 30 degrees a frame
// over the first 30 frames and slowing down over the last 30. Effect 0x3701 starts from its "@firenull" node (else
// its head) when the spin starts (sound 0x1c3) and every 40 frames; effect 0x36ff fires 125 units above it every 10
// frames from frame 30 to 120 (sound 0x1c2). A model with neither node stops the spin at once. Returns 0.
int Enemy_SpinFire(int index)
{
    if (DAT_1020b430[index].state == BARRAGE_RUNNING)
    {
        Vec4 player;
        Vec4 self;
        player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        (player - self).Length();    // computed and dropped
        (player - self).Length2D();  // computed and dropped
        int starting = 0;
        if (DAT_1020b430[index].wait == 0)
        {
            starting = 1;
            Matrix4 effectMatrix;
            Vec4 offset;
            EffectOptions options;
            options.followOwner = 1;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = SPIN_EFFECT_SCALE;
            offset.Fill(0.0f);
            effectMatrix.SetIdentity();
            options.owner = index + EFFECT_OWNER_BASE;
            Node *fire = DAT_10db6ee8[index].root->Find(FIRE_NODE_NAME, 0);
            // the fire node, else the head node; a model with neither stops the spin
            if (fire != 0)
                FUN_1001d8b0(index, SPIN_START_EFFECT, offset, -1, -1, &fire->worldMatrix, 0, 0, &effectMatrix, &options);
            else if (DAT_10db6ee8[index].headNode != 0)
                FUN_1001d8b0(index, SPIN_START_EFFECT, offset, -1, -1, &DAT_10db6ee8[index].headNode->worldMatrix, 0, 0,
                             &effectMatrix, &options);
            else
            {
                DAT_1020b430[index].wait = 0;
                DAT_1020b430[index].state = 0;
                return 0;
            }
            FUN_10065b10(index, SPIN_START_SOUND, 0, 0);
        }
        int frame = ++DAT_1020b430[index].wait;
        // the speed ramps up over the first SPIN_RAMP_FRAMES frames and down over the last ones
        int ramp = min(frame > BARRAGE_FRAMES - frame ? BARRAGE_FRAMES - frame : frame, SPIN_RAMP_FRAMES);
        float heading = (float)ramp / SPIN_RAMP_FRAMES * SPIN_MAX_TURN + DAT_10db6ee8[index].targetHeading;
        DAT_10db6ee8[index].targetHeading = heading;
        DAT_10db6ee8[index].heading = heading;
        if (frame > SPIN_FIRST_SHOT - 1 && frame < SPIN_LAST_SHOT + 1 && frame % SPIN_SHOT_PERIOD == 0)
        {
            Matrix4 effectMatrix;
            Vec4 offset;
            EffectOptions options;
            options.followOwner = 1;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = SPIN_EFFECT_SCALE;
            offset.Fill(0.0f);
            effectMatrix.SetIdentity();
            Node *fire = DAT_10db6ee8[index].root->Find(FIRE_NODE_NAME, 0);
            options.owner = index + EFFECT_OWNER_BASE;
            if (fire != 0)
            {
                effectMatrix.SetTranslation(fire->worldMatrix.GetTranslation());
                effectMatrix.m[MATRIX_TRANSLATION_Z] = self.z + SPIN_SHOT_HEIGHT;
                FUN_1001d8b0(index, SPIN_SHOT_EFFECT, offset, -1, -1, 0, 0, 0, &effectMatrix, &options);
            }
            else if (DAT_10db6ee8[index].headNode != 0)
            {
                effectMatrix.SetTranslation(DAT_10db6ee8[index].headNode->worldMatrix.GetTranslation());
                effectMatrix.m[MATRIX_TRANSLATION_Z] = self.z + SPIN_SHOT_HEIGHT;
                FUN_1001d8b0(index, SPIN_SHOT_EFFECT, offset, -1, -1, 0, 0, 0, &effectMatrix, &options);
            }
            FUN_10065b10(index, SPIN_SHOT_SOUND, 0, 0);
        }
        if (DAT_1020b430[index].wait % SPIN_FIRE_PERIOD == 0)
        {
            Matrix4 effectMatrix;
            Vec4 offset;
            EffectOptions options;
            options.followOwner = 1;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = SPIN_EFFECT_SCALE;
            offset.Fill(0.0f);
            effectMatrix.SetIdentity();
            options.owner = index + EFFECT_OWNER_BASE;
            Node *fire = DAT_10db6ee8[index].root->Find(FIRE_NODE_NAME, 0);
            if (fire != 0)
                FUN_1001d8b0(index, SPIN_START_EFFECT, offset, -1, -1, &fire->worldMatrix, 0, 0, &effectMatrix, &options);
            else if (DAT_10db6ee8[index].headNode != 0)
                FUN_1001d8b0(index, SPIN_START_EFFECT, offset, -1, -1, &DAT_10db6ee8[index].headNode->worldMatrix, 0, 0,
                             &effectMatrix, &options);
        }
        if (DAT_1020b430[index].wait == BARRAGE_FRAMES)
        {
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].wait = 0;
        }
        DAT_10db6ee8[index].PlayMotion(RUN_MOTION, starting, 5, 1.0f);
    }
    else
    {
        FUN_10076790(index, 0);
        if (DAT_10db6ee8[index].motion->layer != BARRAGE_BLOCKING_LAYER &&
            DAT_10db6ee8[index].motion->blending == 0 && FUN_1013f880() % 10000 < BARRAGE_CHANCE)
        {
            DAT_1020b430[index].state = BARRAGE_RUNNING;
            DAT_1020b430[index].wait = 0;
        }
    }
    return 0;
}
