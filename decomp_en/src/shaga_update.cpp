// The per-frame update of the shaga pieces (EN build). Ported from the matched JP source decomp/src/shaga_update.cpp:
// loot that breaks off defeated enemies and equipment. Each of the SHAGA_SLOTS slots has a model node, a velocity, a
// radius, a state counter (g_shagaTargetB) and a rest time (g_shagaCounter; the thousands count how often it landed).
//
// State counter < 0: the piece is on its own; it tumbles under gravity and bounces off the world until it is slower
// than 1 unit per frame, then lies still, tilts upright and waits. Near the player it starts the collection
// (SHAGA_ATTRACT), after SHAGA_REST_FRAMES frames without one it is dropped (SHAGA_DROPPED).
// State counter 101..199: the piece flies to a point above the player.
// State counter 100 and below: it is collected (the money counter rises, up to SHAGA_MONEY_LIMIT) and then removed.

#include <math.h>

#include "anti_tamper.h"
#include "char.h"
#include "engine.h"
#include "node.h"
#include "shaga.h"
#include "sound_effects.h"
#include "vec4.h"
#include "visibility.h"

// HUD money display timer (frames left); set to SHAGA_HUD_MONEY_TIMER whenever the money counter changes.
// EN 0x009e33e8 (JP DAT_104c011f).
extern "C" char g_hudMoneyTimer;

#define SHAGA_FALL_LEVEL (-3000.0f) // a piece below this height is lost into the pit
#define SHAGA_GRAVITY 3.0f
#define SHAGA_LIFT_ABOVE_PLAYER 50.0f
#define SHAGA_GRAB_DISTANCE 200.0f
#define SHAGA_REST_FRAMES 900 // frames a piece lies waiting before it is dropped
#define SHAGA_REST_UNIT 10000 // g_shagaCounter: landed count * this + frames at rest
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
        if (node == g_charNodes[character][2])
            g_charNodes[character][2] = 0;
        else if (node == g_charNodes[character][3])
            g_charNodes[character][3] = 0;
    }
}

static __forceinline void ShagaReleaseHeldBoth(Node *node)
{
    for (int character = SHAGA_FIRST_HAND_CHARACTER; character < SHAGA_HAND_CHARACTER_END; character++)
    {
        if (node == g_charNodes[character][2])
            g_charNodes[character][2] = 0;
        if (node == g_charNodes[character][3])
            g_charNodes[character][3] = 0;
    }
}

// True while shaga piece `slot` is on its own (negative counter) and still moving faster than 1 unit per frame.
static __forceinline int ShagaIsTumbling(int slot)
{
    float speed = g_shagaVelocity[slot].Length();
    return speed > SHAGA_STILL_SPEED && g_shagaTargetB[slot] < 0;
}

// One more coin for the money counter (at most SHAGA_MONEY_LIMIT).
static __forceinline void ShagaCountMoney(void)
{
    int money = g_guardedValue5 + 1;
    if (money > SHAGA_MONEY_LIMIT)
        money = SHAGA_MONEY_LIMIT;
    g_guardedValue5 = money;
}

// Shows the money counter on the HUD for a moment and refreshes the guarded copies of the values.
static __forceinline void ShagaShowMoney(void)
{
    g_hudMoneyTimer = SHAGA_HUD_MONEY_TIMER;
    AntiTamper_StoreValues();
}

// Shaga_Update: Updates every shaga slot once (see the top of the file). Returns 0.
int Shaga_Update(void)
{
    Vec4 position;
    Vec4 work;
    Vec4 previous;
    Matrix4 identity;
    float depth;
    for (int slot = 0; slot < SHAGA_SLOTS; slot++)
    {
        if (g_shagaNode[slot] != 0)
        {
            if (g_shagaTargetB[slot] > 0)
                g_shagaTargetB[slot] = g_shagaTargetB[slot] - 1;
            if (g_shagaTargetB[slot] > SHAGA_NO_COUNTER && g_shagaTargetB[slot] != 0)
            {
                if (ShagaIsTumbling(slot))
                {
                    if (g_shagaCounter[slot] / SHAGA_REST_UNIT != 0)
                        g_shagaCounter[slot] = g_shagaCounter[slot] % SHAGA_REST_UNIT;
                    previous.Copy(g_shagaNode[slot]->matrix.GetTranslation());
                    g_shagaVelocity[slot].z -= SHAGA_GRAVITY;
                    float *radius = &g_shagaRadius[slot];
                    float reach;
                    if (g_shagaVelocity[slot].Length() > *radius * SHAGA_SPEED_LIMIT_RADII)
                    {
                        g_shagaVelocity[slot].Normalized3();
                        g_shagaVelocity[slot].Copy(g_shagaVelocity[slot] * *radius * SHAGA_SPEED_LIMIT_RADII);
                    }
                    position.Copy(g_shagaNode[slot]->matrix.GetTranslation());
                    position.Copy(position + g_shagaVelocity[slot]);
                    position.z += *radius;
                    identity.SetIdentity();
                    if (g_mapRootNode != 0 &&
                        g_mapRootNode->Query(&identity, &position, *radius, &work, &depth, 0, 0) != 0 &&
                        (reach = *radius, reach * reach) > depth)
                    {
                        work.Normalized3();
                        g_shagaVelocity[slot].Mirror(work);
                        if (work.z > 0.0f)
                        {
                            if (work.z > SHAGA_FLAT_NORMAL)
                                g_shagaSettled[slot] = 1;
                            g_shagaVelocity[slot].Copy(g_shagaVelocity[slot] -
                                                    g_shagaVelocity[slot] * work.z * SHAGA_BOUNCE_LOSS);
                        }
                        else
                            g_shagaVelocity[slot].Copy(g_shagaVelocity[slot] -
                                                    g_shagaVelocity[slot] * (1.0f - work.z * SHAGA_BOUNCE_LOSS));
                        if (g_shagaVelocity[slot].Length() < SHAGA_SLOW_DISTANCE)
                        {
                            g_shagaSettled[slot] = 1;
                            if (work.z > SHAGA_STOP_NORMAL)
                                g_shagaVelocity[slot].Fill(0.0f);
                            else if (g_shagaVelocity[slot].z > SHAGA_MIN_FALL_SPEED)
                                g_shagaVelocity[slot].z = SHAGA_MIN_FALL_SPEED;
                        }
                        reach = *radius;
                        position.Copy(position + work * (reach - sqrtf(depth) + 1.0f));
                        if (g_mapRootNode != 0 &&
                            g_mapRootNode->Query(&identity, &position, *radius, &work, &depth, 0, 0) != 0)
                        {
                            Shaga_PushOut(slot);
                            position.Copy(g_shagaNode[slot]->matrix.GetTranslation());
                            position.z += *radius;
                            g_shagaVelocity[slot].Fill(1.0f);
                        }
                    }
                    else if (g_shagaVelocity[slot].Length() < SHAGA_STILL_SPEED)
                    {
                        g_shagaVelocity[slot].Normalized3();
                        g_shagaVelocity[slot].Copy(g_shagaVelocity[slot] * SHAGA_MIN_SPEED_BOOST);
                    }
                    position.z -= g_shagaRadius[slot];
                    if (g_shagaTargetB[slot] > 0)
                        g_shagaNode[slot]->matrix =
                            g_shagaNode[slot]->matrix * RotationX(SHAGA_SPIN_X) * RotationZ(SHAGA_SPIN_Z);
                    else if (g_shagaSettled[slot] != 0)
                    {
                        work.Copy(QuatFromMatrix(g_shagaNode[slot]->matrix));
                        work.x = work.x * SHAGA_LANDING_KEEP;
                        work.y = work.y * SHAGA_LANDING_KEEP;
                        work.z = work.z * SHAGA_LANDING_KEEP;
                        work.w = work.w * SHAGA_LANDING_KEEP + SHAGA_LANDING_PULL;
                        g_shagaNode[slot]->matrix = RotationMatrix(work);
                    }
                    else
                    {
                        float heading = atan2f(g_shagaVelocity[slot].x, g_shagaVelocity[slot].y) + SHAGA_PI;
                        Matrix4 tumble;
                        tumble = RotationZ(-heading) * RotationX(SHAGA_TUMBLE_X) * RotationZ(heading);
                        g_shagaNode[slot]->matrix = g_shagaNode[slot]->matrix * tumble;
                    }
                    if (position.z < SHAGA_FALL_LEVEL)
                    {
                        char kind = g_shagaSizeClass[slot];
                        position.z = SHAGA_FALL_LEVEL;
                        g_shagaTargetB[slot] = SHAGA_PITLOST_BASE;
                        if (kind > 1)
                            g_shagaTargetB[slot] = (kind + SHAGA_PITLOST_KIND_BASE) * 10;
                        else
                            g_shagaTargetB[slot] = SHAGA_PITLOST_BASE;
                        g_shagaNode[slot]->RestartColourPlugs("anime2", g_colourPlugFrames);
                        ShagaReleaseHeld(g_shagaNode[slot]);
                    }
                    g_shagaNode[slot]->matrix.SetTranslation(position);
                    if ((previous - position).Length() < SHAGA_SLOW_DISTANCE)
                        g_shagaVelocity[slot].Copy(g_shagaVelocity[slot] * SHAGA_SLOW_FACTOR);
                    if (g_shagaCounter[slot] % SHAGA_REST_UNIT > 15 && g_shagaCounter[slot] % SHAGA_REST_UNIT < SHAGA_REST_FRAMES)
                    {
                        if ((position - g_characterModels[0].matrix.GetTranslation()).Length() < SHAGA_GRAB_DISTANCE)
                        {
                            g_shagaTargetB[slot] = SHAGA_ATTRACT;
                            g_shagaNode[slot]->RestartColourPlugs("anime2", g_colourPlugFrames);
                            ShagaReleaseHeld(g_shagaNode[slot]);
                        }
                    }
                    if (g_shagaCounter[slot] % SHAGA_REST_UNIT < SHAGA_REST_FRAMES)
                        g_shagaCounter[slot] = g_shagaCounter[slot] + 1;
                    else
                    {
                        g_shagaTargetB[slot] = SHAGA_DROPPED;
                        g_shagaNode[slot]->RestartColourPlugs("anime3", g_colourPlugFrames);
                        ShagaReleaseHeldBoth(g_shagaNode[slot]);
                    }
                }
                else if (g_shagaTargetB[slot] < 0)
                {
                    if (g_shagaCounter[slot] < SHAGA_REST_LIMIT)
                        g_shagaCounter[slot] = g_shagaCounter[slot] + SHAGA_REST_UNIT;
                    position.Copy(g_shagaNode[slot]->matrix.GetTranslation());
                    position.z += g_shagaRadius[slot];
                    identity.SetIdentity();
                    if (g_mapRootNode != 0 && g_shagaCounter[slot] < SHAGA_REST_LIMIT)
                    {
                        if (g_mapRootNode->Query(&identity, &position, g_shagaRadius[slot] + 1.0f, &work, &depth, 0, 0) != 0)
                        {
                            work.Normalized3();
                            if (!(work.z > SHAGA_STEEP_NORMAL))
                                g_shagaVelocity[slot].Copy(work * SHAGA_BOUNCE_UP);
                        }
                        else
                        {
                            position.z -= 1.0f;
                            g_shagaVelocity[slot].z = SHAGA_HOP_SPEED;
                            g_shagaVelocity[slot].y = SHAGA_HOP_SPEED;
                            g_shagaVelocity[slot].RotateZ((float)(Random() % SHAGA_HEADINGS) * SHAGA_PI /
                                                       SHAGA_DEGREES);
                        }
                    }
                    position.z -= g_shagaRadius[slot];
                    work.Copy(QuatFromMatrix(g_shagaNode[slot]->matrix));
                    work.x = work.x * SHAGA_RESTING_KEEP;
                    work.y = work.y * SHAGA_RESTING_KEEP;
                    work.z = work.z * SHAGA_RESTING_KEEP;
                    work.w = work.w * SHAGA_RESTING_KEEP + SHAGA_RESTING_PULL;
                    g_shagaNode[slot]->matrix = RotationMatrix(work);
                    g_shagaNode[slot]->matrix.SetTranslation(position);
                    if ((position - g_characterModels[0].matrix.GetTranslation()).Length() < SHAGA_GRAB_DISTANCE)
                    {
                        g_shagaTargetB[slot] = SHAGA_ATTRACT;
                        g_shagaNode[slot]->RestartColourPlugs("anime2", g_colourPlugFrames);
                        ShagaReleaseHeld(g_shagaNode[slot]);
                    }
                    if (g_shagaCounter[slot] % SHAGA_REST_UNIT < SHAGA_REST_FRAMES)
                        g_shagaCounter[slot] = g_shagaCounter[slot] + 1;
                    else if (g_shagaTargetB[slot] < 0)
                    {
                        g_shagaTargetB[slot] = SHAGA_DROPPED;
                        g_shagaNode[slot]->RestartColourPlugs("anime3", g_colourPlugFrames);
                        ShagaReleaseHeldBoth(g_shagaNode[slot]);
                    }
                }
                else if (g_shagaTargetB[slot] <= SHAGA_DONE_SOON)
                {
                    if (g_shagaTargetB[slot] == SHAGA_DONE_SOON)
                        g_shagaTargetB[slot] = 1;
                    else if (g_shagaTargetB[slot] > SHAGA_PITLOST_END)
                    {
                        if (g_shagaTargetB[slot] % 10 == 0)
                        {
                            ShagaCountMoney();
                            ShagaShowMoney();
                        }
                    }
                    else if (g_shagaTargetB[slot] == SHAGA_PITLOST_END)
                    {
                        ShagaCountMoney();
                        ShagaShowMoney();
                        g_shagaTargetB[slot] = 1;
                    }
                    else if (g_shagaTargetB[slot] > SHAGA_PAID_END)
                    {
                        if (g_shagaTargetB[slot] % 10 == 0)
                        {
                            ShagaCountMoney();
                            Sound_PlayEffect(SHAGA_PICKUP_SOUND, &(const Vec4 &)g_characterModels[0].matrix.GetTranslation(),
                                         1.0f);
                            ShagaShowMoney();
                        }
                        Vec4 away;
                        away.x = SHAGA_FAR_AWAY;
                        g_shagaNode[slot]->matrix.SetTranslation(away);
                    }
                    else if (g_shagaTargetB[slot] == SHAGA_PAID_END)
                    {
                        ShagaCountMoney();
                        Sound_PlayEffect(SHAGA_PICKUP_SOUND, &(const Vec4 &)g_characterModels[0].matrix.GetTranslation(), 1.0f);
                        ShagaShowMoney();
                        g_shagaTargetB[slot] = 1;
                    }
                    else if (g_shagaTargetB[slot] > SHAGA_HIDDEN_END)
                    {
                        if (g_shagaTargetB[slot] == SHAGA_HIDDEN_RELEASE)
                            ShagaReleaseHeldBoth(g_shagaNode[slot]);
                    }
                    else if (g_shagaTargetB[slot] == SHAGA_HIDDEN_END)
                        g_shagaTargetB[slot] = 1;
                    else if (g_shagaTargetB[slot] > SHAGA_FLY_END)
                    {
                        position.Copy(g_shagaNode[slot]->matrix.GetTranslation());
                        Vec4 target;
                        target.Copy(g_characterModels[0].matrix.GetTranslation());
                        target.z += SHAGA_LIFT_ABOVE_PLAYER;
                        position.Copy(position + (target - position) * (1.0f / ((float)g_shagaTargetB[slot] - 100.0f)));
                        work.Copy(QuatFromMatrix(g_shagaNode[slot]->matrix));
                        work.x = work.x * SHAGA_FLYING_KEEP;
                        work.y = work.y * SHAGA_FLYING_KEEP;
                        work.z = work.z * SHAGA_FLYING_KEEP;
                        work.w = work.w * SHAGA_FLYING_KEEP + SHAGA_FLYING_PULL;
                        g_shagaNode[slot]->matrix = RotationMatrix(work);
                        g_shagaNode[slot]->matrix.SetTranslation(position);
                    }
                    else if (g_shagaTargetB[slot] == SHAGA_FLY_END)
                    {
                        char kind = g_shagaSizeClass[slot];
                        if (kind > 1)
                            g_shagaTargetB[slot] = (kind + SHAGA_COLLECT_KIND_BASE) * 10;
                        else
                            g_shagaTargetB[slot] = 1;
                        position.Copy(g_shagaNode[slot]->matrix.GetTranslation());
                        Vec4 target;
                        target.Copy(g_characterModels[0].matrix.GetTranslation());
                        target.z += SHAGA_LIFT_ABOVE_PLAYER;
                        position.Copy(target);
                        work.Copy(QuatFromMatrix(g_shagaNode[slot]->matrix));
                        work.x = work.x * SHAGA_FLYING_KEEP;
                        work.y = work.y * SHAGA_FLYING_KEEP;
                        work.z = work.z * SHAGA_FLYING_KEEP;
                        work.w = work.w * SHAGA_FLYING_KEEP + SHAGA_FLYING_PULL;
                        g_shagaNode[slot]->matrix = RotationMatrix(work);
                        g_shagaNode[slot]->matrix.SetTranslation(position);
                        ShagaCountMoney();
                        Sound_PlayEffect(SHAGA_PICKUP_SOUND, &(const Vec4 &)g_characterModels[0].matrix.GetTranslation(), 1.0f);
                        ShagaShowMoney();
                    }
                }
            }
            else if (g_shagaTargetB[slot] > SHAGA_NO_COUNTER)
            {
                g_shagaNode[slot]->parent->RemoveChild(g_shagaNode[slot], 0);
                g_shagaNode[slot] = 0;
                g_shagaTargetB[slot] = 0;
            }
        }
        else
        {
            if (g_shagaTargetB[slot] != 0)
                g_shagaTargetB[slot] = 0;
        }
    }
    return 0;
}
