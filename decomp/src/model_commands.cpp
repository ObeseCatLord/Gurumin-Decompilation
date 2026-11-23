// Model definitions: each record of the table at 0x10b314c4 (0x4b34 bytes; char_motion.cpp sees it from its file name
// at +4) holds a list of commands read from the model's definition file, applied when the model is set up.

#include <string.h>

// A model definition, 0x4b34 bytes.
struct ModelDef
{
    int commandCount;             // 0x0000
    char path[0x20];              // 0x0004: model file name
    int m_0024;                   // 0x0024: set by "SET" commands of kind 2
    int m_0028;
    char pad_002c[8];
    char commands[0x190][0x10];   // 0x0034: command names ("SET", "DIS", "RND", "DAT")
    int params[0x190][8];         // 0x1934: up to six values per command
};

#include "anim_model.h"

#include "char_slot.h"

extern "C" ModelDef DAT_10b314c4[];
extern "C" int DAT_104c02f0;      // "DAT" source of the player for kind 4
extern "C" int DAT_10c652cc;      // "DAT" source of the player for the other kinds
extern "C" int DAT_104c0314;      // "DAT" source of the other characters

// 0x10012220 (Model_ApplyCommands). Applies the commands of a model definition to a character's scene owner: with
// `b` negative, character `a` with its own model definition; otherwise model definition `a` to character `b`. "SET"
// sets a playback (kind 2 also stores two of its values in the definition), "DIS" a layer parameter, "RND" a layer
// offset and "DAT" a layer source (one of three globals, chosen by the character and the kind).
extern "C" void FUN_10012220(int a, int b)
{
    int index;
    int model;
    if (b >= 0)
    {
        model = a;
        index = b;
    }
    else
    {
        index = a;
        model = DAT_10daaae8[a].params[PARAM_MODEL];
    }
    for (int i = 0; i < DAT_10b314c4[model].commandCount; i++)
    {
        const char *command = DAT_10b314c4[model].commands[i];
        int *p = DAT_10b314c4[model].params[i];
        if (strncmp(command, "SET", 3) == 0)
        {
            DAT_10db6ee8[index].SetPlayback(p[0], p[1], p[2], p[3], p[4], p[5]);
            if (p[0] == 2)
            {
                DAT_10b314c4[model].m_0024 = p[2];
                DAT_10b314c4[model].m_0028 = p[3];
            }
        }
        else if (strncmp(command, "DIS", 3) == 0)
            DAT_10db6ee8[index].SetLayerParameter(p[0], p[1], p[2]);
        else if (strncmp(command, "RND", 3) == 0)
            DAT_10db6ee8[index].SetLayerOffset(p[0], p[1], p[2], p[3]);
        else if (strncmp(command, "DAT", 3) == 0)
        {
            if (index == 0)
            {
                if (p[0] == 4)
                    DAT_10db6ee8[index].SetLayerSource(&DAT_104c02f0, p[0], p[1]);
                else
                    DAT_10db6ee8[index].SetLayerSource(&DAT_10c652cc, p[0], p[1]);
            }
            else
                DAT_10db6ee8[index].SetLayerSource(&DAT_104c0314, p[0], p[1]);
        }
    }
}
