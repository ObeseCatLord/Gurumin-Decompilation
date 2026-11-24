// The player's per-frame scene update (0x1004d960, called by the scene frame FUN_100f5a80 through FUN_1011a060). C++
// linkage: the function keeps an exception frame for its Matrix4 locals:
// picks the player's motion (idle, walking, the 0x59 scene motion), puts the footstep sound and dust on the walk
// frames, eases the camera in front of the map movie, eases the drawn heading and rebuilds the player's world
// matrix, wears the helmet item, and runs the guide: character 16 flies to the nearest guide marker and leaves a
// trail of effects, and the guide beam (DAT_1020c34c) flies to its goal.

#include <math.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "node_data.h"

#include "effect_manager.h"

#include "char_slot.h"

#include "char_vars.h"

#include "camera_vars.h"

#include "map_vars.h"

#include "ui_vars.h"

#include "render_vars.h"

#include "game_vars.h"

#include "path_walk.h"

#include "field_item_menu.h"

#include "dodge_game.h"

#include "scene_command.h"

#include "doppler_loops.h"

#include "player_frame.h"

extern "C" const char *FUN_1013f8a0(const char *name, int folder);

#define PLAYER_FIELD_SCENE_LIMIT 2000                // DAT_104c00d0 up to this: the field scenes
#define PLAYER_CUTSCENE_MOTION_FROM 2010             // DAT_104c00d0 above this: the scene motion plays
#define PLAYER_MOTION_WALK 1
#define PLAYER_MOTION_IDLE 0
#define PLAYER_MOTION_STAND 2
#define PLAYER_MOTION_SCENE 0x59
#define PLAYER_WALK_STEP_LENGTH 11                   // DAT_1020c308 from this: walking, below: idle
#define PLAYER_BLEND_FRAMES 5
#define PLAYER_FOOT_FIRST_FRAME 0x18                 // walk frames 0x18 + 8n (and 10) make a step with a sound
#define PLAYER_FOOT_HALF_FRAME 10
#define PLAYER_FOOT_DUST_EFFECT 0x2da
#define PLAYER_FOOT_SOUNDS 4
#define PLAYER_WALKING_LAYERS 2                      // motion layers below this are walks
#define PLAYER_HIDDEN_X (-200000.0f)
#define PLAYER_HIDE_MOVIE_Y 1700.0f
#define PLAYER_MOVIE_CAMERA_SIDE 0.0f
#define PLAYER_MOVIE_CAMERA_DEPTH 1700.0f
#define PLAYER_MOVIE_SWING_DIVISOR (-5000.0f)
#define PLAYER_MOVIE_SWING_SCALE 0.2618f
#define PLAYER_TILT 0.2618f                          // the player model is tilted back by this angle
#define PLAYER_PI 3.1415f
#define PLAYER_TWO_PI 6.283f
#define PLAYER_TURN_SMOOTHING 3.0f                   // the drawn heading covers a third of the turn each frame
#define PLAYER_ABSENT_X (-10000.0f)                  // a character x below this is not placed
#define PLAYER_ACTOR_NEAR 100.0f
#define PLAYER_ACTOR_LIMIT 0x7c                      // characters 1 .. 0x7b are checked
#define PLAYER_BODY_LIFT 75.0f
#define PLAYER_HELMET_NODE_SIZE 0x40
#define PLAYER_HELMET_FACE_HIDDEN 0x2a               // helmet item that hides the face model
#define GUIDE_CHARACTER 0x10
#define GUIDE_MOTION_FLY 7
#define GUIDE_MOTION_STAND 2
#define GUIDE_FLY_FRAMES 10
#define GUIDE_FAR 99999.0f
#define GUIDE_FADE_HALF 0.5f
#define GUIDE_TRAIL_EFFECT 0x36c5
#define GUIDE_TRAIL_SPACING 35
#define GUIDE_BEAM_START 5.0f
#define GUIDE_BEAM_SPEED_SCALE 1000.0f
#define GUIDE_BEAM_LIST 4
#define GUIDE_BEAM_EFFECT_A 0x7c9
#define GUIDE_BEAM_EFFECT_B 0x7cb
#define GUIDE_BEAM_TAIL 70.0f
#define GUIDE_SHADOW_OPACITY 0xf

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x1004d960 (Player_UpdateFrame). Returns 0.
int Player_UpdateFrame()
{
    Vec4 position;
    Vec4 viewer;
    Vec4 anchor;
    viewer.Copy(DAT_1086e0f8.GetTranslation());
    anchor.Copy(DAT_1086df38.GetTranslation());
    if (DAT_104bf8bc == 1)
    {
        if (DAT_10c63510 == 1 && DAT_1032d69c != 0)
        {
            DAT_10c63510 = 0;
            DAT_1020c2bc = 0;
            DAT_1020c2d8 = 0;
        }
    }
    else if (DAT_104bf8bc > 0 && DAT_1032d69c != 0)
    {
        DAT_104bf8bc = 1;
    }
    if (DAT_1020c2b0 != 0)
    {
        if (DAT_102028dc == 0 && DAT_1032d69c != 0 && DAT_104c03c8 == 0)
        {
            position.Copy(DAT_1086df38.GetTranslation());
            if (DAT_104c00d0 > PLAYER_FIELD_SCENE_LIMIT)
            {
                if (DAT_104c00d0 > PLAYER_CUTSCENE_MOTION_FROM)
                {
                    if (DAT_104c03a0->layer != PLAYER_MOTION_SCENE && DAT_104c03a0->nextLayer != PLAYER_MOTION_SCENE)
                        DAT_104c03a0->Play(PLAYER_MOTION_SCENE, 1, PLAYER_BLEND_FRAMES, 1.0f);
                    DAT_104c03a0->Play(PLAYER_MOTION_SCENE, 0, PLAYER_BLEND_FRAMES, 1.0f);
                }
            }
            else if (DAT_1020c308 < PLAYER_WALK_STEP_LENGTH)
            {
                if (DAT_104c03a0->layer != PLAYER_MOTION_IDLE && DAT_104c03a0->nextLayer != PLAYER_MOTION_IDLE)
                    DAT_104c03a0->Play(PLAYER_MOTION_IDLE, 1, PLAYER_BLEND_FRAMES, 1.0f);
                if (DAT_104c03a0->blending == 0)
                    DAT_104c03a0->Play(PLAYER_MOTION_IDLE, 0, PLAYER_BLEND_FRAMES, 1.0f);
            }
            else
            {
                if (DAT_104c03a0->layer != PLAYER_MOTION_WALK && DAT_104c03a0->nextLayer != PLAYER_MOTION_WALK)
                    DAT_104c03a0->Play(PLAYER_MOTION_WALK, 1, PLAYER_BLEND_FRAMES, 1.0f);
                if (DAT_104c03a0->blending != 0)
                    DAT_104c03a0->Play(PLAYER_MOTION_WALK, 0, PLAYER_BLEND_FRAMES, 1.0f);
            }
            if (DAT_104c03a0->layer < PLAYER_WALKING_LAYERS)
            {
                int frame = DAT_104c03a0->frame;
                if ((frame - PLAYER_FOOT_FIRST_FRAME) % 8 == 0 || frame == PLAYER_FOOT_HALF_FRAME)
                {
                    DAT_104bfafc++;
                    FUN_1003ad20(DAT_104bfafc & (PLAYER_FOOT_SOUNDS - 1), &DAT_1086df38.GetTranslation(), 1.0f);
                    Matrix4 matrix;
                    Vec4 offset;
                    offset.Fill(0.0f);
                    matrix = TranslationMatrix(position.x, position.y, position.z);
                    DAT_10daaa80.Start(PLAYER_FOOT_DUST_EFFECT, offset, -1, 0, 0, 0, 0, &matrix, 0, 0);
                }
                else if ((frame - PLAYER_FOOT_FIRST_FRAME) % 4 == 0)
                {
                    Matrix4 matrix;
                    Vec4 offset;
                    offset.Fill(0.0f);
                    matrix = TranslationMatrix(position.x, position.y, position.z);
                    DAT_10daaa80.Start(PLAYER_FOOT_DUST_EFFECT, offset, -1, 0, 0, 0, 0, &matrix, 0, 0);
                }
            }
        }
        else
        {
            position.Copy(DAT_1086df38.GetTranslation());
            if (DAT_104c00d0 <= PLAYER_FIELD_SCENE_LIMIT)
            {
                DAT_104c03a0->Play(PLAYER_MOTION_STAND, 0, PLAYER_BLEND_FRAMES, 1.0f);
            }
            else if (DAT_104c00d0 > PLAYER_CUTSCENE_MOTION_FROM)
            {
                if (DAT_104c03a0->layer != PLAYER_MOTION_SCENE && DAT_104c03a0->nextLayer != PLAYER_MOTION_SCENE)
                    DAT_104c03a0->Play(PLAYER_MOTION_SCENE, 1, PLAYER_BLEND_FRAMES, 1.0f);
                DAT_104c03a0->Play(PLAYER_MOTION_SCENE, 0, PLAYER_BLEND_FRAMES, 1.0f);
            }
        }
    }
    else
    {
        position.Copy(DAT_1086df38.GetTranslation());
        if (DAT_104c00d0 > PLAYER_FIELD_SCENE_LIMIT)
        {
            if (DAT_104c00d0 > PLAYER_CUTSCENE_MOTION_FROM)
            {
                if (DAT_104c03a0->layer != PLAYER_MOTION_SCENE && DAT_104c03a0->nextLayer != PLAYER_MOTION_SCENE)
                    DAT_104c03a0->Play(PLAYER_MOTION_SCENE, 1, PLAYER_BLEND_FRAMES, 1.0f);
                DAT_104c03a0->Play(PLAYER_MOTION_SCENE, 0, PLAYER_BLEND_FRAMES, 1.0f);
            }
        }
        else if (DAT_1020c304 != 0 && DAT_10c63388 == 0)
        {
            if (DAT_104c03a0->layer != PLAYER_MOTION_WALK && DAT_104c03a0->nextLayer != PLAYER_MOTION_WALK)
                DAT_104c03a0->Play(PLAYER_MOTION_WALK, 1, PLAYER_BLEND_FRAMES, 1.0f);
            DAT_104c03a0->Play(PLAYER_MOTION_WALK, 0, PLAYER_BLEND_FRAMES, 1.0f);
        }
        else
        {
            DAT_104c03a0->Play(PLAYER_MOTION_STAND, 0, PLAYER_BLEND_FRAMES, 1.0f);
        }
    }
    if (DAT_104c0354 == 0)
    {
        Vec4 look;
        if (DAT_1020c2fc < 0)
        {
            if (DAT_10c66d08 != 0)
            {
                DAT_11af3e28.Set(0.0f, -PLAYER_HIDE_MOVIE_Y, PLAYER_HIDE_MOVIE_Y, 0.0f);
                DAT_118e6c78.Set(0.0f, 0.0f, 0.0f, 0.0f);
            }
            else
            {
                float swing = position.x / PLAYER_MOVIE_SWING_DIVISOR;
                DAT_11af3e28.Set(DAT_1020344c * PLAYER_MOVIE_CAMERA_SIDE + swing + position.x,
                                 position.y - DAT_1020344c * PLAYER_MOVIE_CAMERA_DEPTH,
                                 (PLAYER_MOVIE_CAMERA_DEPTH - DAT_1101dddc) * DAT_1020344c + position.z, 0.0f);
                DAT_118e6c78.Set(position.x, position.y, position.z, 0.0f);
                look.Copy(DAT_11af3e28 - DAT_118e6c78);
                look.RotateZ(swing * PLAYER_MOVIE_SWING_SCALE);
                DAT_11af3e28.Copy(DAT_118e6c78 + look);
            }
        }
        else
        {
            Vec4 pathPoint;
            pathPoint.Copy(DAT_104c03ac->extra->AnimatedMatrix(DAT_1020c2fc).GetTranslation());
            float swing = pathPoint.x / PLAYER_MOVIE_SWING_DIVISOR;
            DAT_11af3e28.Set(DAT_1020344c * PLAYER_MOVIE_CAMERA_SIDE + swing + pathPoint.x,
                             pathPoint.y - DAT_1020344c * PLAYER_MOVIE_CAMERA_DEPTH,
                             (PLAYER_MOVIE_CAMERA_DEPTH - DAT_1101dddc) * DAT_1020344c + pathPoint.z, 0.0f);
            DAT_118e6c78.Set(pathPoint.x, pathPoint.y, pathPoint.z, 0.0f);
            look.Copy(DAT_11af3e28 - DAT_118e6c78);
            look.RotateZ(swing * PLAYER_MOVIE_SWING_SCALE);
            DAT_11af3e28.Copy(DAT_118e6c78 + look);
        }
    }
    {
        float heading = DAT_10e3debc;
        float turn = DAT_10e3dec0;
        turn -= heading;
        if (turn != 0.0f)
        {
            while (turn < -PLAYER_PI)
                turn += PLAYER_TWO_PI;
            while (turn > PLAYER_PI)
                turn -= PLAYER_TWO_PI;
            DAT_10db6ee8[0].targetHeading = DAT_10e3dec0;
            heading = turn / PLAYER_TURN_SMOOTHING + heading;
            DAT_10e3debc = heading;
            DAT_10db6ee8[0].heading = heading;
        }
    }
    DAT_1086df38 = Scaling(1.0f, 1.0f, 1.0f) * RotationZ(DAT_10e3debc) * RotationX(PLAYER_TILT);
    if (DAT_10c66d08 != 0)
        position.x = PLAYER_HIDDEN_X;
    int crowded = 0;
    float distance;
    for (int i = 1; i < PLAYER_ACTOR_LIMIT; i++)
    {
        if (DAT_10db6ee8[i].matrix.m[MATRIX_TRANSLATION_X] > PLAYER_ABSENT_X)
        {
            distance = (DAT_10db6ee8[i].matrix.GetTranslation() - position).Length2D();
            if (distance < PLAYER_ACTOR_NEAR)
            {
                crowded = 1;
                break;
            }
        }
    }
    if (crowded == 0)
    {
        DAT_1086df38.SetTranslation(position);
        DAT_10db6ee8[0].SetPosition(position.x, position.y, position.z + PLAYER_BODY_LIFT);
    }
    else
    {
        DAT_1086df38.SetTranslation(anchor);
        DAT_10db6ee8[0].SetPosition(anchor.x, anchor.y, anchor.z + PLAYER_BODY_LIFT);
    }
    DAT_1020c2c8++;
    int helmet = DAT_10daaae8[0].helmet;
    if (DAT_1020c348 != helmet)
    {
        DAT_1020c348 = helmet;
        Node *old = DAT_104c039c->Find("helmet", 0);
        if (old != 0)
            old->parent->RemoveChild(old, 0);
        if (DAT_104c039c->Find("helmet", 0) == 0)
        {
            Node *head = DAT_104c039c->Find("Bip01 Head", 0);
            if (head != 0)
            {
                Node *child = head->AddChild(0, PLAYER_HELMET_NODE_SIZE);
                child->Load(FUN_1013f8a0(DAT_108f678c[helmet], 0), 0, 0, 0);
                strcpy(child->name, "helmet");
                child->parent = head;
                child->Rebuild();
                child->BuildMesh(1, 0);
            }
        }
        Node *face = DAT_104c039c->Find("face_char02bf.lwo", 0);
        if (helmet == PLAYER_HELMET_FACE_HIDDEN)
        {
            if (face != 0)
                face->flags |= NODE_FLAG_HIDDEN;
        }
        else if (face != 0)
        {
            face->flags &= ~NODE_FLAG_HIDDEN;
        }
    }
    if (DAT_10216cc0[0].count != 0)
    {
        Vec4 point;
        Vec4 player;
        int pickedNear = 0;
        int pickedFar = 0;
        float bestFar = GUIDE_FAR;
        player.Copy(DAT_1086df38.GetTranslation());
        DAT_10db6ee8[GUIDE_CHARACTER].mode = 0;
        if (DAT_10c6351c != GUIDE_CHARACTER)
        {
            DAT_10db6ee8[GUIDE_CHARACTER].PlayMotion(GUIDE_MOTION_FLY, 0, PLAYER_BLEND_FRAMES, 1.0f);
            DAT_10db6ee8[GUIDE_CHARACTER].targetHeading =
                Atan2(player.x - DAT_10db6ee8[GUIDE_CHARACTER].matrix._41,
                      player.y - DAT_10db6ee8[GUIDE_CHARACTER].matrix._42) + PLAYER_PI;
        }
        else
        {
            DAT_10db6ee8[GUIDE_CHARACTER].PlayMotion(GUIDE_MOTION_STAND, 0, PLAYER_BLEND_FRAMES, 1.0f);
        }
        if (DAT_1020b830 == 0)
        {
            float bestNear = GUIDE_FAR;
            Vec4 candidate;
            Vec4 guide;
            guide.Copy(DAT_10db6ee8[GUIDE_CHARACTER].matrix.GetTranslation());
            for (int i = 1; i < DAT_10216cc0[0].count + 1; i++)
            {
                candidate.x = (float)DAT_10216cc0[i].x;
                candidate.y = (float)DAT_10216cc0[i].y;
                candidate.z = (float)DAT_10216cc0[i].z;
                if ((candidate - guide).Length2D() < bestNear)
                {
                    if ((candidate - guide).Length2D() < (float)DAT_10216cc0[i].range)
                    {
                        pickedNear = i;
                        bestNear = (candidate - guide).Length2D();
                    }
                }
            }
            if (pickedNear != 0)
                DAT_1020b830 = pickedNear;
        }
        for (int i = 1; i < DAT_10216cc0[0].count + 1; i++)
        {
            point.x = (float)DAT_10216cc0[i].x;
            point.y = (float)DAT_10216cc0[i].y;
            point.z = (float)DAT_10216cc0[i].z;
            if ((point - player).Length2D() < bestFar)
            {
                if ((point - player).Length2D() < (float)DAT_10216cc0[i].range)
                {
                    pickedFar = i;
                    bestFar = (point - player).Length2D();
                }
            }
        }
        if (pickedFar != 0 && DAT_1020b830 != pickedFar)
        {
            DAT_1020b830 = pickedFar;
            DAT_1020b834 = GUIDE_FLY_FRAMES;
            goto fly;
        }
        if (DAT_1020b834 > 0)
        {
fly:
            if (DAT_1020b830 > 0 && DAT_1020b830 < DAT_10216cc0[0].count + 1)
            {
                Vec4 delta;
                Vec4 next;
                Vec4 from;
                delta.x = (float)DAT_10216cc0[DAT_1020b830].x;
                delta.y = (float)DAT_10216cc0[DAT_1020b830].y;
                delta.z = (float)DAT_10216cc0[DAT_1020b830].z;
                delta.Copy(delta - DAT_10db6ee8[GUIDE_CHARACTER].matrix.GetTranslation());
                delta.Copy(delta.Divided((float)DAT_1020b834));
                next.Copy(delta + DAT_10db6ee8[GUIDE_CHARACTER].matrix.GetTranslation());
                from.Copy(DAT_10db6ee8[GUIDE_CHARACTER].matrix.GetTranslation());
                DAT_10db6ee8[GUIDE_CHARACTER].SetPosition(next.x, next.y, next.z);
                DAT_1020b834--;
                DAT_10db6ee8[GUIDE_CHARACTER].alpha = GUIDE_FADE_HALF;
                float length = delta.Length();
                delta.Normalized3();
                for (int along = 0; (float)along < length; along += GUIDE_TRAIL_SPACING)
                {
                    next.Copy(delta * (float)along + from);
                    DAT_10daaa80.Start(GUIDE_TRAIL_EFFECT, next, -1, -1, 0, 0, 0, 0, 0, 0);
                }
            }
            else
            {
                DAT_10db6ee8[GUIDE_CHARACTER].alpha = 1.0f;
            }
        }
        else
        {
            DAT_10db6ee8[GUIDE_CHARACTER].alpha = 1.0f;
        }
    }
    if (DAT_1020c34c != 0)
    {
        if (DAT_1020c34c == 1)
        {
            DAT_10803b08.x = (float)DAT_10c63578;
            DAT_10803b08.y = (float)DAT_10c6357c;
            DAT_10803b08.z = (float)DAT_10c63580;
            DAT_10803b38.Fill(GUIDE_BEAM_START);
            DAT_10803b38.y = 0.0f;
            DAT_1020c34c = 2;
        }
        Matrix4 basis;
        Vec4 origin;
        Vec4 toward;
        toward.Copy(DAT_118e6cb8 * 0.5f + DAT_10803b08 * 0.5f - DAT_10803b08);
        float blend = (float)DAT_10c63590 / GUIDE_BEAM_SPEED_SCALE;
        DAT_10803b38.Copy(DAT_10803b38 * blend + toward * (1.0f - blend));
        DAT_10803b08.Copy(DAT_10803b08 + DAT_10803b38);
        toward.Copy(DAT_10803b38);
        float pitch;
        if (toward.LengthSquared3() < 1.0f)
            pitch = 0.0f;
        else
            pitch = acosf(toward.Length2D() / toward.Length());
        if (toward.z > 0.0f)
            pitch = pitch * -1.0f;
        basis = RotationX(-pitch) * RotationZ(atan2f(toward.x, toward.y) + PLAYER_PI);
        basis.SetTranslation(DAT_10803b08);
        EffectOptions options;
        options.list = GUIDE_BEAM_LIST;
        DAT_10daaa80.Start(GUIDE_BEAM_EFFECT_A, origin, -1, -1, 0, 0, 0, &basis, &options, 0);
        toward.Normalized3();
        basis.SetTranslation(DAT_10803b08 - toward * GUIDE_BEAM_TAIL);
        options.list = GUIDE_BEAM_LIST;
        DAT_10daaa80.Start(GUIDE_BEAM_EFFECT_B, origin, -1, -1, 0, 0, 0, &basis, &options, 0);
        DAT_10db6ee8[GUIDE_CHARACTER].shadowOpacity = GUIDE_SHADOW_OPACITY;
    }
    return 0;
}
