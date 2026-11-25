// Script-driven shot cameras (EN build). Ported from the matched JP source decomp/src/camera_follow.cpp.
#include <math.h>

#include "anim_model.h"
#include "camera.h"
#include "char.h"
#include "effect_manager.h"
#include "visibility.h"

// Camera_Ease: the follow shot. On the first call sets both goals from the current camera (the target goal from the
// followed node when there is one) and every camera vector from them. Then each frame the target goal follows the
// node, the target moves 15 % of the way to its goal plus 85 % of the way to its previous value (likewise the
// position), and the copies are refreshed. Returns 0. Called every frame while the camera runs this shot.
extern "C" int Camera_Ease(void)
{
    if (g_cameraShotStarted == 0)
    {
        Camera_SetMode2();
        g_cameraPositionGoal.Copy(g_cameraPositionNow);
        if (g_cameraFollowNode != 0)
            g_cameraTargetGoal.Copy(g_cameraFollowNode->worldMatrix.GetTranslation());
        else
            g_cameraTargetGoal.Copy(g_cameraTargetNow);
        g_cameraTargetNow.Copy(g_cameraTargetGoal);
        g_cameraPositionNow.Copy(g_cameraPositionGoal);
        g_cameraTargetCopy.Copy(g_cameraTargetNow);
        g_cameraPositionCopy.Copy(g_cameraPositionNow);
        g_cameraTarget.Copy(g_cameraTargetNow);
        g_cameraEye.Copy(g_cameraPositionNow);
        g_cameraPreviousPosition.Copy(g_cameraPositionNow);
        g_cameraPreviousTarget.Copy(g_cameraTargetNow);
        g_cameraShotStarted = 1;
    }
    if (g_cameraFollowNode != 0)
        g_cameraTargetGoal.Copy(g_cameraFollowNode->worldMatrix.GetTranslation());
    // Weights: 0.15 of the way to the goal plus 0.85 of the way to the previous value (written as literals: Vec4's
    // operator* takes the factor by reference, so a named constant would change the code).
    g_cameraTargetNow.Copy(g_cameraTargetNow + (g_cameraTargetGoal - g_cameraTargetNow) * 0.15f +
                           (g_cameraPreviousTarget - g_cameraTargetNow) * 0.85f);
    g_cameraPositionNow.Copy(g_cameraPositionNow + (g_cameraPositionGoal - g_cameraPositionNow) * 0.15f +
                             (g_cameraPreviousPosition - g_cameraPositionNow) * 0.85f);
    g_cameraTargetCopy.Copy(g_cameraTargetNow);
    g_cameraPositionCopy.Copy(g_cameraPositionNow);
    g_cameraTarget.Copy(g_cameraTargetNow);
    g_cameraEye.Copy(g_cameraPositionNow);
    g_cameraPreviousPosition.Copy(g_cameraPositionNow);
    g_cameraPreviousTarget.Copy(g_cameraTargetNow);
    return 0;
}

// Camera_Orbit: like Camera_Ease with an orbit: the position goal keeps the camera's heading around the orbit centre at
// the distance of the orbit point from it, turned by that point's heading; the target goal is the centre (drifting
// 5 % towards the drift node when there is one). The target and position then move 20 % towards their goals plus
// 80 % towards their previous values, and the copies are refreshed. Returns 0. Called every frame while the camera
// runs this shot.
extern "C" int Camera_Orbit(void)
{
    if (g_cameraShotStarted == 0)
    {
        Camera_SetMode2();
        g_cameraShotStarted = 1;
    }
    Vec4 offset;
    offset.Copy(g_cameraPositionNow - g_cameraTargetNow);
    float heading = (float)atan2(offset.x, offset.y) + ANGLE_HALF_TURN;
    offset.x = 0;
    offset.y = -(g_cameraOrbitCentre - g_cameraPlayerPosition).Length2D();
    offset.RotateZ(g_cameraOrbitCentre.w - heading);
    offset.Copy(offset + g_cameraPlayerPosition);
    g_cameraPositionGoal.Copy(offset);
    g_cameraTargetGoal.Copy(g_cameraPlayerPosition);
    if (g_cameraDriftNode != 0)
        g_cameraTargetGoal.Copy(g_cameraTargetGoal * 0.95f + g_cameraDriftNode->worldMatrix.GetTranslation() * 0.05f);
    g_cameraTargetNow.Copy(g_cameraTargetNow + (g_cameraTargetGoal - g_cameraTargetNow) * 0.2f +
                           (g_cameraPreviousTarget - g_cameraTargetNow) * 0.8f);
    g_cameraPositionNow.Copy(g_cameraPositionNow + (g_cameraPositionGoal - g_cameraPositionNow) * 0.2f +
                             (g_cameraPreviousPosition - g_cameraPositionNow) * 0.8f);
    g_cameraTargetCopy.Copy(g_cameraTargetNow);
    g_cameraPositionCopy.Copy(g_cameraPositionNow);
    g_cameraTarget.Copy(g_cameraTargetNow);
    g_cameraEye.Copy(g_cameraPositionNow);
    g_cameraPreviousPosition.Copy(g_cameraPositionNow);
    g_cameraPreviousTarget.Copy(g_cameraTargetNow);
    return 0;
}

// Camera_ItemStand: the shot of the item stand ("kagudai") rising. On the first call it finds the stand in the map
// (setting flag 0x20 on its parent and clearing it on its "itemkagu" item) and keeps its parent node. Every 65 frames
// it starts sparkle 0x75f on a matrix 115 units above the target goal; the stand's parent follows the target goal 50
// units lower. The position goal turns slowly about the target goal (-0.012 rad a frame) and after 90 frames the
// target goal sinks (to 4 units a frame from frame 120). The target and position then move 20 % towards their goals
// plus 80 % towards their previous values, and the copies are refreshed. Returns 0. Called every frame while the
// camera runs this shot.
extern "C" int Camera_ItemStand(void)
{
    if (g_cameraShotStarted == 0)
    {
        Camera_SetMode2();
        g_cameraShotStarted = 1;
        g_charState[0].values[CHARSTATE_CAMERA_FRAMES] = 0;
        Node *stand;
        if (g_mapRootNode != 0 && (stand = g_mapRootNode->Find("@kagudai", 0)) != 0)
        {
            g_itemStandParent = stand->parent;
            g_itemStandParent->SetFlags(ITEM_STAND_NODE_FLAG);
            Node *item = stand->Find("@itemkagu", 0);
            if (item != 0)
                item->ClearFlags(ITEM_STAND_NODE_FLAG);
        }
    }
    if (g_charState[0].values[CHARSTATE_CAMERA_FRAMES] % ITEM_STAND_SPARKLE_PERIOD == 0)
    {
        Vec4 position;
        g_effectManager.Start(ITEM_STAND_SPARKLE_EFFECT, position, -1, -1, &g_characterNodeMatrices[0][0], 0, 0, 0, 0,
                              0);
    }
    g_charState[0].values[CHARSTATE_CAMERA_FRAMES]++;
    if (g_itemStandParent != 0)
    {
        g_itemStandParent->matrix.SetTranslation(g_cameraTargetGoal);
        g_itemStandParent->matrix.m[MATRIX_TRANSLATION_Z] -= ITEM_STAND_DROP;
    }
    g_characterNodeMatrices[0][0].SetIdentity();
    g_characterNodeMatrices[0][0].m[MATRIX_TRANSLATION_Z] = g_cameraTargetGoal.z + ITEM_STAND_SPARKLE_BASE + ITEM_STAND_SPARKLE_REST;
    g_characterNodeMatrices[0][0].m[MATRIX_TRANSLATION_X] = g_cameraTargetGoal.x;
    g_characterNodeMatrices[0][0].m[MATRIX_TRANSLATION_Y] = g_cameraTargetGoal.y;
    g_shotPointX = (int)g_cameraTargetGoal.x;
    g_shotPointY = (int)g_cameraTargetGoal.y;
    g_shotPointZ = (int)g_cameraTargetGoal.z + ITEM_STAND_SPARKLE_INT;
    Vec4 offset;
    offset.Copy(g_cameraPositionGoal - g_cameraTargetGoal);
    offset.RotateZ(ITEM_STAND_ORBIT_STEP);
    g_cameraPositionGoal.Copy(g_cameraTargetGoal + offset);
    if (g_charState[0].values[CHARSTATE_CAMERA_FRAMES] > ITEM_STAND_SINK_FULL)
        g_cameraTargetGoal.z -= ITEM_STAND_SINK_SPEED;
    else if (g_charState[0].values[CHARSTATE_CAMERA_FRAMES] > ITEM_STAND_SINK_START)
        g_cameraTargetGoal.z -= (float)(g_charState[0].values[CHARSTATE_CAMERA_FRAMES] - ITEM_STAND_SINK_START) *
                                ITEM_STAND_SINK_SPEED / (float)ITEM_STAND_SINK_RAMP;
    g_cameraTargetNow.Copy(g_cameraTargetNow + (g_cameraTargetGoal - g_cameraTargetNow) * 0.2f +
                           (g_cameraPreviousTarget - g_cameraTargetNow) * 0.8f);
    g_cameraPositionNow.Copy(g_cameraPositionNow + (g_cameraPositionGoal - g_cameraPositionNow) * 0.2f +
                             (g_cameraPreviousPosition - g_cameraPositionNow) * 0.8f);
    g_cameraTargetCopy.Copy(g_cameraTargetNow);
    g_cameraPositionCopy.Copy(g_cameraPositionNow);
    g_cameraTarget.Copy(g_cameraTargetNow);
    g_cameraEye.Copy(g_cameraPositionNow);
    g_cameraPreviousPosition.Copy(g_cameraPositionNow);
    g_cameraPreviousTarget.Copy(g_cameraTargetNow);
    return 0;
}

// Camera_Between: a two-character shot. The position goal is the head of the character given by g_cameraBetweenParams.x
// (35 units above its head node, or 75 above it without one), the target goal the head of character
// g_cameraBetweenParams.y the same way, or 600 units ahead of and 60 below the first when that is negative. The target
// goal rises by g_cameraBetweenParams.z per cent of the distance between them. The target and position then move 20 %
// towards their goals plus 80 % towards their previous values, and the copies are refreshed. Returns 0. Called every
// frame while the camera runs this shot.
extern "C" int Camera_Between(void)
{
    if (g_cameraShotStarted == 0)
    {
        Camera_SetMode2();
        g_cameraShotStarted = 1;
    }
    int first = (int)(g_cameraBetweenParams.x + 0.5f);
    first = first > 0 ? first : 0;
    float rounded = g_cameraBetweenParams.y < 0.0f ? g_cameraBetweenParams.y - 0.5f : g_cameraBetweenParams.y + 0.5f;
    int second = (int)rounded;
    if (g_characterModels[first].headNode != 0)
    {
        g_cameraPositionGoal.Copy(g_characterModels[first].headNode->worldMatrix.GetTranslation());
        g_cameraPositionGoal.z += BETWEEN_HEAD_OFFSET;
    }
    else
    {
        g_cameraPositionGoal.Copy(g_characterModels[first].matrix.GetTranslation());
        g_cameraPositionGoal.z += BETWEEN_BODY_OFFSET;
    }
    if (second < 0)
    {
        g_cameraTargetGoal.Fill(0.0f);
        g_cameraTargetGoal.y = BETWEEN_AHEAD;
        g_cameraTargetGoal.z = BETWEEN_BELOW;
        g_cameraTargetGoal.RotateZ(-g_characterModels[first].heading);
        g_cameraTargetGoal.Copy(g_cameraTargetGoal + g_cameraPositionGoal);
    }
    else if (g_characterModels[second].headNode != 0)
    {
        g_cameraTargetGoal.Copy(g_characterModels[second].headNode->worldMatrix.GetTranslation());
        g_cameraTargetGoal.z += BETWEEN_HEAD_OFFSET;
    }
    else
    {
        g_cameraTargetGoal.Copy(g_characterModels[second].matrix.GetTranslation());
        g_cameraTargetGoal.z += BETWEEN_BODY_OFFSET;
    }
    if (g_cameraPositionGoal.x > BETWEEN_NO_POSITION)
        g_cameraTargetGoal.z += (g_cameraPositionGoal - g_cameraTargetGoal).Length() * g_cameraBetweenParams.z * BETWEEN_RISE_PER_CENT;
    g_cameraTargetNow.Copy(g_cameraTargetNow + (g_cameraTargetGoal - g_cameraTargetNow) * 0.2f +
                           (g_cameraPreviousTarget - g_cameraTargetNow) * 0.8f);
    g_cameraPositionNow.Copy(g_cameraPositionNow + (g_cameraPositionGoal - g_cameraPositionNow) * 0.2f +
                             (g_cameraPreviousPosition - g_cameraPositionNow) * 0.8f);
    g_cameraTargetCopy.Copy(g_cameraTargetNow);
    g_cameraPositionCopy.Copy(g_cameraPositionNow);
    g_cameraTarget.Copy(g_cameraTargetNow);
    g_cameraEye.Copy(g_cameraPositionNow);
    g_cameraPreviousPosition.Copy(g_cameraPositionNow);
    g_cameraPreviousTarget.Copy(g_cameraTargetNow);
    return 0;
}
