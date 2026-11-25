// EN-only tiny functions without a JP twin (batch 11, section 4): node flag setters, motion list clear, texture file
// table helpers and small engine stubs.
#include <windows.h>
#include <mmsystem.h>

#include "engine.h"
#include "ime.h"
#include "mesh_cache.h"
#include "motion_list.h"
#include "node.h"

// Sets `bits` in this node's flags only (SetFlags walks the subtree). Returns 1. Called for single nodes found by name.
int Node::SetOwnFlags(unsigned int bits)
{
    flags |= bits;
    return 1;
}

// Clears `bits` in this node's flags only (ClearFlags walks the subtree). Returns 1. Called for single nodes found by name.
int Node::ClearOwnFlags(unsigned int bits)
{
    flags &= ~bits;
    return 1;
}

// Forgets every timeline item of the list. Returns 1. Called before a plug's motions are rebuilt.
int MotionList::Clear()
{
    count = 0;
    return 1;
}

// Adds a reference to a shared texture file and returns the new count. Called when a node shares a texture.
extern "C" int TextureFile_AddRef(const TextureRef *texture)
{
    int index = texture->slot;
    g_textureFileRefCount[index]++;
    return g_textureFileRefCount[index];
}

// Checks the texture file name table for a free (empty) entry: 0 when there is one, -1 when the table is full.
extern "C" int TextureNames_FindFree(void)
{
    for (int i = 0; i < TEXTURE_FILE_COUNT; i++)
    {
        if (g_textureFileNames[i][0] == 0)
            return 0;
    }
    return -1;
}

// Sets the second text input flag. Called when the name entry starts.
extern "C" void TextInput_SetFlag(int value)
{
    g_textInputFlag = value;
}

// Starts the frame timer: the deadline of the next frame is now. Returns 1. Called once at start-up.
extern "C" int FrameTimer_Start(void)
{
    g_nextFrameDeadline = timeGetTime();
    return 1;
}

// Shutdown step of a subsystem that has nothing to release. Returns 1. Called during the shutdown sequence.
extern "C" int Shutdown_Noop(void)
{
    return 1;
}
