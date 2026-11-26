// Heading probes of a character and the enemy returning-home test (EN build). The JP twins have no matched source;
// ported from the JP drafts (decomp/src/collide_query.cpp, enemy_moves.cpp) and the EN bytes.
#include <math.h>

#include "anim_model.h"
#include "char_shots.h"
#include "collide_query.h"
#include "enemy_home.h"
#include "node.h"
#include "visibility.h"

// Start height of the home probe above the enemy's feet, and the largest angle (150 degrees in radians) its home may
// lie away from the wanted direction.
const float HOME_PROBE_LIFT = 10.0f;
const float HOME_MAX_TURN = 2.61799f;

// Char_ProbeDirection: see the declaration in collide_query.h. Called by the enemy and character movement code.
extern "C" int Char_ProbeDirection(int index, float angle, Vec4 *ground, float depth, int fromHead, int testOnly,
                                   const Vec4 *offset, const char *name)
{
    Vec4 from;
    Vec4 to;
    Vec4 unused;
    Vec4 hit;
    float zero = 0;
    hit.Fill(zero);
    hit.x = PROBE_NO_HIT;
    if (fromHead != 0)
    {
        Node *head = g_characterModels[index].root->Find("Bip01 Head", 0);
        if (head != 0)
            from.Copy(head->worldMatrix.GetTranslation());
        else
            from.Copy(g_characterModels[index].matrix.GetTranslation());
    }
    else
    {
        from.x = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        from.y = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
        from.z = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z];
        from.w = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_W];
    }
    if (offset != 0)
        from.Copy(from + *offset);
    to.Fill(0);
    to.y -= depth;
    to.RotateZ(angle);
    to.Copy(to + from);
    if (testOnly != 0)
    {
        if (g_mapRootNode != 0 && g_mapRootNode->CastRay(name, PROBE_RAY_FLAGS, COLLISION_MASK_WORLD, &to, &from, &hit, 0) != 0)
            return 1;
        return 0;
    }
    if (g_mapRootNode != 0 && g_mapRootNode->CastRay(name, PROBE_RAY_FLAGS, COLLISION_MASK_WORLD, &to, &from, &hit, 0) != 0)
        (hit - from).Length();   // the original measures the hit distance and drops it
    ground->Copy(hit);
    if (hit.x != PROBE_NO_HIT)
        return 1;
    return 0;
}

// Char_ProbeAhead: see the declaration in collide_query.h. Called by the enemy movement code (Enemy_CanReturnHome).
extern "C" int Char_ProbeAhead(int index, Vec4 *ground, float depth, int fromHead, int testOnly, const Vec4 *offset)
{
    Vec4 from;
    Vec4 to;
    Vec4 unused;
    Vec4 hit;
    float zero = 0;
    hit.Fill(zero);
    hit.x = PROBE_NO_HIT;
    if (fromHead != 0)
    {
        Node *head = g_characterModels[index].root->Find("Bip01 Head", 0);
        if (head != 0)
            from.Copy(head->worldMatrix.GetTranslation());
        else
            from.Copy(g_characterModels[index].matrix.GetTranslation());
    }
    else
    {
        from.x = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        from.y = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
        from.z = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z];
        from.w = g_characterModels[index].matrix.m[MATRIX_TRANSLATION_W];
    }
    if (offset != 0)
        from.Copy(from + *offset);
    to.Fill(0);
    to.y -= depth;
    to.RotateZ(-g_characterModels[index].heading);
    to.Copy(to + from);
    if (testOnly != 0)
    {
        if (g_mapRootNode != 0 && g_mapRootNode->CastRay(0, PROBE_RAY_FLAGS, COLLISION_MASK_WORLD, &to, &from, &hit, 0) != 0)
            return 1;
        return 0;
    }
    if (g_mapRootNode != 0 && g_mapRootNode->CastRay(0, PROBE_RAY_FLAGS, COLLISION_MASK_WORLD, &to, &from, &hit, 0) != 0)
        (hit - from).Length();   // the original measures the hit distance and drops it
    ground->Copy(hit);
    if (hit.x != PROBE_NO_HIT)
        return 1;
    return 0;
}

// Enemy_CanReturnHome: see the declaration in enemy_home.h. Called by the enemy roaming behaviours.
extern "C" int Enemy_CanReturnHome(int index, float distance, float minimumHome, float dirX, float dirY, int unused6,
                                   int unused7, float minimum)
{
    if (distance < minimum)
        return 1;
    if (distance < minimumHome)
        return 1;
    Vec4 offset;
    offset.Fill(0);
    offset.z += HOME_PROBE_LIFT;
    float homeDistance = (g_shotSlots[index].position - g_characterModels[index].matrix.GetTranslation()).Length2D();
    if (Char_ProbeAhead(index, 0, homeDistance, 0, 1, &offset) == 0)
        return 0;
    Vec4 home;
    home.Copy(g_shotSlots[index].position - g_characterModels[index].matrix.GetTranslation());
    float wanted = (float)atan2(dirX, dirY);
    float toHome = (float)atan2(home.x, home.y);
    if ((float)fabs(toHome - wanted) < HOME_MAX_TURN)
        return 1;
    return 0;
}
