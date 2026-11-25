// Frame timer, scene control, paths, random numbers and render state cache (EN build). Ported from the matched JP
// sources decomp/src/engine_1013e.cpp, path_format.cpp, type_parse.cpp and render_state.cpp.
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "game_file.h"
#include "render.h"

// FrameTimer_ResetNext2: next-frame deadline = now + one frame; returns 0. Called when the main loop restarts.
extern "C" int FrameTimer_ResetNext2(void)
{
    g_nextFrameDeadline = timeGetTime() + FRAME_MS;
    return 0;
}

// FrameTimer_ResetNext: next-frame deadline = now + one frame; returns 1. Called when the main loop restarts.
extern "C" int FrameTimer_ResetNext(void)
{
    g_nextFrameDeadline = timeGetTime() + FRAME_MS;
    return 1;
}

// Render_EndScene: ends the open Direct3D scene. Called at the end of each frame.
extern "C" int Render_EndScene(void)
{
    Render_CloseScene();
    return 0;
}

// Render_BeginScene: starts a new Direct3D scene, ending the open one first. Called at the start of each frame.
extern "C" int Render_BeginScene(void)
{
    Render_RestartScene();
    return 0;
}

// MakeDataPath: returns "<data folder>\<name>" in one of four rotating buffers. Called by the sound and file loaders.
extern "C" char *MakeDataPath(const char *name)
{
    g_dataPathCounter++;
    sprintf(g_dataPathBuffers[g_dataPathCounter & (DATA_PATH_BUFFERS - 1)], "%s\\%s", g_dataFolder, name);
    return g_dataPathBuffers[g_dataPathCounter & (DATA_PATH_BUFFERS - 1)];
}

// Path_Data: returns "<folder>\<name>" for data folder number `folder`. The result is one of 16 rotating buffers, so
// it stays valid for the next 15 calls. Used for nearly every file load.
extern "C" const char *Path_Data(const char *name, int folder)
{
    g_folderPathCounter++;
    sprintf(g_folderPathBuffers[g_folderPathCounter & (FOLDER_PATH_BUFFERS - 1)], "%s\\%s", g_folderNames[folder], name);
    return g_folderPathBuffers[g_folderPathCounter & (FOLDER_PATH_BUFFERS - 1)];
}

// Random: returns the next entry of the random table; the position wraps at 0x10000. Called by all random choices.
extern "C" int Random(void)
{
    return g_randomTable[++g_randomPosition];
}

// Render_SetRenderState: sets a Direct3D render state unless the cache says it already has that value. Returns 1
// when the state was (re)written.
extern "C" int Render_SetRenderState(unsigned long state, unsigned long value)
{
    return RenderState_Set(state, value);
}

// Bytes of the random table file (fontr.dat).
const int RANDOM_TABLE_FILE_SIZE = RANDOM_TABLE_SIZE * sizeof(int);
// Data folder number of fontr.dat.
const int RANDOM_TABLE_FOLDER = 0;

// FontRandom_Load: loads the 256 KB table fontr.dat into the random table, or fills it with rand() when the file is
// missing. Also stores `value` as the position in the table. C++ linkage: the GameFile local needs an exception
// frame. Called at start-up. Lives here because the original inlines Path_Data into it.
int FontRandom_Load(short value)
{
    g_randomPosition = value;
    GameFile file;
    if (file.Open(Path_Data("fontr.dat", RANDOM_TABLE_FOLDER), "rb") != 0)
    {
        file.Read(g_randomTable, 1, RANDOM_TABLE_FILE_SIZE);
        file.Close();
    }
    else
    {
        for (int i = 0; i < RANDOM_TABLE_SIZE; i++)
            g_randomTable[i] = rand();
    }
    return 0;
}
