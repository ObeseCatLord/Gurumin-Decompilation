// A character that rides along behind a marker character (0x10047a10, behaviour 0x1f of Char_RunBehaviour
// 0x100dfcf0), like Npc_RideAlarm (char_move.cpp) without the alarm.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_tables.h"

extern "C" int FUN_10043cc0(int index);       // Char_ResetState

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10047a10 (Npc_RideBehindMarker). Rides behind the nearest marker character (kind ',' with a model, among
// 0x5c..0x7b; with none its state is reset) at the distance it started at, moving at most 12 units a frame. When it
// lags within 45 degrees of the marker's way and is farther from the marker than its place, it cuts 10 degrees
// inside the turn instead. It faces the marker's way (motion 2 when settled within one degree, else motion 0) and
// is reset when the marker goes away; while the marker is away it keeps its own position. Returns 0.
extern "C" int FUN_10047a10(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    (player - self).Length();      // computed and not used
    (player - self).Length2D();    // computed and not used
    DAT_10219e40[index] = 30;
    if (DAT_1020b430[index].state == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        DAT_1020b430[index].state = 1;
        float nearestSquared = 99999.0f;
        int nearest = -1;
        Vec4 marker;
        for (int i = 0x5c; i < 0x7c; i++)
        {
            if (DAT_1021f3c0[i] == ',' && DAT_10db6ee8[i].root != 0)
            {
                marker.Copy(DAT_10db6ee8[i].matrix.GetTranslation());
                if ((marker - self).LengthSquared3() < nearestSquared)
                {
                    nearest = i;
                    nearestSquared = (marker - self).LengthSquared3();
                }
            }
        }
        DAT_1020b430[index].marker = nearest;
        DAT_10255160[index][0] = sqrtf(nearestSquared);
        if (nearest == -1)
            FUN_10043cc0(index);
    }
    else if (DAT_1020b430[index].state == 1)
    {
        DAT_10255f10[index] = -1;
        Vec4 place;
        Vec4 step;
        Vec4 marker;
        if (DAT_10db6ee8[DAT_1020b430[index].marker].matrix.m[12] < -20000.0f)
            marker.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        else
            marker.Copy(DAT_10db6ee8[DAT_1020b430[index].marker].matrix.GetTranslation());
        place.y = -DAT_10255160[index][0];
        place.RotateZ(3.1415f - DAT_10db6ee8[DAT_1020b430[index].marker].heading);
        place.Copy(place + marker);
        step.Copy(place - self);
        float length = step.Length();
        if (length > 12.0f)
        {
            step.Normalized3();
            step.Copy(step * 12.0f);
            place.Copy(self + step);
        }
        float turn = atan2f(self.x - place.x, self.y - place.y);
        float facing = atan2f(self.x - marker.x, self.y - marker.y);
        turn -= facing;
        while (turn > 3.1415f)
            turn -= 6.283f;
        while (turn < -3.1415f)
            turn += 6.283f;
        if ((float)fabs(turn) < 0.7854f && (marker - self).Length() < length)
        {
            step.Fill(0.0f);
            step.y = -DAT_10255160[index][0];
            step.RotateZ(3.1415f - facing);
            if (turn < 0.0f)
                step.RotateZ(-0.17453f);
            else
                step.RotateZ(0.17453f);
            step.Copy(step + marker);
            DAT_10db6ee8[index].SetPosition(step.x, step.y, marker.z);
        }
        else
            DAT_10db6ee8[index].SetPosition(place.x, place.y, place.z);
        float heading = DAT_10db6ee8[DAT_1020b430[index].marker].heading;
        DAT_10db6ee8[index].targetHeading = heading;
        turn = heading - DAT_10db6ee8[index].heading;
        while (turn > 3.1415f)
            turn -= 6.283f;
        while (turn < -3.1415f)
            turn += 6.283f;
        float settled = (float)fabs(turn);

        if (settled > 0.01756f || (place - self).LengthSquared3() > 4.0f)
            DAT_10db6ee8[index].PlayMotion(0, 0, 5, 1.0f);
        else
            DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if (DAT_10db6ee8[DAT_1020b430[index].marker].matrix.m[12] < -20000.0f)
        {
            FUN_10043cc0(index);
            DAT_10210b30[index] = 0;
            return 0;
        }
    }
    DAT_10210b30[index] = 0;
    return 0;
}
