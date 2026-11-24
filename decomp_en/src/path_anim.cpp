// Path playback of a model (JP twin: path_anim.cpp).

#include "anim_model.h"

// While track C is playing: samples the path at the current frame, moves the model there and advances one frame.
// At the end frame it loops back to the first frame, or stops on the last frame when not looping. Called every frame
// for models that follow a path. Returns 0.
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
