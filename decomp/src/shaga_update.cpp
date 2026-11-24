// The per-frame update of the "shaga" pieces (shaga_spawn.cpp): loot that breaks off defeated enemies and equipment.
// Each of the 0x28 slots (DAT_10446e00) has a model node, a velocity (DAT_10da9160), a radius (DAT_1030cd98), a
// counter (DAT_1021d018) and a rest time (DAT_10254798; the thousands count how often it landed).
//
// Counter < 0: the piece is on its own (-1 and -character index); it tumbles under gravity and bounces off the
// world until it is slower than 1 unit per frame, then lies still, tilts upright and waits. Near the player it
// starts the collection (counter 0x68), after 900 frames without one it is dropped (0x3f7).
// Counter 101..199: the piece flies to a point above the player.
// Counter 100 and below: it is collected (the money counter DAT_10c6670c rises, up to 999) and then removed.

#include <math.h>

#include "texture_manager.h"

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "char_tables.h"
#include "char_vars.h"
#include "map_vars.h"
#include "game_vars.h"

#include "doppler_loops.h"
#include "shaga.h"

extern "C" int FUN_1000e750(void); // AntiTamper_Store
extern "C" int FUN_1013f880(void); // the game's random number

#define SHAGA_FALL_LEVEL (-3000.0f) // a piece below this height is lost into the pit
#define SHAGA_GRAVITY 3.0f
#define SHAGA_LIFT_ABOVE_PLAYER 50.0f
#define SHAGA_GRAB_DISTANCE 200.0f
#define SHAGA_REST_FRAMES 900 // frames a piece lies waiting before it is dropped
#define SHAGA_REST_UNIT 10000 // DAT_10254798: landed count * this + frames at rest
#define SHAGA_REST_LIMIT 80000
#define SHAGA_NO_COUNTER (-10000)
#define SHAGA_PICKUP_SOUND 0x205
#define SHAGA_MONEY_LIMIT 999
#define SHAGA_HUD_MONEY_TIMER 8
#define SHAGA_FIRST_HAND_CHARACTER 58
#define SHAGA_HAND_CHARACTER_END 0x5c
#define SHAGA_PI 3.1415f
#define SHAGA_DEGREES 180.0f
#define SHAGA_HEADINGS 0x168
#define SHAGA_FAR_AWAY (-200000.0f)

#define SHAGA_LANDING_KEEP 0.8f  // a landed piece turns towards upright: the share of its old rotation ...
#define SHAGA_LANDING_PULL 0.2f  // ... and of the upright one
#define SHAGA_RESTING_KEEP 0.35f // the same for a piece waiting on the ground
#define SHAGA_RESTING_PULL 0.65f
#define SHAGA_FLYING_KEEP 0.9f // and for a piece flying to the player
#define SHAGA_FLYING_PULL 0.1f
#define SHAGA_BOUNCE_LOSS 0.35f // speed lost along the surface normal per bounce
#define SHAGA_FLAT_NORMAL 0.2f  // a surface this level counts as ground (the piece may settle)
#define SHAGA_STOP_NORMAL 0.5   // on ground this level a slow piece stops dead
#define SHAGA_STEEP_NORMAL 0.75f
#define SHAGA_SPEED_LIMIT_RADII 2.0f // at most this many radii per frame
#define SHAGA_MIN_SPEED_BOOST 5.0f   // a piece that crawls without touching anything is set moving
#define SHAGA_SLOW_FACTOR 0.25f
#define SHAGA_BOUNCE_UP 10.0f
#define SHAGA_STILL_SPEED 1.0f
#define SHAGA_SLOW_DISTANCE 3.0f
#define SHAGA_MIN_FALL_SPEED (-4.0f)
#define SHAGA_SPIN_X 0.2f
#define SHAGA_SPIN_Z 0.12f
#define SHAGA_TUMBLE_X 0.6f
#define SHAGA_HOP_SPEED 3.0f

#define SHAGA_PITLOST_BASE 0x191 // counter of a piece lost into the pit, 401 (small kinds)
#define SHAGA_PITLOST_KIND_BASE 0x28
#define SHAGA_ATTRACT 0x68 // counter when the player is near
#define SHAGA_DROPPED 0x3f7
#define SHAGA_COLLECT_KIND_BASE 0x1d
#define SHAGA_FLY_END 100
#define SHAGA_PITLOST_END 400
#define SHAGA_PAID_END 300
#define SHAGA_HIDDEN_END 200
#define SHAGA_HIDDEN_RELEASE 0xcb
#define SHAGA_DONE_SOON 1000

// A node that is gone from the world is no longer held by the characters 58 to 0x5b (hand slots 2 and 3).
static __forceinline void ShagaReleaseHeld(Node *node)
{
    for (int character = SHAGA_FIRST_HAND_CHARACTER; character < SHAGA_HAND_CHARACTER_END; character++)
    {
        if (node == DAT_1021bdf8[character][2])
            DAT_1021bdf8[character][2] = 0;
        else if (node == DAT_1021bdf8[character][3])
            DAT_1021bdf8[character][3] = 0;
    }
}

static __forceinline void ShagaReleaseHeldBoth(Node *node)
{
    for (int character = SHAGA_FIRST_HAND_CHARACTER; character < SHAGA_HAND_CHARACTER_END; character++)
    {
        if (node == DAT_1021bdf8[character][2])
            DAT_1021bdf8[character][2] = 0;
        if (node == DAT_1021bdf8[character][3])
            DAT_1021bdf8[character][3] = 0;
    }
}

// True while shaga piece `slot` is on its own (negative counter) and still moving faster than 1 unit per frame.
static __forceinline int ShagaIsTumbling(int slot)
{
    float speed = DAT_10da9160[slot].Length();
    return speed > SHAGA_STILL_SPEED && DAT_1021d018[slot] < 0;
}

// One more coin for the money counter (at most SHAGA_MONEY_LIMIT).
static __forceinline void ShagaCountMoney(void)
{
    int money = DAT_10c6670c + 1;
    if (money > SHAGA_MONEY_LIMIT)
        money = SHAGA_MONEY_LIMIT;
    DAT_10c6670c = money;
}

// Shows the money counter on the HUD for a moment and refreshes the guarded copies of the values.
static __forceinline void ShagaShowMoney(void)
{
    DAT_104c011f = SHAGA_HUD_MONEY_TIMER;
    FUN_1000e750();
}

// 0x10042a30 (Shaga_Update). Updates every shaga slot once (see the top of the file). Returns 0.
int FUN_10042a30(void)
{
    Vec4 position;
    Vec4 work;
    Vec4 previous;
    Matrix4 identity;
    float depth;
    for (int slot = 0; slot < SHAGA_SLOT_COUNT; slot++)
    {
        if (DAT_10446e00[slot] != 0)
        {
            if (DAT_1021d018[slot] > 0)
                DAT_1021d018[slot] = DAT_1021d018[slot] - 1;
            if (DAT_1021d018[slot] > SHAGA_NO_COUNTER && DAT_1021d018[slot] != 0)
            {
                if (ShagaIsTumbling(slot))
                {
                    if (DAT_10254798[slot] / SHAGA_REST_UNIT != 0)
                        DAT_10254798[slot] = DAT_10254798[slot] % SHAGA_REST_UNIT;
                    previous.Copy(DAT_10446e00[slot]->matrix.GetTranslation());
                    DAT_10da9160[slot].z -= SHAGA_GRAVITY;
                    float *radius = &DAT_1030cd98[slot];
                    float reach;
                    if (DAT_10da9160[slot].Length() > *radius + *radius)
                    {
                        DAT_10da9160[slot].Normalized3();
                        DAT_10da9160[slot].Copy(DAT_10da9160[slot] * *radius * SHAGA_SPEED_LIMIT_RADII);
                    }
                    position.Copy(DAT_10446e00[slot]->matrix.GetTranslation());
                    position.Copy(position + DAT_10da9160[slot]);
                    position.z += *radius;
                    identity.SetIdentity();
                    if (DAT_10255f08 != 0 &&
                        DAT_10255f08->Query(&identity, &position, *radius, &work, &depth, 0, 0) != 0 &&
                        (reach = *radius, reach * reach) > depth)
                    {
                        work.Normalized3();
                        DAT_10da9160[slot].Mirror(work);
                        if (work.z > 0.0f)
                        {
                            if (work.z > SHAGA_FLAT_NORMAL)
                                DAT_102194c8[slot] = 1;
                            DAT_10da9160[slot].Copy(DAT_10da9160[slot] -
                                                    DAT_10da9160[slot] * work.z * SHAGA_BOUNCE_LOSS);
                        }
                        else
                            DAT_10da9160[slot].Copy(DAT_10da9160[slot] -
                                                    DAT_10da9160[slot] * (1.0f - work.z * SHAGA_BOUNCE_LOSS));
                        if (DAT_10da9160[slot].Length() < SHAGA_SLOW_DISTANCE)
                        {
                            DAT_102194c8[slot] = 1;
                            if (work.z > SHAGA_STOP_NORMAL)
                                DAT_10da9160[slot].Fill(0.0f);
                            else if (DAT_10da9160[slot].z > SHAGA_MIN_FALL_SPEED)
                                DAT_10da9160[slot].z = SHAGA_MIN_FALL_SPEED;
                        }
                        reach = *radius;
                        position.Copy(position + work * (reach - sqrtf(depth) + 1.0f));
                        if (DAT_10255f08 != 0 &&
                            DAT_10255f08->Query(&identity, &position, *radius, &work, &depth, 0, 0) != 0)
                        {
                            Shaga_PushOut(slot);
                            position.Copy(DAT_10446e00[slot]->matrix.GetTranslation());
                            position.z += *radius;
                            DAT_10da9160[slot].Fill(1.0f);
                        }
                    }
                    else if (DAT_10da9160[slot].Length() < SHAGA_STILL_SPEED)
                    {
                        DAT_10da9160[slot].Normalized3();
                        DAT_10da9160[slot].Copy(DAT_10da9160[slot] * SHAGA_MIN_SPEED_BOOST);
                    }
                    position.z -= *radius;
                    if (DAT_1021d018[slot] > 0)
                        DAT_10446e00[slot]->matrix =
                            DAT_10446e00[slot]->matrix * RotationX(SHAGA_SPIN_X) * RotationZ(SHAGA_SPIN_Z);
                    else if (DAT_102194c8[slot] != 0)
                    {
                        work.Copy(QuatFromMatrix(DAT_10446e00[slot]->matrix));
                        work.x = work.x * SHAGA_LANDING_KEEP;
                        work.y = work.y * SHAGA_LANDING_KEEP;
                        work.z = work.z * SHAGA_LANDING_KEEP;
                        work.w = work.w * SHAGA_LANDING_KEEP + SHAGA_LANDING_PULL;
                        DAT_10446e00[slot]->matrix = RotationMatrix(work);
                    }
                    else
                    {
                        float heading = atan2f(DAT_10da9160[slot].x, DAT_10da9160[slot].y) + SHAGA_PI;
                        Matrix4 tumble;
                        tumble = RotationZ(-heading) * RotationX(SHAGA_TUMBLE_X) * RotationZ(heading);
                        DAT_10446e00[slot]->matrix = DAT_10446e00[slot]->matrix * tumble;
                    }
                    if (position.z < SHAGA_FALL_LEVEL)
                    {
                        char kind = DAT_1020da24[slot];
                        position.z = SHAGA_FALL_LEVEL;
                        DAT_1021d018[slot] = SHAGA_PITLOST_BASE;
                        if (kind > 1)
                            DAT_1021d018[slot] = (kind + SHAGA_PITLOST_KIND_BASE) * 10;
                        else
                            DAT_1021d018[slot] = SHAGA_PITLOST_BASE;
                        DAT_10446e00[slot]->RestartColourPlugs("anime2", DAT_104bf8a4);
                        ShagaReleaseHeld(DAT_10446e00[slot]);
                    }
                    DAT_10446e00[slot]->matrix.SetTranslation(position);
                    if ((previous - position).Length() < SHAGA_SLOW_DISTANCE)
                        DAT_10da9160[slot].Copy(DAT_10da9160[slot] * SHAGA_SLOW_FACTOR);
                    int resting = DAT_10254798[slot] % SHAGA_REST_UNIT;
                    if (resting > 15 && resting < SHAGA_REST_FRAMES)
                    {
                        if ((position - DAT_10db6ee8[0].matrix.GetTranslation()).Length() < SHAGA_GRAB_DISTANCE)
                        {
                            DAT_1021d018[slot] = SHAGA_ATTRACT;
                            DAT_10446e00[slot]->RestartColourPlugs("anime2", DAT_104bf8a4);
                            ShagaReleaseHeld(DAT_10446e00[slot]);
                        }
                    }
                    if (DAT_10254798[slot] % SHAGA_REST_UNIT < SHAGA_REST_FRAMES)
                        DAT_10254798[slot] = DAT_10254798[slot] + 1;
                    else
                    {
                        DAT_1021d018[slot] = SHAGA_DROPPED;
                        DAT_10446e00[slot]->RestartColourPlugs("anime3", DAT_104bf8a4);
                        ShagaReleaseHeldBoth(DAT_10446e00[slot]);
                    }
                }
                else if (DAT_1021d018[slot] < 0)
                {
                    if (DAT_10254798[slot] < SHAGA_REST_LIMIT)
                        DAT_10254798[slot] = DAT_10254798[slot] + SHAGA_REST_UNIT;
                    position.Copy(DAT_10446e00[slot]->matrix.GetTranslation());
                    position.z += DAT_1030cd98[slot];
                    float *radius = &DAT_1030cd98[slot];
                    identity.SetIdentity();
                    if (DAT_10255f08 != 0 && DAT_10254798[slot] < SHAGA_REST_LIMIT)
                    {
                        if (DAT_10255f08->Query(&identity, &position, *radius + 1.0f, &work, &depth, 0, 0) != 0)
                        {
                            work.Normalized3();
                            if (!(work.z > SHAGA_STEEP_NORMAL))
                                DAT_10da9160[slot].Copy(work * SHAGA_BOUNCE_UP);
                        }
                        else
                        {
                            position.z -= 1.0f;
                            DAT_10da9160[slot].z = SHAGA_HOP_SPEED;
                            DAT_10da9160[slot].y = SHAGA_HOP_SPEED;
                            DAT_10da9160[slot].RotateZ((float)(FUN_1013f880() % SHAGA_HEADINGS) * SHAGA_PI /
                                                       SHAGA_DEGREES);
                        }
                    }
                    position.z -= *radius;
                    work.Copy(QuatFromMatrix(DAT_10446e00[slot]->matrix));
                    work.x = work.x * SHAGA_RESTING_KEEP;
                    work.y = work.y * SHAGA_RESTING_KEEP;
                    work.z = work.z * SHAGA_RESTING_KEEP;
                    work.w = work.w * SHAGA_RESTING_KEEP + SHAGA_RESTING_PULL;
                    DAT_10446e00[slot]->matrix = RotationMatrix(work);
                    DAT_10446e00[slot]->matrix.SetTranslation(position);
                    if ((position - DAT_10db6ee8[0].matrix.GetTranslation()).Length() < SHAGA_GRAB_DISTANCE)
                    {
                        DAT_1021d018[slot] = SHAGA_ATTRACT;
                        DAT_10446e00[slot]->RestartColourPlugs("anime2", DAT_104bf8a4);
                        ShagaReleaseHeld(DAT_10446e00[slot]);
                    }
                    if (DAT_10254798[slot] % SHAGA_REST_UNIT < SHAGA_REST_FRAMES)
                        DAT_10254798[slot] = DAT_10254798[slot] + 1;
                    else if (DAT_1021d018[slot] < 0)
                    {
                        DAT_1021d018[slot] = SHAGA_DROPPED;
                        DAT_10446e00[slot]->RestartColourPlugs("anime3", DAT_104bf8a4);
                        ShagaReleaseHeldBoth(DAT_10446e00[slot]);
                    }
                }
                else if (DAT_1021d018[slot] <= SHAGA_DONE_SOON)
                {
                    int counter = DAT_1021d018[slot];
                    if (counter == SHAGA_DONE_SOON)
                        DAT_1021d018[slot] = 1;
                    else if (counter > SHAGA_PITLOST_END)
                    {
                        if (counter % 10 == 0)
                        {
                            ShagaCountMoney();
                            ShagaShowMoney();
                        }
                    }
                    else if (counter == SHAGA_PITLOST_END)
                    {
                        ShagaCountMoney();
                        ShagaShowMoney();
                        DAT_1021d018[slot] = 1;
                    }
                    else if (counter > SHAGA_PAID_END)
                    {
                        if (counter % 10 == 0)
                        {
                            ShagaCountMoney();
                            FUN_1003ad20(SHAGA_PICKUP_SOUND, &(const Vec4 &)DAT_10db6ee8[0].matrix.GetTranslation(),
                                         1.0f);
                            ShagaShowMoney();
                        }
                        Vec4 away;
                        away.x = SHAGA_FAR_AWAY;
                        DAT_10446e00[slot]->matrix.SetTranslation(away);
                    }
                    else if (counter == SHAGA_PAID_END)
                    {
                        ShagaCountMoney();
                        FUN_1003ad20(SHAGA_PICKUP_SOUND, &(const Vec4 &)DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                        ShagaShowMoney();
                        DAT_1021d018[slot] = 1;
                    }
                    else if (counter > SHAGA_HIDDEN_END)
                    {
                        if (counter == SHAGA_HIDDEN_RELEASE)
                            ShagaReleaseHeldBoth(DAT_10446e00[slot]);
                    }
                    else if (counter == SHAGA_HIDDEN_END)
                        DAT_1021d018[slot] = 1;
                    else if (counter > SHAGA_FLY_END)
                    {
                        position.Copy(DAT_10446e00[slot]->matrix.GetTranslation());
                        Vec4 target;
                        target.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                        target.z += SHAGA_LIFT_ABOVE_PLAYER;
                        position.Copy(position + (target - position) * (1.0f / ((float)DAT_1021d018[slot] - 100.0f)));
                        work.Copy(QuatFromMatrix(DAT_10446e00[slot]->matrix));
                        work.x = work.x * SHAGA_FLYING_KEEP;
                        work.y = work.y * SHAGA_FLYING_KEEP;
                        work.z = work.z * SHAGA_FLYING_KEEP;
                        work.w = work.w * SHAGA_FLYING_KEEP + SHAGA_FLYING_PULL;
                        DAT_10446e00[slot]->matrix = RotationMatrix(work);
                        DAT_10446e00[slot]->matrix.SetTranslation(position);
                    }
                    else if (counter == SHAGA_FLY_END)
                    {
                        char kind = DAT_1020da24[slot];
                        if (kind > 1)
                            DAT_1021d018[slot] = (kind + SHAGA_COLLECT_KIND_BASE) * 10;
                        else
                            DAT_1021d018[slot] = 1;
                        position.Copy(DAT_10446e00[slot]->matrix.GetTranslation());
                        Vec4 target;
                        target.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                        target.z += SHAGA_LIFT_ABOVE_PLAYER;
                        position.Copy(target);
                        work.Copy(QuatFromMatrix(DAT_10446e00[slot]->matrix));
                        work.x = work.x * SHAGA_FLYING_KEEP;
                        work.y = work.y * SHAGA_FLYING_KEEP;
                        work.z = work.z * SHAGA_FLYING_KEEP;
                        work.w = work.w * SHAGA_FLYING_KEEP + SHAGA_FLYING_PULL;
                        DAT_10446e00[slot]->matrix = RotationMatrix(work);
                        DAT_10446e00[slot]->matrix.SetTranslation(position);
                        ShagaCountMoney();
                        FUN_1003ad20(SHAGA_PICKUP_SOUND, &(const Vec4 &)DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                        ShagaShowMoney();
                    }
                }
            }
            else if (DAT_1021d018[slot] > SHAGA_NO_COUNTER)
            {
                DAT_10446e00[slot]->parent->RemoveChild(DAT_10446e00[slot], 0);
                DAT_10446e00[slot] = 0;
                DAT_1021d018[slot] = 0;
            }
        }
        else
        {
            if (DAT_1021d018[slot] != 0)
                DAT_1021d018[slot] = 0;
        }
    }
    return 0;
}
