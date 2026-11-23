// Objects that move along a key-framed path.

#include "key_track.h"

struct PathMover
{
    char pad_0000[0x17ec];
    KeyBank path;
    int endFrame;
    int startFrame;
    int frame;
    int playing;
    int looping;
    int SetPosition(float x, float y, float z);  // 0x10162610 (model_place.cpp)
    int StepPath();
};

// While playing: samples the path at the current frame, moves the object there and advances one frame. At the
// end frame it loops back to startFrame, or stops on the last frame when not looping.
int PathMover::StepPath()
{
    if (playing != 0)
    {
        float x;
        float y;
        float z;
        float w;
        path.Sample(frame, &x, &y, &z, &w, 0);
        SetPosition(x, y, z);
        frame++;
        if (frame >= endFrame)
        {
            if (looping != 0)
            {
                frame = startFrame;
                return 0;
            }
            playing = 0;
            frame--;
        }
    }
    return 0;
}
