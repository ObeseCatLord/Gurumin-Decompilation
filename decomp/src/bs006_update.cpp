// The BS_006 boss (kind 107): a dragon whose body is its own slot plus fifteen piece slots after it (legs, hands, feet, the
// chest plate and the tail; each piece slot follows a node of the dragon's model and keeps the hit flash of that part).
// After the game is over (DAT_10210d30) it runs the defeat scene (an ending model grows and fades while sparks fly from
// its bones and the camera circles it), otherwise the fight: a chain of states picked in state 100, the reaction to hits,
// the movement and the effects.

#include <math.h>
#include <windows.h>
#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "effect_manager.h"
#include "texture_manager.h"
#include "bs006.h"
#include "bgm_track.h"
#include "companion_update.h"
#include "doppler_loops.h"
#include "enemy_chase_hop.h"
#include "field_item_menu.h"
#include "game.h"
#include "football_state.h"
#include "game_main.h"
#include "hop_spit_enemy.h"
#include "last_position.h"
#include "partner_follow.h"
#include "scene_command_ids.h"
#include "scene_command.h"
#include "scene_reset.h"
#include "texture_load.h"
#include "ui_vars.h"
#include "value_command.h"
#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "route_flier.h"
#include "target_race.h"
#include "script_system.h"
#include "world.h"

#define BS006_PI 3.1415f
#define BS006_TWO_PI 6.283f
#define BS006_ENDING_MODEL 88                   // the model of the ending (bs_006pa.it3 loaded as a file, bs_006m.it3 as its model)
#define BS006_FAR_X (-200000.0f)
#define BS006_PEOPLE_FIRST 0x3a
#define BS006_ROOM_FLAG 0x448                   // script flag holding the room
#define BS006_ROOM_FLAG_BASE 1000               // + 2 * room: flag word of the ending
#define BS006_MUSIC_FLAG 0x82c                  // script flag that keeps the music playing after the fight
#define BS006_REACH 1750.0f                     // furthest the boss may move from the middle
#define BS006_RESTING_FOCUS 0xe6                // the value DAT_1101dddc normally has
#define BS006_COMPANION_COUNT 3                 // the first three pieces count as companions of the boss's health
#define BS006_STATE_SET_UP 0
#define BS006_STATE_PICK 100
#define BS006_STATE_PLAY_QUEUED 0x65
#define BS006_MOVE_NONE 0
#define BS006_MOVE_STEP 1                       // moves by the vector, at most BS006_REACH from the middle
#define BS006_MOVE_ADD 2                        // as 1 and adds the vector to the position
#define BS006_MOVE_PLACE 3                      // puts the boss at the vector (at most BS006_REACH from the middle)
#define BS006_EFFECT_SPARK 0x36f5               // + 0 or 1
#define BS006_EFFECT_RING 0x36f7
#define BS006_EFFECT_IMPACT 0x45d
#define BS006_EFFECT_DUST 0x2df
#define BS006_SOUND_STOMP 0x295
#define BS006_PIECE_REACT_FIRST 4               // pieces from this one on do not react to hits
#define BS006_QUARTER_TURN 1.5708f
#define BS006_EFFECT_BREATH 0x36e8              // links two shots: the breath that flies out of the mouth
#define BS006_EFFECT_HAND_FIRE 0x7a2
#define BS006_EFFECT_SHOCK_RING 0x7a6
#define BS006_EFFECT_SHOCK_SPARK 0x7a5
#define BS006_EFFECT_SHOCK_FLASH 0x7a9
#define BS006_EFFECT_SHOCK_DUST 0x138a
#define BS006_EFFECT_ROCK 0x71a
#define BS006_EFFECT_SLAM 0x70c
#define BS006_BREATH_FRAME 0x1fb                // state 6: the motion frame the breath starts at
#define BS006_BREATH_END_FRAME 0x213            // state 6: the motion frame it ends at
#define BS006_ROCKFALL_FRAME 0x226              // state 7: the motion frame the rocks fall at
#define BS006_SLAM_FRAME 0x246                  // state 7: the motion frame of the slam
#define BS006_ROCKFALL_END_FRAME 0x257          // state 7: the motion frame it ends at
#define BS006_EFFECT_JUMP_BURST 0x36ea          // state 9: bursts from the chest when the jump starts
#define BS006_EFFECT_JUMP_CHARGE 0x36f0         // state 9: the charge on every other frame
#define BS006_EFFECT_JUMP_LAUNCH 0x36ed
#define BS006_EFFECT_JUMP_TRAIL 0x36ef
#define BS006_EFFECT_LANDING 0x271a
#define BS006_EFFECT_LANDING_SHOCK 0x6e7
#define BS006_EFFECT_LANDING_SMOKE 0x6db
#define BS006_EFFECT_DEBRIS 0x36f4
#define BS006_JUMP_TURN 0.00873f                // state 9: furthest it turns towards the player in a frame
#define BS006_RISE_TURN 0.08727f                // state 10: the same while it rises
#define BS006_REACH_NEAR 1700.0f                // the jump only moves it while it is this near the middle
#define BS006_JUMP_LAND_FRAME 0x347             // state 9: the motion frame where the leap ends
#define BS006_JUMP_REST_FRAMES 0x7f7            // state 9: frames (from 2000) it rests before the next pick
#define BS006_HIGH_JUMP_TAKEOFF_FRAME 0x157     // state 10: the motion frame it leaves the ground at
#define BS006_HIGH_JUMP_PEAK_FRAME 0x166        // state 10: the motion frame it is at the top
#define BS006_HIGH_JUMP_END_FRAME 0x1e8         // state 10: the motion frame of the landing's end
#define BS006_PIECE_ALIVE 70000                 // a piece with more health than this still counts as whole
#define BS006_ROOM_TURN 0.5236f                 // state 0xb: the camera's side turn (30 degrees)
#define BS006_EFFECT_OWNER_BASE 300             // effects the boss owns carry its index + 300

// The lengths of the nodes' names the setup looks for, one per piece.
#define BS006_NAME_SIZE 0x50

// 0x100d24a0 (Bs006_Update). One frame of the boss in slot `index`. Returns 0 (1 while the defeat scene runs).
int Bs006_Update(int index)
{
    AnimModel &model = DAT_10db6ee8[index];
    AnimModel &ending = DAT_10db6ee8[BS006_ENDING_MODEL];
    Bs006State &st = DAT_1020b430[index].bs006;
    CharSlot &slot = DAT_10daaae8[index];
    int &defeatTimer = DAT_1020b49c[index].value;
    Vec4 playerPos;
    Vec4 ownPos;
    Vec4 move;
    int motion = -1;
    int restart = 0;
    int blendFrames = 5;
    int moveMode = BS006_MOVE_NONE;
    int armed = 0;

    slot.m_0141 = 1;
    model.uniformScale = 0.05f;
    DAT_104c0110 = index;
    DAT_10255a88[index] = 0;
    if (DAT_104bfe1c != 0)
        return 0;
    if (DAT_104c0135 != 0)
        return 0;
    if (DAT_104bfdf0 != 0)
        return 0;

    // The defeat scene.
    if (DAT_10210d30[index] != 0)
    {
        int sparks = 0;
        int sparkSpread = 0;
        DAT_104c00c8 = 1;
        if (DAT_104bfe78 != 0)
        {
            DAT_104c00c8 = 1;
            return 0;
        }
        model.PlayMotion(0x19, 0, 0xf, 1.0f);
        slot.m_0141 = 1;
        DAT_10db6ee8[0].shadowHidden = 0;
        DAT_104c0354 = 2;
        slot.stats[STAT_EFFECT_PARAM] = 0;
        int roomFlags = DAT_10874848.TestScriptFlag(DAT_10874848.TestScriptFlag(BS006_ROOM_FLAG) * 2 + BS006_ROOM_FLAG_BASE);
        DAT_10874848.SetScriptFlag(DAT_10874848.TestScriptFlag(BS006_ROOM_FLAG) * 2 + BS006_ROOM_FLAG_BASE,
                                   roomFlags | (1 << (index - BS006_PEOPLE_FIRST)));
        defeatTimer++;
        if (defeatTimer == 1)
        {
            DAT_104c0358 = defeatTimer;
            FUN_1016a8d0();
            DAT_104c00c8 = 1;
            DAT_1021f3c0[index] = 0x4e;
            DAT_10da9150.Copy(DAT_118e69e8);
            DAT_10868818.Copy(DAT_118e6cb8);
            DAT_10866d98.Copy(DAT_10da9150);
            DAT_10edbd48.Copy(DAT_10868818);
            ending.scale.Fill(0.0f);
            FUN_1003ad20(0x310, 0, 1.0f);
            FUN_1000da20(0x5ac);
        }
        if (DAT_104bfe80 < 2)
            ending.SetPosition(BS006_FAR_X, 0.0f, 0.0f);
        else
            ending.SetPosition(model.matrix._41, model.matrix._42, model.matrix._43);
        ending.heading = ending.targetHeading = model.heading;
        ending.m_0294 = 1;
        ending.motionFrame = model.motionFrame;
        if (defeatTimer > 10 && defeatTimer % 10 == 0)
        {
            if (defeatTimer < 0x5a)
                sparks = 1;
            else if (defeatTimer < 0x78)
            {
                sparks = 4;
                sparkSpread = 10;
            }
            else
            {
                sparkSpread = 0x1e;
                sparks = 8;
                if (defeatTimer >= 0xf0)
                    sparks = 0x14;
            }
        }
        if (defeatTimer == 0x96 && DAT_10874848.TestScriptFlag(BS006_MUSIC_FLAG) == 0)
        {
            Bgm_Play(0, 0);
            DAT_104c012c = 0;
        }
        if (defeatTimer == 300)
        {
            Matrix4 pose;
            Vec4 spot;
            if (DAT_1042d918[BS006_BONE_NECK] != 0)
            {
                for (int step = 0; step < 0x18; step++)
                {
                    pose = RotationZ((float)step * 188.49f / 180.0f);
                    pose.SetTranslation(((Node *)DAT_1042d918[BS006_BONE_NECK])->worldMatrix.GetTranslation());
                    DAT_10daaa80.Start(BS006_EFFECT_RING, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                }
                for (int step = 0; step < 0x18; step++)
                {
                    pose = Scaling(1.5f, 1.5f, 1.5f) * RotationZ((float)step * 188.49f / 180.0f);
                    pose.SetTranslation(((Node *)DAT_1042d918[BS006_BONE_NECK])->worldMatrix.GetTranslation());
                    DAT_10daaa80.Start(BS006_EFFECT_RING, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                }
            }
            FUN_1003ad20(0x311, 0, 1.0f);
        }
        if (sparks != 0)
        {
            Matrix4 pose;
            EffectOptions options;
            Vec4 spot;
            for (int spark = sparks; spark > 0; spark--)
            {
                int pick = FUN_1013f880() % 4;
                Node *bone = 0;
                spot.x = BS006_FAR_X;
                if (pick >= 0)
                {
                    if (pick <= 2)
                    {
                        if (DAT_1042d918[pick] != 0)
                        {
                            spot.Fill(0.0f);
                            bone = (Node *)DAT_1042d918[pick];
                        }
                    }
                    else if (pick == 3 && Bs006Node(index, BS006_NODE_SPHERE) != 0)
                    {
                        spot.Fill(0.0f);
                        bone = Bs006Node(index, BS006_NODE_SPHERE);
                    }
                    if (bone != 0 && -20000.0f <= spot.x)
                    {
                        spot.Fill(0.0f);
                        int frame = 0;
                        if (sparkSpread != 0)
                            frame = FUN_1013f880() % sparkSpread;
                        DAT_10daaa80.Start(BS006_EFFECT_SPARK + FUN_1013f880() % 2, spot, -1, -1, &bone->worldMatrix, 0,
                                           frame, 0, &options, 0);
                    }
                }
            }
        }
        if (defeatTimer == 300)
            FUN_1000db30(0, 100, 0x96);
        else if (defeatTimer == 500)
        {
            DAT_10874848.MarkKilled(slot.routeRecord);
            FUN_1013fb10(3, 0, 10);
            model.SetPosition(BS006_FAR_X, 0.0f, 0.0f);
            ending.SetPosition(BS006_FAR_X, 0.0f, 0.0f);
            DAT_104c00c8 = 0;
            DAT_104c0354 = 0;
            DAT_11018734 = 1;
            FUN_1016a8e0();
            DAT_104c00c8 = 0;
        }
        if (defeatTimer < 200)
        {
            float w = (float)(defeatTimer - 100) / 200.0f;
            if (w >= 0.5f)
                ending.scale.w = 0.5f;
            else if (w > 0.0f)
                ending.scale.w = w;
            else
                ending.scale.w = 0.0f;
        }
        else if (defeatTimer < 0x1c2)
        {
            float w = (float)(defeatTimer - 200) / 25.0f + 0.5f;
            if (w <= 0.5f)
                ending.scale.w = 0.5f;
            else
                ending.scale.w = w;
        }
        else if (defeatTimer < 900)
        {
            float w = (float)(defeatTimer - 0x1c2) / 10.0f * 6.0f + 10.5f;
            if (w <= 1.0f)
                ending.scale.w = 1.0f;
            else
                ending.scale.w = w;
        }
        if (defeatTimer < 200)
        {
            float w = (float)(defeatTimer - 100) / 200.0f;
            if (w < 0.5f && w <= 0.0f)
                ending.scale.x = 0.0f;
            else if (w < 0.5f)
                ending.scale.x = w;
            else
                ending.scale.x = 0.5f;
        }
        else if (defeatTimer < 0x1c2)
        {
            float w = (float)(defeatTimer - 200) / 300.0f + 0.5f;
            if (w >= 1.0f)
                ending.scale.x = 1.0f;
            else if (w > 0.5f)
                ending.scale.x = w;
            else
                ending.scale.x = 0.5f;
        }

        // The pieces keep to their nodes of the model (they flash while the model plays its motion).
        Vec4 spot;
        Matrix4 pose;
        for (int piece = 0; piece < BS006_PIECE_COUNT; piece++)
        {
            int pieceSlot = index + BS006_FIRST_PIECE_SLOT + piece;
            AnimModel &part = DAT_10db6ee8[pieceSlot];
            LastPosition &last = DAT_10802bf8[pieceSlot];
            Bs006Piece &pieceState = DAT_1020b430[pieceSlot].bs006Piece;
            Node *node = DAT_1021bdf8[pieceSlot][0];
            if (node != 0)
            {
                DAT_102199b8[pieceSlot] = 1;
                DAT_10255f10[pieceSlot] = -1;
                part.alpha = 0.0f;
                spot.Fill(0.0f);
                if (pieceState.phase == 2)
                {
                    if (node->parent == 0)
                        spot.Copy(last.position);
                    else
                    {
                        float pitch;
                        float yaw;
                        spot.Copy(node->parent->worldMatrix.GetTranslation() - node->worldMatrix.GetTranslation());
                        yaw = atan2f(spot.x, spot.y) + BS006_PI;
                        spot.y = spot.Length2D();
                        spot.x = 0.0f;
                        if (spot.Length() >= 1.0f)
                        {
                            if (spot.z <= 0.0f)
                                pitch = -acosf(spot.Length2D() / spot.Length());
                            else
                                pitch = acosf(spot.Length2D() / spot.Length());
                        }
                        else
                            pitch = 0.0f;
                        pitch += last.step.x;
                        yaw += last.step.z;
                        pose.SetIdentity();
                        pose.SetTranslation(last.position);
                        pose = pose * RotationX(pitch) * RotationZ(yaw);
                        spot.Copy(pose.GetTranslation());
                    }
                }
                else if (model.motion->layer == 2)
                {
                    if (pieceState.phase == 1)
                    {
                        if (node->parent == 0)
                        {
                            spot.Fill(0.0f);
                            last.step.Fill(0.0f);
                        }
                        else
                        {
                            float pitch;
                            float yaw;
                            spot.Copy(node->parent->worldMatrix.GetTranslation() - node->worldMatrix.GetTranslation());
                            yaw = atan2f(spot.x, spot.y) + BS006_PI;
                            spot.y = spot.Length2D();
                            spot.x = 0.0f;
                            if (spot.Length() >= 1.0f)
                            {
                                if (spot.z <= 0.0f)
                                    pitch = -acosf(spot.Length2D() / spot.Length());
                                else
                                    pitch = acosf(spot.Length2D() / spot.Length());
                            }
                            else
                                pitch = 0.0f;
                            last.step.x = -pitch;
                            last.step.z = -yaw;
                        }
                        spot.Copy(last.position);
                    }
                    pieceState.phase++;
                }
                spot.Copy(spot + node->worldMatrix.GetTranslation() + last.history2);
                part.SetPosition(spot.x, spot.y, spot.z);
                part.m_2384 = 1;
                if (part.collider != 0)
                    part.collider->hasCollision = 1;
                if (part.root != 0)
                    part.root->hasCollision = 1;
                part.root->matrix = Scaling(last.position.w, last.position.w, last.position.w);
                DAT_10daaae8[pieceSlot].m_0141 = 1;
                part.root->M_10194730(0x4d, 0x400);
                DAT_1021f3c0[pieceSlot] = (char)0xfd;
            }
        }

        // The camera circles the dragon, closer and faster as the scene goes on.
        Vec4 offset;
        Matrix4 *aim;
        offset.Fill(0.0f);
        offset.y += 1250.0f;
        offset.z += 400.0f;
        if (DAT_1042d918[BS006_BONE_NECK] != 0)
        {
            DAT_10868818.Copy(DAT_10868818 * 0.75f +
                              ((Node *)DAT_1042d918[BS006_BONE_NECK])->worldMatrix.GetTranslation() * 0.25f);
            aim = &((Node *)DAT_1042d918[BS006_BONE_NECK])->worldMatrix;
        }
        else
            aim = &model.matrix;
        DAT_10edbd48.Copy(aim->GetTranslation());
        float turn = -((float)atan2(DAT_10868818.x - DAT_10da9150.x, DAT_10868818.y - DAT_10da9150.y) + BS006_PI);
        offset.RotateZ(turn - (float)(defeatTimer > 0xf0 ? 0xf0 : defeatTimer) * 0.01396f / 240.0f * (float)defeatTimer);
        DAT_10edbd48.z += 400.0f;
        DAT_10866d98.Copy(DAT_10edbd48 + offset * 1.6f);
        if (defeatTimer < 0xa1)
        {
            float k = (float)(0xa0 - defeatTimer);
            k = (k + 1.0f) * k * 0.5f;
            k = k * k;
            float blend = 1.0f - k / 329241088.0f;
            DAT_10866d98.Copy(DAT_10da9150 + (DAT_10866d98 - DAT_10da9150) * blend);
            DAT_10edbd48.Copy(DAT_10868818 + (DAT_10edbd48 - DAT_10868818) * blend);
        }
        Vec4 reserved;
        float distance = (DAT_10edbd48 - DAT_10866d98).Length();
        float &closest = DAT_10255160[index][0];
        if (distance <= closest)
            closest = distance;
        else
        {
            if (1.0f <= closest)
                closest = (distance - closest) * 0.3f + closest;
            else
                closest = distance;
            offset.Copy(DAT_10866d98 - DAT_10edbd48);
            DAT_10866d98.Copy(DAT_10edbd48 + offset.Normalized3() * closest);
        }
        DAT_11af3e28.Copy(DAT_10866d98);
        DAT_118e6c78.Copy(DAT_10edbd48);
        return 1;
    }

    // The fight: the frame's constants, the health the boss shows and where everybody is.
    int health = slot.stats[STAT_HEALTH];
    DAT_1021f9c0[index] = 0;
    model.m_008d = 1;
    DAT_104c02c5 = (unsigned char)index;
    float bar = (float)health / (float)slot.stats[STAT_MAX_HEALTH] * 80.0f + 40.0f;
    if (bar <= 40.0f)
        bar = 40.0f;
    unsigned char barTarget = (unsigned char)(int)bar / 3;
    if (DAT_104c02c7 == 0)
        DAT_104c02c7 = barTarget;
    else if (barTarget < DAT_104c02c7)
        DAT_104c02c7--;
    else if (DAT_104c02c7 < barTarget)
        DAT_104c02c7++;
    st.lastHealth = health;
    playerPos.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    ownPos.Copy(model.matrix.GetTranslation());
    (playerPos - ownPos).Length();
    float distance2D = (playerPos - ownPos).Length2D();

    if (st.state != BS006_STATE_SET_UP)
    {
        if (st.recoverFrames != 0 && --st.recoverFrames == 0)
            st.focusValue = BS006_RESTING_FOCUS;
        DAT_1101dddc = (float)st.focusValue;
        DAT_1020344c = DAT_10255160[index][1];
        Companion0_Update(index);
        Companion1_Update(index);
        Companion2_Update(index);
        FUN_1001a840(index);

        // The boss's health is what the pieces have left of it.
        slot.stats[STAT_HEALTH] = 0;
        int maxHealth = slot.stats[STAT_MAX_HEALTH];
        int lost = 0;
        for (int companion = 0; companion < BS006_COMPANION_COUNT; companion++)
        {
            int pieceSlot = index + BS006_FIRST_PIECE_SLOT + companion;
            int pieceHealth = DAT_10daaae8[pieceSlot].stats[STAT_HEALTH];
            int damage;
            if (pieceHealth > 70000 && DAT_1020b430[pieceSlot].bs006Piece.healthOverride != 0)
                damage = maxHealth - DAT_1020b430[pieceSlot].bs006Piece.healthOverride + 2000;
            else
                damage = maxHealth - pieceHealth + 2000;
            lost += damage > 0 ? damage : 0;
        }
        int bodyDamage = maxHealth - DAT_10daaae8[index + 5].stats[STAT_HEALTH];
        lost += bodyDamage > 0 ? bodyDamage : 0;
        slot.stats[STAT_HEALTH] = maxHealth - lost;

        // The pieces keep to their nodes of the model.
        Vec4 spot;
        Matrix4 pose;
        for (int piece = 0; piece < BS006_PIECE_COUNT; piece++)
        {
            int pieceSlot = index + BS006_FIRST_PIECE_SLOT + piece;
            AnimModel &part = DAT_10db6ee8[pieceSlot];
            LastPosition &last = DAT_10802bf8[pieceSlot];
            Bs006Piece &pieceState = DAT_1020b430[pieceSlot].bs006Piece;
            Node *node = DAT_1021bdf8[pieceSlot][0];
            if (node != 0)
            {
                DAT_102199b8[pieceSlot] = 1;
                DAT_10255f10[pieceSlot] = -1;
                part.alpha = 0.0f;
                spot.Fill(0.0f);
                if (pieceState.phase == 2)
                {
                    if (node->parent == 0)
                        spot.Copy(last.position);
                    else
                    {
                        float pitch;
                        float yaw;
                        spot.Copy(node->parent->worldMatrix.GetTranslation() - node->worldMatrix.GetTranslation());
                        yaw = atan2f(spot.x, spot.y) + BS006_PI;
                        spot.y = spot.Length2D();
                        spot.x = 0.0f;
                        if (spot.Length() >= 1.0f)
                        {
                            if (spot.z <= 0.0f)
                                pitch = -acosf(spot.Length2D() / spot.Length());
                            else
                                pitch = acosf(spot.Length2D() / spot.Length());
                        }
                        else
                            pitch = 0.0f;
                        pitch += last.step.x;
                        yaw += last.step.z;
                        pose.SetIdentity();
                        pose.SetTranslation(last.position);
                        pose = pose * RotationX(pitch) * RotationZ(yaw);
                        spot.Copy(pose.GetTranslation());
                    }
                }
                else if (model.motion->layer == 2)
                {
                    if (pieceState.phase == 1)
                    {
                        if (node->parent == 0)
                        {
                            spot.Fill(0.0f);
                            last.step.Fill(0.0f);
                        }
                        else
                        {
                            float pitch;
                            float yaw;
                            spot.Copy(node->parent->worldMatrix.GetTranslation() - node->worldMatrix.GetTranslation());
                            yaw = atan2f(spot.x, spot.y) + BS006_PI;
                            spot.y = spot.Length2D();
                            spot.x = 0.0f;
                            if (spot.Length() >= 1.0f)
                            {
                                if (spot.z <= 0.0f)
                                    pitch = -acosf(spot.Length2D() / spot.Length());
                                else
                                    pitch = acosf(spot.Length2D() / spot.Length());
                            }
                            else
                                pitch = 0.0f;
                            last.step.x = -pitch;
                            last.step.z = -yaw;
                        }
                        spot.Copy(last.position);
                    }
                    pieceState.phase++;
                }
                spot.Copy(spot + node->worldMatrix.GetTranslation() + last.history2);
                part.SetPosition(spot.x, spot.y, spot.z);
                part.m_2384 = 1;
                if (part.collider != 0)
                    part.collider->hasCollision = 1;
                if (part.root != 0)
                    part.root->hasCollision = 1;
                part.root->matrix = Scaling(last.position.w, last.position.w, last.position.w);
                if (DAT_10daaae8[pieceSlot].m_0141 != 0)
                {
                    part.root->M_10194730(0x4d, 0x400);
                    DAT_1021f3c0[pieceSlot] = (char)0xfd;
                }
            }
        }
    }

    if (st.state == BS006_STATE_SET_UP)
    {
        if (DAT_10c635f8 == 1)
            slot.stats[STAT_VALUE_4] -= 10;
        if (DAT_10c655f0 == 6)
            slot.stats[STAT_VALUE_4] -= 0x4b;
        st.queuedMotion = 3;
        restart = 1;
        st.maxHealth = slot.stats[STAT_MAX_HEALTH];
        motion = 2;
        blendFrames = 0;
        st.focusValue = BS006_RESTING_FOCUS;
        DAT_10255160[index][1] = 1.5f;
        if (model.root == 0)
            return 0;
        char names[BS006_PIECE_COUNT][BS006_NAME_SIZE] = {
            "Bip01 R Calf", "Bip01 L Calf", "muneate_null", "Sphere03",   "Bip01 R Hand",
            "Bip01 L Hand", "Bip01 R Toe0", "Bip01 L Toe0", "Bone_tail5", "Bone_tail3",
            "Bone_tail1",   "Bip01 L Forearm", "Bip01 Head", "Bip01 Spine", "Bip01 R Forearm"};
        st.frames = 2;
        st.state = BS006_STATE_PICK;
        Vec4 pieceOffset[BS006_PIECE_COUNT];
        Vec4 pieceDrop[BS006_PIECE_COUNT];
        pieceOffset[0].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceOffset[1].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceOffset[2].Copy(Vec4(0.0f, 60.0f, 0.0f, 1.0f));
        pieceOffset[3].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceOffset[4].Copy(Vec4(0.0f, 0.0f, -20.0f, 1.0f));
        pieceOffset[5].Copy(Vec4(0.0f, 0.0f, -20.0f, 1.0f));
        pieceOffset[6].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceOffset[7].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceOffset[8].Copy(Vec4(0.0f, 0.0f, 0.0f, 2.0f));
        pieceOffset[9].Copy(Vec4(0.0f, 0.0f, 0.0f, 2.0f));
        pieceOffset[10].Copy(Vec4(-25.0f, -75.0f, -45.0f, 2.0f));
        pieceOffset[11].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceOffset[12].Copy(Vec4(0.0f, 0.0f, -150.0f, 1.6f));
        pieceOffset[13].Copy(Vec4(0.0f, -75.0f, -210.0f, 2.0f));
        pieceOffset[14].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceDrop[0].Copy(Vec4(0.0f, 0.0f, -30.0f, 1.0f));
        pieceDrop[1].Copy(Vec4(0.0f, 0.0f, -30.0f, 1.0f));
        pieceDrop[2].Copy(Vec4(0.0f, 0.0f, -75.0f, 1.0f));
        pieceDrop[3].Copy(Vec4(0.0f, 0.0f, -120.0f, 1.0f));
        pieceDrop[4].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceDrop[5].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceDrop[6].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceDrop[7].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceDrop[8].Copy(Vec4(0.0f, 0.0f, -150.0f, 1.0f));
        pieceDrop[9].Copy(Vec4(0.0f, 0.0f, -150.0f, 1.0f));
        pieceDrop[10].Copy(Vec4(0.0f, 0.0f, -75.0f, 1.0f));
        pieceDrop[11].Copy(Vec4(0.0f, 0.0f, -75.0f, 1.0f));
        pieceDrop[12].Copy(Vec4(0.0f, 0.0f, -75.0f, 1.0f));
        pieceDrop[13].Copy(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
        pieceDrop[14].Copy(Vec4(0.0f, 0.0f, -75.0f, 1.0f));

        for (int piece = 0; piece < BS006_PIECE_COUNT; piece++)
        {
            int pieceSlot = index + BS006_FIRST_PIECE_SLOT + piece;
            AnimModel &part = DAT_10db6ee8[pieceSlot];
            CharSlot &pieceChar = DAT_10daaae8[pieceSlot];
            part.m_0068 = 0.0f;
            part.mode = 0;
            part.m_0267 = 1;
            pieceChar.kind = -1;
            pieceChar.stats[STAT_HEALTH] = 1000000;
            pieceChar.stats[STAT_MAX_HEALTH] = 1000000;
            DAT_10255a88[pieceSlot] = 0;
            DAT_1021f9c0[pieceSlot] = 0;
            pieceChar.stats[STAT_VALUE_4] = slot.stats[STAT_VALUE_4];
            if (piece <= 3)
            {
                DAT_1020b430[pieceSlot].bs006Piece.active = 1;
                if (piece == 3)
                {
                    DAT_10daaae8[index + 5].stats[STAT_HEALTH] = slot.stats[STAT_MAX_HEALTH];
                    DAT_10daaae8[index + 5].stats[STAT_VALUE_4] = slot.stats[STAT_VALUE_4];
                }
                else
                {
                    DAT_10219e40[pieceSlot] = (char)0xff;
                    pieceChar.held = 1;
                    if (piece == 0)
                    {
                        DAT_1021bdf8[pieceSlot][1] = model.root->Find("R-kneepad", 0);
                        DAT_1021bdf8[pieceSlot][2] = model.root->Find("R-kneepad_null", 0);
                        if (DAT_1021bdf8[pieceSlot][1] != 0)
                        {
                            DAT_1021bdf8[pieceSlot][1]->frameSource = &DAT_1020b430[pieceSlot].bs006Piece.frame;
                            DAT_1021bdf8[pieceSlot][1]->RestartColourPlugs("on", 0);
                        }
                    }
                    else if (piece == 1)
                    {
                        DAT_1021bdf8[pieceSlot][1] = model.root->Find("L-kneepad", 0);
                        DAT_1021bdf8[pieceSlot][2] = model.root->Find("L-kneepad_null", 0);
                        if (DAT_1021bdf8[pieceSlot][1] != 0)
                        {
                            DAT_1021bdf8[pieceSlot][1]->frameSource = &DAT_1020b430[pieceSlot].bs006Piece.frame;
                            DAT_1021bdf8[pieceSlot][1]->RestartColourPlugs("on", 0);
                        }
                    }
                    else if (piece == 2)
                    {
                        DAT_1021bdf8[pieceSlot][1] = model.root->Find("muneate", 0);
                        if (DAT_1021bdf8[pieceSlot][1] != 0)
                        {
                            DAT_1021bdf8[pieceSlot][1]->frameSource = &DAT_1020b430[pieceSlot].bs006Piece.frame;
                            DAT_1021bdf8[pieceSlot][1]->RestartColourPlugs("on", 0);
                        }
                    }
                }
            }
            else
            {
                pieceChar.m_0141 = 1;
                DAT_1020b430[pieceSlot].bs006Piece.active = 0;
                part.root->M_10194730(0x4f, 0x400);
            }
            DAT_1021bdf8[pieceSlot][0] = model.root->Find(names[piece], 0);
            if (DAT_1021bdf8[pieceSlot][0] == 0)
                DAT_1021bdf8[pieceSlot][0] = model.root;
            DAT_10802bf8[pieceSlot].position.Copy(pieceOffset[piece]);
            DAT_10802bf8[pieceSlot].history2.Copy(pieceDrop[piece]);
            part.collider->bounds.w = DAT_10802bf8[pieceSlot].position.w * part.collider->bounds.w;
            part.colliderReach = part.collider->bounds.w * 0.578f;
        }
        Bs006Node(index, BS006_NODE_JAW) = model.root->Find("Bone_jaw", 0);
        Bs006Node(index, BS006_NODE_SPHERE) = model.root->Find("Sphere03", 0);
        Bs006Node(index, BS006_NODE_RIGHT_FINGER) = model.root->Find("Bip01 R Finger0", 0);
        Bs006Node(index, BS006_NODE_LEFT_FINGER) = model.root->Find("Bip01 L Finger0", 0);
        Bs006Node(index, BS006_NODE_RIGHT_TOE) = model.root->Find("Bip01 R Toe0", 0);
        Bs006Node(index, BS006_NODE_LEFT_TOE) = model.root->Find("Bip01 L Toe0", 0);
        Bs006Node(index, BS006_NODE_RIGHT_KNEEPAD_NULL) = model.root->Find("R-kneepad_null", 0);
        Bs006Node(index, BS006_NODE_LEFT_KNEEPAD_NULL) = model.root->Find("L-kneepad_null", 0);
        Bs006Node(index, BS006_NODE_CHEST_NULL) = model.root->Find("muneate_null", 0);
        Bs006Node(index, BS006_NODE_TAIL_1) = model.root->Find("Bone_tail1", 0);
        Bs006Node(index, BS006_NODE_TAIL_2) = model.root->Find("Bone_tail2", 0);
        Bs006Node(index, BS006_NODE_TAIL_3) = model.root->Find("Bone_tail3", 0);
        Bs006Node(index, BS006_NODE_TAIL_4) = model.root->Find("Bone_tail4", 0);
        Bs006Node(index, BS006_NODE_TAIL_5) = model.root->Find("Bone_tail5", 0);
        Bs006Node(index, BS006_NODE_LEFT_FINGER_B) = model.root->Find("Bip01 L Finger0", 0);
        Bs006Node(index, BS006_NODE_RIGHT_FINGER_B) = model.root->Find("Bip01 R Finger0", 0);
        Bs006Node(index, BS006_NODE_LEFT_TOE_B) = model.root->Find("Bip01 L Toe0", 0);
        Bs006Node(index, BS006_NODE_RIGHT_TOE_B) = model.root->Find("Bip01 R Toe0", 0);
        DAT_1042d918[BS006_BONE_NECK] = (int)model.root->Find("Bip01 Neck", 0);
        DAT_1042d918[BS006_BONE_SPINE] = (int)model.root->Find("Bip01 Spine", 0);
        DAT_1042d918[BS006_BONE_TAIL_ROOT] = (int)model.root->Find("Bone_tail_root", 0);
        DAT_1042d918[BS006_BONE_EYE] = (int)model.root->Find("eye", 0);
        ending.LoadFile(FUN_1013f8a0("bs_006pa.it3", 0));
        ending.LoadModel(FUN_1013f8a0("bs_006m.it3", 0));
        ending.mode = 0;
        ending.scale.Fill(0.0f);
        ending.SetPosition(BS006_FAR_X, 0.0f, 0.0f);
        DAT_10836c70.ResetFrameTimer();
    }
    // STAGE: state 1, pseudo.c lines 5193-5413
    // STAGE: state 2, pseudo.c lines 5059-5191
    else if (st.state == 3)
    {
        // STAGE: state 3, pseudo.c lines 1932-1944
    }
    else if (st.state == 4)
    {
        // STAGE: state 4, pseudo.c lines 1945-1963
    }
    // STAGE: state 5, pseudo.c lines 4977-5057
    else if (st.state == 6)
    {
        // The fire breath: three rings of shots fly out of the dragon's mouth, then fire bursts on its hand.
        st.frames++;
        motion = 0x14;
        if (st.frames == 1)
            restart = 1;
        if (model.motionFrame == BS006_BREATH_FRAME)
        {
            FUN_10065b10(index, 400, 1, 0);
            Matrix4 shot;
            Vec4 offset;
            EffectOptions options;
            options.flags |= 2;
            for (int ring = 0; ring < 3; ring++)
            {
                int shotChar;
                int shotSlot;
                shot.SetIdentity();
                float height = (float)ring * 30.0f;
                shot.m[0] = ownPos.x;
                shot.m[1] = ownPos.y;
                shot.m[6] = -7.5f;
                shot.m[10] = 0.0f;
                shot.m[7] = 0.1f;
                shot.m[2] = 180.0f - height;
                shot.m[11] = 14.0f;
                shot.m[15] = 350.0f;
                offset.y = -350.0f;
                float turn = model.heading - 0.61087f;
                shot.m[3] = turn;
                offset.RotateZ(-turn);
                shot.m[12] = offset.x + shot.m[0];
                shot.m[13] = offset.y + shot.m[1];
                offset.z = shot.m[2];
                shot.m[14] = shot.m[2];
                if (FUN_1001c410(2, &shot, &shotChar, &shotSlot, turn, 0.1f, 14.0f, 350.0f) == 0)
                    break;
                Matrix4 *inner = &DAT_10868838[shotChar][shotSlot];
                float zero = 0.0f;
                offset.Fill(zero);
                shot.m[2] = 190.0f - height;
                shot.m[15] = 250.0f;
                offset.y = -250.0f;
                offset.RotateZ(-shot.m[3]);
                shot.m[12] = offset.x + shot.m[0];
                shot.m[13] = offset.y + shot.m[1];
                offset.z = shot.m[2];
                shot.m[14] = shot.m[2];
                if (FUN_1001c410(2, &shot, &shotChar, &shotSlot, turn, 0.1f, 14.0f, 250.0f) == 0)
                    break;
                Matrix4 *outer = &DAT_10868838[shotChar][shotSlot];
                float zeroAgain = 0.0f;
                offset.Fill(zeroAgain);
                FUN_1001d8b0(index, BS006_EFFECT_BREATH, offset, -1, -1, inner, (int)outer, 0, 0, &options);
            }
        }
        if (model.motionFrame > 0x1fa && model.motionFrame < 0x209 && Bs006Node(index, BS006_NODE_LEFT_FINGER_B) != 0)
        {
            EffectOptions options;
            Matrix4 pose;
            Vec4 spot;
            options.flags |= 2;
            options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
            options.m_0008 = 1.0f;
            options.m_000e = 5;
            pose.SetIdentity();
            pose.SetTranslation(Bs006Node(index, BS006_NODE_LEFT_FINGER_B)->worldMatrix.GetTranslation());
            FUN_1001d8b0(index, BS006_EFFECT_HAND_FIRE, spot, -1, -1, 0, 0, 0, &pose, &options);
            spot.y = 200.0f;
            spot.RotateZ(-model.heading);
            spot.Copy(spot + Bs006Node(index, BS006_NODE_LEFT_FINGER_B)->worldMatrix.GetTranslation());
            pose.SetTranslation(spot);
            float zero = 0.0f;
            spot.Fill(zero);
            FUN_1001d8b0(index, BS006_EFFECT_HAND_FIRE, spot, -1, -1, 0, 0, 0, &pose, &options);
            spot.x = -100.0f;
            spot.RotateZ(-model.heading);
            spot.Copy(spot + Bs006Node(index, BS006_NODE_LEFT_FINGER_B)->worldMatrix.GetTranslation());
            pose.SetTranslation(spot);
            float zeroAgain = 0.0f;
            spot.Fill(zeroAgain);
            FUN_1001d8b0(index, BS006_EFFECT_HAND_FIRE, spot, -1, -1, 0, 0, 0, &pose, &options);
        }
        if (model.motionFrame == BS006_BREATH_END_FRAME)
            st.state = BS006_STATE_PICK;
    }
    else if (st.state == 7)
    {
        // The rock fall: the dragon slams both hands down (rings on the hands), rocks fall around the player, then
        // the ground shakes.
        FUN_1000ec70(0xf, 0.5f, 0.5f, 0.8f, 1.0f);
        st.frames++;
        motion = 0x15;
        if (st.frames == 1)
            restart = 1;
        if (model.motionFrame > 0x23a && model.motionFrame < 0x247)
        {
            if ((DAT_104c0380 & 1) != 0 && Bs006Node(index, BS006_NODE_LEFT_FINGER_B) != 0 &&
                Bs006Node(index, BS006_NODE_RIGHT_FINGER_B) != 0)
            {
                Vec4 dir;
                Matrix4 pose;
                EffectOptions options;
                options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                options.flags |= 2;
                options.m_0000 = 0;
                options.m_0008 = 1.5f;
                options.m_000e = 5;

                // The left hand: a ring standing on the hand's motion since the last frame.
                dir.Copy(Bs006Node(index, BS006_NODE_LEFT_FINGER_B)->worldMatrix.GetTranslation() -
                         DAT_10802bf8[index + 1].position);
                float yaw = atan2f(dir.x, dir.y) + BS006_PI;
                float pitch = acosf(dir.Length2D() / dir.Length());
                dir.Copy(dir.Normalized3() * 100.0f);
                if (dir.z < 0.0f)
                    pitch = pitch * -1.0f;
                pose = RotationX(pitch) * RotationZ(yaw);
                pose.SetTranslation(Bs006Node(index, BS006_NODE_LEFT_FINGER_B)->worldMatrix.GetTranslation() + dir);
                float zero = 0.0f;
                dir.Fill(zero);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_RING, dir, -1, -1, 0, 0, 0, &pose, &options);
                pose = RotationX(BS006_QUARTER_TURN) * pose;
                pose.SetTranslation(pose.GetTranslation() - dir * 0.5f);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_SPARK, dir, -1, -1, 0, 0, 0, &pose, &options);
                pose.SetTranslation(pose.GetTranslation() - dir);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_SPARK, dir, -1, -1, 0, 0, 0, &pose, &options);

                // The right hand.
                dir.Copy(Bs006Node(index, BS006_NODE_RIGHT_FINGER_B)->worldMatrix.GetTranslation() -
                         DAT_10802bf8[index + 1].step);
                yaw = atan2f(dir.x, dir.y) + BS006_PI;
                pitch = acosf(dir.Length2D() / dir.Length());
                dir.Copy(dir.Normalized3() * 100.0f);
                if (dir.z < 0.0f)
                    pitch = pitch * -1.0f;
                pose = RotationX(pitch) * RotationZ(yaw);
                pose.SetTranslation(Bs006Node(index, BS006_NODE_RIGHT_FINGER_B)->worldMatrix.GetTranslation() + dir);
                float zeroRight = 0.0f;
                dir.Fill(zeroRight);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_RING, dir, -1, -1, 0, 0, 0, &pose, &options);
                pose = RotationX(BS006_QUARTER_TURN) * pose;
                pose.SetTranslation(pose.GetTranslation() - dir * 0.5f);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_SPARK, dir, -1, -1, 0, 0, 0, &pose, &options);
                pose.SetTranslation(pose.GetTranslation() - dir);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_SPARK, dir, -1, -1, 0, 0, 0, &pose, &options);

                // Flashes on both hands.
                pose.SetIdentity();
                pose.SetTranslation(Bs006Node(index, BS006_NODE_LEFT_FINGER_B)->worldMatrix.GetTranslation());
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_FLASH, dir, -1, -1, 0, 0, 0, &pose, &options);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_DUST, dir, -1, -1, 0, 0, 0, &pose, 0);
                pose.SetTranslation(Bs006Node(index, BS006_NODE_RIGHT_FINGER_B)->worldMatrix.GetTranslation());
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_FLASH, dir, -1, -1, 0, 0, 0, &pose, &options);
                FUN_1001d8b0(index, BS006_EFFECT_SHOCK_DUST, dir, -1, -1, 0, 0, 0, &pose, 0);
            }
            DAT_10802bf8[index + 1].position.Copy(Bs006Node(index, BS006_NODE_LEFT_FINGER_B)->worldMatrix.GetTranslation());
            DAT_10802bf8[index + 1].step.Copy(Bs006Node(index, BS006_NODE_RIGHT_FINGER_B)->worldMatrix.GetTranslation());
        }
        if (model.motionFrame == BS006_ROCKFALL_FRAME)
        {
            FUN_10065b10(index, 0x29a, 1, 0);
            Matrix4 pose;
            Vec4 spot;
            EffectOptions options;
            options.flags |= 2;
            options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
            options.m_0008 = 2.0f;
            for (int rock = 0; rock < 6; rock++)
            {
                do
                {
                    spot.y = (float)(FUN_1013f880() % 0x708);
                    spot.RotateZ((float)(FUN_1013f880() % 0x168) / 180.0f * BS006_PI);
                } while ((ownPos - spot).Length2D() < 300.0f);
                pose.SetIdentity();
                pose.SetTranslation(spot);
                float zero = 0.0f;
                spot.Fill(zero);
                FUN_1001d8b0(index, BS006_EFFECT_ROCK, spot, -1, -1, 0, 0, FUN_1013f880() % 10, &pose, &options);
            }
            spot.Copy(playerPos);
            spot.z = 0.0f;
            pose.SetIdentity();
            pose.SetTranslation(spot);
            if ((ownPos - spot).Length2D() > 300.0f)
            {
                float zero = 0.0f;
                spot.Fill(zero);
                FUN_1001d8b0(index, BS006_EFFECT_ROCK, spot, -1, -1, 0, 0, FUN_1013f880() % 10, &pose, &options);
            }
        }
        if (model.motionFrame == BS006_SLAM_FRAME)
        {
            FUN_10065b10(index, 0x29b, 1, 0);
            FUN_1016b230("19191919191919191928283737464646");
            FUN_10055ee0();
            Vec4 spot;
            Matrix4 pose;
            EffectOptions options;
            options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
            options.m_0008 = 1.1f;
            options.m_000e = 0;
            spot.Copy((Bs006Node(index, BS006_NODE_JAW)->worldMatrix.GetTranslation() +
                       Bs006Node(index, BS006_NODE_SPHERE)->worldMatrix.GetTranslation()) * 0.5f);
            pose.SetIdentity();
            pose.SetTranslation(spot);
            float zero = 0.0f;
            spot.Fill(zero);
            if (DAT_1021f3c0[DAT_1020c3b8] < 0)
                options.m_0008 = options.m_0008 * 0.9f;
            if (DAT_1021f3c0[DAT_1020c3b4] < 0)
                options.m_0008 = options.m_0008 * 0.9f;
            FUN_1001d8b0(index, BS006_EFFECT_SLAM, spot, -1, -1, 0, 0, 0, &pose, &options);
        }
        if (model.motionFrame == BS006_ROCKFALL_END_FRAME)
        {
            st.state = BS006_STATE_PICK;
            FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
    // STAGE: state 8, pseudo.c lines 4611-4975
    else if (st.state == 9)
    {
        // The jump: a burst from the chest, a short wait, then a leap (the vector in DAT_10802bf8[index].position is
        // the speed it carries) that ends when the landing motion reaches its frame.
        st.frames++;
        if (st.frames == 1 && Bs006Node(index, BS006_NODE_CHEST_NULL) != 0)
        {
            EffectOptions options;
            Matrix4 pose;
            Vec4 spot;
            pose = RotationZ(model.heading);
            pose.SetTranslation(Bs006Node(index, BS006_NODE_CHEST_NULL)->worldMatrix.GetTranslation());
            options.flags |= 2;
            for (int burst = 15; burst != 0; burst--)
                FUN_1001d8b0(index, BS006_EFFECT_JUMP_BURST, spot, -1, -1, 0, 0, 0, &pose, &options);
            FUN_10065b10(index, 0x29d, 1, 0);
        }
        if (st.frames < 1000)
        {
            if (st.frames % 2 != 0 && Bs006Node(index, BS006_NODE_CHEST_NULL) != 0)
            {
                EffectOptions options;
                Matrix4 pose;
                Vec4 spot;
                pose = RotationZ(model.heading);
                pose.SetTranslation(Bs006Node(index, BS006_NODE_CHEST_NULL)->worldMatrix.GetTranslation());
                options.flags |= 2;
                FUN_1001d8b0(index, BS006_EFFECT_JUMP_CHARGE, spot, -1, -1, 0, 0, 0, &pose, &options);
            }
            motion = 2;
            if (st.frames > 0x3c)
            {
                motion = 0x1b;
                restart = 1;
                blendFrames = 0;
                st.frames = 1000;
                if (Bs006Node(index, BS006_NODE_CHEST_NULL) != 0)
                {
                    EffectOptions options;
                    Matrix4 pose;
                    Vec4 spot;
                    pose = RotationZ(model.heading);
                    pose.SetTranslation(Bs006Node(index, BS006_NODE_CHEST_NULL)->worldMatrix.GetTranslation());
                    options.flags |= 2;
                    for (int burst = 7; burst != 0; burst--)
                        FUN_1001d8b0(index, BS006_EFFECT_JUMP_LAUNCH, spot, -1, -1, 0, 0, FUN_1013f880() % 5, &pose,
                                     &options);
                }
                float zero = 0.0f;
                DAT_10802bf8[index].position.Fill(zero);
                DAT_10802bf8[index].position.y = 100.0f;
                DAT_10802bf8[index].position.RotateZ(-model.heading);
                FUN_10065b10(index, 0x29e, 1, 0);
            }
            model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
            FUN_1001bfb0(index, BS006_JUMP_TURN);
        }
        else if (st.frames < 2000)
        {
            move.Copy(DAT_10802bf8[index].position);
            move.Copy(move * 0.9f);
            DAT_10802bf8[index].position.Copy(move);
            move.z = 10.0f;
            if (ownPos.Length2D() < BS006_REACH_NEAR)
            {
                moveMode = BS006_MOVE_ADD;
                if (Bs006Node(index, BS006_NODE_RIGHT_FINGER) != 0 && Bs006Node(index, BS006_NODE_LEFT_FINGER) != 0 &&
                    Bs006Node(index, BS006_NODE_RIGHT_TOE) != 0 && Bs006Node(index, BS006_NODE_LEFT_TOE) != 0)
                {
                    Matrix4 pose;
                    Vec4 spot;
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_RIGHT_FINGER)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_RIGHT_FINGER)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_RIGHT_FINGER)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_LEFT_FINGER)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_LEFT_FINGER)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_LEFT_FINGER)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_RIGHT_TOE)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_RIGHT_TOE)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_RIGHT_TOE)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_LEFT_TOE)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_LEFT_TOE)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_LEFT_TOE)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                }
            }
            if (model.motionFrame == BS006_JUMP_LAND_FRAME)
                st.frames = 2000;
        }
        else if (st.frames < 3000)
        {
            move.Copy(DAT_10802bf8[index].position);
            move.Copy(move * 0.9f);
            DAT_10802bf8[index].position.Copy(move);
            move.z = 10.0f;
            if (ownPos.Length2D() < BS006_REACH_NEAR && move.Length2D() > 10.0f)
            {
                moveMode = BS006_MOVE_ADD;
                if (Bs006Node(index, BS006_NODE_RIGHT_FINGER) != 0 && Bs006Node(index, BS006_NODE_LEFT_FINGER) != 0 &&
                    Bs006Node(index, BS006_NODE_RIGHT_TOE) != 0 && Bs006Node(index, BS006_NODE_LEFT_TOE) != 0)
                {
                    Matrix4 pose;
                    Vec4 spot;
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_RIGHT_FINGER)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_RIGHT_FINGER)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_RIGHT_FINGER)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_LEFT_FINGER)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_LEFT_FINGER)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_LEFT_FINGER)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_RIGHT_TOE)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_RIGHT_TOE)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_RIGHT_TOE)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                    pose = TranslationMatrix(Bs006Node(index, BS006_NODE_LEFT_TOE)->worldMatrix._41,
                                             Bs006Node(index, BS006_NODE_LEFT_TOE)->worldMatrix._42,
                                             Bs006Node(index, BS006_NODE_LEFT_TOE)->worldMatrix._43);
                    DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, 0, &pose, 0, 0);
                }
            }
            motion = 2;
            if (st.frames > BS006_JUMP_REST_FRAMES)
                st.state = BS006_STATE_PICK;
        }
        if (st.frames > 1000)
        {
            if (st.frames % 2 != 0)
            {
                if (Bs006Node(index, BS006_NODE_CHEST_NULL) != 0)
                {
                    EffectOptions options;
                    Matrix4 pose;
                    Vec4 spot;
                    pose = RotationZ(model.heading);
                    pose.SetTranslation(Bs006Node(index, BS006_NODE_CHEST_NULL)->worldMatrix.GetTranslation());
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.m_0008 = 3.5f;
                    options.flags |= 2;
                    FUN_1001d8b0(index, BS006_EFFECT_JUMP_TRAIL, spot, -1, -1, 0, 0, 0, &pose, &options);
                }
            }
        }
    }
    else if (st.state == 10)
    {
        // The high jump onto the player: it rises with the jump motion, rocks fall where it took off, it comes down
        // over the player and lands with a shake.
        st.frames++;
        if (st.frames == 1)
        {
            motion = 0xd;
            restart = 1;
        }
        else if (st.frames < 1000)
        {
            if (model.motionFrame == BS006_HIGH_JUMP_TAKEOFF_FRAME)
            {
                EffectOptions options;
                options.flags |= 2;
                FUN_1001d8b0(index, BS006_EFFECT_LANDING, ownPos, -1, -1, 0, 0, 0, 0, &options);
                FUN_10065b10(index, 0x308, 1, 0);
            }
            if (model.motionFrame > BS006_HIGH_JUMP_TAKEOFF_FRAME)
            {
                model.mode = 0;
                moveMode = BS006_MOVE_ADD;
                move.z = 80.0f;
                model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
                FUN_1001bfb0(index, BS006_RISE_TURN);
                DAT_10802bf8[index].position.Copy(move);
            }
            if (model.motionFrame == BS006_HIGH_JUMP_PEAK_FRAME)
            {
                st.frames = 1000;
                float zero = 0.0f;
                DAT_10802bf8[index].history2.Fill(zero);
                DAT_10802bf8[index].history2.y = (float)(FUN_1013f880() % 0x4b0) + 400.0f;
                DAT_10802bf8[index].history2.RotateZ((float)(FUN_1013f880() % 0x2d0) * BS006_PI / 360.0f);
                Matrix4 pose;
                EffectOptions options;
                Vec4 spot;
                pose.SetIdentity();
                options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                options.m_0008 = 1.25f;
                for (int debris = 0; debris < FUN_1013f880() % 6 + 3; debris++)
                {
                    int tries = 0;
                    do
                    {
                        float zeroSpot = 0.0f;
                        spot.Fill(zeroSpot);
                        spot.y = (float)(FUN_1013f880() % 0x6a4);
                        spot.RotateZ((float)(FUN_1013f880() % 0x2d0) * BS006_PI / 360.0f);
                        if ((spot - DAT_10802bf8[index].history2).Length2D() > 450.0f)
                            break;
                        tries++;
                    } while (tries < 10);
                    pose.SetTranslation(spot);
                    float zeroAgain = 0.0f;
                    spot.Fill(zeroAgain);
                    FUN_1001d8b0(index, BS006_EFFECT_DEBRIS, spot, -1, -1, 0, 0, FUN_1013f880() % 30, &pose, &options);
                }
            }
        }
        else if (st.frames < 2000)
        {
            motion = 0xe;
            move.z = 80.0f;
            moveMode = BS006_MOVE_ADD;
            model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
            FUN_1001bfb0(index, BS006_RISE_TURN);
            if (ownPos.z > 2000.0f)
            {
                st.frames = 2000;
                model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
                Vec4 soundPos;
                soundPos.Copy(ownPos);
                soundPos.z = 300.0f;
                FUN_1003ad20(0x299, &soundPos, 1.0f);
            }
            Vec4 away;
            away.Copy(DAT_10802bf8[index].history2 - ownPos);
            away.z = 0.0f;
            if (away.Length2D() > 100.0f)
                away.Copy(away.Normalized3() * 100.0f);
            move.x = away.x * 0.4f + DAT_10802bf8[index].position.x * 0.6f;
            move.y = DAT_10802bf8[index].position.y * 0.6f + away.y * 0.4f;
            DAT_10802bf8[index].position.Copy(move * 0.95f);
        }
        else if (st.frames < 3000)
        {
            model.mode = 1;
            if (ownPos.z >= 1500.0f || model.motion->layer == 0x13 || model.motion->nextLayer == 0x13)
                model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
            else
            {
                motion = 0x13;
                restart = 1;
            }
            if (ownPos.z < DAT_10802bf8[index].step.z && ownPos.z < 10.0f && 10.0f <= DAT_10802bf8[index].step.z)
            {
                EffectOptions options;
                FUN_1016b230("284545");
                FUN_10055ee0();
                options.flags |= 2;
                FUN_1001d8b0(index, BS006_EFFECT_LANDING, ownPos, -1, -1, 0, 0, 0, 0, &options);
                Matrix4 pose;
                Vec4 spot;
                options.m_0000 = 0;
                options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                options.m_0008 = 1.7f;
                options.m_000e = 5;
                pose.SetIdentity();
                pose.SetTranslation(ownPos);
                FUN_1001d8b0(index, BS006_EFFECT_LANDING_SHOCK, spot, -1, -1, 0, 0, 0, &pose, &options);
                FUN_1001d8b0(index, BS006_EFFECT_LANDING_SMOKE, spot, -1, -1, 0, 0, 0, &pose, &options);
                FUN_10065b10(index, 0x309, 1, 0);
            }
            if (ownPos.z > 500.0f)
            {
                Vec4 away;
                away.Copy(DAT_10802bf8[index].history2 - ownPos);
                away.z = 0.0f;
                if (away.Length2D() > 100.0f)
                    away.Copy(away.Normalized3() * 100.0f);
                move.x = away.x * 0.4f + DAT_10802bf8[index].position.x * 0.6f;
                move.y = DAT_10802bf8[index].position.y * 0.6f + away.y * 0.4f;
                DAT_10802bf8[index].position.Copy(move * 0.95f);
            }
            else
            {
                move.x = DAT_10802bf8[index].position.x * 0.6f;
                move.y = DAT_10802bf8[index].position.y * 0.6f;
                DAT_10802bf8[index].position.Copy(move * 0.9f);
            }
            if (model.motionFrame == BS006_HIGH_JUMP_END_FRAME)
            {
                st.state = BS006_STATE_PICK;
                if (FUN_1013f880() % 3 != 0)
                {
                    if (slot.stats[STAT_HEALTH] < slot.stats[STAT_MAX_HEALTH] / 2)
                        st.queuedMotion = 9;
                }
            }
        }
    }
    else if (st.state == 0xb)
    {
        // The last stand: once one of the pieces after the companions is almost gone (or at the first frame) the
        // dragon jumps like in state 10, then the camera swings round it while it roars, it shakes the room with
        // debris and drops back to the middle for the final jumps.
        if (((st.frames == 0 && st.attackPhase != 1 && st.attackPhase != 3 &&
              DAT_10daaae8[index + 2].stats[STAT_HEALTH] > BS006_PIECE_ALIVE) ||
             DAT_10daaae8[index + 3].stats[STAT_HEALTH] > BS006_PIECE_ALIVE) ||
            DAT_10daaae8[index + 4].stats[STAT_HEALTH] > BS006_PIECE_ALIVE)
        {
            st.frames = -1;
            st.state = BS006_STATE_PICK;
        }
        st.frames++;
        if (st.frames != 0)
        {
            if (st.frames < 1000)
            {
                if (st.frames == 1)
                    restart = 1;
                motion = 0xd;
                if (model.motionFrame == BS006_HIGH_JUMP_TAKEOFF_FRAME)
                {
                    EffectOptions options;
                    options.flags |= 2;
                    FUN_1001d8b0(index, BS006_EFFECT_LANDING, ownPos, -1, -1, 0, 0, 0, 0, &options);
                    FUN_10065b10(index, 0x308, 1, 0);
                }
                if (model.motionFrame > BS006_HIGH_JUMP_TAKEOFF_FRAME)
                {
                    model.mode = 0;
                    moveMode = BS006_MOVE_ADD;
                    move.z = 80.0f;
                    model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
                    FUN_1001bfb0(index, BS006_RISE_TURN);
                    DAT_10802bf8[index].position.Copy(move);
                }
                if (model.motionFrame == BS006_HIGH_JUMP_PEAK_FRAME)
                {
                    st.frames = 1000;
                    DAT_10802bf8[index].history2.Copy(ownPos);
                }
            }
            else if (st.frames < 2000)
            {
                armed = 1;
                motion = 0xe;
                move.z = 80.0f;
                moveMode = BS006_MOVE_ADD;
                model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
                FUN_1001bfb0(index, BS006_RISE_TURN);
                if (ownPos.z > 2000.0f)
                {
                    st.frames = 2000;
                    float heading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
                    model.targetHeading = heading;
                    model.heading = heading;
                }
            }
            else if (st.frames < 3000)
            {
                if (st.frames == 0x7d1)
                {
                    FUN_1000da20(0x584);
                    if (Bs006Node(index, BS006_NODE_SPHERE) != 0)
                        DAT_10da93f0.Copy(Bs006Node(index, BS006_NODE_SPHERE)->worldMatrix.GetTranslation());
                    else
                    {
                        DAT_10da93f0.Copy(ownPos);
                        DAT_10da93f0.z = DAT_10da93f0.z + 500.0f;
                    }
                    float zero = 0.0f;
                    DAT_108393d8.Fill(zero);
                    DAT_108393d8.y = -1600.0f;
                    DAT_108393d8.RotateX((float)(FUN_1013f880() % 0x5a - 0x2d) * BS006_PI / 180.0f);
                    if (FUN_1013f880() % 2 != 0)
                    {
                        DAT_108393d8.RotateZ(-model.targetHeading - BS006_ROOM_TURN);
                        DAT_108393d8.w = 0.05236f;
                    }
                    else
                    {
                        DAT_108393d8.RotateZ(BS006_ROOM_TURN - model.targetHeading);
                        DAT_108393d8.w = -0.05236f;
                    }
                    DAT_108393d8.Copy(DAT_108393d8 + DAT_10da93f0);
                    DAT_10c6356c = (int)DAT_108393d8.x;
                    DAT_10c63570 = (int)DAT_108393d8.y;
                    DAT_10c63574 = (int)DAT_108393d8.z;
                    DAT_10c63578 = (int)DAT_10da93f0.x;
                    DAT_10c6357c = (int)DAT_10da93f0.y;
                    DAT_10c63580 = (int)DAT_10da93f0.z;
                    Script_ValueCommand(0x7543, 0x12);
                    FUN_1015c9e0(0xf);
                    DAT_10daaa80.RemoveOwnedBy(index + BS006_EFFECT_OWNER_BASE);
                    FUN_1001c4c0(1, 1);
                }
                if (st.frames > 0x7de && st.frames < 0x7f8)
                {
                    int frame = model.motionFrame;
                    st.frames = 0x7df;
                    if (frame == 0x185)
                    {
                        motion = 0xf;
                        restart = 1;
                        blendFrames = 1;
                        st.frames = 0x802;
                    }
                }
                if (model.motionFrame == 0x18b)
                {
                    FUN_1016b230("1919L");
                    FUN_1013fb10(1, 0x8c, 0xf);
                    FUN_10065b10(index, 0x29c, 1, 0);
                }
                else if (model.motionFrame == 0x1b8)
                {
                    FUN_1016b230("1919193737454545");
                    FUN_1013fb10(1, 0, 0x1e);
                }
                if (model.motionFrame == 0x1c1)
                {
                    st.frames = 3000;
                    Script_ValueCommand(0x7543, 0);
                    DAT_10255160[index][1] = 1.5f;
                    FUN_1015c9e0(0x1e);
                    st.focusValue = -200;
                    st.recoverFrames = 0xd2;
                }
            }
            else if (st.frames < 4000)
            {
                Script_ValueCommand(0x7543, 0);
                motion = 0xe;
                if (st.frames == 0xbd6)
                {
                    Matrix4 pose;
                    Vec4 offset;
                    Vec4 spot;
                    EffectOptions options;
                    float angle = (float)(FUN_1013f880() % 0x168) / 180.0f * BS006_PI;
                    pose.SetIdentity();
                    offset.y = 1700.0f;
                    offset.RotateZ(angle);
                    pose = RotationZ(-angle);
                    options.m_0000 = 0;
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.m_0008 = 2.5f;
                    options.flags |= 2;
                    if (FUN_1013f880() % 2 != 0)
                        FUN_1001d8b0(index, 0x724, spot, -1, -1, 0, 0, 5, &pose, &options);
                    else
                        FUN_1001d8b0(index, 0x72b, spot, -1, -1, 0, 0, 5, &pose, &options);
                    FUN_1003ad20(0x296, 0, 1.0f);
                }
                if (st.frames == 0xc5d)
                {
                    Matrix4 pose;
                    EffectOptions options;
                    Vec4 spot;
                    pose.SetIdentity();
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.m_0008 = 1.25f;
                    for (int debris = 0; debris < FUN_1013f880() % 6 + 5; debris++)
                    {
                        int tries = 0;
                        do
                        {
                            float zeroSpot = 0.0f;
                            spot.Fill(zeroSpot);
                            spot.y = (float)(FUN_1013f880() % 0x6a4);
                            spot.RotateZ((float)(FUN_1013f880() % 0x2d0) * BS006_PI / 360.0f);
                            if ((spot - DAT_10802bf8[index].history2).Length2D() > 450.0f)
                                break;
                            tries++;
                        } while (tries < 10);
                        pose.SetTranslation(spot);
                        float zeroAgain = 0.0f;
                        spot.Fill(zeroAgain);
                        FUN_1001d8b0(index, BS006_EFFECT_DEBRIS, spot, -1, -1, 0, 0, FUN_1013f880() % 30, &pose, &options);
                    }
                }
                if (st.frames > 0xc6c)
                {
                    st.frames = 4000;
                    ownPos.x = 0.0f;
                    ownPos.y = 0.0f;
                    move.Copy(ownPos);
                    moveMode = BS006_MOVE_PLACE;
                    model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
                    Vec4 soundPos;
                    soundPos.Copy(ownPos);
                    soundPos.z = 300.0f;
                    FUN_1003ad20(0x299, &soundPos, 1.0f);
                }
            }
            else if (st.frames < 5000)
            {
                model.mode = 1;
                if (ownPos.z >= 1500.0f || model.motion->layer == 0x13 || model.motion->nextLayer == 0x13)
                    model.targetHeading = atan2f(playerPos.x - ownPos.x, playerPos.y - ownPos.y) + BS006_PI;
                else
                {
                    motion = 0x13;
                    restart = 1;
                }
                if (ownPos.z < DAT_10802bf8[index].step.z && ownPos.z < 10.0f && 10.0f <= DAT_10802bf8[index].step.z)
                {
                    EffectOptions options;
                    FUN_1016b230("284545");
                    FUN_10055ee0();
                    options.flags |= 2;
                    FUN_1001d8b0(index, BS006_EFFECT_LANDING, ownPos, -1, -1, 0, 0, 0, 0, &options);
                    Matrix4 pose;
                    Vec4 spot;
                    options.m_0000 = 0;
                    options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
                    options.m_0008 = 1.7f;
                    options.m_000e = 5;
                    pose.SetIdentity();
                    pose.SetTranslation(ownPos);
                    FUN_1001d8b0(index, BS006_EFFECT_LANDING_SHOCK, spot, -1, -1, 0, 0, 0, &pose, &options);
                    FUN_1001d8b0(index, BS006_EFFECT_LANDING_SMOKE, spot, -1, -1, 0, 0, 0, &pose, &options);
                    FUN_10065b10(index, 0x309, 1, 0);
                }
                if (model.motionFrame == BS006_HIGH_JUMP_END_FRAME)
                {
                    st.state = BS006_STATE_PICK;
                    st.queuedMotion = 0xe;
                    for (int companion = 0; companion < BS006_COMPANION_COUNT; companion++)
                    {
                        int pieceSlot = index + BS006_FIRST_PIECE_SLOT + companion;
                        if (DAT_10daaae8[pieceSlot].stats[STAT_HEALTH] < BS006_PIECE_ALIVE)
                            DAT_1020b430[pieceSlot].downMode = 0x47;
                    }
                    if (st.attackPhase == 1 || st.attackPhase == 3)
                        st.attackPhase++;
                }
            }
        }
    }
    else if (st.state == 0xc)
    {
        // STAGE: state 0xc, pseudo.c lines 3148-3167
    }
    else if (st.state == 0xd)
    {
        // STAGE: state 0xd, pseudo.c lines 3168-3317
    }
    else if (st.state == 0xe)
    {
        // STAGE: state 0xe, pseudo.c lines 3318-3326
    }
    else if (st.state == 0xc9)
    {
        // STAGE: state 0xc9, pseudo.c lines 3327-3349
    }
    else if (st.state == 0xca)
    {
        // STAGE: state 0xca, pseudo.c lines 3350-3484
    }
    // STAGE: state 0xcb, pseudo.c lines 4558-4610
    else if (st.state == 0xcc)
    {
        // STAGE: state 0xcc, pseudo.c lines 3487-3864
    }
    // STAGE: state 0xcd, pseudo.c lines 4541-4557
    else if (st.state == 0x12d)
    {
        // STAGE: state 0x12d, pseudo.c lines 3867-3883
    }
    else if (st.state == 0x12e)
    {
        // STAGE: state 0x12e, pseudo.c lines 3884-3901
    }
    // STAGE: state 0x12f, pseudo.c lines 4428-4540
    else if (st.state == 0x130)
    {
        // STAGE: state 0x130, pseudo.c lines 3903-4018
    }
    else if (st.state == BS006_STATE_PICK)
    {
        // STAGE: state 100 (pick), pseudo.c lines 4033-4427
    }
    else if (st.state == BS006_STATE_PLAY_QUEUED)
    {
        // STAGE: state 0x65, pseudo.c lines 4020-4030
    }

    // The common tail: remember the state, move the boss, play the motion, and the effects of the stomps.
    if (st.state != BS006_STATE_PICK)
        st.lastState = st.state;
    DAT_10802bf8[index].step.Copy(ownPos);
    if (moveMode == BS006_MOVE_STEP)
    {
        if ((ownPos + move).Length2D() > BS006_REACH)
        {
            float reach = ownPos.Length2D();
            if (reach < (ownPos + move).Length2D())
            {
                move.Copy(ownPos + move);
                float height = move.z;
                move.z = 0.0f;
                move.Copy(move.Normalized3() * BS006_REACH);
                move.z = height;
                move.Copy(move - ownPos);
            }
        }
        FUN_100241e0(index, move.x, move.y, move.z);
    }
    else if (moveMode == BS006_MOVE_ADD)
    {
        if ((ownPos + move).Length2D() > BS006_REACH)
        {
            float reach = ownPos.Length2D();
            if (reach < (ownPos + move).Length2D())
            {
                move.Copy(ownPos + move);
                float height = move.z;
                move.z = 0.0f;
                move.Copy(move.Normalized3() * BS006_REACH);
                move.z = height;
                move.Copy(move - ownPos);
            }
        }
        model.matrix._41 = move.x + model.matrix._41;
        model.matrix._42 = move.y + model.matrix._42;
        model.matrix._43 = move.z + model.matrix._43;
    }
    else if (moveMode == BS006_MOVE_PLACE)
    {
        if (move.Length2D() > BS006_REACH)
        {
            float height = move.z;
            move.z = 0.0f;
            move.Copy(move.Normalized3() * BS006_REACH);
            move.z = height;
        }
        model.matrix._41 = move.x;
        model.matrix._42 = move.y;
        model.matrix._43 = move.z;
    }
    if (motion > -1)
        model.PlayMotion(motion, restart, blendFrames, 1.0f);
    if (st.impact == 1 || st.impact == 2)
    {
        Matrix4 pose;
        Vec4 where;
        EffectOptions options;
        options.m_0004 = slot.stats[STAT_EFFECT_PARAM];
        options.m_0008 = 1.0f;
        options.flags |= 2;
        options.m_000e = 5;
        where.y = 100.0f;
        where.RotateZ(-model.heading);
        where.Copy(where + ownPos);
        pose.SetIdentity();
        pose.SetTranslation(where);
        FUN_1001d8b0(index, BS006_EFFECT_IMPACT, where, -1, -1, 0, 0, 0, &pose, &options);
    }
    float *lastHeight = DAT_10255160[index + 1];
    for (int node = BS006_NODE_RIGHT_FINGER; node <= BS006_NODE_LEFT_TOE; node++, lastHeight++)
    {
        Node *bone = Bs006Node(index, node);
        if (bone != 0)
        {
            if (*lastHeight > 30.0f && bone->worldMatrix._43 <= 30.0f)
            {
                FUN_1003ad20(BS006_SOUND_STOMP, &bone->worldMatrix.GetTranslation(), 1.0f);
                Matrix4 pose;
                Vec4 spot;
                pose = TranslationMatrix(bone->worldMatrix._41, bone->worldMatrix._42, bone->worldMatrix._43);
                DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, FUN_1013f880() % 3, &pose, 0, 0);
                DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, FUN_1013f880() % 3, &pose, 0, 0);
                DAT_10daaa80.Start(BS006_EFFECT_DUST, spot, -1, -1, 0, 0, FUN_1013f880() % 3, &pose, 0, 0);
            }
            *lastHeight = bone->worldMatrix._43;
        }
    }
    if (armed != 0)
    {
        DAT_10daaae8[index + 2].m_0141 = 1;
        DAT_10daaae8[index + 3].m_0141 = 1;
        DAT_10daaae8[index + 4].m_0141 = 1;
        DAT_10daaae8[index + 5].m_0141 = 1;
    }
    return 0;
}
