// Node::QueueDrawFading: the lit drawing pass of Node::QueueDraw (node_queue.cpp) for trees whose collision regions
// fade in and out with their visibility cell instead of switching on and off.
// C++ rather than extern "C": the function keeps an exception frame for its matrix locals.

#include <string.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "node_data.h"
#include "mesh.h"
#include "render_state.h"
#include "render_vars.h"
#include "camera_vars.h"
#include "debug_shape.h"

extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *matrix, float margin);   // View_ContainsPoint
// RenderQueue_Push (render_queue.cpp): queues `node` to be drawn later with colour `a` and added colour `b`.
extern "C" int FUN_10199880(Node *node, const void *extra, const Matrix4 *matrix, const Vec4 *a, const Vec4 *b,
                            int group);
// The mesh range of a node that is not animated: every primitive.
#define NODE_FULL_MESH_RANGE 0x1000
// Size of the cube that stands for a bone without skinned children in the skeleton view.
#define SKELETON_BONE_SIZE 2.0f
// Side of the cube drawn for a bounding sphere of radius 1 (a bit more than 1 / sqrt(3)).
#define SPHERE_CUBE_SCALE 0.578f

#define FADE_STEPS 30   // m_1899 of a fully visible collision region

// 0x1019a6b0 (Node::QueueDrawFading). Same as Node::QueueDraw (0x10199e20), except that a collision region "CDLnn"
// does not vanish at once: its brightness step m_1899 counts up to FADE_STEPS while its visibility cell nn is on and
// down to 0 while it is off, the region is skipped at 0, and its alpha is scaled by m_1899 / FADE_STEPS (node_fade.cpp
// reads the step too). Draws this node tree with the light set `lights` under `parent`, seen through
// `view`: a node is drawn when its flags are 0 or share a bit with `passMask` (and it has NODE_FLAG_HIGHLIGHT_PASS
// while only the highlight pass draws), it is not skinned, hidden or a special-pass node and its mesh is built and
// inside the view volume. Its colour is `colour` times its tint plus `addColour` and its own added colour (white and
// nothing when 0). NODE_FLAG_QUEUED nodes go to the render queue instead. A root named "fogset" sets the fog.
// The debug views draw skinned nodes as their skeleton (DAT_11b002c4) and special-pass nodes as shapes
// (DAT_10f654bc). A node with a cycling child (m_1858) passes only that child on; otherwise every child is drawn
// (with the colours of the node's colour tracks) and the queue callback runs. Caller: the map drawing pass (0x10022830). Returns 0 when the node is skipped (overhead pass, switched-off collision region,
// NODE_FLAG_SKIP_SUBTREE), else 1.
int Node::QueueDrawFading(const Matrix4 *parent, const Matrix4 *view, const void *lights, unsigned int passMask,
                    const Vec4 *colour, const Vec4 *addColour)
{
    if ((flags & NODE_FLAG_NOT_OVERHEAD) && DAT_10f65520 != 0)
        return 0;
    if (visibilityCells != 0 && DAT_11b044e0 == 0) {
        UpdateVisibleCells();
        DAT_11b04514 = 1;
    }
    // A collision region "CDLnn" fades in while its visibility cell nn is switched on and out while it is off.
    if (DAT_11b04514 != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L') {
        if (DAT_11afffe8[(name[3] - '0') * 10 + (name[4] - '0')] == 0) {
            if (m_1899 == 0)
                return 0;
            m_1899--;
        } else {
            m_1899++;
            if (m_1899 > FADE_STEPS)
                m_1899 = FADE_STEPS;
        }
    }

    Vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
    Vec4 none(0, 0, 0, 0);
    if (colour == 0)
        colour = &white;
    if (addColour == 0)
        addColour = &none;

    if (this->parent == 0 && name[0] == 'f' && _strnicmp("fogset", name, 6) == 0) {
        float nearZ = fogNear;
        float farZ = fogFar;
        DAT_10f668f8.Copy(fogColour);
        DAT_10f66968 = nearZ;
        DAT_10f66970 = 0;
        DAT_10f6696c = 1.0f / (farZ - nearZ);
        DAT_10f66974 = fogDensity;
    }

    if (DAT_11b002c4 != 0 && skinned) {
        // Skeleton view: a line to every skinned child, or a cube when there is none.
        int drewBone = 0;
        for (int i = 0; i < childCount; i++) {
            if (children[i]->skinned) {
                Matrix4 identity;
                identity.SetIdentity();
                Vec4 from = worldMatrix.GetTranslation();
                from.w = 1.0f;
                Vec4 to = children[i]->worldMatrix.GetTranslation();
                to.w = 1.0f;
                drewBone = 1;
                Debug_DrawShape(DEBUG_SHAPE_CONE, &from, &to, &identity, view);
            }
        }
        if (!drewBone) {
            Vec4 size;
            size.Set(SKELETON_BONE_SIZE, SKELETON_BONE_SIZE, SKELETON_BONE_SIZE, 1.0f);
            Matrix4 place;
            place.SetIdentity();
            place.Translate(worldMatrix.translation.x, worldMatrix.translation.y, worldMatrix.translation.z);
            Debug_DrawShape(DEBUG_SHAPE_ELLIPSOID, &size, 0, &place, view);
        }
    } else {
        if (flags & NODE_FLAG_SKIP_SUBTREE) {
            if (visibilityCells != 0)
                DAT_11b04514 = 0;
            return 0;
        }
        if ((flags == 0 || (passMask & flags)) && (DAT_11b00028 == 0 || (flags & NODE_FLAG_HIGHLIGHT_PASS)) &&
            !skinned && !(flags & NODE_FLAG_SPECIAL_PASSES) && !(flags & NODE_FLAG_HIDDEN) && built != 0) {
            if (mesh != 0 && hasCollision != 0 && packCount != 0 &&
                ((flags & NODE_FLAG_NEVER_CULLED) ||
                 FUN_101ae4d0(&(bounds + worldMatrix.GetTranslation()), view, cullMargin) != 0)) {
                if (flags & NODE_FLAG_QUEUED) {
                    Vec4 fadedColour = *colour * tint;
                    fadedColour.w = ((float)m_1899 / (float)FADE_STEPS) * fadedColour.w;
                    FUN_10199880(this, 0, 0, &fadedColour, &(*addColour + m_02bc), group);
                } else {
                    if (skin != 0 && skin->boneCount != 0)
                        mesh->UploadBones(skin->skinMatrices, skin->boneCount, &worldMatrix, (int)view);
                    int first;
                    int last;
                    if (animated) {
                        int shown = frame;
                        if (shown >= frameCount)
                            shown = frameCount - 1;
                        last = (shown + 1) * frameSize;
                        first = frameSize * shown;
                    } else {
                        last = NODE_FULL_MESH_RANGE;
                        first = 0;
                    }
                    mesh->SetRange(first, last);
                    FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_WRAP, 0);
                    if (flags & NODE_FLAG_NO_STATE_9)
                        FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
                    Vec4 fadedColour = *colour * tint;
                    fadedColour.w = ((float)m_1899 / (float)FADE_STEPS) * fadedColour.w;
                    mesh->DrawLit(&worldMatrix, view, drawMode, lights, &fadedColour, &(*addColour + m_02bc));
                    if (flags & NODE_FLAG_NO_STATE_9)
                        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
                    if (DAT_11b044f4 != 0)
                        mesh->DrawDebugBox(&worldMatrix, view, box);
                }
            }
        } else if (mesh != 0 && DAT_10f654bc != 0) {
            // Shape view of the special-pass nodes.
            if (flags & NODE_FLAG_SHAPE_SPHERE) {
                Vec4 size;
                float side = bounds.w * SPHERE_CUBE_SCALE;
                size.Set(side, side, side, 1.0f);
                Matrix4 place;
                place.SetIdentity();
                place.Translate(bounds.x + worldMatrix.translation.x, bounds.y + worldMatrix.translation.y,
                                bounds.z + worldMatrix.translation.z);
                Debug_DrawShape(DEBUG_SHAPE_ELLIPSOID, &size, 0, &place, view);
            } else if (flags & NODE_FLAG_SHAPE_BOX) {
                Matrix4 place;
                place.SetIdentity();
                place.Translate(worldMatrix.translation.x, worldMatrix.translation.y, worldMatrix.translation.z);
                Debug_DrawShape(DEBUG_SHAPE_BOX, &box[NODE_BOX_MIN], &box[NODE_BOX_MAX], &place, view);
            } else if (flags & NODE_FLAG_SHAPE_SOLID_BOX) {
                Matrix4 place;
                place.SetIdentity();
                place.Translate(worldMatrix.translation.x, worldMatrix.translation.y, worldMatrix.translation.z);
                Debug_DrawShape(DEBUG_SHAPE_RINGS, &box[NODE_BOX_MIN], &box[NODE_BOX_MAX], &place, view);
            }
        }
    }

    if (m_1858 != 0) {
        // Only the current cycling child is drawn.
        if (!(flags & NODE_FLAG_HIDDEN))
            children[m_1858 - 1]->QueueDrawFading(&worldMatrix, view, lights, passMask, colour, addColour);
    } else {
        // The children get the colours of this node's colour tracks (extra->m_0070) when it has them.
        Vec4 childColour;
        Vec4 childAddColour;
        if (extra != 0 && extra->m_0070[0] != 0 && extra->m_0070[1] != 0)
            colour = &(*colour * tint);
        childColour.Copy(*colour);
        if (extra != 0 && extra->m_0070[2] != 0 && extra->m_0070[3] != 0)
            addColour = &(*addColour + m_02bc);
        childAddColour.Copy(*addColour);
        for (int i = 0; i < childCount; i++)
            children[i]->QueueDrawFading(&worldMatrix, view, lights, passMask, &childColour, &childAddColour);
        if (queueCallback != 0)
            queueCallback(&worldMatrix, view, lights, passMask, &childColour, &childAddColour);
    }
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return 1;
}
