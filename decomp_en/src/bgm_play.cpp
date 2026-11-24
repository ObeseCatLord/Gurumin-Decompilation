// Starting a background music track by number (EN build). Ported from decomp/src/bgm_play.cpp. The EN build keeps no
// anti-tamper noise on a failed header check. C++ rather than extern "C": the function keeps an exception frame for its
// GameFile local.
#include "bgm.h"
#include "engine.h"
#include "game_file.h"
#include "game_vars.h"
#include <stdio.h>
#include <string.h>

// Bytes of a track path buffer.
const int BGM_PATH_SIZE = 0x100;
// Bytes of a track's base name buffer.
const int BGM_NAME_SIZE = 0x20;
// Size of one header word of a .de2 file.
const int BGM_HEADER_WORD = 4;

// Bgm_Play: starts track `index` (0-0x40; any number plays track 0 while the always-track-zero flag is set) unless it
// is already playing: streams bgm\<name>.de2 (looped unless its mode is 'o', from its start position less 1000, fading
// the old track out unless this is track 0), then opens the file from the data folder and checks its header against the
// name key, reading the spectrum data into the spectrum buffer. Returns 1, or 0 when nothing started. Called when a
// scene or the title screen asks for music.
int Bgm_Play(int index, int unused)
{
    int track;
    char path[BGM_PATH_SIZE];
    char name[BGM_NAME_SIZE];
    int sum;
    int check;
    int unusedHeaderWord;
    int size;
    if (g_bgmAlwaysTrackZero != 0)
        track = 0;
    else
    {
        track = index;
        if (track < 0)
            return 0;
        if (track > BGM_LAST_TRACK)
            return 0;
    }
    if (track == g_bgmPlaying)
        return 0;
    g_bgmPlaying = (unsigned char)track;
    sprintf(path, "bgm\\%s.de2", g_bgmTracks[track].name);
    if (g_soundMuted == 0)
        Bgm_StartStream(path, g_bgmTracks[track].a, g_bgmTracks[track].b,
                        __max(g_bgmTracks[track].start - BGM_START_BIAS, 0), g_bgmTracks[track].mode != BGM_MODE_ONCE,
                        track == 0);
    GameFile file;
    if (track >= 0)
    {
        sprintf(path, "%s\\bgm\\%s.de2", g_dataFolder, g_bgmTracks[track].name);
        if (file.Open(path, "rb"))
        {
            memset(name, 0, sizeof(name));
            Path_GetBaseName(name, path);
            file.Read(&sum, 1, BGM_HEADER_WORD);
            file.Read(&check, 1, BGM_HEADER_WORD);
            file.Read(&unusedHeaderWord, 1, BGM_HEADER_WORD);
            file.Read(&size, 1, BGM_HEADER_WORD);
            if (Bgm_CheckHeader(name, sum, check))
            {
                size ^= check;
                file.Read(g_bgmSpectrum, 1, size);
            }
            file.Close();
        }
    }
    g_bgmStartTick = GetTickCount();
    return 1;
}
