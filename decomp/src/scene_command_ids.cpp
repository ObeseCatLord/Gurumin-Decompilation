// The scene commands from 30000 on: the second half of the script "value" calls (value_command.cpp handles the
// numbers below). They place, show and remove the map's characters, gimmicks and effect models, aim the camera and
// the player, read positions into the registers DAT_10c6356c.. and run small map specific checks. Arguments and
// results travel in the same registers (script flags 0x1a4.. are the extra arguments).

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game.h"
#include "game_vars.h"
#include "map_vars.h"
#include "world.h"
#include "camera_vars.h"
#include "scene_command.h"
#include "script_system.h"
#include "text_window.h"
#include "effect_manager.h"
#include "effect_holds.h"
#include "map_records.h"
#include "sound_vars.h"
#include "render_vars.h"
#include "value_command.h"
#include "wisp_caster_enemy.h"
#include "hop_spit_enemy.h"
#include "partner_follow.h"
#include "field_item_menu.h"
#include "game_main.h"
#include "char_spawn.h"
#include "target_race.h"
#include "doppler_loops.h"
#include "anim_step_vars.h"
#include "player_frame.h"
#include "scene_frame_vars.h"
#include "scene_command_ids.h"
#include "game_main.h"                 // FUN_10137b10 (the frame step)

// 0x10001fd0: the game's out-of-line absolute value of a float (math_float.cpp calls it floorf).
#define FLOAT_SIGN_CLEARED floorf

// The shot point's registers hold whole units; the commands convert them back to floats.
#define REGISTER_X DAT_10c6356c
#define REGISTER_Y DAT_10c63570
#define REGISTER_Z DAT_10c63574

#define SLOT_FIRST_PEOPLE 0x3a
#define SLOT_FIRST_OBJECT 0x5c
#define SLOT_END_PEOPLE 0x5c
#define SLOT_END_OBJECT 0x7c
#define OFF_SCREEN (-200000.0f)
#define HEADING_OFFSET 3.14159f
#define HEADING_OFFSET_SHORT 3.1415f
#define FLAG_ARG_A 0x1a4
#define FLAG_ARG_B 0x1a5
#define FLAG_ARG_C 0x1a6
#define FLAG_ARG_D 0x1a7
#define FLAG_ARG_E 0x1a8
#define FLAG_ARG_F 0x1a9
#define FLAG_ARG_G 0x1aa
#define FLAG_ARG_H 0x1ab
#define FLAG_ARG_I 0x1ac
#define FLAG_ARG_J 0x1ad
#define EFFECT_PLACE_MARK 0x3f4
#define SOUND_APPEAR 0x2d0
#define SOUND_LEVER 0x37e

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Marks character `index` as in play the first time a command touches it and remembers where it stands.
static void MarkInPlay(int index)
{
    if (DAT_1020d540[index] == 0)
    {
        DAT_1020d540[index] = 1;
        DAT_1021b4c8[index] = (char)(index % 5);
        DAT_10867e18[index].Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    }
}

// 0x1009e380 (Script_SceneCommand). Runs scene command `id` (30000..30129) with `value`; the extra arguments are
// the script flags 0x1a4.., the results go to the registers DAT_10c6356c... Returns 0.
int Script_SceneCommand(int id, int value)
{
    Vec4 scratchA;
    Vec4 scratchB;
    Vec4 scratchC;
    Matrix4 scratchMatrix;

    if (id == SCENE_ID_CLEAR_SCRATCH)
    {
        DAT_10c635b4 = 0;
        DAT_10c635b8 = 0;
        return 0;
    }
    if (id == SCENE_ID_HIDE_GIMMICK_AT_MODEL)
    {
        int count = FUN_10017b80();
        for (int i = 0; i < count; i++)
        {
            if (DAT_10860df0[i].position.x == DAT_10db6ee8[value].matrix.translation.x &&
                DAT_10860df0[i].position.y == DAT_10db6ee8[value].matrix.translation.y)
            {
                DAT_10860df0[i].position.Set(OFF_SCREEN, 0.0f, 0.0f, 0.0f);
                break;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_TOGGLE_LEVER)
    {
        Vec4 where;
        if (DAT_10874848.TestScriptFlag(DAT_1032d4b8[value]) == 0)
        {
            where = DAT_10db6ee8[value].matrix.GetTranslation();
            FUN_1003ad20(SOUND_LEVER, &where, 1.0f);
            DAT_10874848.SetScriptFlag(DAT_1032d4b8[value], 1);
            DAT_10db6ee8[value].root->RestartColourPlugs("CLOSE0", DAT_104bf8a4);
        }
        else
        {
            where = DAT_10db6ee8[value].matrix.GetTranslation();
            FUN_1003ad20(SOUND_LEVER, &where, 1.0f);
            DAT_10874848.SetScriptFlag(DAT_1032d4b8[value], 0);
            DAT_10db6ee8[value].root->RestartColourPlugs("OPEN0", DAT_104bf8a4);
        }
        return 0;
    }
    if (id == SCENE_ID_SET_TURNING)
    {
        if (value == 1)
            DAT_104c038c = 1.0f;
        else
            DAT_104c038c = 0.0f;
        return 0;
    }
    if (id == SCENE_ID_SET_VARIANT_AND_MODE)
    {
        DAT_10db6ee8[REGISTER_X].mode = value % 100;
        int variant = (value / 100) % 100;
        if (variant != 0)
            DAT_10db6ee8[REGISTER_X].m_0267 = (char)(variant - 1);
        return 0;
    }
    if (id == SCENE_ID_READ_PLAYER_POSITION)
    {
        REGISTER_X = (int)DAT_118e69e8.x;
        REGISTER_Y = (int)DAT_118e69e8.y;
        REGISTER_Z = (int)DAT_118e69e8.z;
        return 0;
    }
    if (id == SCENE_ID_MIRROR_CAMERA)
    {
        DAT_11af3e28.x = DAT_11af3e28.x * -1.0f;
        DAT_118e69e8.x = DAT_11af3e28.x;
        DAT_118e6c78.x = DAT_118e6c78.x * -1.0f;
        DAT_118e6cb8.x = DAT_118e6c78.x;
        return 0;
    }
    if (id == SCENE_ID_PLAYER_SET_HEIGHT)
    {
        DAT_10db6ee8[0].SetPosition(DAT_10db6ee8[0].matrix.translation.x, DAT_10db6ee8[0].matrix.translation.y,
                                    (float)value + DAT_10db6ee8[0].matrix.translation.z);
        return 0;
    }
    if (id == SCENE_ID_PLAYER_JUMP)
    {
        DAT_10db6ee8[0].verticalSpeed = (float)value;
        DAT_10db6ee8[0].m_229c = 0;
        DAT_10db6ee8[0].m_238c = 0;
        DAT_10db6ee8[0].m_2388 = 1;
        DAT_10db6ee8[0].matrix.translation.z = DAT_10db6ee8[0].matrix.translation.z + 5.0f;
        return 0;
    }
    if (id == SCENE_ID_CAMERA_JUMP_TO_REGISTERS)
    {
        DAT_118e69e8.x = (float)REGISTER_X;
        DAT_118e69e8.y = (float)REGISTER_Y;
        DAT_118e69e8.z = (float)REGISTER_Z;
        DAT_11af3e28.Copy(DAT_118e69e8);
        DAT_118e6cb8.Copy(DAT_10db6ee8[DAT_11018730].matrix.GetTranslation());
        DAT_118e6c78.Copy(DAT_118e6cb8);
        DAT_11af3f48.Copy(DAT_118e69e8);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_10f65688.Copy(DAT_118e69e8);
        return 0;
    }
    if (id == SCENE_ID_HIDE_GIMMICK_AT_REGISTERS)
    {
        int count = FUN_10017b80();
        for (int i = 0; i < count; i++)
        {
            if (DAT_10860df0[i].position.x == (float)REGISTER_X && DAT_10860df0[i].position.y == (float)REGISTER_Y)
            {
                DAT_10860df0[i].position.Set(OFF_SCREEN, 0.0f, 0.0f, 0.0f);
                break;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_MOVE_PEOPLE_AWAY)
    {
        for (int i = 0x48; i < DAT_1021f338; i++)
        {
            if ((int)DAT_10db6ee8[i].matrix.translation.x == REGISTER_X && (int)DAT_10db6ee8[i].matrix.translation.y == REGISTER_Y && DAT_1021f3c0[i] == 6)
                DAT_10db6ee8[i].SetPosition(OFF_SCREEN, DAT_10db6ee8[i].matrix.translation.y,
                                            DAT_10db6ee8[i].matrix.translation.z);
        }
        return 0;
    }
    if (id == SCENE_ID_ADD_TO_EFFECT_HOLDS)
    {
        for (int i = 0; i < DAT_1020b22c; i++)
        {
            if (DAT_1020dcf8[i].position[0] == REGISTER_X && DAT_1020dcf8[i].position[1] == REGISTER_Y)
                DAT_1020dcf8[i].position[2] += value;
        }
        return 0;
    }
    if (id == SCENE_ID_AIM_STEP_AT_MODEL)
    {
        Vec4 start;
        Vec4 end;
        start.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        end.Copy(DAT_10db6ee8[value].matrix.GetTranslation());
        Vec4 difference = end - start;
        end.Copy(difference);
        start.Copy(end);
        Vec4 step = start.Normalized3() * 8.0f;
        end.Copy(step);
        float first = FLOAT_SIGN_CLEARED(start.x);
        float steps;
        if (FLOAT_SIGN_CLEARED(start.y) > first)
            steps = start.y / end.y;
        else
            steps = start.x / end.x;
        int stepCount = (int)steps;
        REGISTER_X = (int)end.x;
        REGISTER_Y = (int)end.y;
        REGISTER_Z = stepCount;
        return 0;
    }
    if (id == SCENE_ID_PEOPLE_ARRIVE)
    {
        Vec4 base = DAT_10db6ee8[0].matrix.GetTranslation();
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value)
            {
                DAT_10210b30[i] = 1;
                DAT_10453008[i] = -1;
                Vec4 position = DAT_10db6ee8[i].matrix.GetTranslation();
                position.x = position.x * -1.0f - (float)REGISTER_X;
                if ((DAT_10c63588 & 1) == 0)
                {
                    DAT_10db6ee8[i].targetHeading = Atan2(base.x - position.x, base.y - position.y) + HEADING_OFFSET;
                    DAT_10db6ee8[i].heading = DAT_10db6ee8[i].targetHeading;
                }
                DAT_10db6ee8[i].SetPosition(position.x, position.y, position.z);
                DAT_10db6ee8[i].PlayMotion(2, 0, 5, 1.0f);
                DAT_104c4438[i].Copy(DAT_10db6ee8[i].matrix.GetTranslation());
                Matrix4 turn = RotationZ(DAT_10db6ee8[i].heading);
                turn.SetTranslation(DAT_10db6ee8[i].matrix.GetTranslation());
                if ((DAT_10c63588 & 2) == 0)
                    DAT_10daaa80.Start(EFFECT_PLACE_MARK, position, -1, -1, 0, 0, 0, &turn, 0, 0);
                DAT_102199b8[i] = 2;
                DAT_10210b30[i] = 0;
                DAT_10254390[i] = (DAT_10c63590 == 100000) ? DAT_10c6358c : 0;
                if ((DAT_10c63588 & 4) == 0)
                    FUN_1003ad20(SOUND_APPEAR, &position, 1.0f);
                MarkInPlay(i);
                if ((DAT_10c63588 & 8) == 0 && DAT_10daaae8[i].kind == CHAR_KIND_ENEMY && DAT_1020b430[i].behaviour == 0x1a)
                    FUN_10043cc0(i);
            }
        }
        return 0;
    }
    if (id == SCENE_ID_PEOPLE_ARRIVE_SIMPLE)
    {
        Vec4 base = DAT_10db6ee8[0].matrix.GetTranslation();
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value)
            {
                DAT_10210b30[i] = 1;
                DAT_10453008[i] = -10000;
                Vec4 position = DAT_10db6ee8[i].matrix.GetTranslation();
                position.x = position.x * -1.0f - (float)REGISTER_X;
                DAT_10db6ee8[i].targetHeading = Atan2(base.x - position.x, base.y - position.y) + HEADING_OFFSET;
                DAT_10db6ee8[i].heading = DAT_10db6ee8[i].targetHeading;
                DAT_10db6ee8[i].SetPosition(position.x, position.y, position.z);
                DAT_10db6ee8[i].PlayMotion(2, 1, 5, 1.0f);
                DAT_104c4438[i].Copy(DAT_10db6ee8[i].matrix.GetTranslation());
                Matrix4 turn = RotationZ(DAT_10db6ee8[i].heading);
                turn.SetTranslation(DAT_10db6ee8[i].matrix.GetTranslation());
                DAT_10daaa80.Start(EFFECT_PLACE_MARK, position, -1, -1, 0, 0, 0, &turn, 0, 0);
                DAT_102199b8[i] = 1;
                DAT_10254390[i] = (DAT_10c63590 == 100000) ? DAT_10c6358c : 0;
                FUN_1003ad20(SOUND_APPEAR, &position, 1.0f);
                DAT_10c63588 = 1;
                MarkInPlay(i);
            }
        }
        return 0;
    }
    if (id == SCENE_ID_SET_ARRIVAL_ARGS)
    {
        DAT_10c63590 = 100000;
        DAT_10c6358c = value;
        return 0;
    }
    if (id == SCENE_ID_CLEAR_PERSON)
    {
        DAT_1020d540[value] = 0;
        DAT_10256420[value] = 0;
        DAT_102199b8[value] = 1;
        return 0;
    }
    if (id == SCENE_ID_SET_CAMERA_MODE)
    {
        if (value == 0)
        {
            if (DAT_104c0354 == 0xc || DAT_104c0354 == 0xd)
                FUN_100165e0(0);
            FUN_1016a8e0();
        }
        else
        {
            if (DAT_104c0354 == 0xc || DAT_104c0354 == 0xd)
                FUN_100165e0(0);
            DAT_10866d98.x = (float)REGISTER_X;
            DAT_10866d98.y = (float)REGISTER_Y;
            DAT_10866d98.z = (float)REGISTER_Z;
            DAT_10edbd48.x = (float)DAT_10c63578;
            DAT_10edbd48.y = (float)DAT_10c6357c;
            DAT_10edbd48.z = (float)DAT_10c63580;
            DAT_104d29d8.Copy(DAT_118e69e8);
            DAT_10f08aa0.Copy(DAT_118e6cb8);
            if (value != 0xc && value != 0xd)
            {
                if (value == 8 || value == 0x12)
                {
                    DAT_118e69e8.Copy(DAT_10866d98);
                    DAT_11af3e28.Copy(DAT_10866d98);
                    DAT_10f65688.Copy(DAT_10866d98);
                    DAT_104d29d8.Copy(DAT_10866d98);
                    DAT_118e6cb8.Copy(DAT_10edbd48);
                    DAT_118e6c78.Copy(DAT_10edbd48);
                    DAT_10f668e8.Copy(DAT_10edbd48);
                    DAT_10f08aa0.Copy(DAT_10edbd48);
                    if (value == 8)
                        DAT_10868838[0][0].SetTranslation(DAT_10edbd48);
                }
                FUN_1016a8d0();
            }
        }
        DAT_104c0354 = value;
        return 0;
    }
    if (id == SCENE_ID_READ_PLAYER_REGISTERS_B)
    {
        REGISTER_X = (int)DAT_118e69e8.x;
        REGISTER_Y = (int)DAT_118e69e8.y;
        REGISTER_Z = (int)DAT_118e69e8.z;
        return 0;
    }
    if (id == SCENE_ID_RENAME_COLLISION_NODES)
    {
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value && DAT_10210d30[i] != 0)
            {
                DAT_10210d30[i] = 2;
                DAT_102199b8[i] = 2;
                int delayClass = (DAT_10daaae8[i].routeRecord % 10 - 1);
                DAT_10453008[i] = DAT_101d6564[(delayClass & ((delayClass < 1) - 1))];
            }
        }
        return 0;
    }
    if (id == SCENE_ID_PEOPLE_ARRIVE_FAR)
    {
        REGISTER_X = -100000;
        Script_ValueCommand(SCENE_ID_PEOPLE_ARRIVE_SIMPLE, value);
        return 0;
    }
    if (id == SCENE_ID_PEOPLE_RETURN)
    {
        Vec4 base = DAT_10db6ee8[0].matrix.GetTranslation();
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value)
            {
                DAT_10daaae8[i].routeRecord = 0;
                DAT_102199b8[i] = 1;
                Vec4 position = DAT_10db6ee8[i].matrix.GetTranslation();
                DAT_10db6ee8[i].targetHeading = Atan2(base.x - position.x, base.y - position.y) + HEADING_OFFSET;
                DAT_10db6ee8[i].heading = DAT_10db6ee8[i].targetHeading;
                Matrix4 turn = RotationZ(DAT_10db6ee8[i].heading);
                turn.SetTranslation(DAT_10db6ee8[i].matrix.GetTranslation());
                DAT_10daaa80.Start(EFFECT_PLACE_MARK, position, -1, -1, 0, 0, 0, &turn, 0, 0);
                MarkInPlay(i);
            }
        }
        return 0;
    }
    if (id == SCENE_ID_PEOPLE_ACTIVATE)
    {
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value)
            {
                DAT_102199b8[i] = 1;
                if (DAT_1020d540[i] == 0)
                {
                    DAT_1020d540[i] = 1;
                    DAT_1021b4c8[i] = (char)(i % 5);
                    DAT_10867e18[i].Copy(DAT_10db6ee8[i].matrix.GetTranslation());
                }
            }
        }
        return 0;
    }
    if (id == SCENE_ID_FIND_PEOPLE_BY_ROUTE)
    {
        REGISTER_X = 0;
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value)
            {
                REGISTER_X = i;
                break;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_FIND_OBJECT_SLOT)
    {
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_10db6ee8[i].switchNode == value && DAT_1021f3c0[i] != '`')
                FUN_100143c0(i);
        }
        return 0;
    }
    if (id == SCENE_ID_RESET_CHARACTER_BY_ROUTE)
    {
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value)
            {
                if (DAT_10db6ee8[i].root != 0)
                {
                    DAT_10db6ee8[i].root->M_10194630((char)i, 0x400);
                    DAT_10db6ee8[i].root->M_101946b0(0xfe, 0x400);
                    DAT_10db6ee8[i].root->M_10194730(0x2f, 0x400);
                    DAT_10db6ee8[i].root->M_10194630(0, 0x200);
                    DAT_10db6ee8[i].root->M_101946b0(0xff, 0x200);
                    DAT_10db6ee8[i].root->M_10194730(0x2f, 0x200);
                }
                DAT_10daaae8[i].stats[STAT_HEALTH] = DAT_10daaae8[i].stats[STAT_MAX_HEALTH];
                DAT_10210d30[i] = 0;
                for (int word = 0; word < 16; word++)
                    DAT_1020b430[i].words[word] = 0;
                DAT_10db6ee8[i].alpha = 1.0f;
                DAT_108025f8[i][0].Fill(0.0f);
                DAT_108025f8[i][1].Fill(0.0f);
                DAT_1030b8f8[i] = 0;
                DAT_1030c480[i] = 0;
                DAT_10467510[i] = 0;
                DAT_10db6ee8[i].m_02a8 = 0;
                DAT_10db6ee8[i].holdState = 0;
                DAT_10db6ee8[i].holdWeight = 0.0f;
                DAT_10254598[i] = 0x5b;
                DAT_103086b0[i] = 1;
                DAT_10db6ee8[i].scale.Fill(0.0f);
                DAT_10467ba0[i] = 0;
                DAT_102239d0[i] = 0;
                DAT_10210d30[i] = 0;
                DAT_104686b0[i] = 0;
                DAT_1020d4c0[i] = 0;
                DAT_102550e0[i] = 0;
                DAT_10254090[i] = 1;
                DAT_10218fc0[i] = 0;
                DAT_10210b30[i] = 0;
                DAT_108025f8[i][0].Fill(0.0f);
                DAT_108025f8[i][1].Fill(0.0f);
                DAT_10315350[i] = 0;
                DAT_1021f6c0[i] = 0;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_ACTIVATE_PERSON)
    {
        DAT_102199b8[value] = 1;
        MarkInPlay(value);
        return 0;
    }
    if (id == SCENE_ID_REVEAL_OBJECT)
    {
        int found = 0;
        int wanted = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        float x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        float y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        float z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_D);
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_1021f3c0[i] == 5)
            {
                if (DAT_10db6ee8[i].matrix.translation.x < -20000.0f && DAT_10db6ee8[i].switchNode == wanted && found == 0)
                    found = i;
                else if (DAT_10db6ee8[i].matrix.translation.x > -20000.0f && DAT_10db6ee8[i].switchNode == wanted)
                {
                    DAT_10db6ee8[i].m_028c = 0;
                    DAT_1021f3c0[i] = (char)0xfb;
                    FUN_10065b10(i, 0x33a, 0, 0);
                    Vec4 where = DAT_10db6ee8[i].matrix.GetTranslation();
                    FUN_1003a7d0("O_001br.it3", where.x, where.y, where.z, 0, 0.0f, 0.0f, 0.0f);
                    DAT_10db6ee8[i].SetPosition(-250000.0f, 0.0f, 0.0f);
                }
            }
            else if (DAT_1021f3c0[i] == (char)0xfb && DAT_10db6ee8[i].switchNode == wanted && found == 0)
                found = i;
        }
        if (found != 0)
        {
            DAT_10db6ee8[found].m_028c = 4;
            DAT_1021f3c0[found] = 5;
            DAT_10db6ee8[found].SetPosition(x, y, z);
            DAT_10c6357c = found;
        }
        return 0;
    }
    if (id == SCENE_ID_FACE_AWAY_FROM_MODEL)
    {
        Vec4 first;
        Vec4 second;
        first = DAT_10db6ee8[0].matrix.GetTranslation();
        second = DAT_10db6ee8[0].matrix.GetTranslation();
        float heading = Atan2(first.x - second.x, first.y - second.y) + HEADING_OFFSET_SHORT;
        DAT_10db6ee8[value].heading = heading;
        DAT_10db6ee8[value].targetHeading = heading;
        return 0;
    }
    if (id == SCENE_ID_SET_MOTION_SPEED)
    {
        DAT_10db6ee8[0].AddMotionSpeed((float)value);
        DAT_10db6ee8[0].PlayMotion(9, 1, 0, 1.0f);
        DAT_10db6ee8[0].m_229c = 5;
        return 0;
    }
    if (id == SCENE_ID_SET_FOLLOW_ANGLE)
    {
        DAT_1101dddc = (float)value;
        return 0;
    }
    if (id == SCENE_ID_SET_CAMERA_SCALE)
    {
        DAT_1020344c = (float)value / 100.0f;
        return 0;
    }
    if (id == SCENE_ID_FIND_NTH_OBJECT_OF_KIND)
    {
        char kind = '_';
        if (value == 1)
            kind = 6;
        else if (value == 2)
            kind = 5;
        else if (value == 3)
            kind = 'Q';
        int found = 0;
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_1021f3c0[i] == kind)
            {
                found++;
                if (DAT_10874848.TestScriptFlag(FLAG_ARG_B) < found)
                {
                    REGISTER_X = i;
                    break;
                }
            }
        }
        return 0;
    }
    if (id == SCENE_ID_PLAYER_TAKE_DAMAGE)
    {
        DAT_104c00cc = 0;
        float health = (float)(value - DAT_10daaae8[0].stats[4]);
        if (health <= 1.0f)
            health = 1.0f;
        int damage = (int)health;
        if ((DAT_104c02c8 & 0x40) != 0)
            FUN_1005dbf0((damage + (damage >> 0x1f & 3)) >> 2);
        else if ((DAT_104c02c8 & 0x30) == 0x30)
            FUN_1005dbf0(damage / 3);
        else if ((DAT_104c02c8 & 0x20) != 0)
            FUN_1005dbf0(damage / 2);
        else if ((DAT_104c02c8 & 0x10) == 0)
            FUN_1005dbf0(damage);
        else
        {
            damage = damage * 3;
            FUN_1005dbf0((damage + (damage >> 0x1f & 3)) >> 2);
        }
        return 0;
    }
    if (id == SCENE_ID_SHOW_KEY_PROMPT)
    {
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_J) != 0)
            FUN_10065b10(value, 0x356, 0, 0);
        FUN_1000d620(value, DAT_10874848.TestScriptFlag(FLAG_ARG_A), 0x3c);
        return 0;
    }
    if (id == SCENE_ID_PLAYER_MOTION_AND_SOUND)
    {
        DAT_10db6ee8[0].AddMotionSpeed((float)DAT_10874848.TestScriptFlag(FLAG_ARG_B));
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_C) != 0)
            FUN_10065b10(value, 0x2a3, 0, 0);
        return 0;
    }
    if (id == SCENE_ID_PLAYER_STOP_MOTION)
    {
        DAT_10db6ee8[0].PlayMotion(2, 1, 0, 1.0f);
        return 0;
    }
    if (id == SCENE_ID_STORE_CARRIED_ITEM)
    {
        int carrier = 0;
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_10daaae8[i].kind == 0x2f)
            {
                if (value == carrier)
                {
                    for (int compartment = 0; compartment < 4; compartment++)
                    {
                        if (DAT_1020b430[i].words[compartment * 4 + 3] == 0)
                        {
                            int *slot = &DAT_1020b430[i].words[compartment * 4];
                            slot[0] = DAT_104c0024;
                            if (DAT_10874848.TestScriptFlag(FLAG_ARG_J) != 0)
                                slot[0] += 10000;
                            slot[2] = DAT_1042d9e4;
                            slot[1] = DAT_102559f0;
                            slot[3] = DAT_104bfd0c;
                            DAT_102559f0 = 0;
                            DAT_1042d9e4 = 0;
                            DAT_104bfd0c = 0;
                            break;
                        }
                    }
                }
                else
                    carrier++;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_FIND_NEAR_ENEMIES)
    {
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_1021f3c0[i] == 3 && DAT_10db6ee8[i].root != 0 && DAT_10db6ee8[i].matrix.translation.x > -50000.0f &&
                DAT_10210d30[i] == 0 && DAT_10daaae8[i].kind != 0xe && DAT_10daaae8[i].kind != 0xf &&
                DAT_10daaae8[i].kind != 5)
            {
                DAT_10db6ee8[i].SetPosition(DAT_104c4438[i].x, DAT_104c4438[i].y, DAT_104c4438[i].z);
            }
        }
        return 0;
    }
    if (id == SCENE_ID_SET_MODE_FROM_FLAG)
    {
        DAT_10db6ee8[value].mode = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_RECORD_SCRIPT_EVENT)
    {
        int best = -1;
        int bestTime = 999999;
        int entry = 0;
        for (entry = 0; entry < 10; entry++)
        {
            if (DAT_1020dcd0[entry] == 0 || DAT_1021a370[entry] == 0)
                break;
            if (DAT_1020dcd0[entry] < bestTime || best == -1)
            {
                bestTime = DAT_1020dcd0[entry];
                best = entry;
            }
        }
        if (entry == 10)
        {
            DAT_10874848.RegisterEvent(DAT_1021a370[best]);
            DAT_1020dcd0[best] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
            DAT_1021a370[best] = value;
        }
        else
        {
            DAT_1020dcd0[entry] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
            DAT_1021a370[entry] = value;
        }
        return 0;
    }
    if (id == SCENE_ID_PLACE_CART)
    {
        if (DAT_10255f08 != 0)
        {
            Node *cart = DAT_10255f08->Find("@trocco", 0);
            if (cart != 0)
            {
                REGISTER_X = 1;
                FUN_100158b0(cart, value);
                DAT_10edbd58[0].Fill(0.0f);
                DAT_10edbd58[1].Fill(0.0f);
                DAT_10edbd58[2].Fill(0.0f);
                DAT_10edbd58[0].x = OFF_SCREEN;
                DAT_10edbd58[1].x = OFF_SCREEN;
                DAT_10edbd58[2].x = OFF_SCREEN;
            }
            else
                REGISTER_X = 0;
        }
        return 0;
    }
    if (id == SCENE_ID_SET_FOLLOW_COUNT)
    {
        DAT_104c03b4 = value;
        return 0;
    }
    if (id == SCENE_ID_START_EFFECT_AT_FLAGS)
    {
        Vec4 position;
        Matrix4 placement;
        EffectOptions options;
        float scale = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_H) * 0.01f;
        position.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        position.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        position.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_I) != 0 && DAT_10874848.TestScriptFlag(0x9c1) != 0)
            position.x = position.x * -1.0f;
        placement = Scaling(scale, scale, scale);
        placement.SetTranslation(position);
        options.m_000e = (char)DAT_10874848.TestScriptFlag(FLAG_ARG_G);
        options.m_0000 = DAT_10874848.TestScriptFlag(FLAG_ARG_E);
        options.m_0004 = DAT_10874848.TestScriptFlag(FLAG_ARG_F);
        DAT_10daaa80.Start(DAT_10874848.TestScriptFlag(FLAG_ARG_D), position, -1, -1, 0, 0, value, &placement, &options, 0);
        return 0;
    }
    if (id == SCENE_ID_COLOUR_FADE)
    {
        if (value == 0)
            value = 0xf;
        float blue = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C) / 100.0f;
        float green = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B) / 100.0f;
        float red = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A) / 100.0f;
        FUN_1000ec70(value, red, green, blue, 1.0f);
        return 0;
    }
    if (id == SCENE_ID_WAKE_HIDDEN_OBJECT)
    {
        if (value > 0x5b && value < SLOT_END_OBJECT && DAT_1021f3c0[value] == (char)0xfb)
        {
            DAT_10db6ee8[value].m_028c = 4;
            DAT_1021f3c0[value] = 5;
        }
        return 0;
    }
    if (id == SCENE_ID_FIND_FREE_SPOT)
    {
        Vec4 playerPosition = DAT_10db6ee8[0].matrix.GetTranslation();
        Vec4 centre;
        centre.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_D);
        centre.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_E);
        Vec4 offset = playerPosition - centre;
        offset.z = 0.0f;
        Vec4 unit = offset;
        unit.Normalized3();
        Vec4 trial = unit * 8.0f;
        int ring = 8;
        int spotX = 0;
        int spotY = 0;
        while (trial.LengthSquared3() < 64.0f)
        {
            unit.Normalized3();
            trial = unit * (float)ring;
            spotX = (int)trial.x;
            spotY = (int)trial.y;
            trial.x = (float)spotX;
            trial.y = (float)spotY;
            ring++;
        }
        FLOAT_SIGN_CLEARED(centre.x);
        FLOAT_SIGN_CLEARED(centre.y);
        REGISTER_Z = (int)playerPosition.z;
        REGISTER_X = spotX;
        REGISTER_Y = spotY;
        return 0;
    }
    if (id == SCENE_ID_PEOPLE_ARRIVE_RANGE)
    {
        for (int i = DAT_10874848.TestScriptFlag(FLAG_ARG_C); i <= DAT_10874848.TestScriptFlag(FLAG_ARG_D); i++)
            Script_ValueCommand(SCENE_ID_PEOPLE_ARRIVE_SIMPLE, i);
        return 0;
    }
    if (id == SCENE_ID_BRING_OBJECT_BACK)
    {
        Vec4 base = DAT_10db6ee8[0].matrix.GetTranslation();
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value && (DAT_10daaae8[i].kind == 0x29 || DAT_10daaae8[i].kind == 4))
            {
                DAT_10daaae8[i].routeRecord = 0;
                DAT_102199b8[i] = 1;
                Vec4 position = DAT_10db6ee8[i].matrix.GetTranslation();
                DAT_10db6ee8[i].targetHeading = Atan2(base.x - position.x, base.y - position.y) + HEADING_OFFSET;
                DAT_10db6ee8[i].heading = DAT_10db6ee8[i].targetHeading;
                Matrix4 turn = RotationZ(DAT_10db6ee8[i].heading);
                turn.SetTranslation(DAT_10db6ee8[i].matrix.GetTranslation());
                DAT_10daaa80.Start(EFFECT_PLACE_MARK, position, -1, -1, 0, 0, 0, &turn, 0, 0);
                MarkInPlay(i);
            }
        }
        return 0;
    }
    if (id == SCENE_ID_SET_COUNTER_FROM_FLAG)
    {
        DAT_10255f10[value] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_PLACE_JUMP_MARKS)
    {
        for (int i = 0; i < DAT_104bfdc4; i++)
        {
            if (DAT_10874848.TestScriptFlag(FLAG_ARG_A) == (int)DAT_1021b548[i]->matrix.translation.x &&
                DAT_10874848.TestScriptFlag(FLAG_ARG_B) == (int)DAT_1021b548[i]->matrix.translation.y)
            {
                DAT_1021b548[i]->SetFlags(0x20);
                DAT_1021b548[i]->ClearFlags(4);
            }
        }
        return 0;
    }
    if (id == SCENE_ID_SHOW_KEY_PROMPT_FOR)
    {
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_J) != 0)
            FUN_10065b10(value, 0x356, 0, 0);
        FUN_1000d620(value, DAT_10874848.TestScriptFlag(FLAG_ARG_A), DAT_10874848.TestScriptFlag(FLAG_ARG_B));
        return 0;
    }
    if (id == SCENE_ID_CLEAR_CARRIERS)
    {
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_10daaae8[i].kind == 0x2f)
            {
                for (int word = 0; word < 16; word++)
                    DAT_1020b430[i].words[word] = 0;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_ADD_JUMP_NODE)
    {
        if (DAT_104bfdc4 < 0x20 && DAT_10255f08 != 0)
        {
            DAT_10467e20[DAT_104bfdc4] = DAT_10255f08;
            Node *jump = DAT_10255f08->AddChild(0, 0x40);
            sprintf(jump->name, "oboejump%02d", DAT_104bfdc4);
            jump->BuildMesh(1, 0);
            jump->matrix = TranslationMatrix((float)DAT_10874848.TestScriptFlag(FLAG_ARG_A),
                                             (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B),
                                             (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C));
            DAT_1021b548[DAT_104bfdc4] = jump;
            DAT_104bfdc4++;
            if (DAT_1049f0f0 < 0x30)
            {
                DAT_1032d5b8[DAT_1049f0f0] = &jump->matrix.translation;
                DAT_1021bbc0[DAT_1049f0f0] = (char)value;
                DAT_1049f0f0++;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_SELECT_CAMERA_PATH)
    {
        if (value > -2 && value < 0x20)
            FUN_1016b050(value);
        return 0;
    }
    if (id == SCENE_ID_PLACE_CAMERA_BEHIND_PLAYER)
    {
        scratchA.Fill(0.0f);
        scratchA.x = -(float)DAT_102559f0;
        scratchA.y = -(float)DAT_1042d9e4;
        Vec4 behind = scratchA * 100.0f;
        Vec4 camera = DAT_10db6ee8[0].matrix.GetTranslation() + behind;
        DAT_118e69e8.Copy(camera);
        DAT_11af3e28.Copy(camera);
        DAT_11018734 = 1;
        DAT_10db6ee8[0].targetHeading = Atan2((float)DAT_102559f0, (float)DAT_1042d9e4) + HEADING_OFFSET_SHORT;
        DAT_10db6ee8[0].heading = DAT_10db6ee8[0].targetHeading;
        return 0;
    }
    if (id == SCENE_ID_CHASE_PLAYER_STEP)
    {
        if (DAT_10453008[0] == 1)
        {
            DAT_104c4438[0] = DAT_10db6ee8[value].matrix.GetTranslation();
            DAT_10453008[0] = 2;
        }
        else
        {
            Vec4 fromGoal = DAT_10db6ee8[value].matrix.GetTranslation() - DAT_104c4438[0];
            Vec4 fromHome = DAT_10db6ee8[value].matrix.GetTranslation() - DAT_104c4438[1];
            if (fromGoal.Length2D() < fromHome.Length2D())
            {
                REGISTER_X = 0;
                DAT_10db6ee8[0].m_008d = 0;
                return 0;
            }
            Vec4 toPlayer = DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[value].matrix.GetTranslation();
            if (toPlayer.Length() < 10.0f)
            {
                REGISTER_X = -1;
                Vec4 from;
                Vec4 to;
                Vec4 hit;
                from.Copy(DAT_10db6ee8[value].matrix.GetTranslation());
                from.z += 15.0f;
                DAT_104c4438[0].Copy(from);
                to.z += 15.0f;
                if (DAT_10255f08 == 0 || DAT_10255f08->CastSegment(0, 4, &from, &to, &hit, 0) == 0)
                {
                    if (DAT_10c655e0 != 0 && DAT_10c62ed0 == 0x260)
                    {
                        REGISTER_X = 0;
                        DAT_10db6ee8[0].m_008d = 0;
                        return 0;
                    }
                }
                else
                    DAT_10db6ee8[value].SetPosition(DAT_104c4438[0].x, DAT_104c4438[0].y, DAT_104c4438[0].z);
            }
            DAT_104c4438[1].Copy(DAT_10db6ee8[value].matrix.GetTranslation());
        }
        Vec4 toGoal = DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[value].matrix.GetTranslation();
        toGoal.Copy(toGoal);
        float distance = toGoal.Length2D();
        int ticks = (int)(distance / 9.0f);
        if (distance < 16.0f || ticks < 2)
        {
            REGISTER_X = 0;
            DAT_10453008[0] = 0;
        }
        else
        {
            REGISTER_X = 1;
            Vec4 step = toGoal.Normalized3() * 9.0f;
            REGISTER_Y = (int)step.x;
            REGISTER_Z = (int)step.y;
            DAT_10c63578 = ticks;
        }
        return 0;
    }
    if (id == SCENE_ID_SET_CAMERA_NOT_SMOOTHED)
    {
        DAT_11018734 = 1;
        return 0;
    }
    if (id == SCENE_ID_FOLLOW_PLAYER_STEP)
    {
        if (DAT_10453008[0] == 1)
        {
            scratchA = DAT_10db6ee8[value].matrix.GetTranslation();
            DAT_104c4438[1].x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_E);
            DAT_104c4438[1].y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_F);
            DAT_10453008[0] = 2;
        }
        else
        {
            Vec4 fromGoal = DAT_10db6ee8[value].matrix.GetTranslation() - DAT_104c4438[0];
            Vec4 fromHome = DAT_10db6ee8[value].matrix.GetTranslation() - DAT_104c4438[1];
            if (fromHome.Length2D() <= fromGoal.Length2D())
                return 0;
            REGISTER_X = 0;
            DAT_10453008[0] = 1;
            REGISTER_Y = 0;
            REGISTER_Z = 0;
            DAT_10c63578 = 0;
            return 0;
        }
        Vec4 toPlayer = DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[value].matrix.GetTranslation();
        toPlayer.z = 0.0f;
        DAT_10db6ee8[value].targetHeading = Atan2(toPlayer.x, toPlayer.y) + HEADING_OFFSET_SHORT;
        float distance = toPlayer.Length2D();
        int ticks = (int)(distance / (float)DAT_10874848.TestScriptFlag(FLAG_ARG_G));
        {
            REGISTER_X = 1;
            Vec4 step = toPlayer.Normalized3() * (float)DAT_10874848.TestScriptFlag(FLAG_ARG_G);
            REGISTER_Y = (int)(step.x + (step.x <= 0.0f ? -0.5f : 0.5f));
            REGISTER_Z = (int)(step.y + (step.y <= 0.0f ? -0.5f : 0.5f));
            DAT_10c63578 = ticks;
            if (value == 0)
                step = step * 0.5f;
            return 0;
        }
        REGISTER_X = 0;
        DAT_10453008[0] = 1;
        REGISTER_Y = 0;
        REGISTER_Z = 0;
        DAT_10c63578 = 0;
        return 0;
    }
    if (id == SCENE_ID_FOLLOW_PLAYER_BEGIN)
    {
        REGISTER_X = 0;
        DAT_10453008[0] = 1;
        return 0;
    }
    if (id == SCENE_ID_FREEZE_CAMERA)
    {
        DAT_104c0354 = 5;
        DAT_11af3e28.Copy(DAT_118e69e8);
        DAT_118e6c78.Copy(DAT_118e6cb8);
        DAT_104d29d8.Copy(DAT_118e69e8);
        DAT_10f08aa0.Copy(DAT_118e6cb8);
        return 0;
    }
    if (id == SCENE_ID_SET_FIRST_CHAR_STATE)
    {
        if (value < 0)
            DAT_1020b430[0].words[2] = 0;
        else
        {
            DAT_1020b430[0].words[0] = value;
            DAT_1020b430[0].words[1] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
            DAT_1020b430[0].words[2] = 1;
        }
        return 0;
    }
    if (id == SCENE_ID_SET_GROUP_TIMER)
    {
        DAT_102199b8[value] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_WAIT_CAMERA_SCRIPT)
    {
        Vec4 look;
        Vec4 zero;
        zero.Fill(0.0f);
        look.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        look.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        look.z = OFF_SCREEN;
        int savedKind = 0;
        if (value != 0)
        {
            savedKind = DAT_10daaae8[value].kind;
            DAT_10daaae8[value].kind = -1;
        }
        DAT_104c0360 = 1;
        DAT_10c6358c = value;
        DAT_10c6357c = 0;
        DAT_10c63580 = 0;
        DAT_10c63584 = 0;
        DAT_10c63588 = 0;
        DAT_104c4438[0].Copy(look);
        DAT_104c4438[1].Copy(look);
        DAT_104c4438[2].Copy(look);
        DAT_104c4438[3].Copy(zero);
        while (DAT_104c0360 != 0 && FUN_10137b10() != 0)
            ;
        DAT_104bfd0c = 0;
        if (value != 0)
            DAT_10daaae8[value].kind = savedKind;
        return 0;
    }
    if (id == SCENE_ID_SET_CAMERA_FOLLOW)
    {
        if (value == 0)
            DAT_104c0354 = 0;
        else
        {
            DAT_118e69e8.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            DAT_104c0354 = 6;
        }
        return 0;
    }
    if (id == SCENE_ID_PLAYER_WALK_TO)
    {
        DAT_10db6ee8[0].AddMotionSpeed(35.0f);
        DAT_10db6ee8[0].PlayMotion(9, 1, 0, 1.0f);
        DAT_10db6ee8[0].SetPosition(DAT_10db6ee8[0].matrix.translation.x, DAT_10db6ee8[0].matrix.translation.y,
                                    DAT_10db6ee8[0].matrix.translation.z);
        DAT_10db6ee8[0].m_238c = 0;
        DAT_1020b430[0].words[0] = value;
        DAT_104c0154 = 1;
        return 0;
    }
    if (id == SCENE_ID_READ_CAMERA_GOAL)
    {
        DAT_10c63578 = (int)DAT_10edbd48.x;
        DAT_10c6357c = (int)DAT_10edbd48.y;
        DAT_10c63580 = (int)DAT_10edbd48.z;
        return 0;
    }
    if (id == SCENE_ID_START_EFFECT_ON_SPARKLE)
    {
        EffectOptions options;
        Vec4 zero;
        DAT_10868838[0][0].SetIdentity();
        DAT_10868838[0][0].translation.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        DAT_10868838[0][0].translation.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        DAT_10868838[0][0].translation.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        options.owner = value + 0xfa;
        DAT_10daaa80.Start(value, zero, -1, -1, &DAT_10868838[0][0], 0, 0, 0, &options, 0);
        return 0;
    }
    if (id == SCENE_ID_FACE_POINT)
    {
        float y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B) - DAT_10db6ee8[value].matrix.translation.y;
        float x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A) - DAT_10db6ee8[value].matrix.translation.x;
        DAT_10db6ee8[value].targetHeading = Atan2(x, y) + HEADING_OFFSET_SHORT;
        return 0;
    }
    if (id == SCENE_ID_REMOVE_EFFECTS_OF)
    {
        DAT_10daaa80.RemoveOwnedBy(value);
        return 0;
    }
    if (id == SCENE_ID_SET_GUIDE_SPEED)
    {
        if (value == 0)
            DAT_10255160[0][0] = 0.0f;
        else
            DAT_10255160[0][0] = 0.02f;
        return 0;
    }
    if (id == SCENE_ID_NOP_7579)
    {
        return 0;
    }
    if (id == SCENE_ID_SET_REGISTER_Y)
    {
        REGISTER_Y = value;
        return 0;
    }
    if (id == SCENE_ID_START_FRAME_BLEND)
    {
        FUN_1013fb10(DAT_10874848.TestScriptFlag(FLAG_ARG_A), DAT_10874848.TestScriptFlag(FLAG_ARG_B), value);
        return 0;
    }
    if (id == SCENE_ID_FIND_SAFE_SPOT)
    {
        REGISTER_X = 0;
        int slot = 0;
        for (slot = SLOT_FIRST_OBJECT; slot < SLOT_END_OBJECT; slot++)
        {
            if (DAT_1021f3c0[slot] == 'z' || DAT_1021f3c0[slot] == (char)-0x7a)
            {
                DAT_1021f3c0[slot] = 'z';
                break;
            }
        }
        if (DAT_10c62ed0 == 0x262 || DAT_10c62ed0 == 0x133 || DAT_10874848.TestScriptFlag(0x860) != 0 ||
            slot == SLOT_END_OBJECT || DAT_10874848.TestScriptFlag(0x602) != 0)
        {
            if (DAT_10874848.TestScriptFlag(0x602) != 0 || DAT_10c62ed0 == 0x262 || DAT_10c62ed0 == 0x133 ||
                DAT_10874848.TestScriptFlag(0x860) != 0)
                REGISTER_X = 0;
        }
        else
        {
            Vec4 offset;
            Vec4 probe;
            offset.z = -50.0f;
            offset.RotateZ(-DAT_10db6ee8[0].heading);
            REGISTER_X = 1;
            int tries = 0;
            probe = DAT_10db6ee8[0].matrix.GetTranslation() + offset;
            int nearKind = FUN_1001ba40(slot, &probe);
            while (true)
            {
                Vec4 twice = offset * 2.0f;
                if (DAT_10db6ee8[slot].IsClearOfMapLimited(&offset) != 0 && DAT_10db6ee8[slot].IsClearAny(&offset) != 0 &&
                    DAT_10db6ee8[slot].IsClearOfMapLimited(&twice) != 0 && DAT_10db6ee8[slot].IsClearAny(&twice) != 0 &&
                    nearKind == 0)
                    break;
                offset.RotateZ(0.5236f);
                tries++;
                if (tries > 11)
                {
                    REGISTER_X = 0;
                    break;
                }
                probe = DAT_10db6ee8[0].matrix.GetTranslation() + offset;
                nearKind = FUN_1001ba40(slot, &probe);
            }
            if (DAT_10874848.TestScriptFlag(FLAG_ARG_A) != 0 && DAT_10874848.TestScriptFlag(FLAG_ARG_B) == 0)
            {
                Vec4 spot;
                int step = 0;
                spot.Fill(0.0f);
                do
                {
                    spot = DAT_10db6ee8[0].matrix.GetTranslation() + offset * 2.0f;
                    DAT_10db6ee8[slot].SetPosition(spot.x, spot.y, spot.z);
                    if (DAT_10db6ee8[slot].IsClearAny(0) != 0 && DAT_10db6ee8[slot].IsClearOfMapLimited(0) != 0)
                        break;
                    step++;
                    offset = offset * 1.1f;
                } while (step < 5);
                if (step == 5)
                {
                    REGISTER_X = 0;
                    DAT_10db6ee8[slot].SetPosition(OFF_SCREEN, 0.0f, 0.0f);
                }
                else
                {
                    DAT_10db6ee8[slot].targetHeading = Atan2(spot.x - probe.x, spot.y - probe.y) + HEADING_OFFSET_SHORT;
                    DAT_10c66cdc[0] = (int)spot.x;
                    DAT_10c66cdc[1] = (int)spot.y;
                    DAT_10c66cdc[2] = (int)spot.z;
                    switch (value)
                    {
                    case 1:
                        DAT_10db6ee8[slot].scale.Set(0.5f, 0.7f, 1.0f, 0.0f);
                        DAT_10c635e0 = 1;
                        break;
                    case 2:
                        DAT_10db6ee8[slot].scale.Set(1.0f, 1.0f, 0.4f, 0.0f);
                        DAT_10c635e0 = 2;
                        break;
                    case 3:
                        DAT_10db6ee8[slot].scale.Set(0.5f, 1.0f, 0.7f, 0.0f);
                        DAT_10c635e0 = 3;
                        break;
                    default:
                        DAT_10db6ee8[slot].scale.Set(1.0f, 0.5f, 1.0f, 0.0f);
                        DAT_10c635e0 = 4;
                        break;
                    case 5:
                        DAT_10db6ee8[slot].scale.Set(0.25f, 0.25f, 0.25f, 0.0f);
                        DAT_10c635e0 = 5;
                        break;
                    case 6:
                        DAT_10db6ee8[slot].scale.Set(1.0f, 1.0f, 1.0f, 0.0f);
                        DAT_10c635e0 = 6;
                        break;
                    }
                }
            }
        }
        return 0;
    }
    if (id == SCENE_ID_SET_FREE_MODE)
    {
        DAT_10db6ee8[value].mode = 1;
        DAT_10db6ee8[value].m_0267 = 0;
        return 0;
    }
    if (id == SCENE_ID_SET_SCRIPT_MODE)
    {
        DAT_10db6ee8[value].mode = 0;
        DAT_10db6ee8[value].m_0267 = 1;
        return 0;
    }
    if (id == SCENE_ID_SET_TURNING_SPEED)
    {
        if (value != 0)
            DAT_104c038c = -(float)abs(value) - 0.1f;
        else
            DAT_104c038c = 0.0f;
        return 0;
    }
    if (id == SCENE_ID_SHOW_ROCKS)
    {
        for (int i = 1; i < 0x7c; i++)
        {
            if (DAT_10daaae8[i].kind == 0x3f)
            {
                int index = 1;
                Node *rock = DAT_10255f08->Find("@BS_005rocks:", 0);
                while (rock != 0)
                {
                    rock->SetFlags(0x20);
                    rock = DAT_10255f08->Find("@BS_005rocks:", index);
                    index++;
                }
            }
        }
        return 0;
    }
    if (id == SCENE_ID_READ_CAMERA_OFFSET)
    {
        Vec4 toTarget = DAT_118e69e8 - DAT_118e6cb8;
        scratchC.Copy(toTarget);
        float length = scratchC.LengthSquared3();
        float height = scratchC.z;
        scratchC.z = 0.0f;
        scratchC.Normalized3();
        Vec4 along = scratchC * 750.0f;
        scratchC.Copy(along);
        scratchC.z = height * 750.0f / length;
        if (370.0f < scratchC.z)
            scratchC.z = 370.0f;
        scratchC.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_D) + scratchC.x;
        scratchC.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_E) + scratchC.y;
        scratchC.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_F) + scratchC.z;
        REGISTER_X = (int)scratchC.x;
        REGISTER_Y = (int)scratchC.y;
        REGISTER_Z = (int)scratchC.z;
        return 0;
    }
    if (id == SCENE_ID_SET_EFFECT_HOLD)
    {
        for (int i = 0; i < DAT_1020b22c; i++)
        {
            if (DAT_1020dcf8[i].effect == value)
            {
                DAT_1020dcf8[i].position[0] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
                DAT_1020dcf8[i].position[1] = DAT_10874848.TestScriptFlag(FLAG_ARG_B);
                DAT_1020dcf8[i].position[2] = DAT_10874848.TestScriptFlag(FLAG_ARG_C);
                DAT_1020dcf8[i].age = 0;
                Vec4 position;
                position.x = (float)DAT_1020dcf8[i].position[0];
                position.y = (float)DAT_1020dcf8[i].position[1];
                position.z = (float)DAT_1020dcf8[i].position[2];
                DAT_10daaa80.Start(value, position, -1, -1, 0, 0, 0, 0, 0, 0);
                break;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_FIND_FLOATING_ENEMY)
    {
        REGISTER_X = 0;
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_1021f3c0[i] == 'B' && DAT_10db6ee8[i].matrix.translation.x < -20000.0f)
            {
                REGISTER_X = i;
                REGISTER_Y = (int)DAT_10db6ee8[i].matrix.translation.y;
                REGISTER_Z = (int)DAT_10db6ee8[i].matrix.translation.z;
                DAT_10c63578 = (int)DAT_10db6ee8[i].matrix.translation.x;
                break;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_PLACE_DISPLAY_STAND)
    {
        if (DAT_10255f08 != 0)
        {
            Node *stand = DAT_10255f08->Find("@kagudai", 0);
            if (stand != 0)
            {
                Node *standParent = stand->parent;
                standParent->SetFlags(0x20);
                standParent->matrix.translation.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_D);
                standParent->matrix.translation.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_E);
                standParent->matrix.translation.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_F);
                Node *items = DAT_10255f08->Find("@itemkagu", 0);
                if (items != 0)
                    items->ClearFlags(0x20);
                REGISTER_X = DAT_10874848.TestScriptFlag(FLAG_ARG_D);
                REGISTER_Y = DAT_10874848.TestScriptFlag(FLAG_ARG_E);
                REGISTER_Z = DAT_10874848.TestScriptFlag(FLAG_ARG_F) + 0x91;
                Script_ValueCommand(SCENE_ID_SET_EFFECT_HOLD, 0x75f);
            }
        }
        return 0;
    }
    if (id == SCENE_ID_RANDOM_PLAYER_MOTION)
    {
        DAT_10c66c98 = FUN_1013f880() % 3 + 0x6a;
        return 0;
    }
    if (id == SCENE_ID_HIDE_FUTA)
    {
        if (DAT_10255f08 != 0 && DAT_10255f08->Find("futa", 0) != 0)
            DAT_10255f08->Find("futa", 0)->SetFlags(0x20);
        return 0;
    }
    if (id == SCENE_ID_HIDE_MAP_NODES)
    {
        if (DAT_10255f08 != 0)
        {
            DAT_10255f08->SetFlags(0x20);
            Vec4 zero;
            zero.Fill(0.0f);
        }
        if (DAT_10255f0c != 0)
        {
            DAT_10255f0c->SetFlags(0x20);
            Vec4 zero;
            zero.Fill(0.0f);
        }
        return 0;
    }
    if (id == SCENE_ID_SIGHT_TEST)
    {
        Vec4 playerPosition = DAT_10db6ee8[0].matrix.GetTranslation();
        Vec4 other;
        other.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        other.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        float cameraHeading = Atan2(DAT_118e69e8.x - playerPosition.x, DAT_118e69e8.y - playerPosition.y);
        float pointHeading = Atan2(other.x - playerPosition.x, other.y - playerPosition.y);
        float difference = pointHeading - cameraHeading;
        while (difference > 3.1415f)
            difference -= 6.283f;
        while (difference < -3.1415f)
            difference += 6.283f;
        float limit = (float)value * 3.1415f / 180.0f;
        if (FLOAT_SIGN_CLEARED(difference) <= limit)
            DAT_10c63574 = 0;
        else
            DAT_10c63574 = 1;
        return 0;
    }
    if (id == SCENE_ID_SET_AREA_BASE)
    {
        if (DAT_10446e00[0] != 0)
            DAT_1021d018[0] = DAT_1021d018[0] % 1000 + (value + 10) * -1000;
        return 0;
    }
    if (id == SCENE_ID_PREPARE_CHARACTER)
    {
        if (value < 0x3b)
            value = 0x3a;
        else if (value > 0x5a)
            value = 0x5b;
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_A) != 0)
            DAT_10db6ee8[value].alpha = 1.0f;
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_B) != 0)
            DAT_10db6ee8[value].mode = 1;
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_C) != 0)
            DAT_10daaae8[value].m_0141 = 0;
        FUN_10043cc0(value);
        return 0;
    }
    if (id == SCENE_ID_TURN_SWITCH_ON)
    {
        if (DAT_1021f3c0[value] == 0x13 || DAT_1021f3c0[value] == 0x14)
        {
            FUN_1005f530(value);
            DAT_1021b3c8[value] = 0;
            DAT_10db6ee8[value].root->RestartColourPlugs("OPEN0", DAT_104bf8a4);
            int model = DAT_10db6ee8[value].m_0284;
            if (model == 0xcd || model == 0xce || model == 0xe7 || model == 0xe8)
            {
                Vec4 zero;
                Matrix4 placement = DAT_10db6ee8[value].matrix;
                zero.Fill(0.0f);
                DAT_10daaa80.Start(0xfbe, zero, -1, 0, 0, 0, 0, &placement, 0, 0);
            }
            DAT_1020b128[value] = 2;
        }
        return 0;
    }
    if (id == SCENE_ID_RESET_ENEMY)
    {
        if (DAT_10daaae8[value].kind == CHAR_KIND_ENEMY)
        {
            if (DAT_1020b430[value].behaviour == 0xb || DAT_1020b430[value].behaviour == 0xc)
                FUN_1000ec70(0xf, 1.0f, 1.0f, 1.0f, 1.0f);
            for (int word = 0; word < 16; word++)
                DAT_1020b430[value].words[word] = 0;
            for (int node = 0; node < 9; node++)
                DAT_1021bdf8[value][node] = 0;
            DAT_1020b430[value].words[2] = DAT_10874848.TestScriptFlag(FLAG_ARG_B);
            DAT_1020b430[value].behaviour = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        }
        return 0;
    }
    if (id == SCENE_ID_SET_CAMERA_SHIFT_A)
    {
        DAT_10802be8.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        DAT_10802be8.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        DAT_10802be8.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        DAT_10c67ff0.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_D);
        DAT_10c67ff0.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_E);
        DAT_10c67ff0.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_F);
        if (value > 0)
            DAT_104c035c = value;
        return 0;
    }
    if (id == SCENE_ID_SET_CAMERA_SHIFT_B)
    {
        DAT_10da93e0.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        DAT_10da93e0.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        DAT_10da93e0.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        DAT_10874838.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_D);
        DAT_10874838.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_E);
        DAT_10874838.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_F);
        if (value > 0)
            DAT_104c035c = value;
        return 0;
    }
    if (id == SCENE_ID_SET_NODE_COLOURS)
    {
        int mode = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        if (mode == 2)
        {
            DAT_10db6ee8[value].root->M_10194730(0x20, 0x400);
            DAT_10db6ee8[value].root->M_10194730(0x20, 0x200);
            DAT_1021f940[value] = 0;
        }
        else if (mode == 1)
        {
            DAT_10db6ee8[value].root->M_10194730(0x2f, 0x400);
            DAT_10db6ee8[value].root->M_10194730(0x2f, 0x200);
            DAT_1021f940[value] = 0;
        }
        else
        {
            DAT_10db6ee8[value].root->M_10194730(0x2d, 0x400);
            DAT_10db6ee8[value].root->M_10194730(0x2d, 0x200);
            DAT_1021f940[value] = 0x1f;
        }
        return 0;
    }
    if (id == SCENE_ID_SET_FREE_PERSON_MODEL)
    {
        int found = 0;
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_1021f3c0[i] == ',')
            {
                if (found == value)
                {
                    DAT_10db6ee8[i].m_0284 = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
                    break;
                }
                found++;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_SET_FOLLOWER_FLAG)
    {
        DAT_10db6ee8[value].m_008d = (char)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_SET_GREETING_COUNTDOWN)
    {
        DAT_10254390[value] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_TEST_MODEL_IN_VIEW)
    {
        float margin = DAT_10db6ee8[value].loadedRoot->cullMargin;
        Vec4 centre = DAT_10db6ee8[value].loadedRoot->bounds + DAT_10db6ee8[value].matrix.GetTranslation();
        REGISTER_X = FUN_101ae4d0(&centre, &DAT_108392d8, margin);
        return 0;
    }
    if (id == SCENE_ID_PLAY_SOUND_AT_MODEL)
    {
        if (value < 0 || value > 0x7c)
            FUN_1003ad20(DAT_10874848.TestScriptFlag(FLAG_ARG_A), 0, 1.0f);
        else
            FUN_10065b10(value, DAT_10874848.TestScriptFlag(FLAG_ARG_A), 1, 0);
        return 0;
    }
    if (id == SCENE_ID_READ_MODEL_IDLE)
    {
        REGISTER_X = DAT_10db6ee8[value].m_238c;
        return 0;
    }
    if (id == SCENE_ID_SET_CAMERA_SHIFT_FRAMES)
    {
        DAT_104c035c = value;
        return 0;
    }
    if (id == SCENE_ID_SET_MODEL_ALPHA)
    {
        DAT_10db6ee8[value].alpha = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A) / 100.0f;
        return 0;
    }
    if (id == SCENE_ID_SET_MODEL_HOLDING)
    {
        DAT_10db6ee8[value].m_0294 = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_READ_PLAYER_REGISTERS_C)
    {
        REGISTER_X = (int)DAT_118e69e8.x;
        REGISTER_Y = (int)DAT_118e69e8.y;
        REGISTER_Z = (int)DAT_118e69e8.z;
        return 0;
    }
    if (id == SCENE_ID_READ_LEVER_VALUE)
    {
        for (int i = 0; i < 0x40; i++)
        {
            if (DAT_10860df0[i].type == 0xb && DAT_10860df0[i].id == value)
            {
                REGISTER_X = DAT_10860df0[i].value;
                break;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_READ_BEHIND_MODEL)
    {
        Vec4 offset;
        offset.z = -(float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        offset.RotateZ(-DAT_10db6ee8[value].heading);
        REGISTER_X = (int)offset.x;
        REGISTER_Y = (int)offset.y;
        return 0;
    }
    if (id == SCENE_ID_WAKE_ENEMIES_NEAR)
    {
        int slot = value & ((value < 1) - 1);
        if (slot > 0x80)
            slot = 0x80;
        FUN_1001b670(slot, (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A), 0);
        return 0;
    }
    if (id == SCENE_ID_SET_ROUTE_RECORD)
    {
        DAT_10daaae8[value].routeRecord = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_FIND_NTH_OF_KIND)
    {
        int count = 0;
        int wanted = DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        int first = SLOT_FIRST_OBJECT;
        int last = SLOT_END_OBJECT;
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_A) == 0)
        {
            first = SLOT_FIRST_PEOPLE;
            last = SLOT_END_PEOPLE;
        }
        REGISTER_X = first;
        for (int i = first; i < last; i++)
        {
            if (DAT_10daaae8[i].kind == value)
            {
                count++;
                if (wanted < count)
                {
                    REGISTER_X = i;
                    break;
                }
            }
        }
        return 0;
    }
    if (id == SCENE_ID_START_SPIRIT_EFFECT)
    {
        int slot;
        for (slot = SLOT_FIRST_OBJECT; slot < SLOT_END_OBJECT; slot++)
        {
            if (DAT_1021f3c0[slot] == 'z' || DAT_1021f3c0[slot] == (char)-0x7a)
            {
                DAT_1021f3c0[slot] = 'z';
                if (DAT_10874848.TestScriptFlag(FLAG_ARG_A) != 0)
                {
                    DAT_10db6ee8[slot].SetPosition(OFF_SCREEN, 0.0f, 0.0f);
                    return 0;
                }
                break;
            }
        }
        if (slot != SLOT_END_OBJECT)
        {
            Matrix4 placement;
            Vec4 position;
            placement.SetIdentity();
            position = DAT_10db6ee8[value].matrix.GetTranslation();
            if (DAT_10db6ee8[value].headNode == 0)
                position.z += 120.0f;
            placement.SetTranslation(position);
            DAT_10daaa80.Start(0x36c9, position, -1, -1, &DAT_10db6ee8[slot].matrix, 0, 0, &placement, 0, 0);
            FUN_10065b10(slot, 0x187, 1, 0);
        }
        return 0;
    }
    if (id == SCENE_ID_READ_DISTANCE_TO_POINT)
    {
        Vec4 point;
        point.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        point.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        point.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        Vec4 difference = DAT_10db6ee8[0].matrix.GetTranslation() - point;
        DAT_10c63578 = (int)difference.Length();
        return 0;
    }
    if (id == SCENE_ID_SET_PERSON_SPEED)
    {
        DAT_10db6ee8[value].m_0267 = (char)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_DEFEAT_ENEMY)
    {
        FUN_100627d0(value);
        return 0;
    }
    if (id == SCENE_ID_MAKE_ENEMY)
    {
        for (int word = 0; word < 16; word++)
            DAT_1020b430[value].words[word] = 0;
        DAT_1020b430[value].behaviour = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        DAT_1020b430[value].words[2] = DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        DAT_10daaae8[value].kind = CHAR_KIND_ENEMY;
        DAT_1021f9c0[value] = 1;
        DAT_10255a88[value] = 1;
        return 0;
    }
    if (id == SCENE_ID_SET_FLAG_1021F9C0)
    {
        DAT_1021f9c0[value] = (char)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_SET_FLAG_10255A88)
    {
        DAT_10255a88[value] = (char)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_SET_LISTED_PARTS)
    {
        DAT_1032d868 = value;
        return 0;
    }
    if (id == SCENE_ID_THROW_FROM_MODEL)
    {
        Node *effectNode = DAT_10255f08->Find("@efc_Null", 0);
        if (effectNode != 0)
        {
            EffectOptions options;
            Vec4 position = DAT_10db6ee8[value].matrix.GetTranslation();
            Matrix4 placement = RotationZ(DAT_10db6ee8[value].heading + 0.2618f);
            placement.SetTranslation(DAT_10db6ee8[value].matrix.GetTranslation());
            options.m_0000 = DAT_10daaae8[value].stats[STAT_EFFECT_PARAM];
            options.m_0008 = 1.2f;
            FUN_1001d8b0(value, 0x3e9, position, -1, -1, &effectNode->worldMatrix, 0, 0, &placement, &options);
            FUN_1001d8b0(value, 0x3ec, position, -1, -1, &effectNode->worldMatrix, 0, 0x14, &placement, &options);
            FUN_1001d8b0(value, 0x36fd, position, -1, -1, &effectNode->worldMatrix, 0, 0x14, &placement, &options);
            FUN_10065b10(value, 0x1cb, 0, 0);
            FUN_10054550(0x1cc, 0x14, &position);
            FUN_10054550(0x1ca, 0x14, &position);
        }
        return 0;
    }
    if (id == SCENE_ID_SET_HEALTH)
    {
        if (value < 0x5b)
        {
            if (value < 0x3b)
                value = 0x3a;
        }
        else
            value = 0x5b;
        DAT_10daaae8[value].stats[STAT_HEALTH] = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        return 0;
    }
    if (id == SCENE_ID_DESTROY_ENEMY)
    {
        FUN_100541f0(value);
        return 0;
    }
    if (id == SCENE_ID_READ_ROAMER_STILL)
    {
        REGISTER_X = DAT_10210d30[value] != 0;
        return 0;
    }
    if (id == SCENE_ID_SPAWN_ENEMY_AT_REGISTERS)
    {
        for (int i = SLOT_FIRST_PEOPLE; i < SLOT_END_PEOPLE; i++)
        {
            if (DAT_10daaae8[i].routeRecord == value && DAT_10210d30[i] != 0)
                FUN_10050510((float)REGISTER_X, (float)REGISTER_Y, (float)REGISTER_Z, 0, 1);
        }
        return 0;
    }
    if (id == SCENE_ID_FIND_OBJECT_OF_KIND_P)
    {
        for (int i = SLOT_FIRST_OBJECT; i < SLOT_END_OBJECT; i++)
        {
            if (DAT_1021f3c0[i] == 'P' && DAT_10db6ee8[i].switchNode == value)
            {
                REGISTER_X = i;
                break;
            }
        }
        return 0;
    }
    if (id == SCENE_ID_WAIT_FOR_GAME_STATE)
    {
        while (DAT_1020b430[SLOT_FIRST_PEOPLE].words[0] == 0xb && DAT_1020b430[SLOT_FIRST_PEOPLE].words[1] < 3001 &&
               DAT_1020b430[SLOT_FIRST_PEOPLE].words[1] > 1999)
        {
            if (DAT_10daaae8[SLOT_FIRST_PEOPLE].kind != 0x6b || FUN_10137b10() == 0)
                break;
        }
        return 0;
    }
    if (id == SCENE_ID_PLAY_DELAYED_SOUND)
    {
        if (DAT_10874848.TestScriptFlag(FLAG_ARG_A) < 2)
            FUN_10054550(value, 1, 0);
        else
            FUN_10054550(value, DAT_10874848.TestScriptFlag(FLAG_ARG_A), 0);
        return 0;
    }
    if (id == SCENE_ID_CLEAR_THREE_VECTORS)
    {
        return 0;
    }
    if (id == SCENE_ID_PLAY_SOUND_OPTIONAL)
    {
        if (value < 0 || value > 0x7c)
        {
            int sound = DAT_10874848.TestScriptFlag(FLAG_ARG_A);
            if (DAT_104c0332 == 0)
                FUN_1003ad20(sound, 0, 1.0f);
        }
        else
            FUN_10065b10(value, DAT_10874848.TestScriptFlag(FLAG_ARG_A), 1, 1);
        return 0;
    }
    if (id == SCENE_ID_HIDE_CAMERA_POSITIONS_NEAR)
    {
        Vec4 point;
        point.x = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_A);
        point.y = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_B);
        point.z = (float)DAT_10874848.TestScriptFlag(FLAG_ARG_C);
        if (DAT_10255f08 != 0)
        {
            for (int i = 0; i < DAT_1049f0f0; i++)
            {
                if (DAT_1032d5b8[i] != 0)
                {
                    Vec4 difference = point;
                    if ((point - difference).Length() < (float)value)
                        DAT_1032d5b8[i]->x = OFF_SCREEN;
                }
            }
        }
        return 0;
    }

    return 0;
}
