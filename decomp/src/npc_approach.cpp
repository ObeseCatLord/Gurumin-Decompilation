// An NPC walking up to the player: on the first frame it starts walking and picks how near it stops; then it keeps
// turning towards the player until it is near enough (or released by its code), when its state is reset.

#include <math.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    float Length();                              // 0x101ac4e0: length of x/y/z
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
};

struct CharMatrix
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

struct CharModel
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0
};

// A character record (world.h Character), only the fields used here.
struct Character
{
    char pad_0000[0x10];
    CharModel model;              // 0x10
    char pad_0011[0x2327];
    CharMatrix matrix;            // 0x2338: world matrix (translation at 0x2368)
    char pad_2378[0x114];
    float heading;                // 0x248c: facing angle in radians
    float targetHeading;          // 0x2490: angle the character turns towards
    char pad_2494[8];
};

// Field 0x24a8 of each character record (it lies past the 0x249c bytes world.h gives a record, so it is declared
// on its own with the record stride).
struct CharMoveLock
{
    int locked;                   // 0 = the character may be moved by the game
    char pad_0004[0x2498];
};

// CharState (world.h) as the approach uses it.
struct ApproachState
{
    int started;                  // 0x00: 1 once the approach has begun
    int pad_0004;
    int code;                     // 0x08: stop distance / 10 (% 100) and release flag (/ 100)
    int stopDistance;             // 0x0c
    char pad_0010[0x28];
    int releasable;               // 0x38: non-zero = the code's release flag ends the approach
    int pad_003c;
};

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

extern "C" Character DAT_10db6ed8[0x80];
extern "C" CharMoveLock DAT_10db9380[0x80];
extern "C" ApproachState DAT_1020b430[0x80];
extern "C" void FUN_10043cc0(int slot);         // Char_ResetState

// 0x10045ed0 (Npc_ApproachPlayer). Runs one frame of the approach of character `slot`: the first frame unlocks it,
// sets the stop distance to 10 * (code % 100) and starts the walk motion. Later frames keep walking, turn it
// towards the player and reset its state once it is nearer than the stop distance (heights counting double) or,
// when releasable, the code's release flag is set. Returns 0.
extern "C" int FUN_10045ed0(int slot)
{
    Vec4 offset;
    offset = DAT_10db6ed8[0].matrix.GetTranslation() - DAT_10db6ed8[slot].matrix.GetTranslation();
    offset.z *= 2.0f;
    float distance = offset.Length();
    if (DAT_1020b430[slot].started == 0)
    {
        DAT_1020b430[slot].started = 1;
        DAT_10db9380[slot].locked = 0;
        DAT_1020b430[slot].stopDistance = DAT_1020b430[slot].code % 100 * 10;
        DAT_10db6ed8[slot].model.PlayMotion(2, 0, 5, 1.0f);
        return 0;
    }
    float turn = DAT_10db6ed8[slot].heading - DAT_10db6ed8[slot].targetHeading;
    while (turn > 3.1415f)
        turn -= 6.283f;
    while (turn < -3.1415f)
        turn += 6.283f;
    DAT_10db6ed8[slot].model.PlayMotion(2, 0, 5, 1.0f);
    DAT_10db6ed8[slot].targetHeading =
        Atan2(DAT_10db6ed8[0].matrix.m[12] - DAT_10db6ed8[slot].matrix.m[12],
              DAT_10db6ed8[0].matrix.m[13] - DAT_10db6ed8[slot].matrix.m[13]) +
        3.1415f;
    if (distance < (float)DAT_1020b430[slot].stopDistance ||
        DAT_1020b430[slot].releasable != 0 && DAT_1020b430[slot].code / 100 != 0)
        FUN_10043cc0(slot);
    return 0;
}
