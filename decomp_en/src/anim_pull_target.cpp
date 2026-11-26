// Pulling a model towards a position (EN build, EN-only: no JP twin found).
#include "anim_model.h"

// Frames the model is drawn towards its pull target after SetPullTarget.
const int ANIM_PULL_FRAMES = 0x17;

// Sets the position the model is drawn towards and the frames the pull lasts (none when `target` is 0). Returns 0.
// Called by the character update loop (EN 0x60cb10).
int AnimModel::SetPullTarget(Vec4 *target)
{
    int frames = ANIM_PULL_FRAMES;
    pullTarget = target;
    if (target == 0)
        frames = 0;
    pullFrames = frames;
    return 0;
}
