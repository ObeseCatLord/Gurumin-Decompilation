// Placing a character's shadow on the ground (EN build). Ported from the matched JP source decomp/src/char_ground.cpp.
#include "camera.h"
#include "char.h"
#include "collide_regions.h"
#include "model_bank.h"
#include "node.h"

// Distance the cast starts behind the character, against the light direction.
const float GROUND_CAST_REACH = 1000.0f;

// Height above the character's position the cast ends at.
const float GROUND_CAST_TOP = 10.0f;

// Height the shadow is lifted above the hit point.
const float GROUND_HIT_LIFT = 10.0f;

// Height the shadow sits at when the cast hits nothing.
const float GROUND_NONE_Z = -3500.0f;

// Char_PlaceOnGround: when g_alwaysPlaceOnGround or `force` is set, casts from GROUND_CAST_REACH units back along the
// light direction of character `index` to just above it against the world collision and moves the character's shadow
// matrix to just above the hit point (keeping the cast's x and y); without a hit it goes to GROUND_NONE_Z. Returns 0.
extern "C" int Char_PlaceOnGround(int index, int force)
{
    if (g_alwaysPlaceOnGround != 0 || force != 0)
    {
        Vec4 from;
        Vec4 ground;
        Vec4 top = g_characterModels[index].matrix.GetTranslation();
        from.Copy(top);
        top.z += GROUND_CAST_TOP;
        Vec4 direction = *(const Vec4 *)g_modelBank.LightDirection(&g_characterModels[index], &g_cameraPositionNow, &g_cameraTargetNow);
        direction.Normalized3();
        float distance = GROUND_CAST_REACH;
        from.Copy(from - direction * distance);
        if (g_worldRoot != 0 && g_worldRoot->CastSegment2(0, MAP_COLLISION_KIND, &from, &top, &ground, 0) != 0)
        {
            ground.x = from.x;
            ground.y = from.y;
            ground.z += GROUND_HIT_LIFT;
        }
        else
        {
            ground.x = from.x;
            ground.y = from.y;
            ground.z = GROUND_NONE_Z;
        }
        g_characterModels[index].shadowMatrix.SetTranslation(ground);
    }
    return 0;
}
