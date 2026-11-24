// Command queue to the sound engine (soundarc) in the EN build (JP: app_0002.cpp, which used window messages).
// This is the one declaration of it in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_SOUND_ARC_H
#define GURUMIN_EN_SOUND_ARC_H

// Entries of the command queue (the queue index is a byte).
const int SOUND_ARC_QUEUE_SIZE = 0x100;
// Commands.
const int SOUND_ARC_COMMAND_PLAY_STREAM = 0x65;     // play a file on a music stream
const int SOUND_ARC_COMMAND_PLAY_3D = 0x96;         // play a 3D sound effect
// Music streams that exist.
const unsigned SOUND_ARC_LAST_STREAM = 1;
const int SOUND_ARC_STREAM_PARAMS = 4;              // playback parameters of a music stream command
const int SOUND_ARC_3D_VALUES = 10;                 // floats of a 3D sound command

// One queued sound engine command (0x1c0 bytes).
struct SoundArcCommand
{
    char path[0x100];                       // 0x000: file path
    int command;                            // 0x100: SOUND_ARC_COMMAND_*
    int stream;                             // 0x104: music stream
    int params[SOUND_ARC_STREAM_PARAMS];    // 0x108: playback parameters
    char m_0118[0x13c - 0x118];             // 0x118: not used by EN code yet
    unsigned long queuedAt;                 // 0x13c: time the command was queued (0 for stream commands)
    char m_0140[0x164 - 0x140];             // 0x140: not used by EN code yet
    float values[SOUND_ARC_3D_VALUES];      // 0x164: position, volume, pitch and so on, then the sound id
    char m_018c[0x1c0 - 0x18c];             // 0x18c: not used by EN code yet
};

// Non-zero once the sound engine is running. EN 0x02974090 (JP DAT_1020aee4).
extern "C" int g_soundArcRunning;
// Index of the next free queue entry (wraps at 256). EN 0x029740c5.
extern "C" unsigned char g_soundArcQueueNext;
// The command queue. EN 0x029740c8.
extern "C" SoundArcCommand g_soundArcQueue[SOUND_ARC_QUEUE_SIZE];

// Queues "play file on music stream" with four parameters. Returns -1 when queued, else 0. EN 0x7c5750.
extern "C" int SoundArc_PlayStream(int stream, const char *path, int a, int b, int c, int d);
// Queues "play 3D sound" with ten values. Returns 0. EN 0x7c5e60.
extern "C" int SoundArc_Play3D(float id, float a, float b, float c, float d, float e, float f, float g, float h, float i);

#endif
