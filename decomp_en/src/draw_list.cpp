// Drawing of the queued nodes (EN build). Ported from the matched JP source decomp/src/draw_list.cpp.
#include "map_view.h"
#include "node.h"
#include "node_data.h"
#include "render_queue.h"
#include "slot_table.h"
#include "texture_mgr.h"

const int MESH_FULL_RANGE_END = 0x400;           // SetRange end for a mesh with a single frame
const int NODE_FLAG_NO_DEPTH_WRITE = 0x800000;   // Node::flags: drawn with depth writes off
const int DRAW_MODE_MAP = 2;                     // Node::drawMode: drawn with its own light set and view
const int DRAW_MODE_LIT_PARAM = 4;               // mode passed to DrawLit for a map pass node
const int RENDER_MAP_PASS = 10;                  // Render_Command code that switches the map pass shading (value 2 = on)

// Draws every queued node of pass `flag` with the view `view`: uploads its bones when it is skinned, limits the mesh
// to the frame shown, sets its colour and parameters and draws it (a map pass node uses its own light set and view
// matrix, with the map pass flag changed for the call). The draw colour and fade are restored afterwards.
// Returns 0. Called by the overhead map pass (Map_DrawOverhead).
extern "C" int DrawList_DrawPass(DrawList *list, Matrix4 *view, Vec4 *eye, int flag)
{
    Vec4 savedColour = g_drawColour;
    float savedFade = g_drawFade;
    for (int i = 0; i < g_renderQueueCount; i++)
    {
        if (g_renderQueuePass[i] != flag)
            continue;
        NodeSkin *skin = g_renderQueueNodes[i]->skin;
        if (skin != 0 && skin->boneCount != 0)
            g_renderQueueNodes[i]->mesh->UploadBones(skin->skinMatrices, skin->boneCount, &g_renderQueueNodes[i]->worldMatrix, (int)view);
        Node *node = g_renderQueueNodes[i];
        if (node->animated != 0)
        {
            int frame = node->frame;
            if (frame >= node->frameCount)
                frame = node->frameCount - 1;
            node->mesh->SetRange(node->frameSize * frame, (frame + 1) * node->frameSize);
        }
        else
            node->mesh->SetRange(0, MESH_FULL_RANGE_END);
        if (g_renderQueueNodes[i]->flags & NODE_FLAG_NO_DEPTH_WRITE)
            Render_Command(RENDER_DEPTH_WRITE, 0, 0);
        g_drawColour.Copy(g_renderQueueColour[i]);
        g_drawFade = g_renderQueueFade[i];
        int mode = g_renderQueueNodes[i]->drawMode;
        if (mode == DRAW_MODE_MAP)
        {
            int previous = g_mapPassRunning;
            g_mapPassRunning = g_renderQueueMapPass[i];
            Render_Command(RENDER_MAP_PASS, mode, 0);
            Render_Command(RENDER_BLEND_MODE, 0, 0);
            g_renderQueueNodes[i]->mesh->DrawLit(&g_renderQueueNodes[i]->worldMatrix,
                                                 &g_renderQueueMatrices[g_renderQueueMatrixIndex[i]], DRAW_MODE_LIT_PARAM,
                                                 &g_renderQueueLightSets[g_renderQueueLightIndex[i]],
                                                 &g_renderQueueParamsA[i], &g_renderQueueParamsB[i]);
            g_mapPassRunning = previous;
            Render_Command(RENDER_MAP_PASS, 0, 0);
        }
        else
            g_renderQueueNodes[i]->mesh->DrawLit(&g_renderQueueNodes[i]->worldMatrix, view, mode, eye,
                                                 &g_renderQueueParamsA[i], &g_renderQueueParamsB[i]);
        if (g_renderQueueNodes[i]->flags & NODE_FLAG_NO_DEPTH_WRITE)
            Render_Command(RENDER_DEPTH_WRITE, 1, 0);
    }
    g_drawColour.Copy(savedColour);
    g_drawFade = savedFade;
    return 0;
}
