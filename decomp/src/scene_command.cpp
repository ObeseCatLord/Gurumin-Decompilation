// Scene_RunCommand: the scene commands scripts give (status panel text, pictures, messages, node flags, effects).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "visibility.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "camera_vars.h"
#include "effect_manager.h"
#include "game_vars.h"
#include "map_vars.h"
#include "text_window.h"
#include "texture_load.h"
#include "ui_vars.h"
#include "world.h"
#include "scene_command.h"

extern "C" int FUN_1000c020(char *out, const char *text);                   // Text_Substitute
extern "C" int FUN_10161920(const char *name);                              // Texture_FindByName
extern "C" int FUN_10174c70(int slot, int force);                           // Texture_Release
extern "C" int FUN_1000ed70(const char *text, const char *subtext, int frames);   // opens the message window
extern "C" int FUN_1000edf0(const char *text, const char *subtext, int frames);   // same, alternative style
extern "C" int FUN_10056260(int slot, const char *line1, const char *line2, const char *line3, const char *line4);
extern "C" int FUN_1016b230(const char *text);                              // sets the title caption
extern "C" int FUN_1003a7d0(const char *path, float x, float y, float z, int start, float ox, float oy, float oz);
extern "C" int FUN_10017b80(void);                                          // first free type-14 effect slot

// Radians per degree as the original computes it (3.1415 / 180).
#define SCENE_PI 3.1415f
#define SCENE_HALF_TURN 180.0f
// Node flag bits the commands switch.
#define SCENE_FLAG_ACTIVE 4
#define SCENE_FLAG_HIDDEN NODE_FLAG_HIDDEN
// x of a slot position that is out of sight.
#define SCENE_FAR_AWAY -200000.0f
// A node counts as near the register position within this distance; an effect slot as at a node within this one.
#define SCENE_NEAR_DISTANCE 100.0f
#define SCENE_SAME_SPOT_DISTANCE 10.0f
#define SCENE_ALT_MESSAGE_FRAMES 30

// 0x10083eb0 (Scene_RunCommand; a C++ function: the original has an unwind frame for the matrix temporaries). Runs scene command `command` (SCENE_CMD_*) with the number `arg` and the node or
// file name `name`. Most commands look the node up below the map's root node (DAT_10255f08) and change its flags,
// group or attachment, start an effect at it, or read its position into the registers DAT_10c6356c...
// Returns 0.
int Scene_RunCommand(int command, int arg, const char *name)
{
    Vec4 position;
    Vec4 zeroVec;
    Matrix4 placement;
    char text[256];
    char substituted[256];

    if (command == SCENE_CMD_SET_PANEL_LINE)
    {
        strcpy(text, name);
        strcpy(DAT_1020da50[arg], text);
    }
    else if (command == SCENE_CMD_SET_PANEL_LINE_SUBSTITUTED)
    {
        strcpy(text, name);
        FUN_1000c020(substituted, text);
        strcpy(DAT_1020da50[arg], substituted);
    }
    else if (command == SCENE_CMD_LOAD_PICTURE)
    {
        const char *extension = name + strlen(name) - 3;
        if (_stricmp(extension, "iti") == 0)
            DAT_1049efd8[arg].slot = FUN_10174ce0(FUN_1013f8a0(name, 0), 0, 0);
        else if (_stricmp(extension, "bmp") == 0)
            DAT_1049efd8[arg].slot = FUN_10173550(FUN_1013f8a0(name, 0), 0, 0);
        else
            fread(0, 1, 1, 0);
        DAT_10255960[DAT_10c6356c].picture = arg;
    }
    else if (command == SCENE_CMD_RELEASE_PICTURE)
    {
        int texture = FUN_10161920(name);
        if (texture >= 0)
        {
            for (int i = 0; i < PICTURE_RECORD_COUNT; i++)
            {
                if (DAT_1049efd8[i].slot == texture)
                    DAT_1049efd8[i].slot = -1;
            }
            FUN_10174c70(texture, 1);
        }
    }
    else if (command == SCENE_CMD_SHOW_MESSAGE)
        FUN_1000ed70(name, 0, arg > 0 ? arg : 0);
    else if (command == SCENE_CMD_OPEN_MESSAGE)
        FUN_10056260(arg, name, "", "", "");
    else if (command == SCENE_CMD_SHOW_MESSAGE_ALT)
        FUN_1000edf0(name, 0, SCENE_ALT_MESSAGE_FRAMES);
    else if (command == SCENE_CMD_LOAD_MAIN_MODEL)
    {
        DAT_10db717c = 1;
        DAT_10db9250 = 0;
        DAT_10db6ee8[0].LoadModel(FUN_1013f8a0(name, 0));
        DAT_104bfdf0 = arg;
    }
    else if (command == SCENE_CMD_FOLLOW_FRAME_COUNTER)
    {
        Node *node = DAT_10255f08->Find(name, 0);
        if (node != 0)
            node->frameSource = (int *)&DAT_104bf8a4;
    }
    else if (command == SCENE_CMD_GROUP_VISIBILITY)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node;
            int index;
            switch (arg)
            {
            case 0:
                index = 0;
                do
                {
                    node = DAT_10255f08->Find(name, index);
                    if (node && node->group == DAT_10c6356c)
                    {
                        node->M_1019b210(SCENE_FLAG_ACTIVE);
                        break;
                    }
                    index++;
                } while (node);
                break;
            case 1:
                index = 0;
                do
                {
                    node = DAT_10255f08->Find(name, index);
                    if (node && node->group == DAT_10c6356c)
                    {
                        node->M_1019b190(SCENE_FLAG_ACTIVE);
                        break;
                    }
                    index++;
                } while (node);
                break;
            case 2:
                index = 0;
                do
                {
                    node = DAT_10255f08->Find(name, index);
                    if (node != 0 && node->group == DAT_10c6356c)
                        node->M_1019b210(SCENE_FLAG_ACTIVE);
                    index++;
                } while (node != 0);
                break;
            case 3:
                index = 0;
                do
                {
                    node = DAT_10255f08->Find(name, index);
                    if (node != 0 && node->group == DAT_10c6356c)
                        node->M_1019b190(SCENE_FLAG_ACTIVE);
                    index++;
                } while (node != 0);
                break;
            }
        }
    }
    else if (command == SCENE_CMD_START_EFFECT_AT_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node = DAT_10255f08->Find(name, 0);
            if (node != 0)
            {
                float zero = 0.0f;
                zeroVec.Fill(zero);
                position.Copy(node->worldMatrix.GetTranslation());
                placement.SetIdentity();
                placement.SetTranslation(position);
                DAT_10daaa80.Start(arg, zeroVec, -1, -1, 0, 0, 0, &placement, 0, 0);
                DAT_10c63580 = 1;
            }
            else
                DAT_10c63580 = 0;
        }
    }
    else if (command == SCENE_CMD_SHOW_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node = DAT_10255f08->Find(name, 0);
            if (node != 0)
            {
                node->SetFlags(SCENE_FLAG_ACTIVE);
                node->ClearFlags(SCENE_FLAG_HIDDEN);
            }
        }
    }
    else if (command == SCENE_CMD_HIDE_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node = DAT_10255f08->Find(name, 0);
            if (node != 0)
            {
                node->ClearFlags(SCENE_FLAG_ACTIVE);
                node->SetFlags(SCENE_FLAG_HIDDEN);
            }
        }
    }
    else if (command == SCENE_CMD_COUNT_NODES)
    {
        Node *node = DAT_10255f08;
        int index = 0;
        DAT_10c6356c = 0;
        while (node != 0)
        {
            node = DAT_10255f08->Find(name, index);
            DAT_10c6356c++;
            index++;
        }
    }
    else if (command == SCENE_CMD_SET_NODE_GROUP)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node = DAT_10255f08->Find(name, 0);
            if (node != 0)
                node->group = arg;
        }
    }
    else if (command == SCENE_CMD_SET_NODE_FLAGS)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node = DAT_10255f08->Find(name, 0);
            if (node != 0)
            {
                switch (arg)
                {
                case 0:
                    node->SetFlags(SCENE_FLAG_HIDDEN);
                    break;
                case 1:
                    node->ClearFlags(SCENE_FLAG_HIDDEN);
                    break;
                case 2:
                    node->SetFlags(SCENE_FLAG_ACTIVE);
                    break;
                case 3:
                    node->ClearFlags(SCENE_FLAG_ACTIVE);
                    break;
                }
            }
        }
    }
    else if (command == SCENE_CMD_SELECT_NODE_INDEX)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node = DAT_10255f08->Find(name, arg);
            if (node != 0)
            {
                node->SetFlags(SCENE_FLAG_HIDDEN);
                node->ClearFlags(SCENE_FLAG_ACTIVE);
            }
        }
    }
    else if (command == SCENE_CMD_SET_TITLE_CAPTION)
        FUN_1016b230(name);
    else if (command == SCENE_CMD_SET_CARRIED_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            for (int slot = SCENE_OBJECT_SLOT_FIRST; slot < SCENE_OBJECT_SLOT_END; slot++)
            {
                if (DAT_10daaae8[slot].kind == 0x23 && DAT_1020b430[slot].state == 8)
                {
                    if (arg != 0)
                        DAT_1021bdf8[slot][0] = DAT_10255f08->Find(name, 0);
                    else
                        DAT_1021bdf8[slot][0] = 0;
                    break;
                }
            }
        }
    }
    else if (command == SCENE_CMD_FOLLOW_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            if (arg != 0)
            {
                if (DAT_104bf8c0 != 0xf)
                {
                    DAT_104c0354 = 0xb;
                    DAT_104c039c = DAT_10255f08->Find(name, 0);
                    DAT_118e69e8.x = (float)DAT_10c63570;
                    DAT_118e69e8.y = (float)DAT_10c63574;
                    DAT_118e69e8.z = (float)DAT_10c63578;
                }
            }
            else
            {
                DAT_104c0354 = 0;
                if (DAT_104bf8c0 != 0xf)
                    DAT_104c039c = 0;
            }
        }
    }
    else if (command == SCENE_CMD_PLACE_EFFECT_MODEL_AT_PANEL_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            Node *node = DAT_10255f08->Find(DAT_1020da50[0], 0);
            Vec4 where;
            if (node != 0)
                where.Copy(node->worldMatrix.GetTranslation());
            else
            {
                float zero = 0.0f;
                where.Fill(zero);
            }
            FUN_1003a7d0(name, where.x, where.y, where.z, 0, 0, 0, 0);
        }
    }
    else if (command == SCENE_CMD_ADD_TEST_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            Vec4 where;
            where.x = (float)DAT_10c6356c;
            where.y = (float)DAT_10c63570;
            where.w = 0;
            where.z = (float)DAT_10c63574;
            Node *child = DAT_10255f08->AddChild(0, 0x40);
            child->Load(FUN_1013f8a0(name, 0), 0, 0, 0);
            child->BuildMesh(1, 0);
            child->matrix = RotationX((float)DAT_10c63578 * SCENE_PI / SCENE_HALF_TURN) *
                            RotationZ((float)DAT_10c6357c * SCENE_PI / SCENE_HALF_TURN);
            child->matrix.SetTranslation(where);
            strcpy(child->name, "ktest30020");
            VisData *visibility = DAT_10255f08->visibilityCells;
            if (visibility != 0)
                child->TagRegion(visibility->boxes, (const char (*)[0x40])visibility, &child->matrix);
        }
    }
    else if (command == SCENE_CMD_RENAME_COLLISION_NODES)
    {
        if (DAT_10255f08 != 0 && DAT_10255f08->visibilityCells != 0)
        {
            if (strlen(name) > 3)
            {
                Node *node;
                for (;;)
                {
                    node = DAT_10255f08->Find(name, 0);
                    if (node == 0)
                        break;
                    node->name[0] = 'L';
                    node->name[1] = 'D';
                    node->name[2] = 'C';
                }
            }
        }
    }
    else if (command == SCENE_CMD_PLACE_EFFECT_MODEL)
        FUN_1003a7d0(name, (float)DAT_10c6356c, (float)DAT_10c63570, (float)DAT_10c63574, 0, 0, 0, 0);
    else if (command == SCENE_CMD_SET_ATTACHED_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            int length = (int)strlen(name);
            if (length != 0)
            {
                int group = DAT_10c6356c;
                DAT_1021bdf8[arg][group] = DAT_10255f08->Find(name, 0);
            }
            else
                DAT_1021bdf8[arg][DAT_10c6356c] = 0;
        }
    }
    else if (command == SCENE_CMD_START_EFFECT_AT_MODEL)
    {
        Vec4 start;
        Matrix4 matrix;
        matrix.SetIdentity();
        Node *node = DAT_10db6ee8[arg].root->Find(name, 0);
        if (node != 0)
        {
            matrix.SetTranslation(node->worldMatrix.GetTranslation());
            DAT_10daaa80.Start(DAT_10c63578, start, -1, -1, &node->worldMatrix, 0, 0, &matrix, 0, 0);
        }
        else
        {
            matrix.SetTranslation(DAT_10db6ee8[arg].matrix.GetTranslation());
            DAT_10daaa80.Start(DAT_10c63578, start, -1, -1, 0, 0, 0, &matrix, 0, 0);
        }
    }
    else if (command == SCENE_CMD_REMOVE_EFFECT_SLOT_NEAR_NODE)
    {
        if (DAT_10255f08 != 0)
        {
            Vec4 where;
            Node *node = DAT_10255f08->Find(name, 0);
            if (node != 0)
                where.Copy(node->worldMatrix.GetTranslation());
            else
                where.x = SCENE_FAR_AWAY;
            int count = FUN_10017b80();
            for (int i = 0; i < count; i++)
            {
                if ((DAT_10860df0[i].position - where).Length2D() < SCENE_SAME_SPOT_DISTANCE)
                {
                    DAT_10860df0[i].position.Set(SCENE_FAR_AWAY, 0, 0, 0);
                    break;
                }
            }
        }
    }
    else if (command == SCENE_CMD_READ_NODE_POSITION)
    {
        Node *node = 0;
        if (DAT_10255f08 != 0)
            node = DAT_10255f08->Find(name, 0);
        if (node == 0)
        {
            DAT_10c6356c = 0;
            DAT_10c63570 = 0;
            DAT_10c63574 = 0;
        }
        else if (arg != 0)
        {
            Vec4 from;
            from.x = (float)DAT_10c6356c;
            from.y = (float)DAT_10c63570;
            from.z = (float)DAT_10c63574;
            if ((from - node->worldMatrix.GetTranslation()).Length() < SCENE_NEAR_DISTANCE)
                DAT_10c6356c = 1;
            else
                DAT_10c6356c = 0;
        }
        else
        {
            DAT_10c6356c = (int)node->worldMatrix.m[12];
            DAT_10c63570 = (int)node->worldMatrix.m[13];
            DAT_10c63574 = (int)node->worldMatrix.m[14];
        }
    }
    else if (command == SCENE_CMD_START_EFFECT_BEHIND_MODEL)
    {
        Vec4 start = DAT_10db6ee8[arg].matrix.GetTranslation();
        Matrix4 matrix;
        matrix = RotationZ(DAT_10db6ee8[arg].heading);
        matrix.SetTranslation(start);
        float zero = 0.0f;
        zeroVec.Fill(zero);
        DAT_10daaa80.Start(atol(name), zeroVec, -1, -1, 0, 0, 0, &matrix, 0, 0);
    }
    return 0;
}
