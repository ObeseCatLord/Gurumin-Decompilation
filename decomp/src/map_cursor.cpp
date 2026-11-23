// The map cursor: a marker that moves from node to node along a map record's route (DAT_10210fc0 rows of x, y, a
// node kind and a code) and follows a rider that lands on it. Called once per frame by the cursor's behaviour
// (0x10051570).

#include "anim_model.h"

#include "char_slot.h"

#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"
#include "map_records.h"

// Node kinds (RoutePoint::z of the cursor's route).
#define NODE_JUMP 1           // code = the node to jump to
#define NODE_ENTRY 2          // code = the node to start from
#define NODE_RIDER_STOP 4     // stops the cursor; a rider lands here
#define NODE_BACK 5           // the cursor goes back one node
#define NODE_PORTAL 6         // code = route * 10000 + node: continues there, looking back
#define NODE_PORTAL_FORWARD 7 // the same, looking forward

// Cursor modes.
#define CURSOR_MODE_JUMP 1    // follow the node's jump, or go to the requested node
#define CURSOR_MODE_ENTRY 2   // follow the node's entry, or go to the requested node

// The cursor is on a node when it is within this many units of it on both axes.
#define NODE_RANGE 10.0f
// Frames the cursor keeps counting as ridden after a rider landed on it.
#define ARRIVAL_HOLD_FRAMES 15
// Rider detection: the rider must stand up to 100 units above, within 200 units (squared 40000).
#define RIDER_HEIGHT 100
#define RIDER_DISTANCE_SQUARED 40000
// The hold must be longer than this for a rider to speed the cursor up.
#define RIDER_SPEED_HOLD 10
// Factor that stopping at a rider node applies to the cursor's speed scale.
#define RIDER_SPEED_FACTOR 2.5f

extern "C" int FUN_100249c0(int lower, int upper, int maxHeight, int maxDistanceSquared);
extern "C" int FUN_100286b0(int angle);

// 0x10051190 (MapCursor_Step). Moves cursor `index` (standing on node `step` of its route record) one step: it
// notes whether a rider landed on it (the hold counts the frames since), doubles its speed scale while the hold
// lasts, adjusts the node to head for when the rider's arrival or departure changed (*kind, *previousStep, *wasRidden
// keep the previous node's kind, the node before and the rider flag between calls) and, when the cursor at
// (*x, *y) reached the node, picks the next one: `target` (-1 = the node's own jump or entry) in modes 1 and 2,
// otherwise by the node's kind (a rider node reverses the cursor and changes its speed, a back node goes back, a
// portal switches to its route). Writes the node to *outStep and its position to *x and *y. Returns 0. Does
// nothing without `wasRidden`.
extern "C" int FUN_10051190(int index, int step, int *outStep, float *x, float *y, int mode, int target, int unused,
                            int *kind, int *previousStep, int *wasRidden)
{
    if (wasRidden == 0)
        return 0;
    int landed = FUN_100249c0(index, 0, RIDER_HEIGHT, RIDER_DISTANCE_SQUARED);
    int held = landed;
    if (landed != 0)
        DAT_1020b430[index].arrivalHold = ARRIVAL_HOLD_FRAMES;
    int *hold = &DAT_1020b430[index].arrivalHold;
    if (*hold > 0)
    {
        held = 1;
        *hold = *hold - 1;
    }
    CharSlot &slot = DAT_10daaae8[index];
    if (slot.m_0141 != 0 && held != 0)
        DAT_10db6ee8[index].speedScale = DAT_10db6ee8[index].speedScale + DAT_10db6ee8[index].speedScale;
    if (kind != 0 && previousStep != 0 && *hold > 0)
    {
        if (held != 0 && *wasRidden == 0 && landed != 0)
        {
            if (*previousStep > step && *kind == NODE_RIDER_STOP)
                step = *previousStep;
            else if (*previousStep == step && *kind == NODE_RIDER_STOP)
                step = step + 1;
        }
        else if (landed == 0 && *hold == 1)
        {
            *hold = 0;
            if (*previousStep < step && *kind == NODE_RIDER_STOP)
                step = *previousStep;
            else if (*previousStep == step && *kind == NODE_RIDER_STOP)
                step = step - 1;
        }
        *wasRidden = landed;
        DAT_10c63520 = landed != 0;
    }
    float nodeX = (float)DAT_10210fc0[slot.routeRecord].points[step].x;
    float nodeY = (float)DAT_10210fc0[slot.routeRecord].points[step].y;
    if (nodeX - NODE_RANGE < *x && nodeX + NODE_RANGE > *x && nodeY - NODE_RANGE < *y && nodeY + NODE_RANGE > *y)
    {
        if (previousStep != 0)
            *previousStep = step;
        if (kind != 0)
            *kind = DAT_10210fc0[slot.routeRecord].points[step].z;
        if (mode == CURSOR_MODE_JUMP)
        {
            if (target != -1)
                step = target;
            else if (DAT_10210fc0[slot.routeRecord].points[step].z == NODE_JUMP &&
                     DAT_10210fc0[slot.routeRecord].points[step].code > -1)
                step = DAT_10210fc0[slot.routeRecord].points[step].code;
        }
        else if (mode == CURSOR_MODE_ENTRY)
        {
            if (target != -1)
                step = target;
            else if (DAT_10210fc0[slot.routeRecord].points[step].z == NODE_ENTRY)
            {
                int entry = DAT_10210fc0[slot.routeRecord].points[step].code;
                step = entry < 0 ? 0 : entry;
            }
        }
        else
        {
            int nodeKind = DAT_10210fc0[slot.routeRecord].points[step].z;
            if (nodeKind == NODE_ENTRY)
            {
                int entry = DAT_10210fc0[slot.routeRecord].points[step].code;
                step = entry < 0 ? 0 : entry;
            }
            else if (nodeKind == NODE_RIDER_STOP)
            {
                slot.m_0141 = 1;
                slot.stats[2] = -1;
                DAT_10255f10[index] = -1;
                if (held != 0 && *hold > RIDER_SPEED_HOLD)
                {
                    DAT_10db6ee8[index].speedScale = DAT_10db6ee8[index].speedScale * RIDER_SPEED_FACTOR;
                    step = step + 1;
                }
                else
                {
                    *hold = 0;
                    step = step - 1;
                }
            }
            else if (nodeKind == NODE_BACK)
                step = step - 1;
            else if (nodeKind != NODE_PORTAL_FORWARD)
                step = step + 1;
        }
        int landedKind = DAT_10210fc0[slot.routeRecord].points[step].z;
        if (landedKind == NODE_PORTAL)
        {
            int link = DAT_10210fc0[slot.routeRecord].points[step].code;
            step = FUN_100286b0(link % 10000);
            slot.routeRecord = FUN_100286b0(link / 10000);
            if (previousStep != 0)
                *previousStep = step - 1;
            if (kind != 0)
                *kind = DAT_10210fc0[slot.routeRecord].points[step].z;
        }
        else if (landedKind == NODE_PORTAL_FORWARD)
        {
            int link = DAT_10210fc0[slot.routeRecord].points[step].code;
            step = FUN_100286b0(link % 10000);
            slot.routeRecord = FUN_100286b0(link / 10000);
            if (previousStep != 0)
                *previousStep = step + 1;
            if (kind != 0)
                *kind = DAT_10210fc0[slot.routeRecord].points[step].z;
        }
        nodeX = (float)DAT_10210fc0[slot.routeRecord].points[step].x;
        nodeY = (float)DAT_10210fc0[slot.routeRecord].points[step].y;
    }
    *outStep = step;
    *x = nodeX;
    *y = nodeY;
    return 0;
}
