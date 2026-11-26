// Track B playback and playback slot offsets of an AnimModel (EN build, EN-only: no JP twin found).
#include "anim_model.h"
#include "engine_memory.h"

// While track B is playing: samples it at the current frame (the sampled value is not used) and advances one frame.
// At the end frame it loops back to the first frame, or stops on the last frame when not looping. Returns 0.
// No direct caller.
int AnimModel::StepTrackB()
{
    if (trackB.playing != 0)
    {
        float x;
        float y;
        float z;
        float w;
        trackB.keys.Sample(trackB.frame, &x, &y, &z, &w, 0);
        trackB.frame++;
        if (trackB.frame >= trackB.last)
        {
            if (trackB.looping != 0)
            {
                trackB.frame = trackB.first;
                return 0;
            }
            trackB.playing = 0;
            trackB.frame--;
        }
    }
    return 0;
}

// AnimModel::SetSlotOffset: sets the (x, y) offset of slot `slot` in layer `layer` of the motion playback (creates the
// playback on first use). A layer or slot out of range is ignored. Returns 1. Called by the model commands.
int AnimModel::SetSlotOffset(int layer, int slot, int x, int y)
{
    if (playback == 0)
    {
        playback = (Playback *)g_engineAlloc(PLAYBACK_SIZE, "");
        playback->Init();
        playback->BindModel(this);
    }
    Playback *state = playback;
    if (layer <= PLAYBACK_LAYER_COUNT - 1 && slot <= PLAYBACK_SLOT_COUNT - 1)
    {
        state->slotOffsetX[layer][slot] = x;
        state->slotOffsetY[layer][slot] = y;
    }
    return 1;
}
