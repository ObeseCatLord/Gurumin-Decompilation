// Declarations shared by the scene script runner (src/world_script.cpp).
#ifndef GURUMIN_WORLD_SCRIPT_H
#define GURUMIN_WORLD_SCRIPT_H

#include <stdlib.h>

#include "world_renderer.h"

// 0x1016b1e0 (camera.cpp): sets caption line `index` (clamped to 0..3) to `text` and restarts its timer.
extern "C" int FUN_1016b1e0(int index, const char *text);

// Codes of the host callback (WorldRenderer::callback) the script uses; the argument is a pointer to a block of ints.
#define WORLD_EVENT_QUERY_JUMP 0x29a      // asks whether to jump to the end of the scene (argument: 1 when past frame 30)
#define WORLD_EVENT_SOUND_EFFECT 0x28b    // SEF
#define WORLD_EVENT_PLAY_EFFECT 0x28e     // PE
#define WORLD_EVENT_PLAY_SKILL 0x2a2      // PK
#define WORLD_EVENT_BGM 0x353             // BGM
#define WORLD_EVENT_VOICE_STOP 0x35c      // voice off (a zero voice number)
#define WORLD_EVENT_VOICE_STOP_MODEL 0x352
#define WORLD_EVENT_VOICE_TEXT 0x35e      // voice from a text
#define WORLD_EVENT_VOICE 0x354           // voice from a number
#define WORLD_EVENT_VOICE_ALT_TEXT 0x35f
#define WORLD_EVENT_VOICE_ALT 0x355
#define WORLD_EVENT_TRIGGER 0x28d         // TRE
#define WORLD_EVENT_TRIGGER_PREPARE 0x1615
#define WORLD_EVENT_REF_PREPARE 0x1614
#define WORLD_EVENT_REF 0x28c
#define WORLD_EVENT_MAP_ARGS 0x320        // MPA
#define WORLD_EVENT_MOVIE_AVI 0x5dc       // SPL: mmv
#define WORLD_EVENT_MOVIE_MV3 0x5de
#define WORLD_EVENT_MOVIE_MPEG 0x5e1
#define WORLD_EVENT_MOVIE_POLL_MMV 0x5dd  // asks whether the movie still plays
#define WORLD_EVENT_MOVIE_POLL_MV3 0x5df
#define WORLD_EVENT_MOVIE_POLL_MPEG 0x5e2
#define WORLD_EVENT_MOVIE_STOP_MPEG 0x5e3
#define WORLD_EVENT_MOVIE_END 0x5e0       // sent when a movie that ended is released
#define WORLD_EVENT_CAPTION 2000          // ZIM
#define WORLD_EVENT_END 999               // END
#define WORLD_EVENT_COSTUME 100           // LMP
#define WORLD_EVENT_DND 0x2ef
#define WORLD_EVENT_NPG 0x458
#define WORLD_EVENT_ESP 0x457


// The fields of one script line, read one after the other (field 0 is the command).
struct ScriptFields
{
    WorldRenderer *world;
    int line;
    int next;
    char text[0x100];

    ScriptFields(WorldRenderer *world, int line) : world(world), line(line), next(1)
    {
    }
    char *Text()
    {
        world->GetMessageField(line, next++, text);
        return text;
    }
    long Int()
    {
        return atol(Text());
    }
    double Float()
    {
        return atof(Text());
    }
    void Skip()
    {
        Text();
    }
};

#endif
