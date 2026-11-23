#include <math.h>

// A character that walks one of the map's routes (0x100451c0, behaviour 9 of Char_RunBehaviour 0x100dfcf0),
// optionally carrying another character on its back.

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_walker[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

// The last position and step of each character (route_walk.cpp StepRecord); here the position holds the velocity.
struct LastPosition
{
    Vec4 position;
    Vec4 step;
    char pad_0020[0x20];
};

#include "char_slot.h"

#include "route_point.h"

extern "C" LastPosition DAT_10802bf8[0x80];
extern "C" RoutePoint DAT_10210fc0[][16];     // the routes of the map
#include "char_tables.h"
#include "world.h"
extern "C" int FUN_10043cc0(int index);       // Char_ResetState
extern "C" int FUN_1002f320(int walker, int route, int point, int *outPoint, float *position, int *outAction,
                            int *outValue, int *outGroup, int radius);  // Route_Follow

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x100451c0 (Npc_WalkRoute). Walks route talk % 100 from point 1 at the speed in the route's first point (x),
// easing its velocity (0.9 old + 0.1 new), turning to the point ahead and pausing where a point's value says so.
// A point whose action is in the route's fade mask (first point's code) fades it out over 15 frames and takes it
// out of play; a point with value 1 (or being told to react) drops the rider (talk / 100 characters later), which
// is reset. While it walks, the rider sits 50 units above it. Returns 0.
extern "C" int FUN_100451c0(int index)
{
    Vec4 player;
    Vec4 self;
    Vec4 target;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    target.Copy(self);
    int point;
    int action;
    int value;
    int group;
    DAT_10db6ee8[index].mode = 0;
    if (DAT_10210b30[index] != 0)
    {
        DAT_1020b430[index].state = 4;
        DAT_10210b30[index] = 0;
        DAT_10db6ee8[index].alpha = 1.0f;
    }
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].nextPoint = 1;
        DAT_1020b430[index].reversed = 0;
        DAT_1020b430[index].route = DAT_1020b430[index].talk % 100;
        FUN_1002f320(index, DAT_1020b430[index].talk % 100, 1, &point, &target.x, &action, &value, &group, 20);
        DAT_1020b430[index].nextPoint = point;
        DAT_1020b430[index].state = 2;
        DAT_1020b430[index].wait = 0;
        DAT_1020b430[index].rider = DAT_1020b430[index].talk / 100 + index;
        DAT_1020b430[index].speed = DAT_10210fc0[DAT_1020b430[index].talk % 100][0].x;
        DAT_1020b430[index].fadeActions = DAT_10210fc0[DAT_1020b430[index].talk % 100][0].code;
        if (value > 1)
        {
            DAT_1020b430[index].pointPause = value;
            DAT_1020b430[index].state = 1;
            DAT_1020b430[index].wait = 0;
        }
    }
    else if (DAT_1020b430[index].state == 1)
    {
        if (++DAT_1020b430[index].wait > DAT_1020b430[index].pointPause)
        {
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = 2;
        }
    }
    else if (DAT_1020b430[index].state == 2)
    {
        Vec4 step;
        Vec4 velocity;
        int reached = FUN_1002f320(index, DAT_1020b430[index].route, DAT_1020b430[index].nextPoint, &point, &target.x,
                                   &action, &value, &group, DAT_1020b430[index].speed / 2 + 20);
        DAT_10db6ee8[index].targetHeading = (float)atan2(target.x - self.x, target.y - self.y) + 3.1415f;
        DAT_10db6ee8[index].PlayMotion(0, 0, 5, 1.0f);
        if (DAT_1020b430[index].nextPoint != point)
        {
            DAT_1020b430[index].state = 2;
            DAT_1020b430[index].wait = 0;
            if (((1 << reached) & DAT_1020b430[index].fadeActions) != 0)
            {
                DAT_1020b430[index].state = 5;
                DAT_1020b430[index].wait = 0;
                if (value == 2)
                {
                    DAT_1020b430[index].state = 2;
                    DAT_1020b430[index].wait = 0;
                }
                else if (value == 1)
                {
                    if (DAT_1020b430[index].rider != 0)
                        DAT_1020b430[index].state = 4;
                }
                else if (value > 1)
                {
                    DAT_1020b430[index].pointPause = value;
                    DAT_1020b430[index].state = 1;
                    DAT_1020b430[index].wait = 0;
                }
            }
            else if (value == 1)
            {
                if (DAT_1020b430[index].rider != 0)
                    DAT_1020b430[index].state = 4;
            }
            else if (value > 1)
            {
                DAT_1020b430[index].pointPause = value;
                DAT_1020b430[index].state = 1;
                DAT_1020b430[index].wait = 0;
            }
        }
        step.Copy(target - self);
        step.Normalized3();
        step.Copy(step * (float)DAT_1020b430[index].speed);
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.9f + step * 0.1f);
        Vec4 position;
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + DAT_10802bf8[index].position);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
        DAT_1020b430[index].nextPoint = point;
    }
    else if (DAT_1020b430[index].state == 4)
    {
        int rider = DAT_1020b430[index].rider;
        if (rider != 0 && DAT_10db6ee8[rider].root != 0 && DAT_10210d30[rider] == 0 && DAT_10daaae8[rider].stats[STAT_HEALTH] > 0)
            FUN_10043cc0(rider);
        DAT_1020b430[index].rider = 0;
        DAT_1020b430[index].state = 2;
        Vec4 position;
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.9f);
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + DAT_10802bf8[index].position);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
    }
    else if (DAT_1020b430[index].state == 5)
    {
        DAT_10db6ee8[index].alpha = (float)(15 - DAT_1020b430[index].wait++) / 15.0f;
        if (DAT_1020b430[index].wait == 15)
        {
            DAT_10db6ee8[index].alpha = 1.0f;
            DAT_10db6ee8[index].SetPosition(-200000.0f, 0, 0);
            DAT_1020d540[index] = 0;
            DAT_1021b4c8[index] = 0;
            DAT_10867e18[index].Fill(0.0f);
            DAT_10256420[index] = 0;
        }
        Vec4 position;
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.9f);
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + DAT_10802bf8[index].position);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
    }
    if (DAT_10210d30[index] == 0)
    {
        int rider = DAT_1020b430[index].rider;
        if (rider != 0 && DAT_10db6ee8[rider].root != 0 && DAT_10210d30[rider] == 0 && DAT_10daaae8[rider].stats[STAT_HEALTH] > 0)
        {
            Vec4 position;
            position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            DAT_10db6ee8[DAT_1020b430[index].rider].SetPosition(position.x, position.y, position.z + 50.0f);
            DAT_10254598[DAT_1020b430[index].rider] = 0;
        }
    }
    return 0;
}
