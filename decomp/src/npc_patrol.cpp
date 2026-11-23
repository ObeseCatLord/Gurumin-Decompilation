// A character that patrols one of the map's routes (0x10044b10, behaviour 8 of Char_RunBehaviour 0x100dfcf0): it
// stands at a route point watching the player, then walks to the next point.

#include "last_position.h"
#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_walker[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "route_point.h"

#include "map_records.h"
#include "char_tables.h"
#include "char_vars.h"
#include "world.h"

#define PATROL_SIGHT_RANGE 750.0f
#define PATROL_SIGHT_ANGLE 1.0995574f
#define PATROL_SIGHT_NEAR_RANGE 100.0f
#define PATROL_POINT_RADIUS 20
#define PATROL_ALERT_PROMPT 15
#define PATROL_ALERT_FRAMES 60
#define PATROL_WAKE_RANGE 1500.0f
#define PATROL_TURN_FRAMES 7
#define PATROL_FACE_TOLERANCE 0.12217f
#define PATROL_ALERT_SPEED 15.0f
#define PATROL_BLOCKED_SPEED 32.0f

extern "C" int FUN_10043cc0(int index);                                     // Char_ResetState
extern "C" int FUN_1000d620(int index, int key, int frames);                // KeyPrompt_Show
extern "C" int FUN_1001b670(int index, float range, int count, ...);       // Enemies_WakeNearby
extern "C" int FUN_100241e0(int index, float x, float y, float z);          // Character_SetMove
extern "C" int FUN_10043e20(const Vec4 *target, const Vec4 *eye, float heading, float range, float angleRange,
                            float nearRange, int checkWalls);               // Char_CanSee
extern "C" int FUN_1002f320(int walker, int route, int point, int *outPoint, float *position, int *outAction,
                            int *outValue, int *outGroup, int radius);      // Route_Follow

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10044b10 (Npc_PatrolRoute). A character posted on a route (talk id = start point * 100 + route). Every frame it
// checks whether it sees the player (from its head, 750 units, 63 degrees either side or 100 units) or was told to
// react. Standing at a point (state 1) it faces the player (idle motion 2 once within about 7 degrees, else the
// walking motion when the point's group is set) and heads for the next point (state 2: units per frame from the
// route's first point, running when the map blocks it) where it turns for 8 frames (state 3) and carries on. Digits
// of the point's value pick flags: bit 0 of value / 1000 marks it as running (m_008d), bit 1 keeps it from being
// reset. Reaching a point whose action is in the route's mask resets the character; seeing the player at a point
// with action above 1 wakes the enemies within 1500 and resets it. Returns 0.
extern "C" int FUN_10044b10(int index)
{
    Vec4 player;
    Vec4 self;
    Vec4 target;
    int point;
    int group;
    int value;
    int keep = 0;
    int action;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    target.Copy(self);
    int reacts;
    {
        Vec4 head;
        Vec4 eye;
        head.Copy(self);
        eye.Copy(player);
        head.z += 75.0f;
        eye.z += 50.0f;
        reacts = FUN_10043e20(&head, &eye, DAT_10db6ee8[index].heading, PATROL_SIGHT_RANGE, PATROL_SIGHT_ANGLE,
                              PATROL_SIGHT_NEAR_RANGE, 1) |
                 DAT_10210b30[index];
    }
    if (DAT_10210b30[index] != 0 && DAT_1020b430[index].sawPlayer == 0)
        DAT_1020b430[index].sawPlayer = 2;
    DAT_10210b30[index] = 0;
    if (DAT_1020b430[index].state == 0)
    {
        int route = DAT_1020b430[index].talk % 100;
        int start = DAT_1020b430[index].talk / 100;
        DAT_1020b430[index].route = route;
        DAT_1020b430[index].nextPoint = start;
        FUN_1002f320(index, route, start, &point, &target.x, &action, &value, &group, PATROL_POINT_RADIUS);
        DAT_1020b430[index].nextPoint = point;
        DAT_1020b430[index].state = 1;
        keep = 1;
        DAT_1020b430[index].speed = DAT_10210fc0[DAT_1020b430[index].talk % 100].points[0].x;
        DAT_1020b430[index].wait = 0;
        DAT_1020b430[index].fadeActions = DAT_10210fc0[DAT_1020b430[index].talk % 100].points[0].code;
        DAT_1020b430[index].arrivalAction = 0;
    }
    else if (DAT_1020b430[index].state == 1)
    {
        FUN_1002f320(index, DAT_1020b430[index].route, DAT_1020b430[index].nextPoint, &point, &target.x, &action,
                     &value, &group, PATROL_POINT_RADIUS);
        DAT_1020b430[index].nextPoint = point;
        if ((value / 1000 & 2) != 0)
            keep = 1;
        float angle = Atan2(player.x - self.x, player.y - self.y);
        angle += 3.1415f;
        DAT_10db6ee8[index].targetHeading = angle;
        float turn = angle - DAT_10db6ee8[index].heading;
        while (turn > 3.1415f)
            turn -= 6.283f;
        while (turn < -3.1415f)
            turn += 6.283f;
        if (turn > PATROL_FACE_TOLERANCE)
            DAT_10db6ee8[index].PlayMotion(group == 0 ? 0 : 1, 0, 5, 1.0f);
        else if (turn < -PATROL_FACE_TOLERANCE)
            DAT_10db6ee8[index].PlayMotion(group == 0 ? 0 : 1, 0, 5, 1.0f);
        else
            DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if (reacts != 0)
        {
            FUN_1000d620(index, PATROL_ALERT_PROMPT, PATROL_ALERT_FRAMES);
            DAT_10db6ee8[index].AddMotionSpeed(PATROL_ALERT_SPEED);
            DAT_1020b430[index].state = 3;
            if (DAT_1020b430[index].arrivalAction > 1)
            {
                FUN_1001b670(index, PATROL_WAKE_RANGE, 0);
                FUN_10043cc0(index);
            }
        }
    }
    else if (DAT_1020b430[index].state == 2)
    {
        Vec4 step;
        Vec4 ahead;
        int reached = FUN_1002f320(index, DAT_1020b430[index].route, DAT_1020b430[index].nextPoint, &point,
                                   &target.x, &action, &value, &group, PATROL_POINT_RADIUS);
        DAT_10db6ee8[index].targetHeading = Atan2(target.x - self.x, target.y - self.y) + 3.1415f;
        if ((value / 1000 & 1) != 0)
            DAT_10db6ee8[index].m_008d = 1;
        else
            DAT_10db6ee8[index].m_008d = 0;
        if ((value / 1000 & 2) != 0)
            keep = 1;
        DAT_10db6ee8[index].PlayMotion(group != 0, 0, 5, 1.0f);
        if (DAT_1020b430[index].nextPoint != point)
        {
            DAT_1020b430[index].arrivalAction = reached;
            DAT_1020b430[index].state = 1;
            DAT_1020b430[index].wait = 0;
            if (((1 << reached) & DAT_1020b430[index].fadeActions) != 0)
                FUN_10043cc0(index);
        }
        step.Copy(target - self);
        step.z = 0;
        step.Normalized3();
        step.Copy(step * (float)DAT_1020b430[index].speed);
        FUN_100241e0(index, step.x, step.y, 0);
        ahead.Copy(step);
        ahead.z += 10.0f;
        ahead.Copy(ahead * 8.0f);
        step.z += 40.0f;
        if (DAT_10db6ee8[index].m_008d == 0 && DAT_10db6ee8[index].m_238c != 0 &&
            (DAT_10db6ee8[index].IsClearOfMapLimited(&step) == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(&ahead) == 0))
            DAT_10db6ee8[index].AddMotionSpeed(PATROL_BLOCKED_SPEED);
    }
    else if (DAT_1020b430[index].state == 3)
    {
        Vec4 unusedA;
        Vec4 unusedB;
        FUN_1002f320(index, DAT_1020b430[index].route, DAT_1020b430[index].nextPoint, &point, &target.x, &action,
                     &value, &group, PATROL_POINT_RADIUS);
        DAT_10db6ee8[index].targetHeading = atan2f(target.x - self.x, target.y - self.y) + 3.1415f;
        if (++DAT_1020b430[index].wait > PATROL_TURN_FRAMES)
        {
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = 2;
        }
        keep = 1;
    }
    if (DAT_1020b430[index].sawPlayer == 1 && keep == 0)
        FUN_10043cc0(index);
    return 0;
}
