// Model files (.it3) (EN build): the pre-pass that reads the chunks of a model file, and the full loader. Ported from
// the matched JP source decomp/src/model_prescan.cpp.
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "game_file.h"
#include "node.h"

// Characters of the base name that go into the key sum and xor of the chunk check.
const int MODEL_KEY_NAME_LENGTH = 32;
// Start value of the xor check.
const unsigned int MODEL_KEY_CHECK_START = 0x146e;
// Size of the buffer a model's base name is built in.
const int MODEL_NAME_BUFFER_SIZE = 256;

// Node::PreScanFile: opens model file `path` and runs the chunk pre-pass (ScanChunks) on it. Returns 1, or 0 after
// reporting "3Dデータがない" ("no 3D data") when the file cannot be opened. Called when a model is loaded for a motion
// (AnimModel::LoadModel) and for attached models.
int Node::PreScanFile(const char *path, int arg)
{
    GameFile file;
    if (file.Open(path, "rb") != 0)
    {
        ScanChunks(&file, arg);
        file.Close();
        return 1;
    }
    ErrorReport("3D\x83" "f\x81[\x83^\x82\xaa\x82\xc8\x82\xa2", path, 0);
    return 0;
}

// Node::LoadIt3: releases the node and loads model file `path` into it: the host is told, the file's chunks are
// parsed with the sum and xor (start MODEL_KEY_CHECK_START) of its upper-cased base name as the check, and the load
// time is reported (both tick counts are taken after the parse, so it reads 0). Returns 1, or 0 after reporting
// "3Dデータがない" ("no 3D data") when the file cannot be opened. Called by Node::Load.
int Node::LoadIt3(const char *path, int a, int b)
{
    char name[MODEL_NAME_BUFFER_SIZE];
    Release();
    if (g_modelLoadNotify != 0)
        g_modelLoadNotify(path, 0);
    GameFile file;
    FILE *opened = file.Open(path, "rb");
    if (opened != 0)
    {
        int keySum = 0;
        unsigned int keyCheck = MODEL_KEY_CHECK_START;
        memset(name, 0, sizeof(name));
        Path_GetBaseName(name, path);
        _strupr(name);
        for (int i = 0; i < MODEL_KEY_NAME_LENGTH; i++)
        {
            keySum += name[i];
            keyCheck ^= name[i];
        }
        ParseChunks(&file, a, b, keySum, keyCheck);
        DWORD start = GetTickCount();
        DWORD end = GetTickCount();
        ErrorReport("\x83\x8d\x81[\x83h\x8e\x9e\x8a\xd4", "ms", end - start);   // "load time"
        file.Close();
        return 1;
    }
    ErrorReport("3D\x83" "f\x81[\x83^\x82\xaa\x82\xc8\x82\xa2", path, 0);
    return 0;
}
