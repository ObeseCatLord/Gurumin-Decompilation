#include <math.h>
#include "vec4.h"

#include "node.h"
#include "camera_vars.h"
#include "char_vars.h"
#include "camera_modes.h"

extern "C" void FUN_1016a8d0(void);

#define SHIFT_FADE 0.7f
#define SHIFT_KEEP 0.85f
#define SHIFT_STEP 0.15f
#define SHIFT_FROM_PREVIOUS 0.5f
#define SHIFT_MIN_RADIUS 10.0f
#define PI_UPPER 3.1415f
#define PI_LOWER (-3.1415f)
#define TWO_PI 6.283f

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10019670 (Camera_ShiftOrbit). Like Camera_Shift, but the shift runs in polar form. While it runs
// (DAT_104c035c counts down) the current camera and the shift goals (DAT_10802be8, DAT_10c67ff0) are expressed as a
// radius, a heading and a height about the orbit centres DAT_10da93e0 (position) and DAT_10874838 (target). Each
// frame the radius, heading (the shorter way round) and height move 1/n of the way to the goals, n = 1 + half the
// frames left; the point at that polar position, relative to the current camera, moves the offsets DAT_108393d8 and
// DAT_10da93f0 15 % towards it. Otherwise the offsets shrink to 70 %. The target and position then move halfway
// from their previous values by the offsets, and the copies are refreshed. Returns 0.
extern "C" int FUN_10019670(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_104c0358 = 1;
        DAT_108393d8.Fill(0.0f);
        DAT_10da93f0.Fill(0.0f);
    }
    DAT_10866d98.Copy(DAT_118e69e8);
    DAT_10edbd48.Copy(DAT_118e6cb8);
    if (DAT_104c035c == 0 || --DAT_104c035c == 0)
    {
        DAT_108393d8.Copy(DAT_108393d8 * SHIFT_FADE);
        DAT_10da93f0.Copy(DAT_10da93f0 * SHIFT_FADE);
    }
    else
    {
        Vec4 positionPoint;
        Vec4 targetPoint;
        int half = (int)((float)DAT_104c035c * -0.5f);
        int stepCount = 1 - half;
        float positionRadius = (DAT_10866d98 - DAT_10da93e0).Length2D();
        float targetRadius = (DAT_10edbd48 - DAT_10874838).Length2D();
        float positionGoalRadius = (DAT_10802be8 - DAT_10da93e0).Length2D();
        float targetGoalRadius = (DAT_10c67ff0 - DAT_10874838).Length2D();
        float steps = (float)stepCount;
        float radiusAhead = (positionGoalRadius - positionRadius) / steps + positionRadius;
        float targetRadiusAhead = (targetGoalRadius - targetRadius) / steps + targetRadius;
        positionPoint.Copy(DAT_10da93e0 - DAT_10866d98);
        targetPoint.Copy(DAT_10874838 - DAT_10edbd48);
        float positionHeading = Atan2(positionPoint.x, positionPoint.y);
        float targetHeading = Atan2(targetPoint.x, targetPoint.y);
        positionPoint.Copy(DAT_10da93e0 - DAT_10802be8);
        targetPoint.Copy(DAT_10874838 - DAT_10c67ff0);
        float positionGoalHeading;
        float targetGoalHeading;
        if (positionPoint.Length2D() < SHIFT_MIN_RADIUS)
            positionGoalHeading = positionHeading;
        else
            positionGoalHeading = Atan2(positionPoint.x, positionPoint.y);
        if (targetPoint.Length2D() < SHIFT_MIN_RADIUS)
            targetGoalHeading = targetHeading;
        else
            targetGoalHeading = Atan2(targetPoint.x, targetPoint.y);
        float turn = positionGoalHeading - positionHeading;
        while (turn > PI_UPPER)
            turn -= TWO_PI;
        while (turn < PI_LOWER)
            turn += TWO_PI;
        float headingAhead = turn / steps + positionHeading;
        float targetTurn = targetGoalHeading - targetHeading;
        while (targetTurn > PI_UPPER)
            targetTurn -= TWO_PI;
        while (targetTurn < PI_LOWER)
            targetTurn += TWO_PI;
        float targetHeadingAhead = targetTurn / steps + targetHeading;
        float heightAhead = (DAT_10802be8.z - DAT_10866d98.z) / steps + DAT_10866d98.z;
        float targetHeightAhead = (DAT_10c67ff0.z - DAT_10edbd48.z) / steps + DAT_10edbd48.z;
        positionPoint.Fill(0.0f);
        targetPoint.Fill(0.0f);
        positionPoint.y = -radiusAhead;
        targetPoint.y = -targetRadiusAhead;
        positionPoint.RotateZ(-headingAhead);
        targetPoint.RotateZ(-targetHeadingAhead);
        positionPoint.Copy(positionPoint + DAT_10da93e0);
        targetPoint.Copy(targetPoint + DAT_10874838);
        positionPoint.z = heightAhead;
        targetPoint.z = targetHeightAhead;
        positionPoint.Copy(positionPoint - DAT_10866d98);
        targetPoint.Copy(targetPoint - DAT_10edbd48);
        DAT_108393d8.Copy(DAT_108393d8 * SHIFT_KEEP + positionPoint * SHIFT_STEP);
        DAT_10da93f0.Copy(DAT_10da93f0 * SHIFT_KEEP + targetPoint * SHIFT_STEP);
    }
    DAT_10866d98.Copy(DAT_108393d8);
    DAT_10edbd48.Copy(DAT_10da93f0);
    DAT_118e6cb8.Copy(DAT_118e6cb8 + DAT_10edbd48 * SHIFT_FROM_PREVIOUS +
                      (DAT_10f08aa0 - DAT_118e6cb8) * SHIFT_FROM_PREVIOUS);
    DAT_118e69e8.Copy(DAT_118e69e8 + DAT_10866d98 * SHIFT_FROM_PREVIOUS +
                      (DAT_104d29d8 - DAT_118e69e8) * SHIFT_FROM_PREVIOUS);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}
