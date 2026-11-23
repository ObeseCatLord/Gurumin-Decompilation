// Balloon objects (slot kind 'B' in character slots 0x5c..0x7b) react to a blast: each one turns to face the
// blast and is pushed by its strength; a close blast bursts it.

#include <math.h>
#include <stdlib.h>
#include "vec4.h"

#include "matrix4.h"

Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360

#include "node.h"

// A character record (world.h Character) as a balloon uses it.
struct Character
{
    char pad_0000[0x284];
    int burst;                    // 0x284: 1 once the balloon has burst
    int angle;                    // 0x288: direction of the blast (in 1/1000 radians)
    int push;                     // 0x28c: strength of the push (a multiple of 13, plus 3)
    char pad_0290[0x206c];
    Node *root;               // 0x22fc: root node of the model
    char pad_2300[0x38];
    Matrix4 matrix;               // 0x2338: world matrix
    char pad_2378[0x124];
};

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

extern "C" Character DAT_10db6ed8[0x80];
extern "C" char DAT_1021f3c0[0x80];              // kind of each character slot ('B' = balloon)
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);  // Sound_PlayEffect

// 0x100543a0 (Balloon_Blast). Lets every balloon that has not burst react to a blast at `blast`: it gets the
// direction of the blast and a push of a quarter of (600 - distance), rounded down to a multiple of 13 plus 3.
// When that quarter is over 50 the balloon bursts: its model is moved far away (z -10000) and sound 0x2e5 plays.
// Returns 0.
extern "C" int FUN_100543a0(Vec4 blast)
{
    for (int slot = 0x5c; slot < 0x80; slot++)
    {
        if (DAT_1021f3c0[slot] == 'B' && DAT_10db6ed8[slot].burst == 0)
        {
            Vec4 position;
            position.Copy(DAT_10db6ed8[slot].matrix.GetTranslation());
            float angle = Atan2(position.x - blast.x, position.y - blast.y);
            DAT_10db6ed8[slot].angle = (int)(angle * 1000.0f);
            int strength = (int)(__max(600.0f - (position - blast).Length2D(), 0.0f) * 0.25f);
            DAT_10db6ed8[slot].push = strength / 13 * 13 + 3;
            if (strength > 50)
            {
                DAT_10db6ed8[slot].burst = 1;
                DAT_10db6ed8[slot].root->children[0]->matrix = TranslationMatrix(0, 0, -10000.0f);
                FUN_1003ad20(0x2e5, &position, 1.0f);
            }
        }
    }
    return 0;
}
