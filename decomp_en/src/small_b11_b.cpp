// EN-only tiny functions without a JP twin (batch 11, section 4): table clears and global setters.
#include "effect_nodes.h"
#include "game.h"
#include "node_cache.h"
#include "small_b11.h"

AnimModel *g_boundModels;
int g_boundModelsExtra;
int g_menuCursorReported;
int g_textInputLimit;
int g_menuChoice;
int g_menuChoiceMode;

// Clears the root node of every model file entry. Called when the model files are reset.
extern "C" void ModelFileRoots_Clear(void)
{
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
        g_modelFileRoots[i] = 0;
}

// Clears the in-use flag of every effect node. Called when the effect nodes are reset.
extern "C" void EffectNodes_ClearLoaded(void)
{
    for (int i = 0; i < EFFECT_NODE_COUNT; i++)
        g_effectNodeLoaded[i] = 0;
}

// Clears the cached model tree of every cache slot. Called at start-up.
extern "C" void NodeCache_ClearAll(void)
{
    for (int i = 0; i < NODE_CACHE_SIZE; i++)
        g_cachedNodes[i] = 0;
}

// Remembers the character model array and resets the extra value. Returns 0. Called at start-up.
extern "C" int Models_Bind(AnimModel *models)
{
    g_boundModels = models;
    g_boundModelsExtra = 0;
    return 0;
}

// Window callback of the 0x222 menu: remembers the cursor position. Returns 0.
extern "C" int MenuCursor_Report(int cursor, char *caption)
{
    g_menuCursorReported = cursor;
    return 0;
}

// Sets the text input length limit. Returns 1. Called when the name entry starts.
extern "C" int TextInput_SetLimit(int limit)
{
    g_textInputLimit = limit;
    return 1;
}

// Records a menu choice and marks it as picked. Returns 0.
extern "C" int MenuChoice_Set(int value)
{
    g_menuChoice = value;
    g_menuChoiceMode = MENU_CHOICE_PICKED;
    return 0;
}
