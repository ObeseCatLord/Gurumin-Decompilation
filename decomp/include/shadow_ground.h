// Putting a character's shadow matrix on the ground below it, as an inline helper (the body of Char_PlaceOnGround
// 0x1001c090 as the enemy behaviours have it inlined). Include after char_vars.h, map_vars.h, model_bank.h and node.h.
#ifndef GURUMIN_SHADOW_GROUND_H
#define GURUMIN_SHADOW_GROUND_H

#define SHADOW_GROUND_LIFT 10.0f          // the ground probe ends this far above the character
#define SHADOW_GROUND_DEPTH 1000.0f       // and starts this far back along the model bank's direction
#define SHADOW_NO_GROUND (-3500.0f)       // the shadow height when the probe finds nothing

// Casts down for character `index` against world collision (mask 0x4000000) and moves its shadow matrix to the hit
// point (keeping the cast's start x and y; z = SHADOW_NO_GROUND without a hit).
__forceinline void PlaceShadowOnGround(int index)
{
    Vec4 from;
    Vec4 ground;
    Vec4 top = DAT_10db6ee8[index].matrix.GetTranslation();
    from.Copy(top);
    top.z += SHADOW_GROUND_LIFT;
    Vec4 direction = *DAT_10804bf8.LightDirection(&DAT_10db6ee8[index], &DAT_118e69e8, &DAT_118e6cb8);
    direction.Normalized3();
    from.Copy(from - direction * SHADOW_GROUND_DEPTH);
    if (DAT_10255f08 != 0 && DAT_10255f08->CastSegment(0, 0x4000000, &from, &top, &ground, 0) != 0)
    {
        ground.x = from.x;
        ground.y = from.y;
        DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
        return;
    }
    ground.x = from.x;
    ground.y = from.y;
    ground.z = SHADOW_NO_GROUND;
    DAT_10db6ee8[index].shadowMatrix.SetTranslation(ground);
}

#endif
