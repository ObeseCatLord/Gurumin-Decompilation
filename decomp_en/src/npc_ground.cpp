// An NPC that stands on the ground (EN build). Ported from the JP source decomp/src/char_ground.cpp (Npc_StandOnGround).
#include "camera.h"
#include "char.h"
#include "collide_regions.h"
#include "model_bank.h"
#include "node.h"

// Motion layer, blend and frame range of the idle motion, and its playback speed.
const int NPC_IDLE_LAYER = 2;
const int NPC_IDLE_FIRST_FRAME = 0;
const int NPC_IDLE_LAST_FRAME = 5;
const float NPC_IDLE_SPEED = 1.0f;
// Behaviour step of an NPC that has started standing.
const int NPC_STATE_STANDING = 1;
// Distance the ground cast starts behind the NPC against the light direction, and the height of its end above the NPC.
const float STAND_CAST_REACH = 1000.0f;
const float STAND_CAST_TOP = 10.0f;
// Height of the shadow when the cast hits nothing.
const float STAND_NONE_Z = -3500.0f;

// Npc_StandOnGround: keeps NPC `index` idle in motion 2. On its first call, when g_alwaysPlaceOnGround is set, its shadow
// matrix is put on the ground below it like Char_PlaceOnGround does (without the check for a loaded world; the cast's x and
// y are kept whether or not something is hit). Returns 0. Called by the character dispatch for NPC kinds.
extern "C" int Npc_StandOnGround(int index)
{
    g_charTalkPending[index] = 0;
    g_characterModels[index].PlayMotion(NPC_IDLE_LAYER, NPC_IDLE_FIRST_FRAME, NPC_IDLE_LAST_FRAME, NPC_IDLE_SPEED);
    g_characterModels[index].idling = 1;
    if (g_charState[index].state == 0)
    {
        g_charState[index].state = NPC_STATE_STANDING;
        if (g_alwaysPlaceOnGround != 0)
        {
            Vec4 from;
            Vec4 ground;
            Vec4 top = g_characterModels[index].matrix.GetTranslation();
            from.Copy(top);
            top.z += STAND_CAST_TOP;
            Vec4 direction = *(const Vec4 *)g_modelBank.LightDirection(&g_characterModels[index], &g_cameraPositionNow, &g_cameraTargetNow);
            direction.Normalized3();
            float distance = STAND_CAST_REACH;
            from.Copy(from - direction * distance);
            int hit = g_mapRootNode->CastSegment2(0, MAP_COLLISION_KIND, &from, &top, &ground, 0);
            ground.x = from.x;
            ground.y = from.y;
            if (hit != 0)
            {
                g_characterModels[index].shadowMatrix.SetTranslation(ground);
                return 0;
            }
            ground.z = STAND_NONE_Z;
            g_characterModels[index].shadowMatrix.SetTranslation(ground);
        }
    }
    return 0;
}
