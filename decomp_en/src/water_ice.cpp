// Freezing of the water of a map (EN build). Ported from the matched JP source decomp/src/water_ice.cpp.
#include "anim_model.h"
#include "char.h"
#include "map_view.h"
#include "node.h"
#include "water_ice.h"

// Node flags the ice and water nodes are switched with.
const unsigned int NODE_FLAG_COLLISION_ON = 0x4000000;   // the node takes part in the map collision
const unsigned int NODE_FLAG_ICE_SOLID = 2;              // the ice node is solid

// Water_SetFrozen: freezes (`frozen` != 0) or thaws the water of the current map, when the map has all three water
// nodes. Freezing lifts the player onto the ice when below it and switches the node flags so the ice is solid; thawing
// switches them back. Returns 0. Called by the freeze and thaw spells.
extern "C" int Water_SetFrozen(int frozen)
{
    if (g_mapNode != 0 && g_waterNode != 0 && g_iceNode != 0)
    {
        if (frozen != 0)
        {
            if (g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Z] < g_mapBottomZ)
                g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Z] = g_mapBottomZ;
            g_waterFrozen = 1;
            g_mapNode->SetFlags(NODE_FLAG_HIDDEN);
            g_iceNode->ClearFlags(NODE_FLAG_HIDDEN);
            g_iceNode->SetFlags(NODE_FLAG_HIDDEN);
            g_iceNode->ClearFlags(NODE_FLAG_HIDDEN);
            g_iceNode->children[0]->ClearFlags(NODE_FLAG_HIDDEN);
            g_waterNode->SetFlags(NODE_FLAG_COLLISION_ON | NODE_FLAG_KEEP_MESH);
            g_iceNode->SetFlags(NODE_FLAG_ICE_SOLID);
            return 0;
        }
        g_waterFrozen = 0;
        g_iceNode->SetFlags(NODE_FLAG_HIDDEN);
        g_waterNode->ClearFlags(NODE_FLAG_KEEP_MESH);
        g_mapNode->ClearFlags(NODE_FLAG_HIDDEN);
    }
    return 0;
}
