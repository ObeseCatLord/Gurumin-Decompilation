// Input recordings for the demo (attract) mode (EN build). JP twin: demo_record.cpp.
#include <stdio.h>

#include "demo_record.h"

// Records the current input (28 button bytes and five values) as the next frame and makes it the end of the
// recording. There is no check against the frame limit. Returns 0.
int DemoRecording::RecordFrame()
{
    for (int k = 0; k < DEMO_BUTTON_COUNT; k++)
        buttons[k][playFrame] = buttons_now[k];
    for (int k = 0; k < DEMO_VALUE_COUNT; k++)
        values[k][playFrame] = values_now[k];
    playFrame++;
    frameCount = playFrame;
    return 0;
}

// Plays back the next recorded frame: its button bytes and values become the current input. Does nothing once the
// recording has ended. Returns 0.
int DemoRecording::PlayFrame()
{
    if (playFrame < frameCount)
    {
        for (int k = 0; k < DEMO_BUTTON_COUNT; k++)
            buttons_now[k] = buttons[k][playFrame];
        for (int k = 0; k < DEMO_VALUE_COUNT; k++)
            values_now[k] = values[k][playFrame];
        playFrame++;
    }
    return 0;
}

// Loads a recording from `path`: the frame count, then per frame the five values and the 28 button bytes. A missing
// file leaves the recording as it is. Returns 0. Called by the game start-up.
int DemoRecording::Load(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file != 0)
    {
        fread(&frameCount, 1, 4, file);
        for (int frame = 0; frame < frameCount; frame++)
        {
            for (int k = 0; k < DEMO_VALUE_COUNT; k++)
                fread(&values[k][frame], 1, 4, file);
            for (int k = 0; k < DEMO_BUTTON_COUNT; k++)
                fread(&buttons[k][frame], 1, 1, file);
        }
        fclose(file);
    }
    return 0;
}
