// Scans of the character slots (EN build). Ported from the matched JP source decomp/src/actor_scan.cpp.
#include "char.h"
#include "node.h"

// Enemy_FindReusableSlot: returns the first enemy slot that has a model and is either out of the world or finished
// (health <= 0 while its phase is 2), or -1. Called when a new enemy needs a slot.
extern "C" int Enemy_FindReusableSlot(void)
{
    for (int i = ENEMY_SLOT_FIRST; i < ENEMY_SLOT_END; i++)
    {
        if (g_charKind[i] == CHAR_KIND_ENEMY_SLOT && i >= ENEMY_SLOT_FIRST)
        {
            if ((g_characterModels[i].matrix.m[MATRIX_TRANSLATION_X] < CHAR_OFFWORLD_X ||
                 (g_charRecords[i].stats[STAT_HEALTH] <= 0 && g_charPhase[i] == CHAR_PHASE_FINISHED)) &&
                g_characterModels[i].root != 0)
                return i;
        }
    }
    return -1;
}

// Enemy_ClearIdleHealth: sets the health of every live, active enemy that is still in the world and has no pending
// action (it does not share a model) to 0. Returns 1. Called through the script command table.
extern "C" int Enemy_ClearIdleHealth(void)
{
    for (int i = ENEMY_SLOT_FIRST; i < ENEMY_SLOT_END; i++)
    {
        if (g_charKind[i] == CHAR_KIND_ENEMY_SLOT && i >= ENEMY_SLOT_FIRST &&
            g_characterModels[i].matrix.m[MATRIX_TRANSLATION_X] > CHAR_INWORLD_X &&
            g_charRecords[i].stats[STAT_HEALTH] > 0 && g_charPhase[i] == 0 && g_characterModels[i].root != 0 &&
            g_charRecords[i].sharesModel == 0)
            g_charRecords[i].stats[STAT_HEALTH] = 0;
    }
    return 1;
}

// Object_IsNearPoint: 1 when the point (x, y) lies near one of the objects in slots 0x5c..0x7b whose kind byte is
// 0x5c: its 50x50 square around the point overlaps the object's 250x250 square (x/y only). The first argument is
// unused.
extern "C" int Object_IsNearPoint(int unused, const Vec4 *point)
{
    Vec4 position;
    for (int i = OBJECT_SLOT_FIRST; i < ENEMY_SLOT_END; i++)
    {
        if (g_charKind[i] == OBJECT_KIND_NEAR_POINT)
        {
            position.Copy(g_characterModels[i].matrix.GetTranslation());
            if (position.x - OBJECT_NEAR_HALF_SIZE < point->x + POINT_NEAR_HALF_SIZE &&
                position.x + OBJECT_NEAR_HALF_SIZE > point->x - POINT_NEAR_HALF_SIZE &&
                position.y - OBJECT_NEAR_HALF_SIZE < point->y + POINT_NEAR_HALF_SIZE &&
                position.y + OBJECT_NEAR_HALF_SIZE > point->y - POINT_NEAR_HALF_SIZE)
                return 1;
        }
    }
    return 0;
}

// Char_IsPartyNear: 1 when any of the five party characters other than `index` itself stands within `range` (x/y) of
// character `index`'s position, moved by `offset` when that is given. Returns 0 otherwise. Called by the party
// behaviours.
extern "C" int Char_IsPartyNear(int index, const Vec4 *offset, int range)
{
    Vec4 position;
    if (offset != 0)
        position.Copy(g_characterModels[index].matrix.GetTranslation() + *offset);
    else
        position.Copy(g_characterModels[index].matrix.GetTranslation());
    Vec4 other;
    for (int i = 0; i < PARTY_SIZE; i++)
    {
        if (g_partyCharacters[i] != index)
        {
            other.Copy(g_characterModels[g_partyCharacters[i]].matrix.GetTranslation());
            float distance = (other - position).Length2D();
            if (distance < (float)range)
                return 1;
        }
    }
    return 0;
}

// Object_SlowNearBlocker: slows `velocity` down to 5 % when it would take `position` closer (in x/y) to one of the
// objects of kind 0x14 in slots 0x5c..0x7b that is less than 400 units away. Returns 0.
extern "C" int Object_SlowNearBlocker(const Vec4 *position, Vec4 *velocity)
{
    Vec4 delta;
    for (int i = OBJECT_SLOT_FIRST; i < ENEMY_SLOT_END; i++)
    {
        if (g_charKind[i] == OBJECT_KIND_BLOCKER)
        {
            delta.Copy(g_characterModels[i].matrix.GetTranslation() - *position);
            float distance = delta.Length2D();
            delta.Copy(delta - *velocity);
            float distanceAfter = delta.Length2D();
            if (distance < BLOCKER_RANGE && distance > distanceAfter)
            {
                velocity->Copy(*velocity * BLOCKER_SLOWDOWN);
                return 0;
            }
        }
    }
    return 0;
}

// Char_IsStandingOn: 1 when character `upper` stands between 20 and 20 + `maxHeight` units above character `lower`,
// their squared distance in x/y is at most `maxDistanceSquared`, and `upper` has its idle flag set. Returns 0
// otherwise.
extern "C" int Char_IsStandingOn(int lower, int upper, int maxHeight, int maxDistanceSquared)
{
    Vec4 top;
    Vec4 bottom;
    top.Copy(g_characterModels[upper].matrix.GetTranslation());
    bottom.Copy(g_characterModels[lower].matrix.GetTranslation());
    float height = top.z - (bottom.z + STAND_MIN_HEIGHT);
    top.z = 0;
    bottom.z = 0;
    float distanceSquared = (top - bottom).LengthSquared3();
    if (height < 0.0f || height > (float)maxHeight || distanceSquared > (float)maxDistanceSquared)
        return 0;
    return g_characterModels[upper].idleMotionEnabled != 0;
}

// Colour values a respawned enemy's node tree is reset to: set A is 0xfe, set B is 0x2f, both for every node (-1 mask).
const unsigned char RESPAWN_COLOUR_A = 0xfe;
const unsigned char RESPAWN_COLOUR_B = 0x2f;
const unsigned int RESPAWN_COLOUR_MASK = 0xffffffff;
// Alpha of a respawned model: fully visible.
const float RESPAWN_ALPHA = 1.0f;

// Enemy_Respawn: respawns an enemy in the first enemy slot that is free or finished: resets the model's node
// colours, restores the slot's health, clears the busy and spawn flags, makes the model fully visible and places it
// at (x, y, z). Returns the slot, or -1 when none is available. Called by the spawn scripts.
extern "C" int Enemy_Respawn(float x, float y, float z)
{
    int i = Enemy_FindReusableSlot();
    if (i >= 0)
    {
        g_characterModels[i].root->SetColourA(RESPAWN_COLOUR_A, RESPAWN_COLOUR_MASK);
        g_characterModels[i].root->SetColourB(RESPAWN_COLOUR_B, RESPAWN_COLOUR_MASK);
        g_charRecords[i].stats[STAT_HEALTH] = g_charRecords[i].stats[STAT_MAX_HEALTH];
        g_charPhase[i] = 0;
        g_charSpawnFlag[i] = 0;
        g_characterModels[i].alpha = RESPAWN_ALPHA;
        g_characterModels[i].Teleport(x, y, z);
        return i;
    }
    return -1;
}
