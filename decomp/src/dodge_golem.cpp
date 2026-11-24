// The map 0x132 dodge game's stone golem pieces: after the game is over the golem's fists and rocks (the nodes
// "@BS_005fists:" and "@BS_005rocks:" of the model tree) are driven by character slots 0x3d..0x4c. C++ rather than
// extern "C": the function keeps an exception frame for its Matrix4 locals.

#include <math.h>
#include "vec4.h"
#include "matrix4.h"
#include "anim_model.h"
#include "node.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "char_hikari.h"
#include "char_slot.h"
#include "dodge_game.h"
#include "dodge_golem.h"
#include "effect_manager.h"
#include "enemy_roam_spit.h"
#include "last_position.h"
#include "map_vars.h"
#include "mole_game.h"
#include "player_frame.h"
#include "target_race.h"

// The game's atan2 wrapper (float arguments and result).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Piece steps (CharState::state).
#define PIECE_SETUP 0
#define PIECE_ATTACHED 1
#define PIECE_FALLING 2
#define PIECE_SNAPPING 3
#define PIECE_THROWN 4

// Marker slots (kind 'Q' = taken, negated while the piece is hidden) are searched in this range of slots.
#define MARKER_SLOT_FIRST 0x5c
#define MARKER_SLOT_END 0x7c
#define MARKER_TAKEN 0x51
#define MARKER_SLOT_OFFSET 0x3c
#define MARKER_HIDDEN_X -200000.0f

// Effects: the trail of a piece is effect index + 0x755, with a glow 0x7a2; 0x7a3 is the burst when it lands.
#define TRAIL_EFFECT_BASE 0x755
#define GLOW_EFFECT 0x7a2
#define BURST_EFFECT 0x7a3

// Falling in (PIECE_FALLING): the piece appears 2200..3720 units out (900 added for the ring) at a random angle,
// 150 units up, and spins down for the first 30 frames' worth of growth, then wobbles.
#define FALL_RADIUS_RANGE 0x578
#define FALL_RADIUS_MIN 2200.0f
#define FALL_ANGLE_RANGE 0x168
#define FALL_TRIES 10
#define RING_RADIUS 900.0f
#define PROBE_HEIGHT 500.0f
#define FALL_HEIGHT 150.0f
#define DEGREES_PER_HALF_TURN 180.0f
#define GROW_FRAMES 0x1e
#define SPIN_STEP_X 0.01f
#define SPIN_STEP_Z 0.02f
#define WOBBLE_PERIOD 0x3c
#define WOBBLE_RISE_END 0xf
#define WOBBLE_FALL_START 0x2d
#define WOBBLE_MIDDLE 0x1e

// Snapping (PIECE_SNAPPING) lasts SNAP_FRAMES frames.
#define SNAP_FRAMES 0x3c
#define SNAP_DONE_MODE 4

// Thrown (PIECE_THROWN): flies 60 units a frame towards the player; one that is further than this (squared) bursts.
#define THROW_SPEED 60.0f
#define THROW_RANGE_SQUARED 4000000.0f
#define THROW_ROCK_SINK 5.0f

// Attached (PIECE_ATTACHED): the piece rides on the golem's left arm bone; a hand is lifted a little.
#define PIECE_HOVER 65.0f
#define HEADING_HALF_TURN 3.1415f
#define HEADING_FULL_TURN 6.283f
#define NODE_HIDE_BITS 0x20
#define ARM_BONE "Bip01 L UpperArm"

// 0x1005fc00 (DodgeGame_UpdateGolemPiece). `index` is the piece's character slot (0x3d, 0x3e = fists, 0x3f..0x4c =
// rocks). Setup finds the piece's node, the marker slot that carries it and the heading it has to the arm. Attached,
// the piece follows the arm's heading and its marker; it is destroyed when the marker or a linked piece dies. After the
// game is over it falls in from a random place (PIECE_FALLING), snaps back to its place (PIECE_SNAPPING) and is
// thrown at the player (PIECE_THROWN), spawning trail effects each step. Returns 0.
int DodgeGame_UpdateGolemPiece(int index)
{
    float scratch;
    if (DAT_1020b430[index].state == PIECE_SETUP)
    {
        int found = 0;
        for (int marker = MARKER_SLOT_FIRST; marker < MARKER_SLOT_END; marker++)
        {
            if (DAT_1021f3c0[marker] == MARKER_TAKEN || DAT_1021f3c0[marker] == -MARKER_TAKEN)
            {
                found++;
                if (found == index - MARKER_SLOT_OFFSET)
                {
                    DAT_1020b430[index].debrisMarker = marker;
                    break;
                }
            }
        }
        if (index == GOLEM_FIRST_FIST || index == GOLEM_LAST_FIST)
        {
            DAT_1021bdf8[index][3] = DAT_10e3dd2c->Find("@BS_005fists:", index % 10 - 1);
            DAT_1021bdf8[index][1] = 0;
            if (index == GOLEM_FIRST_FIST)
                DAT_1021bdf8[GOLEM_FIRST_FIST][0] = DAT_10e3dd2c->Find("Dummy02", 0);
            else
                DAT_1021bdf8[index][0] = DAT_10e3dd2c->Find("Dummy01", 0);
            DAT_1021bdf8[index][2] = DAT_1021bdf8[index][0];
        }
        else
        {
            if (index < GOLEM_FIRST_LINKED_ROCK)
            {
                Node *rock = DAT_10e3dd2c->Find("@BS_005rocks:", index - GOLEM_FIRST_ROCK);
                DAT_1021bdf8[index][3] = rock;
                DAT_1021bdf8[index][1] = 0;
                DAT_1021bdf8[index][2] = DAT_1021bdf8[index][0] = rock->parent;
                DAT_1020b430[index].debrisPartnerA = 0;
                DAT_1020b430[index].debrisPartnerB = 0;
                rock->ClearFlags(NODE_HIDE_BITS);
                DAT_1020b430[index].debrisShown = 1;
            }
            else
            {
                Node *rock = DAT_10e3dd2c->Find("@BS_005rocks:", index - GOLEM_FIRST_ROCK);
                DAT_1021bdf8[index][3] = rock;
                DAT_1021bdf8[index][1] = 0;
                DAT_1021bdf8[index][2] = DAT_1021bdf8[index][0] = rock->parent;
                switch (index)
                {
                case 0x48:
                    DAT_1020b430[index].debrisPartnerA = index - 9;
                    DAT_1021bdf8[index][4] = DAT_1021bdf8[index - 9][0];
                    DAT_1020b430[index].debrisPartnerB = index - 8;
                    DAT_1021bdf8[index][5] = DAT_1021bdf8[index - 8][0];
                    break;
                case 0x49:
                case 0x4a:
                case 0x4b:
                    DAT_1020b430[index].debrisPartnerA = index - 7;
                    DAT_1021bdf8[index][4] = DAT_1021bdf8[index - 7][0];
                    DAT_1020b430[index].debrisPartnerB = index - 6;
                    DAT_1021bdf8[index][5] = DAT_1021bdf8[index - 6][0];
                    break;
                case 0x4c:
                    DAT_1020b430[index].debrisPartnerA = index - 5;
                    DAT_1021bdf8[index][4] = DAT_1021bdf8[index - 5][0];
                    DAT_1020b430[index].debrisPartnerB = index - 0xd;
                    DAT_1021bdf8[index][5] = DAT_1021bdf8[index - 0xd][0];
                    break;
                }
            }
            int marker = DAT_1020b430[index].debrisMarker;
            DAT_1021f3c0[marker] = -DAT_1021f3c0[marker];
            DAT_10db6ee8[marker].matrix.translation.x = MARKER_HIDDEN_X;
            DAT_1021bdf8[index][3]->SetFlags(NODE_HIDE_BITS);
            DAT_1020b430[index].debrisShown = 0;
        }
        DAT_10802bf8[index].position.Copy(DAT_10db6ee8[DAT_1020b430[index].debrisMarker].matrix.GetTranslation());
        DAT_1020b430[index].state = PIECE_ATTACHED;
        Vec4 shoulder;
        Vec4 anchor;
        shoulder.Copy(DAT_10e3dd2c->Find(ARM_BONE, 0)->worldMatrix.GetTranslation() -
                      DAT_10e3dd68.GetTranslation());
        anchor.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() - DAT_10e3dd68.GetTranslation());
        float *armHeading = &DAT_10255160[index][0];
        *armHeading = scratch = (float)atan2(shoulder.x, shoulder.y);
        scratch = (float)atan2(anchor.x, anchor.y);
        *armHeading = scratch - *armHeading;
        DAT_10802bf8[index].step.Copy(DAT_1021bdf8[index][3]->parent->matrix.GetTranslation());
        DAT_10db6ee8[DAT_1020b430[index].debrisMarker].alpha = 0;
        DAT_10db6ee8[DAT_1020b430[index].debrisMarker].root->Find("@O_034bcbox:", 0)->group = 1;
    }
    else if (DAT_1020b430[index].state == PIECE_ATTACHED)
    {
        if (DAT_1021f3c0[DAT_1020b430[index].debrisMarker] > 0)
        {
            DAT_10db6ee8[DAT_1020b430[index].debrisMarker].alpha = 0;
            Vec4 a;
            Vec4 b;
            Vec4 c;
            float heading;
            if (index > GOLEM_LAST_FIST)
            {
                b.Copy(DAT_10e3dd2c->Find(ARM_BONE, 0)->worldMatrix.GetTranslation() -
                       DAT_10e3dd68.GetTranslation());
                c.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() - DAT_10e3dd68.GetTranslation());
                float armAngle = FUN_10001f80(b.x, b.y);
                float swing = FUN_10001f80(c.x, c.y) - armAngle;
                heading = DAT_10255160[index][0] - swing;
                while (heading > HEADING_HALF_TURN)
                    heading -= HEADING_FULL_TURN;
                while (heading < -HEADING_HALF_TURN)
                    heading += HEADING_FULL_TURN;
            }
            else
                heading = 0;
            a.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() - DAT_10e3dd68.GetTranslation());
            if (index > GOLEM_LAST_FIST)
            {
                Vec4 d;
                d.Copy(a);
                d.RotateZ(DAT_10e3debc - heading);
                DAT_1021bdf8[index][3]->matrix = RotationZ(heading);
                DAT_1021bdf8[index][3]->matrix.translation.x =
                    d.x - DAT_1021bdf8[index][3]->parent->matrix.translation.x;
                DAT_1021bdf8[index][3]->matrix.translation.y =
                    d.y - DAT_1021bdf8[index][3]->parent->matrix.translation.y;
                DAT_1021bdf8[index][3]->matrix.translation.z =
                    d.z - DAT_1021bdf8[index][3]->parent->matrix.translation.z;
            }
            DAT_10255160[index][1] = heading;
            a.RotateZ(-heading);
            a.z -= PIECE_HOVER;
            a.Copy(a + DAT_10e3dd68.GetTranslation());
            DAT_10db6ee8[DAT_1020b430[index].debrisMarker].matrix.translation.x = a.x;
            DAT_10db6ee8[DAT_1020b430[index].debrisMarker].matrix.translation.y = a.y;
            DAT_10db6ee8[DAT_1020b430[index].debrisMarker].matrix.translation.z = a.z;
            DAT_10868838[index][0].SetTranslation(a);
            DAT_10868838[index][0].translation.z += PIECE_HOVER;
            if (DAT_1020c2b0 != 7 && DAT_1020c2b0 != 9)
            {
                if (DAT_1020b430[index].debrisPartnerA != 0 || DAT_1020b430[index].debrisPartnerB != 0)
                {
                    if ((DAT_1020b430[index].debrisPartnerA != 0 &&
                         DAT_1021f3c0[DAT_1020b430[DAT_1020b430[index].debrisPartnerA].debrisMarker] < 0) ||
                        (DAT_1020b430[index].debrisPartnerB != 0 &&
                         DAT_1021f3c0[DAT_1020b430[DAT_1020b430[index].debrisPartnerB].debrisMarker] < 0))
                        FUN_100541f0(DAT_1020b430[index].debrisMarker);
                }
            }
        }
    }
    else if (DAT_1020b430[index].state == PIECE_FALLING)
    {
        DAT_1020b430[index].wait++;
        if (DAT_1020b430[index].wait == 1)
        {
            Vec4 position;
            Vec4 from;
            Vec4 to;
            Vec4 hit;
            int tries;
            for (tries = 0; tries < FALL_TRIES; tries++)
            {
                scratch = 0;
                position.Fill(scratch);
                scratch = (float)(FUN_1013f880() % FALL_RADIUS_RANGE);
                position.y = scratch + FALL_RADIUS_MIN;
                scratch = (float)(FUN_1013f880() % FALL_ANGLE_RANGE);
                position.RotateZ(scratch * HEADING_HALF_TURN / DEGREES_PER_HALF_TURN);
                position.y += RING_RADIUS;
                from.Copy(position);
                to.Copy(position);
                from.z = PROBE_HEIGHT;
                to.z = -PROBE_HEIGHT;
                if (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, 4, &from, &to, &hit) == 0)
                {
                    DAT_10868838[index][0].SetIdentity();
                    DAT_10868838[index][0].SetTranslation(position);
                    DAT_10868838[index][0].translation.z = FALL_HEIGHT;
                    scratch = (float)(FUN_1013f880() % FALL_ANGLE_RANGE);
                    DAT_10255160[index][2] = scratch * HEADING_HALF_TURN / DEGREES_PER_HALF_TURN;
                    scratch = (float)(FUN_1013f880() % FALL_ANGLE_RANGE);
                    DAT_10255160[index][3] = scratch * HEADING_HALF_TURN / DEGREES_PER_HALF_TURN;
                    break;
                }
            }
            if (tries == FALL_TRIES)
                DAT_1020b430[index].wait = 0;
        }
        else
        {
            Vec4 position;
            Matrix4 matrix;
            EffectOptions options;
            matrix.SetIdentity();
            matrix = RotationX(DAT_10255160[index][2]) * RotationZ(DAT_10255160[index][3]);
            int growth = DAT_1020b430[index].wait - 1;
            if (growth < GROW_FRAMES)
            {
                scratch = (float)growth / 30.0f;
                matrix = matrix * Scaling(scratch, scratch, scratch);
            }
            matrix.SetTranslation(DAT_10868838[index][0].GetTranslation());
            options.m_0004 = DAT_10db03c8;
            options.m_0000 = 0;
            FUN_1001d8b0(index, index + TRAIL_EFFECT_BASE, position, -1, -1, 0, 0, 0, &matrix, &options);
            FUN_1001d8b0(index, GLOW_EFFECT, position, -1, -1, 0, 0, 0, &matrix, &options);
            DAT_10255160[index][2] += SPIN_STEP_X;
            DAT_10255160[index][3] += SPIN_STEP_Z;
            position.Copy(DAT_10868838[index][0].GetTranslation());
            position.y -= RING_RADIUS;
            position.RotateZ(0.01f);
            position.y += RING_RADIUS;
            int step = DAT_1020b430[index].wait % WOBBLE_PERIOD;
            if (step >= WOBBLE_RISE_END && step < WOBBLE_FALL_START)
                step = WOBBLE_MIDDLE - step;
            else if (step >= WOBBLE_FALL_START)
                step -= WOBBLE_PERIOD;
            position.z += (float)step;
            DAT_10868838[index][0].SetTranslation(position);
        }
    }
    else if (DAT_1020b430[index].state == PIECE_SNAPPING)
    {
        DAT_1020b430[index].wait++;
        Vec4 a;
        Vec4 b;
        Vec4 c;
        float heading;
        if (index > GOLEM_LAST_FIST)
        {
            b.Copy(DAT_10e3dd2c->Find(ARM_BONE, 0)->worldMatrix.GetTranslation() -
                   DAT_10e3dd68.GetTranslation());
            c.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() - DAT_10e3dd68.GetTranslation());
            float armAngle = FUN_10001f80(b.x, b.y);
            float swing = FUN_10001f80(c.x, c.y) - armAngle;
            heading = DAT_10255160[index][0] - swing;
            while (heading > HEADING_HALF_TURN)
                heading -= HEADING_FULL_TURN;
            while (heading < -HEADING_HALF_TURN)
                heading += HEADING_FULL_TURN;
        }
        else
            heading = 0;
        a.Copy(DAT_1021bdf8[index][0]->worldMatrix.GetTranslation() - DAT_10e3dd68.GetTranslation());
        DAT_10255160[index][1] = heading;
        a.RotateZ(-heading);
        a.Copy(a + DAT_10e3dd68.GetTranslation());
        while (DAT_10255160[index][2] > 0)
            DAT_10255160[index][2] -= HEADING_FULL_TURN;
        while (DAT_10255160[index][3] > 0)
            DAT_10255160[index][3] -= HEADING_FULL_TURN;
        int left = SNAP_FRAMES - DAT_1020b430[index].wait;
        if (left > 0)
            DAT_10255160[index][2] = 1.0f / (float)left * DAT_10255160[index][2];
        else
            DAT_10255160[index][2] = 0;
        if (left > 0)
            DAT_10255160[index][3] = 1.0f / (float)left * DAT_10255160[index][3];
        else
            DAT_10255160[index][3] = 0;
        Vec4 position;
        EffectOptions options;
        Matrix4 matrix;
        matrix.SetIdentity();
        matrix = RotationX(DAT_10255160[index][2]) * RotationZ(DAT_10255160[index][3]);
        matrix.SetTranslation(DAT_10868838[index][0].GetTranslation());
        options.m_0004 = DAT_10db03c8;
        options.m_0000 = 0;
        FUN_1001d8b0(index, index + TRAIL_EFFECT_BASE, position, -1, -1, 0, 0, 0, &matrix, &options);
        FUN_1001d8b0(index, GLOW_EFFECT, position, -1, -1, 0, 0, 0, &matrix, &options);
        float t1 = -FUN_10001f80(a.x, a.y - RING_RADIUS);
        float turn = -FUN_10001f80(DAT_10868838[index][0].translation.x, DAT_10868838[index][0].translation.y - RING_RADIUS);
        turn = t1 - turn;
        while (turn > HEADING_FULL_TURN)
            turn -= HEADING_FULL_TURN;
        while (turn < 0.0f)
            turn += HEADING_FULL_TURN;
        left = SNAP_FRAMES - DAT_1020b430[index].wait;
        if (left > 0)
            turn = 1.0f / (float)left * turn;
        position.Copy(DAT_10868838[index][0].GetTranslation());
        position.y -= RING_RADIUS;
        position.RotateZ(turn);
        a.y -= RING_RADIUS;
        float positionRadius = position.Length2D();
        scratch = a.Length2D();
        float positionZ = position.z;
        float anchorZ = a.z;
        position.z = 0;
        position.Normalized3();
        left = SNAP_FRAMES - DAT_1020b430[index].wait;
        if (left > 0)
        {
            float closing = (positionRadius - scratch) / (float)left;
            position.Copy(position * positionRadius - position * closing);
            float dz = positionZ - anchorZ;
            position.z = positionZ - dz / (float)(SNAP_FRAMES - DAT_1020b430[index].wait);
        }
        else
        {
            position.Copy(position * scratch);
            position.z = anchorZ;
        }
        position.y += RING_RADIUS;
        DAT_10868838[index][0].SetTranslation(position);
        if (DAT_1020b430[index].wait > SNAP_FRAMES)
        {
            int marker = DAT_1020b430[index].debrisMarker;
            DAT_10db6ee8[marker].m_0288 = 0;
            DAT_10db6ee8[marker].m_028c = SNAP_DONE_MODE;
            DAT_1021f3c0[marker] = MARKER_TAKEN;
            DAT_1020b430[index].state = PIECE_ATTACHED;
            DAT_1020b430[index].wait = 0;
        }
    }
    else if (DAT_1020b430[index].state == PIECE_THROWN)
    {
        Vec4 step;
        step.Copy(DAT_10868838[index][0].GetTranslation() - DAT_10e3dd68.GetTranslation());
        if (step.LengthSquared3() > THROW_RANGE_SQUARED)
        {
            step.Normalized3();
            step.Copy(step * THROW_SPEED);
            DAT_10868838[index][0].translation.x += step.x;
            DAT_10868838[index][0].translation.y += step.y;
            Matrix4 matrix;
            Vec4 position;
            EffectOptions options;
            options.m_0004 = DAT_10db03c8;
            options.m_0008 = 1.0f;
            options.m_000e = 0;
            options.m_0000 = 0;
            matrix = RotationZ(DAT_10255160[index][1]);
            matrix.SetTranslation(DAT_10868838[index][0].GetTranslation());
            FUN_1001d8b0(index, BURST_EFFECT, position, -1, -1, 0, 0, 0, &matrix, &options);
            DAT_1020b430[index].state = PIECE_ATTACHED;
            DAT_1020b430[index].wait = 0;
        }
        else
        {
            step.Normalized3();
            step.Copy(step * THROW_SPEED);
            DAT_10868838[index][0].translation.x += step.x;
            DAT_10868838[index][0].translation.y += step.y;
            if (index > 0x47)
                DAT_10868838[index][0].translation.z -= THROW_ROCK_SINK;
            Matrix4 matrix;
            Vec4 position;
            EffectOptions options;
            options.m_0004 = DAT_10db03c8;
            options.m_0008 = 1.0f;
            options.m_000e = 0;
            options.m_0000 = 0;
            matrix = RotationZ(DAT_10255160[index][1]);
            matrix.SetTranslation(DAT_10868838[index][0].GetTranslation());
            FUN_1001d8b0(index, index + TRAIL_EFFECT_BASE, position, -1, -1, 0, 0, 0, &matrix, &options);
            FUN_1001d8b0(index, GLOW_EFFECT, position, -1, -1, 0, 0, 0, &matrix, &options);
        }
    }
    if (DAT_1020b430[index].state > 0)
    {
        if (DAT_10db03bc <= 0)
        {
            if (DAT_1020b430[index].state > 1)
            {
                Matrix4 matrix;
                Vec4 position;
                EffectOptions options;
                options.m_0004 = DAT_10db03c8;
                options.m_0008 = 1.0f;
                options.m_000e = 0;
                options.m_0000 = 0;
                matrix = RotationZ(DAT_10255160[index][1]);
                matrix.SetTranslation(DAT_10868838[index][0].GetTranslation());
                DAT_10daaa80.Start(BURST_EFFECT, position, -1, -1, 0, 0, 0, &matrix, &options, &DAT_10219770[index]);
                DAT_1020b430[index].state = PIECE_ATTACHED;
                DAT_1020b430[index].wait = 0;
            }
            else
            {
                if (DAT_1021f3c0[DAT_1020b430[index].debrisMarker] > 0)
                    FUN_100541f0(DAT_1020b430[index].debrisMarker);
            }
        }
        if (DAT_1021f3c0[DAT_1020b430[index].debrisMarker] < 0 && DAT_1020b430[index].debrisShown != 0)
        {
            DAT_1021bdf8[index][3]->SetFlags(NODE_HIDE_BITS);
            DAT_1020b430[index].debrisShown = 0;
        }
        else if (DAT_1021f3c0[DAT_1020b430[index].debrisMarker] > 0 && DAT_1020b430[index].debrisShown == 0)
        {
            DAT_1021bdf8[index][3]->ClearFlags(NODE_HIDE_BITS);
            DAT_1020b430[index].debrisShown = 1;
        }
    }
    return 0;
}
