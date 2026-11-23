// Putting a character on the ground below it. The character records are viewed from their model part at
// 0x10db6ee8 (like char_move.cpp).

#include "node.h"
#include "map_vars.h"
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "model_bank.h"

#include "char_tables.h"
#include "char_vars.h"

#include "char_state.h"

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
            Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
            direction.Normalized3();
            float distance = 1000.0f;
            from.Copy(from - direction * distance);
            int hit = DAT_10255f08->CastSegment(0, 0x4000000, &from, &top, &ground, 0);
            ground.y = from.y;
            ground.x = from.x;
            if (hit != 0)
            {
                DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
                return 0;
            }
            ground.z = -3500.0f;
            DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
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
        Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
        direction.Normalized3();
        float distance = 1000.0f;
        from.Copy(from - direction * distance);
        if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment(0, 0x4000000, &from, &top, &ground, 0) != 0)
        {
            ground.x = from.x;
            ground.y = from.y;
            DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
            return 0;
        }
        ground.x = from.x;
        ground.y = from.y;
        ground.z = -3500.0f;
        DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
    }
    return 0;
}
