// ModelDef: declared here once (first used by src/model_commands.cpp).
#ifndef GURUMIN_MODEL_DEF_H
#define GURUMIN_MODEL_DEF_H

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

#endif
