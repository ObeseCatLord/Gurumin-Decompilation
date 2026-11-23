// Model files (.it3): the pre-pass that reads the INFO/MAN2 chunks of a model file before loading it.

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "game_file.h"

#include "node.h"

extern "C" void (*DAT_10f65538)(const char *path, int a);   // load notification of the host (may be 0)
extern "C" int FUN_10161120(char *name, const char *path);  // Path_GetBaseName

extern "C" int FUN_1013edb0(const char *format, ...);  // error report (empty in the release build)

// Opens model file `path` and runs the chunk pre-pass (Node::ScanChunks) on it. Returns 1, or 0 after reporting
// "3Dデータがない" ("no 3D data") when the file cannot be opened.
int Node::PreScanFile(const char *path, int arg)
{
    GameFile file;
    if (file.Open(path, "rb") != 0)
    {
        ScanChunks(&file, arg);
        file.Close();
        return 1;
    }
    FUN_1013edb0("3D\x83" "f\x81[\x83^\x82\xaa\x82\xc8\x82\xa2", path, 0);
    return 0;
}

// 0x101abe30 (Model_LoadIt3). Releases the node and loads model file `path` into it: the host is told, the file's
// chunks are parsed with the sum and xor (start 0x146e) of its upper-cased base name as the check, and the load time
// is reported (both tick counts are taken after the parse, so it reads 0). Returns 1, or 0 after reporting
// "3Dデータがない" ("no 3D data") when the file cannot be opened.
int Node::LoadIt3(const char *path, int a, int b)
{
    char name[256];
    Release();
    if (DAT_10f65538 != 0)
        DAT_10f65538(path, 0);
    GameFile file;
    FILE *opened = file.Open(path, "rb");
    if (opened != 0)
    {
        memset(name, 0, sizeof(name));
        int keySum = 0;
        unsigned int keyCheck = 0x146e;
        FUN_10161120(name, path);
        _strupr(name);
        for (int i = 0; i < 32; i++)
        {
            keySum += name[i];
            keyCheck ^= name[i];
        }
        ParseChunks(&file, a, b, keySum, keyCheck);
        DWORD start = GetTickCount();
        DWORD end = GetTickCount();
        FUN_1013edb0("\x83\x8d\x81[\x83h\x8e\x9e\x8a\xd4", "ms", end - start);   // "load time"
        file.Close();
        return 1;
    }
    FUN_1013edb0("3D\x83" "f\x81[\x83^\x82\xaa\x82\xc8\x82\xa2", path, 0);
    return 0;
}
