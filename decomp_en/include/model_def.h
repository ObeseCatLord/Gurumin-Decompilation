// Model definitions of the character table (EN build; JP: decomp/include/model_def.h). This is the one declaration
// of ModelDef in decomp_en: include this header, never redeclare it. The same table is viewed as ModelPath (file name
// only, base 4 bytes higher) in char.h.
#ifndef GURUMIN_EN_MODEL_DEF_H
#define GURUMIN_EN_MODEL_DEF_H

// Number of command lines a definition holds, bytes of a command name, and values per command.
const int MODEL_COMMAND_MAX = 0x190;
const int MODEL_COMMAND_NAME_SIZE = 0x10;
const int MODEL_COMMAND_VALUES = 8;
// Characters of a command name that are compared ("SET", "DIS", "RND", "DAT").
const int MODEL_COMMAND_NAME_LENGTH = 3;

// A model definition (0x4b34 bytes; one per model number in g_modelDefs).
struct ModelDef
{
    int commandCount;                                                  // 0x0000: command lines read
    char path[0x20];                                                   // 0x0004: model file name
    int m_0024;                                                        // 0x0024: set by "SET" commands of layer 2 (first value)
    int m_0028;                                                        // 0x0028: set by "SET" commands of layer 2 (second value)
    char pad_002c[8];                                                  // 0x002c: not yet named
    char commands[MODEL_COMMAND_MAX][MODEL_COMMAND_NAME_SIZE];         // 0x0034: command names ("SET", "DIS", "RND", "DAT")
    int params[MODEL_COMMAND_MAX][MODEL_COMMAND_VALUES];               // 0x1934: up to six values per command
};

typedef char check_model_def[sizeof(ModelDef) == 0x4b34 ? 1 : -1];

extern "C" ModelDef g_modelDefs[];   // EN 0x0120126c (JP DAT_10b314c4): the model definitions

// Motion data the "DAT" commands bind to a playback slot (not yet typed): the shared set of the player model number 0
// (for value 4), the set of the other model-0 values, and the set of every other model.
extern "C" char g_motionSourcePlayerAlt[];     // EN 0x00b91c84
extern "C" char g_motionSourcePlayer[];        // EN 0x01335074
extern "C" char g_motionSourceCharacter[];     // EN 0x00b91ebc

// Applies the commands of the model definition `model` to character slot `slot` (SET = motion layer, DIS = follow-up,
// RND = slot offset, DAT = motion source). With a negative `slot`, `model` is the slot and its own model definition
// is used. EN 0x643440 (JP 0x10012220).
extern "C" void Model_ApplyCommands(int model, int slot);

#endif
