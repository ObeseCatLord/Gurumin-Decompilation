// EN-only tiny methods without a JP twin (batch 11, section 4): world message ids, file end test, forwarding calls.
#include "game.h"
#include "game_file.h"
#include "small_b11.h"
#include "world_renderer.h"

// Tells whether the open file has reached its end. Called by the loaders while reading records.
int GameFile::AtEnd()
{
    return feof(handle);
}

// Id of line `index` of the character's .itm file. Called by the caption code.
int WorldRenderer::GetMessageId(int index)
{
    return messageIds[index];
}

// Stores the id of line `index` of the character's .itm file. Called while the file is read.
int WorldRenderer::SetMessageId(int index, int id)
{
    messageIds[index] = (short)id;
    return id;
}

// Id of the line that FindMessageLine finds for `key`. Called by the scripts' message commands.
int WorldRenderer::GetMessageIdOfKey(int key)
{
    return messageIds[FindMessageLine(key)];
}

// Passes the call on to the model's node tree; 0 when the model has none. Called by the character code.
int AnimModel::RunRootCall(int a, int b)
{
    if (root != 0)
        return root->UnrecoveredCall(a, b);
    return 0;
}

// Stores m_7650 of the game object. Returns 0. Called through a menu window callback.
int Game::SetValue7650(int value)
{
    m_7650 = value;
    return 0;
}

// Turns a stored script value into its signed number: values above 5000 are negative ones stored as value - 10000.
extern "C" int ScriptValue_ToSigned(int stored)
{
    if (stored > SCRIPT_VALUE_NEGATIVE_START)
        stored -= SCRIPT_VALUE_WRAP;
    return stored;
}

// Sets whether the cursor position of menu window `id` is remembered. Returns 1. Called when a menu window opens.
int Game::SetCursorRemembered(int id, char value)
{
    cursorRemembered[id] = value;
    return 1;
}
