// The light platform with a call button (behaviour of the per-kind update 0x100f9670): the route-riding light of
// char_hikari.cpp with a "Box01" button model on it. It waits at its start, the player leaving it presses the button and
// it carries the player (and a character found at its start) along its route; it can be switched off and on again.

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "map_records.h"
#include "game.h"

#include "hikari_button.h"

extern "C" const char *FUN_1013f8a0(const char *name, int folder);   // builds the data path of `name`

// Puts the light `slot` (route record `record`) on its start point: the first route point of kind HIKARI_START_KIND
// when its record's script flag is set, else point 0; the position is unpacked from the point (kinds 6, 7, 9 and 10 are
// relative or wide), the first target is looked up and the light's step is cleared. The original expands no inline
// function in this unit (Game::TestScriptFlag stays a call) and writes this out twice.
#pragma inline_depth(0)
#define PLACE_ON_ROUTE(slot, record) \
    { \
    int row = HIKARI_ROUTE_ROWS; \
    if (DAT_10210fc0[record].points[0].code != 0 && DAT_10874848.TestScriptFlag(DAT_10210fc0[record].points[0].code)) \
    { \
        row = 0; \
        while (row < HIKARI_ROUTE_ROWS && DAT_10210fc0[record].points[row].z != HIKARI_START_KIND) \
            row++; \
    } \
    if (row != HIKARI_ROUTE_ROWS) \
    { \
        DAT_1020b430[slot].hikariPoint = row; \
        DAT_1020b430[slot].targetPoint = row; \
    } \
    else \
    { \
        DAT_1020b430[slot].hikariPoint = 0; \
        DAT_1020b430[slot].targetPoint = 0; \
    } \
    const RoutePoint &start = DAT_10210fc0[0].points[record * MAP_RECORD_ROWS + DAT_1020b430[slot].targetPoint]; \
    int packed = start.x; \
    int part = packed / HIKARI_COORD_UNIT; \
    int rest = packed % HIKARI_COORD_UNIT; \
    if (part > HIKARI_COORD_LIMIT) \
        part -= HIKARI_COORD_UNIT; \
    DAT_10db6ee8[slot].matrix.m[12] = (float)part; \
    if (rest > HIKARI_COORD_LIMIT) \
        rest -= HIKARI_COORD_UNIT; \
    DAT_10db6ee8[slot].matrix.m[13] = (float)rest; \
    part = start.y / HIKARI_COORD_UNIT; \
    if (part > HIKARI_COORD_LIMIT) \
        part -= HIKARI_COORD_UNIT; \
    float height = (float)part; \
    int kind = start.z; \
    DAT_10db6ee8[slot].matrix.m[14] = height; \
    if (kind == 6 || kind == 7 || kind == 9 || kind == 10) \
    { \
        if (kind == 10) \
        { \
            part = packed / HIKARI_WIDE_UNIT; \
            int wide = packed % HIKARI_WIDE_UNIT; \
            if (part > HIKARI_COORD_LIMIT) \
                part -= HIKARI_COORD_UNIT; \
            DAT_10db6ee8[slot].matrix.m[12] = (float)part; \
            DAT_10db6ee8[slot].matrix.m[13] = (float)wide; \
        } \
        if (kind == 9) \
        { \
            if (DAT_10db6ee8[slot].matrix.m[12] < 0.0f) \
                DAT_10db6ee8[slot].matrix.m[12] += HIKARI_RELATIVE_WRAP; \
            if (height < 0.0f) \
                DAT_10db6ee8[slot].matrix.m[14] = height + HIKARI_RELATIVE_WRAP; \
        } \
        else \
        { \
            if (DAT_10db6ee8[slot].matrix.m[12] < 0.0f && kind == 6) \
                DAT_10db6ee8[slot].matrix.m[12] += HIKARI_RELATIVE_WRAP; \
            if (DAT_10db6ee8[slot].matrix.m[13] < 0.0f) \
                DAT_10db6ee8[slot].matrix.m[13] += HIKARI_RELATIVE_WRAP; \
            if (height < 0.0f && kind == 6) \
                DAT_10db6ee8[slot].matrix.m[14] = height + HIKARI_RELATIVE_WRAP; \
        } \
    } \
    target.Copy(matrix->GetTranslation()); \
    FUN_100286d0(slot, &DAT_1020b430[slot].hikariPoint, DAT_1020b430[slot].targetPoint, &next, &target, &group, \
                 &value, &action); \
    DAT_1020b430[slot].targetPoint = next; \
    target.Copy(matrix->GetTranslation()); \
    zero = 0.0f; \
    DAT_10802bf8[slot].step.Fill(zero); \
    DAT_1020b430[slot].hikariSound = 0; \
    }

// 0x10079470 (Char_HikariButtonRide). One frame of the button light `slot`. A global switch (buttonLiftOff and
// DAT_102024b4) pauses it (button "stop", hikariReady 3) and resumes it (button blue, hikariReady 2). On its first call
// it finds its "hikari" node and its "Box01" button (loading the button model when it has none) and goes to its start
// point like Char_HikariRide; behaviour BUTTON_MODE_START does the same again and leaves it waiting
// (BUTTON_MODE_WAITING) with the button red. Waiting, it presses the button ("up") and goes idle once the player is
// more than BUTTON_WAIT_DISTANCE from it. Moving, it follows its route like Char_HikariRide (speed counter, wobble in
// mode 2, a character found at its start is carried), lifts the player when it stops under him and plays the start and
// arrival sounds. `halfSpeed` makes it twice as slow. Returns 0.
extern "C" int FUN_10079470(int slot, int halfSpeed)
{
    Vec4 target;
    Vec4 step;
    Vec4 rider;
    Vec4 player;
    Matrix4 *matrix = &DAT_10db6ee8[slot].matrix;
    int value;
    int action;
    int lift;
    int next;
    int group;
    float zero;
    Node *box;
    target.Copy(matrix->GetTranslation());
    player.Copy(DAT_10db6ee8[BUTTON_PLAYER_SLOT].matrix.GetTranslation());
    next = 0;
    lift = 0;
    DAT_10db6ee8[slot].mode = 0;
    DAT_10db6ee8[slot].m_0294 = 1;
    if (DAT_1020b430[slot].buttonLiftOff == 0 && DAT_102024b4 != 0)
    {
        if (DAT_1020b430[slot].hikariReady == BUTTON_READY_PAUSED)
        {
            DAT_1020b430[slot].hikariReady = HIKARI_READY;
            if (DAT_1021bdf8[slot][1] != 0)
                DAT_1021bdf8[slot][1]->RestartColourPlugs(BUTTON_MOTION_IDLE, DAT_104bf8a4);
        }
    }
    else if (DAT_1020b430[slot].hikariReady == HIKARI_READY)
    {
        if (DAT_1021bdf8[slot][1] != 0)
            DAT_1021bdf8[slot][1]->RestartColourPlugs(BUTTON_MOTION_OFF, DAT_104bf8a4);
        DAT_1020b430[slot].hikariReady = BUTTON_READY_PAUSED;
    }
    if (DAT_1020b430[slot].hikariReady != HIKARI_READY && DAT_1020b430[slot].hikariReady != BUTTON_READY_PAUSED)
    {
        int record = DAT_10db6ee8[slot].bankIndex;
        DAT_1020b430[slot].hikariReady = HIKARI_READY;
        DAT_1021bdf8[slot][0] = DAT_10db6ee8[slot].root->Find(HIKARI_NODE, 0);
        DAT_1021bdf8[slot][1] = DAT_10db6ee8[slot].root->Find(BUTTON_NODE, 0);
        if (DAT_1021bdf8[slot][1] == 0)
        {
            Node *child = DAT_10db6ee8[slot].root->AddChild(0, BUTTON_MODEL_SIZE);
            child->Load(FUN_1013f8a0(BUTTON_MODEL, 0), 0, 0, 0);
            child->BuildMesh(1, 0);
            DAT_1021bdf8[slot][1] = DAT_10db6ee8[slot].root->Find(BUTTON_NODE, 0);
            DAT_1021bdf8[slot][1]->RestartColourPlugs(BUTTON_MOTION_IDLE, DAT_104bf8a4);
        }
        PLACE_ON_ROUTE(slot, record)
    }
    if (DAT_1020b430[slot].behaviour == BUTTON_MODE_START)
    {
        int record = DAT_10db6ee8[slot].bankIndex;
        DAT_1020b430[slot].behaviour = BUTTON_MODE_WAITING;
        DAT_1020b430[slot].hikariReady = HIKARI_READY;
        DAT_1021bdf8[slot][0] = DAT_10db6ee8[slot].root->Find(HIKARI_NODE, 0);
        if (DAT_1021bdf8[slot][1] != 0)
            DAT_1021bdf8[slot][1]->RestartColourPlugs(BUTTON_MOTION_ARMED, DAT_104bf8a4);
        PLACE_ON_ROUTE(slot, record)
    }
    if ((DAT_10c635ec != 0 && DAT_10c62ed0 == HIKARI_MAP_A) ||
        (DAT_10c65600 != 0 && DAT_10c62ed0 == HIKARI_MAP_B && DAT_10c635bc == 0))
    {
        DAT_1020b430[slot].behaviour = HIKARI_MODE_RESET;
        DAT_10c635ec = 0;
        DAT_10c635bc = 1;
    }
    if (DAT_1020b430[slot].wait == 0 && DAT_1020b430[slot].behaviour != 0)
    {
        if (DAT_1020b430[slot].behaviour == BUTTON_MODE_WAITING)
        {
            if ((target - player).Length() <= BUTTON_WAIT_DISTANCE)
                return 0;
            DAT_10db6ee8[slot].root->Find(BUTTON_PRESS_NODE, 0)->RestartColourPlugs(BUTTON_MOTION_PRESS, DAT_104bf8a4);
            box = DAT_1021bdf8[slot][1];
            DAT_1020b430[slot].behaviour = HIKARI_MODE_ENDED;
        }
        else
        {
            if (DAT_1020b430[slot].hikariSound == 0)
            {
                if (DAT_1020b430[slot].hikariStopped == 0)
                    FUN_10065b10(slot, BUTTON_SOUND_START, 1, 0);
                DAT_1020b430[slot].hikariSound = 1;
                if (DAT_1021bdf8[slot][0] != 0)
                    DAT_1021bdf8[slot][0]->RestartColourPlugs(LIFT_MOTION_RUN, DAT_104bf8a4);
                if (DAT_1021bdf8[slot][1] != 0)
                    DAT_1021bdf8[slot][1]->RestartColourPlugs(BUTTON_MOTION_ARMED, DAT_104bf8a4);
            }
            DAT_1020b430[slot].hikariStopped = 0;
            if (FUN_100286d0(slot, &DAT_1020b430[slot].hikariPoint, DAT_1020b430[slot].targetPoint, &next, &target,
                             &group, &value, &action) == 1 &&
                DAT_1020b430[slot].behaviour != 0)
            {
                FUN_10065b10(slot, BUTTON_SOUND_ARRIVE, 1, 0);
                DAT_1020b430[slot].hikariScanned = 0;
                DAT_1020b430[slot].targetPoint = next;
                DAT_1020b430[slot].behaviour = BUTTON_MODE_WAITING;
                DAT_1020b430[slot].hikariHold = HIKARI_END_HOLD;
                return 0;
            }
            if (DAT_1020b430[slot].hikariScanned == 0)
            {
                int carried;
                DAT_1020b430[slot].hikariScanned = 1;
                for (carried = HIKARI_FIRST_CARRIED; carried < HIKARI_END_CARRIED; carried++)
                {
                    if (slot != carried &&
                        DAT_10db6ee8[slot].matrix.m[12] + HIKARI_REACH_XY > DAT_10db6ee8[carried].matrix.m[12] - HIKARI_REACH_XY &&
                        DAT_10db6ee8[slot].matrix.m[12] - HIKARI_REACH_XY < DAT_10db6ee8[carried].matrix.m[12] + HIKARI_REACH_XY &&
                        DAT_10db6ee8[slot].matrix.m[13] + HIKARI_REACH_XY > DAT_10db6ee8[carried].matrix.m[13] - HIKARI_REACH_XY &&
                        DAT_10db6ee8[slot].matrix.m[13] - HIKARI_REACH_XY < DAT_10db6ee8[carried].matrix.m[13] + HIKARI_REACH_XY &&
                        DAT_10db6ee8[slot].matrix.m[14] + HIKARI_REACH_Z > DAT_10db6ee8[carried].matrix.m[14] - HIKARI_REACH_Z &&
                        DAT_10db6ee8[slot].matrix.m[14] - HIKARI_REACH_Z < DAT_10db6ee8[carried].matrix.m[14] + HIKARI_REACH_Z)
                    {
                        DAT_1020b430[slot].hikariFollower = carried;
                        DAT_104bfdd8 = carried;
                        break;
                    }
                }
                if (carried == HIKARI_END_CARRIED)
                    DAT_1020b430[slot].hikariFollower = 0;
            }
            DAT_1020b430[slot].targetPoint = next;
            DAT_10db6ee8[slot].motionFrame = DAT_104bf8a4;
            step.Copy(target - matrix->GetTranslation());
            if (DAT_1020b430[slot].hikariSpeed < HIKARI_TOP_SPEED)
                DAT_1020b430[slot].hikariSpeed++;
            if (group == 2 || group == 4)
            {
                if (step.Length() < (float)DAT_1020b430[slot].hikariSpeed * HIKARI_SLOW_SCALE)
                    DAT_1020b430[slot].hikariSpeed =
                        (int)(HIKARI_SLOW_BASE - (HIKARI_SLOW_RANGE - step.Length()) * HIKARI_SLOW_FACTOR);
            }
            if (step.x != 0.0f || step.y != 0.0f || step.z != 0.0f)
                step.Normalized3();
            if (halfSpeed != 0)
                step.Copy(step * ((float)DAT_1020b430[slot].hikariSpeed / HIKARI_HALF_DIVISOR));
            else
                step.Copy(step * ((float)DAT_1020b430[slot].hikariSpeed / HIKARI_FAST_DIVISOR));
            if (DAT_1020b430[slot].behaviour == HIKARI_MODE_SHAKY && FUN_1013f880() % HIKARI_SHAKE_PERIOD == 0)
            {
                step.x = (float)(FUN_1013f880() % HIKARI_SHAKE_RANGE - HIKARI_SHAKE_CENTRE) * HIKARI_SHAKE_SCALE + step.x;
                step.y = (float)(FUN_1013f880() % HIKARI_SHAKE_RANGE - HIKARI_SHAKE_CENTRE) * HIKARI_SHAKE_SCALE + step.y;
            }
            if (DAT_1020b430[slot].behaviour < HIKARI_MODE_RESUMED)
            {
                rider.Copy(DAT_10db6ee8[BUTTON_PLAYER_SLOT].matrix.GetTranslation());
                if (step.z > 0.0f)
                    lift = BUTTON_LIFT_RISING;
            }
            if (DAT_1020b430[slot].hikariFollower != 0)
            {
                DAT_10db6ee8[DAT_1020b430[slot].hikariFollower].SetPosition(
                    step.x + DAT_10db6ee8[slot].matrix.m[12], step.y + DAT_10db6ee8[slot].matrix.m[13],
                    step.z + DAT_10db6ee8[slot].matrix.m[14] + HIKARI_FOLLOWER_HEIGHT);
                if (DAT_104bfdd0 == 0)
                    DAT_104bfdd8 = DAT_1020b430[slot].hikariFollower;
            }
            if (DAT_1020b430[slot].behaviour == HIKARI_MODE_RESET)
            {
                DAT_10db6ee8[slot].matrix.m[12] = target.x;
                DAT_10db6ee8[slot].matrix.m[13] = target.y;
                DAT_10db6ee8[slot].matrix.m[14] = target.z;
            }
            else if (DAT_10db6ee8[slot].IsClearAny(&step) == 0 && lift == 0)
            {
                if (DAT_10db6ee8[BUTTON_PLAYER_SLOT].matrix.GetTranslation().Distance(matrix->GetTranslation()) <
                    BUTTON_RIDER_LIMIT)
                    DAT_10db6ee8[BUTTON_PLAYER_SLOT].matrix.m[14] += BUTTON_RIDER_LIFT;
                DAT_1020b430[slot].behaviour = HIKARI_MODE_ENDED;
                DAT_1020b430[slot].hikariStopped = 1;
            }
            else
            {
                DAT_10db6ee8[slot].matrix.m[12] += step.x;
                DAT_10db6ee8[slot].matrix.m[13] += step.y;
                DAT_10db6ee8[slot].matrix.m[14] += step.z;
            }
            zero = 0.0f;
            DAT_10db6ee8[slot].velocity.Fill(zero);
            DAT_10802bf8[slot].step.Copy(step);
            return 0;
        }
    }
    else
    {
        if (DAT_1020b430[slot].hikariStopped != 0)
        {
            if (DAT_10db6ee8[slot].IsClearAny(&DAT_10802bf8[slot].step) != 0)
            {
                DAT_1020b430[slot].hikariStopped = 0;
                DAT_1020b430[slot].behaviour = HIKARI_MODE_RESUMED;
                return 0;
            }
            if (DAT_10db6ee8[BUTTON_PLAYER_SLOT].matrix.GetTranslation().Distance(matrix->GetTranslation()) <
                BUTTON_RIDER_LIMIT)
                DAT_10db6ee8[BUTTON_PLAYER_SLOT].matrix.m[14] += BUTTON_RIDER_LIFT;
            return 0;
        }
        DAT_1020b430[slot].hikariSpeed = HIKARI_IDLE_SPEED;
        if (DAT_1020b430[slot].hikariSound == 0)
            return 0;
        DAT_1020b430[slot].hikariSound = 0;
        if (DAT_1021bdf8[slot][0] != 0)
            DAT_1021bdf8[slot][0]->RestartColourPlugs(BUTTON_MOTION_OFF, DAT_104bf8a4);
        box = DAT_1021bdf8[slot][1];
    }
    if (box != 0)
        box->RestartColourPlugs(BUTTON_MOTION_IDLE, DAT_104bf8a4);
    return 0;
}
