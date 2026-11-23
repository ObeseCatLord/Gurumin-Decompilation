// Starting a background music track by number: the track is streamed by soundarc.dll (bgm_stream.cpp) and the
// .de2 file's header is checked against its name key (bgm_header.cpp). A failed check plants random noise in the
// anti-tamper table DAT_104c0270 (anti_tamper.cpp). C++ rather than extern "C": the function keeps an exception
// frame for its GameFile local.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// A music track (0x30 bytes per entry).
struct BgmTrack
{
    char name[0x10];              // file name in bgm\ without ".de2"
    char mode;                    // 0x10: 'o' = played once (no loop)
    char pad_0011[0xf];
    int a;                        // 0x20: passed to soundarc (loop parameters)
    int b;                        // 0x24
    int start;                    // 0x28: start position (1000 = the beginning)
    char pad_002c[4];
};

struct GameFile
{
    FILE *handle;
    char name[0x24];
    GameFile();                                  // 0x101ad9e0
    ~GameFile();                                 // 0x101b2570
    FILE *Open(const char *path, const char *mode);   // 0x101b2310: the stdio handle, 0 on failure
    int Read(void *buffer, int size, int count);  // 0x101b23a0
    void Close();                                // 0x101b22f0
};

extern "C" BgmTrack DAT_108db358[0x41];
#include "sound_vars.h"

#include "game_vars.h"

extern "C" DWORD DAT_104c00e4;                   // tick count when the track was started
extern "C" void FUN_10010480(const char *name, int a, int b, int c, int d, int fade);   // Bgm_StartStream
extern "C" int FUN_10161120(char *out, const char *path);                              // Path_GetBaseName
extern "C" int Bgm_CheckHeader(char *name, int sum, unsigned int check);               // 0x10001cb0

// 0x1003aef0 (Bgm_Play). Starts track `index` (0-0x40; any number plays track 0 while DAT_104c033c is set) unless
// it is already playing: streams bgm\<name>.de2 (looped unless its mode is 'o', from its start position less 1000,
// fading the old track out unless this is track 0), then opens the file from the data folder and checks its
// header against the name key, reading the rest of the header into DAT_102564a8. When the file is missing or the
// check fails, a random value is written to a random entry of DAT_104c0270. Returns 1, or 0 when nothing started.
int Bgm_Play(int index)
{
    char path[0x100];
    char name[0x20];
    int sum;
    int check;
    int unused;
    int size;
    if (DAT_104c033c != 0)
        index = 0;
    else
    {
        if (index < 0)
            return 0;
        if (index > 0x40)
            return 0;
    }
    if (index == DAT_10202358)
        return 0;
    DAT_10202358 = (unsigned char)index;
    sprintf(path, "bgm\\%s.de2", DAT_108db358[index].name);
    if (DAT_104c01b0 == 0)
        FUN_10010480(path, DAT_108db358[index].a, DAT_108db358[index].b, __max(DAT_108db358[index].start - 1000, 0),
                     DAT_108db358[index].mode != 'o', index == 0);
    GameFile file;
    if (index >= 0)
    {
        sprintf(path, "%s\\bgm\\%s.de2", DAT_10f627d0, DAT_108db358[index].name);
        if (file.Open(path, "rb"))
        {
            memset(name, 0, sizeof(name));
            FUN_10161120(name, path);
            file.Read(&sum, 1, 4);
            file.Read(&check, 1, 4);
            file.Read(&unused, 1, 4);
            file.Read(&size, 1, 4);
            if (Bgm_CheckHeader(name, sum, check))
            {
                size ^= check;
                file.Read(DAT_102564a8, 1, size);
                file.Close();
            }
            else
            {
                int noise = rand();
                DAT_104c0270[rand() % 16] = noise;
                file.Close();
            }
        }
        else
        {
            int noise = rand();
            DAT_104c0270[rand() % 16] = noise;
        }
    }
    DAT_104c00e4 = GetTickCount();
    return 1;
}
