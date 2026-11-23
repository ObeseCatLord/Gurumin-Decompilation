// Objects that move along a key-framed path.

#include "key_track.h"

#include "anim_model.h"

// While playing: samples the path at the current frame, moves the object there and advances one frame. At the
// end frame it loops back to startFrame, or stops on the last frame when not looping.
int AnimModel::StepPath()
{
    if (trackC.playing != 0)
    {
        float x;
        float y;
        float z;
        float w;
        trackC.keys.Sample(trackC.frame, &x, &y, &z, &w, 0);
        SetPosition(x, y, z);
        trackC.frame++;
        if (trackC.frame >= trackC.last)
        {
            if (trackC.looping != 0)
            {
                trackC.frame = trackC.first;
                return 0;
            }
            trackC.playing = 0;
            trackC.frame--;
        }
    }
    return 0;
}
