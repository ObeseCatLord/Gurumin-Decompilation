// Background music tracks of the EN build (JP: decomp/include/bgm_track.h, sound_vars.h, bgm_spectrum.h). This is the
// one declaration of the track table and of the music stream state in decomp_en: include this header, never redeclare.
#ifndef GURUMIN_EN_BGM_H
#define GURUMIN_EN_BGM_H

#include <windows.h>

// Tracks in the table (0..0x40).
const int BGM_TRACK_COUNT = 0x41;
// Highest track number Bgm_Play accepts.
const int BGM_LAST_TRACK = BGM_TRACK_COUNT - 1;
// Track mode of music played once (no loop).
const char BGM_MODE_ONCE = 'o';
// The start position of a track counts from this many (1000 = the beginning).
const int BGM_START_BIAS = 1000;
// Music level steps of the options menu (0..10) and the attenuation per missing step, in hundredths.
const int BGM_VOLUME_STEPS = 10;
const int BGM_ATTENUATION_PER_STEP = 500;
// Fade-out time of the old track in milliseconds.
const int BGM_FADE_OUT_MS = 2000;
// Number of music streams (the two alternate).
const int BGM_STREAM_MASK = 1;

// A music track (0x30 bytes per entry).
struct BgmTrack
{
    char name[0x10];              // 0x00: file name in bgm\ without ".de2"
    char mode;                    // 0x10: 'o' = played once (no loop)
    char m_0011[0xf];             // 0x11: not used by EN code yet
    int a;                        // 0x20: passed to soundarc (loop parameters)
    int b;                        // 0x24: passed to soundarc
    int start;                    // 0x28: start position (1000 = the beginning)
    int m_002c;                   // 0x2c: divisor of the beat length
};

extern "C" BgmTrack g_bgmTracks[BGM_TRACK_COUNT];  // EN 0x00fab100 (JP DAT_108db358): the track table
extern "C" int g_bgmPlaying;                       // EN 0x00908784 (JP DAT_10202358): track playing
extern "C" char g_bgmAlwaysTrackZero;              // EN 0x00bc887c (JP DAT_104c033c): non-zero = every request plays track 0
extern "C" unsigned char g_bgmSpectrum[];          // EN 0x0092f2a0 (JP DAT_102564a8): spectrum frames of the playing track (the .de2 header after its checks)
extern "C" DWORD g_bgmStartTick;                   // EN 0x009e33b0 (JP DAT_104c00e4): tick count when the track started
extern "C" char g_bgmStreamActive;                 // EN 0x00b493f0 (JP DAT_104bfd54): non-zero once a music stream has been started
extern "C" int g_bgmStream;                        // EN 0x00b493f4 (JP DAT_104bfd58): music stream in use (0 or 1)
// Sound engine commands (function pointers into the sound engine).
extern "C" int (*g_soundArcPlayStreamCommand)(int stream, const char *path, int c, int a, int b, int d);   // EN 0x00909498 (JP FUN_10001a90): plays a file on a music stream
extern "C" void (*g_soundArcStopStream)(int stream);                    // EN 0x009094b8 (JP FUN_10001ba0): stops a music stream
extern "C" void (*g_soundArcFadeStream)(int stream, int milliseconds);  // EN 0x009094bc (JP FUN_10001bf0): fades a music stream out

// Spectrum table of the music bars: bands per channel, bytes per frame, samples per frame and the right channel's offset.
const int BGM_SPECTRUM_BANDS = 32;
const int BGM_SPECTRUM_FRAME_SIZE = 0x50;
const int BGM_SPECTRUM_SAMPLES_PER_FRAME = 0x5be;
const int BGM_SPECTRUM_RIGHT_OFFSET = 0x28;
extern "C" int g_bgmRequested;                     // EN 0x009e33f4 (JP DAT_104c012c): track the game asked for (-1 = none)
extern "C" int g_bgmBarsShown;                     // EN 0x0222693c (JP DAT_1020aee4): non-zero = the music bars are drawn (the spectrum is loaded)
extern "C" int g_bgmPositionFromSoundArc;          // EN 0x00a13960 (JP DAT_104c014c): non-zero = the music position is read from soundarc
extern "C" short g_bgmStartOffset;                 // EN 0x009e3390 (JP DAT_104c00c0): position the track started from
extern "C" int g_bgmPosition;                      // EN 0x009e1604 (JP DAT_10254088): playback position in 44.1 kHz samples
extern "C" float g_bgmBars[2 * BGM_SPECTRUM_BANDS];   // EN 0x009e12a0 (JP DAT_104bff20): smoothed band levels, left channel then right
extern "C" int (*g_soundArcGetPosition)(int stream);  // EN 0x009094dc (JP FUN_10001c80): playback position of a music stream
// Sound engine start-up commands (function pointers into the sound engine; names provisional).
extern "C" void (*g_soundArcSetWindow)(HWND window);   // EN 0x00909478: gives the sound engine the game window
extern "C" void (*g_soundArcFinishStart)(void);        // EN 0x009094e0: finishes the sound engine start-up after the window is set
// Hands the game window to the sound engine and finishes its start-up; `unused` is not read. Returns 1. Called by the
// game start-up at EN 0x5f2cd0. EN 0x7aa970 (sound_arc_start.cpp, EN-only).
extern "C" int SoundArc_Start(HWND window, const char *unused);
// Updates the music: starts the requested track, computes g_bgmPosition and eases the bars. EN 0x5a8d70 (JP 0x1005c680).
extern "C" int Bgm_Update(void);

// Plays music file `name` on the other stream. EN 0x63e6d0 (JP 0x10010480).
extern "C" void Bgm_StartStream(const char *name, int a, int b, int c, int d, int fade);
// Checks a .de2 header against the key made from its file name; 1 when valid. EN 0x7aa9a0 (JP 0x10001cb0).
extern "C" int Bgm_CheckHeader(char *name, int sum, unsigned int check);
// Starts track `index` unless it is already playing. EN 0x5a5500 (JP 0x1003aef0).
int Bgm_Play(int index, int unused);

#endif
