// Putting a character on the ground below it. The character records are viewed from their model part at
// 0x10db6ee8 (like char_move.cpp).

#include "vec4.h"

#include "matrix4.h"

struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    char pad_0000[0x267];
    char m_0267;              // 0x267: 1 while an NPC idles (char_move.cpp)
    char pad_0268[0x20c0];
    Matrix4 matrix;        // 0x2328: world matrix
    char pad_2368[0xcc];
    Matrix4 m_2434;        // 0x2434: a second matrix that receives the ground position
    char pad_2474[0x28];
};

// The model bank at 0x10804bf8 (model_bank.cpp).
struct ModelBank
{
    // 0x10164c10: a direction for model `model` from the player's position and velocity (0 when the bank is off).
    const Vec4 *M_10164c10(CharRecord *model, Vec4 *position, Vec4 *velocity);
};

struct CollisionWorld
{
    // 0x101aaa20 (collide_query.cpp): casts the segment `from`-`to` against the collision of the nodes called `name`
    // (0 = all) with the given mask; the nearest hit point goes to `hit`. Returns 1 on a hit.
    int CastSegment(const char *name, unsigned mask, Vec4 *from, Vec4 *to, Vec4 *hit, float *t);
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" ModelBank DAT_10804bf8;
extern "C" CollisionWorld *DAT_10255f08;
extern "C" Vec4 DAT_118e69e8;            // player position
extern "C" Vec4 DAT_118e6cb8;            // player velocity
extern "C" int DAT_102022b8;             // non-zero = always place characters on the ground
extern "C" char DAT_10255a88[0x80];

// Per-character script state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = not started yet
    char pad_0004[0x3c];
};

extern "C" NpcState DAT_1020b430[0x80];

// 0x10011dd0 (Npc_StandOnGround). Keeps NPC `index` idle in motion 2. On its first call, when DAT_102022b8 is set,
// it is put on the ground like Char_PlaceOnGround does (without the check for a loaded world; the start x and y
// are kept whether or not something is hit). Returns 0.
extern "C" int FUN_10011dd0(int index)
{
    DAT_10255a88[index] = 0;
    DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
    DAT_10db6ee8[index].m_0267 = 1;
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].state = 1;
        if (DAT_102022b8 != 0)
        {
            Vec4 from;
            Vec4 ground;
            Vec4 top = DAT_10db6ee8[index].matrix.GetTranslation();
            from.Copy(top);
            top.z += 10.0f;
            Vec4 direction = *DAT_10804bf8.M_10164c10(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
            direction.Normalized3();
            float distance = 1000.0f;
            from.Copy(from - direction * distance);
            int hit = DAT_10255f08->CastSegment(0, 0x4000000, &from, &top, &ground, 0);
            ground.y = from.y;
            ground.x = from.x;
            if (hit != 0)
            {
                DAT_10db6ee8[index].m_2434.SetTranslation(ground);
                return 0;
            }
            ground.z = -3500.0f;
            DAT_10db6ee8[index].m_2434.SetTranslation(ground);
        }
    }
    return 0;
}

// 0x1001c090 (Char_PlaceOnGround). When DAT_102022b8 or `force` is set, casts from 1000 units back along the model
// bank's direction for character `index` to 10 units above it against world collision (mask 0x4000000) and moves
// the character's second matrix to the hit point, keeping the cast's start x and y; without a hit it goes to
// z = -3500. The direction is not normalised (the result of Normalized3 is dropped). Returns 0.
extern "C" int FUN_1001c090(int index, int force)
{
    if (DAT_102022b8 != 0 || force != 0)
    {
        Vec4 from;
        Vec4 ground;
        Vec4 top = DAT_10db6ee8[index].matrix.GetTranslation();
        from.Copy(top);
        top.z += 10.0f;
        Vec4 direction = *DAT_10804bf8.M_10164c10(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
        direction.Normalized3();
        float distance = 1000.0f;
        from.Copy(from - direction * distance);
        if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment(0, 0x4000000, &from, &top, &ground, 0) != 0)
        {
            ground.x = from.x;
            ground.y = from.y;
            DAT_10db6ee8[index].m_2434.SetTranslation(ground);
            return 0;
        }
        ground.x = from.x;
        ground.y = from.y;
        ground.z = -3500.0f;
        DAT_10db6ee8[index].m_2434.SetTranslation(ground);
    }
    return 0;
}
