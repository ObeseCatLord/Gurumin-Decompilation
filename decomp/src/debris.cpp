// Debris particles: one pool of 256 slots kept as parallel arrays. Slots 0-127 take the simple pieces of
// Debris_Spawn, slots 128-255 the thrown pieces of Debris_SpawnFrom and Debris_SpawnFromCharacter. FUN_1000e340 draws
// them (rotation from `spin`, position and scale per slot).

#include "vec4.h"

// A character record (world.h Character, base 0x10db6ed8), only the model scale used here.
struct CharacterScale
{
    char pad_0000[0x22c];
    Vec4 scale;               // 0x22c: model scale
    char pad_023c[0x249c - 0x23c];
};

#define DEBRIS_SLOTS 0x100

extern "C" int DAT_1030bdf8[DEBRIS_SLOTS];             // piece type, 0 = free slot
extern "C" Vec4 DAT_10da9400[DEBRIS_SLOTS];            // position
extern "C" Vec4 DAT_10c66ef0[DEBRIS_SLOTS];            // velocity
extern "C" Vec4 DAT_10866df8[DEBRIS_SLOTS];            // scale
extern "C" char DAT_1021b628[DEBRIS_SLOTS];            // cleared on spawn
extern "C" char DAT_104677a0[DEBRIS_SLOTS];            // cleared on spawn
extern "C" char DAT_102195f0[DEBRIS_SLOTS];            // cleared on spawn
extern "C" int DAT_104469f8[DEBRIS_SLOTS];             // frames left (0 for the simple pieces)
extern "C" int DAT_104465f8[DEBRIS_SLOTS];             // spin: rotation in tenths of a radian
extern "C" int DAT_10446eb0[DEBRIS_SLOTS];             // colour of a thrown piece
extern "C" int DAT_10467288[0x80];                     // each character's debris colour
extern "C" CharacterScale DAT_10db6ed8[0x80];
extern "C" int FUN_1013f880(void);                     // the game's random number

// 0x1000dd90 (Debris_Spawn). Starts a simple piece of type `type` at (x, y, z) in the first free slot of 0-127:
// random spin, a random sideways velocity (-5..5 each way) with 20 upwards, full scale. Returns 1, or 0 when the
// slots are all taken.
extern "C" int FUN_1000dd90(float x, float y, float z, int type)
{
    int i;
    for (i = 0; i < 0x80; i++)
    {
        if (DAT_1030bdf8[i] == 0)
            break;
    }
    if (i == 0x80)
        return 0;
    DAT_1030bdf8[i] = type;
    DAT_10da9400[i].x = x;
    DAT_10da9400[i].y = y;
    DAT_10da9400[i].z = z;
    DAT_1021b628[i] = 0;
    DAT_104677a0[i] = 0;
    int spin = FUN_1013f880() % 100;
    DAT_104469f8[i] = 0;
    DAT_102195f0[i] = 0;
    DAT_104465f8[i] = spin;
    DAT_10c66ef0[i].Set((float)(FUN_1013f880() % 6 - FUN_1013f880() % 6),
                        (float)(FUN_1013f880() % 6 - FUN_1013f880() % 6), 20.0f, 0);
    DAT_10866df8[i].x = 1.0f;
    DAT_10866df8[i].y = 1.0f;
    DAT_10866df8[i].z = 1.0f;
    return 1;
}

// 0x1000df10 (Debris_SpawnFromCharacter). Starts a thrown piece at (x, y, z) in the first free slot of 128-255:
// type `type` or `type + 1` at random, 880 frames to live, random spin, thrown along `direction` at speed 25 turned
// a little about each axis (up to 5 * 0.28 radians) and 3 upwards, with the colour and model scale of character
// `owner`. Like Debris_SpawnFrom, the result of Normalized3 is dropped, so the direction keeps its length before it
// is scaled by 25. Returns 1, or 0 when the slots are all taken.
extern "C" int FUN_1000df10(float x, float y, float z, const Vec4 *direction, int type, int owner)
{
    int i;
    for (i = 0x80; i < DEBRIS_SLOTS; i++)
    {
        if (DAT_1030bdf8[i] == 0)
            break;
    }
    if (i == DEBRIS_SLOTS)
        return 0;
    DAT_1030bdf8[i] = FUN_1013f880() % 2 + type;
    DAT_10da9400[i].x = x;
    DAT_10da9400[i].y = y;
    DAT_10da9400[i].z = z;
    DAT_1021b628[i] = 0;
    DAT_104677a0[i] = 0;
    int spin = FUN_1013f880() % 100;
    DAT_102195f0[i] = 0;
    DAT_104469f8[i] = 880;
    DAT_104465f8[i] = spin;
    DAT_10c66ef0[i].Copy(*direction);
    DAT_10c66ef0[i].Normalized3();
    float amount = 25.0f;
    DAT_10c66ef0[i].Copy(DAT_10c66ef0[i] * amount);
    amount = (float)(FUN_1013f880() % 10 - 5);
    DAT_10c66ef0[i].RotateZ(amount * 0.28f);
    amount = (float)(FUN_1013f880() % 10 - 5);
    DAT_10c66ef0[i].RotateX(amount * 0.28f);
    amount = (float)(FUN_1013f880() % 10 - 5);
    DAT_10c66ef0[i].RotateY(amount * 0.28f);
    DAT_10c66ef0[i].z += 3.0f;
    DAT_10446eb0[i] = DAT_10467288[owner];
    DAT_10866df8[i].Copy(DAT_10db6ed8[owner].scale);
    return 1;
}

// 0x1000e130 (Debris_SpawnFrom). Like Debris_SpawnFromCharacter with type `type`, colour `colour`, smaller turns (up
// to 5 * 0.1528 radians) and full scale. Returns 1, or 0 when the slots are all taken.
extern "C" int FUN_1000e130(float x, float y, float z, const Vec4 *direction, int type, int colour)
{
    int i;
    for (i = 0x80; i < DEBRIS_SLOTS; i++)
    {
        if (DAT_1030bdf8[i] == 0)
            break;
    }
    if (i == DEBRIS_SLOTS)
        return 0;
    DAT_1030bdf8[i] = type;
    DAT_10da9400[i].x = x;
    DAT_10da9400[i].y = y;
    DAT_10da9400[i].z = z;
    DAT_1021b628[i] = 0;
    DAT_104677a0[i] = 0;
    int spin = FUN_1013f880() % 100;
    DAT_102195f0[i] = 0;
    DAT_104469f8[i] = 880;
    DAT_104465f8[i] = spin;
    DAT_10c66ef0[i].Copy(*direction);
    DAT_10c66ef0[i].Normalized3();
    float amount = 25.0f;
    DAT_10c66ef0[i].Copy(DAT_10c66ef0[i] * amount);
    amount = (float)(FUN_1013f880() % 10 - 5);
    DAT_10c66ef0[i].RotateZ(amount * 0.1528f);
    amount = (float)(FUN_1013f880() % 10 - 5);
    DAT_10c66ef0[i].RotateX(amount * 0.1528f);
    amount = (float)(FUN_1013f880() % 10 - 5);
    DAT_10c66ef0[i].RotateY(amount * 0.1528f);
    DAT_10c66ef0[i].z += 3.0f;
    DAT_10446eb0[i] = colour;
    amount = 1.0f;
    DAT_10866df8[i].Fill(amount);
    return 1;
}
