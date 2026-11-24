// Debris pieces (EN build). Ported from the matched JP source decomp/src/debris.cpp.
#include "debris.h"
#include "engine.h"
#include "char.h"

// Debris_Spawn: starts a simple piece of type `type` at (x, y, z) in the first free slot of the simple slots: random
// spin, a random sideways velocity (-5..5 each way) with 20 upwards, full scale. Returns 1, or 0 when the slots are
// all taken. Called by the map scripts and the break effects.
extern "C" int Debris_Spawn(float x, float y, float z, int type)
{
    int i;
    for (i = 0; i < DEBRIS_SIMPLE_SLOTS; i++)
    {
        if (g_debrisType[i] == 0)
            break;
    }
    if (i == DEBRIS_SIMPLE_SLOTS)
        return 0;
    g_debrisType[i] = type;
    g_debrisPosition[i].x = x;
    g_debrisPosition[i].y = y;
    g_debrisPosition[i].z = z;
    g_debrisSettled[i] = 0;
    g_debrisBounces[i] = 0;
    int spin = Random() % DEBRIS_SPIN_RANGE;
    g_debrisLife[i] = 0;
    g_debrisSpin[i] = spin;
    g_debrisHeld[i] = 0;
    g_debrisVelocity[i].Set((float)(Random() % DEBRIS_SIDEWAYS_RANGE - Random() % DEBRIS_SIDEWAYS_RANGE),
                            (float)(Random() % DEBRIS_SIDEWAYS_RANGE - Random() % DEBRIS_SIDEWAYS_RANGE),
                            DEBRIS_UPWARD_SPEED, 0);
    g_debrisScale[i].x = 1.0f;
    g_debrisScale[i].y = 1.0f;
    g_debrisScale[i].z = 1.0f;
    return 1;
}

// Thrown pieces live in slots DEBRIS_SIMPLE_SLOTS..DEBRIS_SLOTS - 1 and stay for this many frames.
const int DEBRIS_THROWN_LIFE = 880;
// Speed a thrown piece starts at and its extra upward speed.
const float DEBRIS_THROW_SPEED = 25.0f;
const float DEBRIS_THROW_LIFT = 3.0f;
// Range of each random turn of the throw direction (-5..4 steps) and the angle of a step for the two spawn functions.
const int DEBRIS_TURN_RANGE = 10;
const int DEBRIS_TURN_OFFSET = 5;
const float DEBRIS_TURN_STEP_CHARACTER = 0.28f;
const float DEBRIS_TURN_STEP_PLAIN = 0.1528f;

// Debris_SpawnFromCharacter: starts a thrown piece at (x, y, z) in the first free thrown slot: type `type` or
// `type + 1` at random, 880 frames to live, random spin, thrown along `direction` at speed 25 turned a little about
// each axis and 3 upwards, with the colour and model scale of character `owner`. The result of Normalized3 is
// dropped, so the direction keeps its length before it is scaled by 25. Returns 1, or 0 when the slots are all taken.
extern "C" int Debris_SpawnFromCharacter(float x, float y, float z, const Vec4 *direction, int type, int owner)
{
    int i;
    for (i = DEBRIS_SIMPLE_SLOTS; i < DEBRIS_SLOTS; i++)
    {
        if (g_debrisType[i] == 0)
            break;
    }
    if (i == DEBRIS_SLOTS)
        return 0;
    g_debrisType[i] = Random() % 2 + type;
    g_debrisPosition[i].x = x;
    g_debrisPosition[i].y = y;
    g_debrisPosition[i].z = z;
    g_debrisSettled[i] = 0;
    g_debrisBounces[i] = 0;
    int spin = Random() % DEBRIS_SPIN_RANGE;
    g_debrisLife[i] = DEBRIS_THROWN_LIFE;
    g_debrisSpin[i] = spin;
    g_debrisHeld[i] = 0;
    g_debrisVelocity[i].Copy(*direction);
    g_debrisVelocity[i].Normalized3();
    float amount = DEBRIS_THROW_SPEED;
    g_debrisVelocity[i].Copy(g_debrisVelocity[i] * amount);
    g_debrisVelocity[i].RotateZ((float)(Random() % DEBRIS_TURN_RANGE - DEBRIS_TURN_OFFSET) * DEBRIS_TURN_STEP_CHARACTER);
    g_debrisVelocity[i].RotateX((float)(Random() % DEBRIS_TURN_RANGE - DEBRIS_TURN_OFFSET) * DEBRIS_TURN_STEP_CHARACTER);
    g_debrisVelocity[i].RotateY((float)(Random() % DEBRIS_TURN_RANGE - DEBRIS_TURN_OFFSET) * DEBRIS_TURN_STEP_CHARACTER);
    g_debrisVelocity[i].z += DEBRIS_THROW_LIFT;
    g_debrisColour[i] = g_characterColour[owner];
    g_debrisScale[i].Copy(g_characterModels[owner].scale);
    return 1;
}

// Debris_SpawnFrom: like Debris_SpawnFromCharacter with type `type`, colour `colour`, smaller turns (up to
// 5 * 0.1528 radians) and full scale. Returns 1, or 0 when the slots are all taken.
extern "C" int Debris_SpawnFrom(float x, float y, float z, const Vec4 *direction, int type, int colour)
{
    int i;
    for (i = DEBRIS_SIMPLE_SLOTS; i < DEBRIS_SLOTS; i++)
    {
        if (g_debrisType[i] == 0)
            break;
    }
    if (i == DEBRIS_SLOTS)
        return 0;
    g_debrisType[i] = type;
    g_debrisPosition[i].x = x;
    g_debrisPosition[i].y = y;
    g_debrisPosition[i].z = z;
    g_debrisSettled[i] = 0;
    g_debrisBounces[i] = 0;
    int spin = Random() % DEBRIS_SPIN_RANGE;
    g_debrisLife[i] = DEBRIS_THROWN_LIFE;
    g_debrisSpin[i] = spin;
    g_debrisHeld[i] = 0;
    g_debrisVelocity[i].Copy(*direction);
    g_debrisVelocity[i].Normalized3();
    float amount = DEBRIS_THROW_SPEED;
    g_debrisVelocity[i].Copy(g_debrisVelocity[i] * amount);
    g_debrisVelocity[i].RotateZ((float)(Random() % DEBRIS_TURN_RANGE - DEBRIS_TURN_OFFSET) * DEBRIS_TURN_STEP_PLAIN);
    g_debrisVelocity[i].RotateX((float)(Random() % DEBRIS_TURN_RANGE - DEBRIS_TURN_OFFSET) * DEBRIS_TURN_STEP_PLAIN);
    g_debrisVelocity[i].RotateY((float)(Random() % DEBRIS_TURN_RANGE - DEBRIS_TURN_OFFSET) * DEBRIS_TURN_STEP_PLAIN);
    g_debrisVelocity[i].z += DEBRIS_THROW_LIFT;
    g_debrisColour[i] = colour;
    amount = 1.0f;
    g_debrisScale[i].Fill(amount);
    return 1;
}
