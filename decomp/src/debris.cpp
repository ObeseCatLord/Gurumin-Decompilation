// Debris particles: one pool of 256 slots kept as parallel arrays. Slots 0-127 take the simple pieces of
// Debris_Spawn, slots 128-255 the thrown pieces of Debris_SpawnFrom and Debris_SpawnFromCharacter. FUN_1000e340 draws
// them (rotation from `spin`, position and scale per slot).

#include "vec4.h"

#include "anim_model.h"

#define DEBRIS_SLOTS 0x100

#include "char_tables.h"
#include "char_vars.h"
#include "matrix4.h"
#include "debris.h"
#include "model_bank.h"
#include "render_vars.h"

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
    DAT_10866df8[i].Copy(DAT_10db6ee8[owner].scale);
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

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space
Matrix4 TranslationMatrix(float x, float y, float z);     // 0x101b1360
Matrix4 RotationX(float angle);                           // 0x101b1ad0
Matrix4 RotationZ(float angle);                           // 0x101b1b40

#define DEBRIS_SIMPLE_SLOTS 0x80      // slots 0-127 take the simple pieces, 128-255 the thrown ones
#define DEBRIS_LIFETIME 900           // frames a thrown piece lives (it is started at 880)
#define DEBRIS_FADE_SIMPLE 30         // frames the simple pieces fade out over, at the end of their life
#define DEBRIS_FADE_THROWN 20         // frames the thrown pieces fade out over
#define DEBRIS_SPIN_STEP 0.1f         // radians per spin unit
#define DEBRIS_TUMBLE_PER_SLOT 5      // spin units a thrown piece turns about z per slot number

// 0x1000e340 (Debris_Draw). Draws every used piece: the model of its type under a matrix of its position and its spin (the
// thrown pieces also tumble about z by a different amount per slot), the screen position computed on the way is
// unused. The model is queued with the piece's scale as colour and an opacity that fades out in the last frames of
// its life; a thrown piece with a colour sets DAT_10f655b4 to it while it is drawn. Returns 0.
int FUN_1000e340(void)
{
    for (int i = 0; i < DEBRIS_SLOTS; i++)
    {
        if (DAT_1030bdf8[i] != 0)
        {
            Vec4 screen;
            screen.Copy(Project(DAT_10da9400[i], DAT_108392d8));
            screen.x = (screen.x + 1.0f) * 320.0f;
            screen.y = (2.0f - (screen.y + 1.0f)) * 240.0f;
            Matrix4 matrix;
            if (i >= DEBRIS_SIMPLE_SLOTS)
                matrix = RotationZ((float)(i * DEBRIS_TUMBLE_PER_SLOT) * DEBRIS_SPIN_STEP) *
                         RotationX((float)(unsigned int)DAT_104465f8[i] * DEBRIS_SPIN_STEP) *
                         TranslationMatrix(DAT_10da9400[i].x, DAT_10da9400[i].y, DAT_10da9400[i].z);
            else
                matrix = RotationZ((float)(unsigned int)DAT_104465f8[i] * DEBRIS_SPIN_STEP) *
                         TranslationMatrix(DAT_10da9400[i].x, DAT_10da9400[i].y, DAT_10da9400[i].z);
            Vec4 colour = DAT_10866df8[i];
            if (i >= DEBRIS_SIMPLE_SLOTS)
                colour.w = (float)(unsigned int)(DEBRIS_LIFETIME - DAT_104469f8[i]) / (float)DEBRIS_FADE_THROWN;
            else if ((unsigned int)DAT_104469f8[i] < DEBRIS_LIFETIME - DEBRIS_FADE_SIMPLE)
                colour.w = 1.0f;
            else
                colour.w = (float)(unsigned int)(DEBRIS_LIFETIME - DAT_104469f8[i]) / (float)DEBRIS_FADE_SIMPLE;
            DAT_104d2a08[DAT_1030bdf8[i]].StartProximityPass(matrix.m, &DAT_108392d8, &DAT_108392d8, 0, 0);
            if (i >= DEBRIS_SIMPLE_SLOTS)
            {
                if (DAT_10446eb0[i] >= 0)
                    DAT_10f655b4 = DAT_10446eb0[i];
            }
            DAT_104d2a08[DAT_1030bdf8[i]].QueueDraw(&matrix, &DAT_108392d8,
                                                    DAT_10804bf8.LightDirection(0, &DAT_118e69e8, &DAT_118e69e8), -1,
                                                    &colour, 0);
            DAT_10f655b4 = 0;
        }
    }
    return 0;
}
