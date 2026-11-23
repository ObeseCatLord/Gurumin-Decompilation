// Following the camera target along the map's camera paths: finds the path frame closest to the target, hands over
// to another path when the target comes nearer to it, and arms the end-of-path countdown.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "key_track.h"

#include "camera_path_select.h"

#include "camera_vars.h"

#define PATH_FAR_DISTANCE 99999.0f
#define PATH_REFINE_STEP 0.05f
#define PATH_EDGE_FRAMES 2.1f
#define PATH_STATE_NONE 0
#define PATH_STATE_TRACKING 1
#define PATH_STATE_LEAVING 2

// 0x1016b6c0 (Camera_TrackPathFrame). Keeps DAT_1101c800 (frame of the current path) on the frame closest to the camera
// target. State 0 first scans every frame of every path (z counted twice) and picks the closest. In state 2 (leaving)
// or on a path whose setting flag is set, every path is scanned again and the closest frame wins (switching path
// resets the countdown DAT_1101d8ac, or flags DAT_11018734 when there is none). In state 1 the frame is then refined
// on the current path by whole frames, then in 0.05 steps around it, and may not move more than one frame per call.
// Within 2.1 frames of either end of the path (path camera not running) state 2 and a countdown start. Returns 0.
extern "C" float FUN_1016b6c0(void)
{
    int path = 0;
    if (DAT_1101d89c == 0 || DAT_118e69f8[0].frames == 0)
        return 0.0f;
    Vec4 point;
    Vec4 target;
    target.Copy(DAT_1101d884->world.GetTranslation());
    float distance;
    float nearest = 0.0f;
    int startPath = DAT_1101d8a4;
    int previousPath = startPath;
    int changed = 0;
    if (DAT_1101d8a0 == PATH_STATE_NONE)
    {
        int bestPath;
        float bestFrame;
        for (; path < DAT_1101d89c; path++)
        {
            float length = (float)DAT_118e69f8[path].frames[DAT_118e69f8[path].count - 1];
            float f;
            int frame = 0;
            for (f = 0.0f; f < length; f = (float)++frame)
            {
                DAT_118e69f8[path].Sample(frame, &point.x, &point.y, &point.z, &point.w, 0);
                distance = point.DistanceZWeighted(target);
                if (nearest >= distance || (path == 0 && frame == 0))
                {
                    nearest = distance;
                    bestFrame = f;
                    bestPath = path;
                }
            }
        }
        DAT_1101d8a0 = PATH_STATE_TRACKING;
        DAT_1101d8a4 = bestPath;
        DAT_1101c800 = bestFrame;
    }
    int state = DAT_1101d8a0;
    startPath = DAT_1101d8a4;
    if (state == PATH_STATE_LEAVING ||
        (DAT_1101d890 == 0 && state == PATH_STATE_TRACKING && DAT_10ff17e0[startPath].m_0008 != 0))
    {
        int settled = 0;
        DAT_118e69f8[startPath].Sample(DAT_1101c800, &point.x, &point.y, &point.z, &point.w, 0);
        DAT_10ff2ff0 = point.DistanceZWeighted(target);
        nearest = DAT_10ff2ff0;
        for (int path = 0; path < DAT_1101d89c; path++)
        {
            DAT_10fdea90[path] = PATH_FAR_DISTANCE;
            float length = (float)DAT_118e69f8[path].frames[DAT_118e69f8[path].count - 1];
            for (float f = 0.0f; f < length; f += 1.0f)
            {
                DAT_118e69f8[path].Sample(f, &point.x, &point.y, &point.z, &point.w, 0);
                distance = point.DistanceZWeighted(target);
                if (distance < DAT_10fdea90[path])
                    DAT_10fdea90[path] = distance;
                if (distance <= nearest)
                {
                    settled = 0;
                    DAT_10ff2ff0 = distance;
                    DAT_1101c800 = f;
                    DAT_1101d8a4 = path;
                    nearest = distance;
                    if (DAT_10ff17e0[path].m_0008 == 0)
                    {
                        if (state == PATH_STATE_LEAVING)
                        {
                            changed = 1;
                            if (previousPath != path)
                            {
                                if (DAT_1101d890 != 0)
                                    DAT_10feb3d8 = 0;
                                else
                                {
                                    DAT_1101d8a8 = DAT_1101d8ac;
                                    if (DAT_1101d8ac == 0)
                                    {
                                        settled = 1;
                                        DAT_1101d8a0 = PATH_STATE_TRACKING;
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                    else if (state == PATH_STATE_LEAVING && startPath != path)
                    {
                        changed = 1;
                        if (previousPath != path)
                        {
                            if (DAT_1101d890 != 0)
                                DAT_10feb3d8 = 0;
                            else
                            {
                                DAT_1101d8a8 = DAT_1101d8ac;
                                if (DAT_1101d8ac == 0)
                                {
                                    settled = 1;
                                    DAT_1101d8a0 = PATH_STATE_TRACKING;
                                    continue;
                                }
                            }
                        }
                    }
                    DAT_1101d8a0 = PATH_STATE_TRACKING;
                }
            }
        }
        if (DAT_1101d8a4 != startPath && state == PATH_STATE_TRACKING)
        {
            changed = 1;
            if (previousPath != DAT_1101d8a4)
            {
                if (DAT_1101d890 == 0)
                {
                    DAT_1101d8a8 = (DAT_10ff17e0[startPath].m_0004 + DAT_10ff17e0[startPath].m_0000) / 10;
                    DAT_1101d8ac = DAT_1101d8a8;
                    if (DAT_1101d8a8 == 0)
                        DAT_11018734 = 1;
                }
                else
                    DAT_10feb3d8 = 0;
            }
        }
        if (settled)
            DAT_11018734 = 1;
    }
    if (DAT_1101d8a0 != PATH_STATE_TRACKING)
        return 0.0f;

    float startFrame = DAT_1101c800;
    DAT_118e69f8[DAT_1101d8a4].Sample(DAT_1101c800, &point.x, &point.y, &point.z, &point.w, 0);
    DAT_10ff2ff0 = point.Distance(target);
    float length = (float)DAT_118e69f8[DAT_1101d8a4].frames[DAT_118e69f8[DAT_1101d8a4].count - 1];
    nearest = DAT_10ff2ff0;
    for (float f = 0.0f; f < length; f += 1.0f)
    {
        DAT_118e69f8[DAT_1101d8a4].Sample(f, &point.x, &point.y, &point.z, &point.w, 0);
        distance = point.Distance(target);
        if (distance < nearest)
        {
            DAT_10ff2ff0 = distance;
            DAT_1101c800 = f;
            nearest = distance;
        }
    }
    for (float f = DAT_1101c800 - 1.0f; f < DAT_1101c800 + 1.0f; f += PATH_REFINE_STEP)
    {
        DAT_118e69f8[DAT_1101d8a4].Sample(f, &point.x, &point.y, &point.z, &point.w, 0);
        distance = point.Distance(target);
        if (distance < nearest)
        {
            DAT_10ff2ff0 = distance;
            DAT_1101c800 = f;
            nearest = distance;
        }
    }
    if (fabs(DAT_1101c800 - startFrame) > 1.0)
    {
        float step = DAT_1101c800 - startFrame;
        DAT_1101c800 = step / (float)fabs(step) + startFrame;
    }
    if (DAT_1101d890 == 0)
    {
        if (DAT_1101c800 <= PATH_EDGE_FRAMES && state == PATH_STATE_TRACKING && changed == 0)
        {
            DAT_1101d8ac = DAT_10ff17e0[DAT_1101d8a4].m_0000 / 5;
            DAT_1101d8a0 = PATH_STATE_LEAVING;
        }
        float end = (float)DAT_118e69f8[DAT_1101d8a4].frames[DAT_118e69f8[DAT_1101d8a4].count - 1];
        if (end - PATH_EDGE_FRAMES <= DAT_1101c800 && state == PATH_STATE_TRACKING && changed == 0)
        {
            DAT_1101d8ac = DAT_10ff17e0[DAT_1101d8a4].m_0004 / 5;
            DAT_1101d8a0 = PATH_STATE_LEAVING;
        }
    }
    return 0.0f;
}
