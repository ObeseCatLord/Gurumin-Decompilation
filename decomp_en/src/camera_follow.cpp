// Script-driven shot cameras (EN build). Ported from the matched JP source decomp/src/camera_follow.cpp.
#include <math.h>

#include "anim_model.h"
#include "camera.h"
#include "char.h"
#include "effect_manager.h"
#include "engine.h"
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

// Settings of the pull-to-item-stand shot, kept in elements of the first saved node matrix (the shot shares it with the
// item stand sparkle).
#define PULL_ORBIT_DISTANCE g_characterNodeMatrices[0][0].m[0]    // distance of the position goal from the target goal
#define PULL_ORBIT_HEIGHT g_characterNodeMatrices[0][0].m[1]      // height of the position goal when the shot started
#define PULL_ORBIT_UNUSED_STEP g_characterNodeMatrices[0][0].m[2] // set to the turn per frame, never read
#define PULL_ORBIT_SCALE_RATE g_characterNodeMatrices[0][0].m[4]  // change of PULL_ORBIT_SCALE each frame
#define PULL_ORBIT_TURN g_characterNodeMatrices[0][0].m[6]        // radians the position goal turns each frame
#define PULL_ORBIT_SCALE g_characterNodeMatrices[0][0].m[8]       // factor on the orbit distance
#define PULL_START_DISTANCE 800.0f     // distance of the position goal from the target goal at the start
#define PULL_TURN_PER_FRAME 0.008f     // radians the position goal turns each frame
#define PULL_WALL_CLEARANCE 25.0f      // distance kept in front of a wall
#define PULL_WALL_NEAR 50.0f           // walls closer than this get a shrunk distance instead
#define PULL_WALL_NEAR_SHRINK 0.75f    // factor on the distance to a near wall
#define PULL_MAX_STEP 15.0f            // furthest the player is moved towards the stand in a frame
#define PULL_TARGET_LIFT 70.0f         // height of the target goal above the player
#define PULL_EASE_GOAL 0.15f           // share of the way to its goal the camera moves each frame
#define PULL_EASE_PREVIOUS 0.85f       // share of the way to the previous value
const int PULL_STATE_ENDING = 1;              // g_cameraShiftFrames value that ends the shot
const int PULL_STATE_ENDED = 2;               // g_cameraShiftFrames value after the shot ended
const unsigned int PULL_MASK_CAMERA = 4;      // map shape mask of the camera walls ("CA" shapes)
const int PULL_VARIANT_CHOICES = 2;           // each digit of the variant is one of two values

// Camera_PullToItemStand: the camera position goal stays on the line from the target goal through the current camera,
// turned 0.008 rad a frame and set to PULL_ORBIT_DISTANCE * PULL_ORBIT_SCALE from the target goal (the settings are set
// up on the first call, which also rolls the variant for the player's slot). The goal is then moved in front of walls:
// a segment from 25 units beyond the goal back to the target is cast against the map's "CA" shapes (mask 4) and then
// its "CS" shapes, and a hit puts the goal 25 units short of it (or at 3/4 of the distance when that is less than 50).
// g_cameraShiftFrames = 1 ends the shot (follow mode, state 2). While the map has a "@kagudai" node the player is moved
// towards it (at most 15 units a frame, when the map allows) and the target goal is 70 units above the player. The
// target and position then move 15 % towards their goals plus 85 % towards their previous values (only while the shot
// runs), and the copies are refreshed. Returns 0. Called every frame while the camera runs this shot.
extern "C" int Camera_PullToItemStand(void)
{
    if (g_cameraShiftFrames == 0)
    {
        if (g_cameraShotStarted == 0)
        {
            Camera_SetMode2();
            g_cameraPositionGoal.Copy(g_cameraPositionNow);
            g_cameraTargetGoal.Copy(g_cameraTargetNow);
            float scale = 1.0f;
            g_cameraPositionGoal.Copy((g_cameraPositionGoal - g_cameraTargetGoal) * scale + g_cameraTargetGoal);
            g_cameraShotStarted = 1;
            g_charState[0].values[CHARSTATE_CAMERA_FRAMES] = 0;
            ZeroMatrix();
            g_charState[0].values[CHARSTATE_CAMERA_FRAMES] =
                ((Random() % PULL_VARIANT_CHOICES + 1) * 16) | (Random() % PULL_VARIANT_CHOICES * 4 + 4) |
                (Random() % PULL_VARIANT_CHOICES + 1);
            (g_cameraPositionGoal - g_cameraTargetGoal).Length2D();
            PULL_ORBIT_HEIGHT = g_cameraPositionGoal.z;
            PULL_ORBIT_UNUSED_STEP = PULL_TURN_PER_FRAME;
            PULL_ORBIT_SCALE_RATE = 0.0f;
            PULL_ORBIT_TURN = PULL_TURN_PER_FRAME;
            PULL_ORBIT_SCALE = 1.0f;
            PULL_ORBIT_DISTANCE = PULL_START_DISTANCE;
        }
        PULL_ORBIT_SCALE = PULL_ORBIT_SCALE + PULL_ORBIT_SCALE_RATE;
        Vec4 orbitOffset;
        orbitOffset.Copy(g_cameraPositionGoal - g_cameraTargetGoal);
        orbitOffset.RotateZ(-PULL_ORBIT_TURN);
        orbitOffset.Normalized3();
        orbitOffset.Copy(orbitOffset * PULL_ORBIT_DISTANCE * PULL_ORBIT_SCALE);
        g_cameraPositionGoal.Copy(g_cameraTargetGoal + orbitOffset);
        {
            Vec4 farPoint;
            Vec4 target;
            Vec4 hit;
            target.Copy(g_cameraTargetGoal);
            farPoint.Copy(g_cameraPositionGoal - g_cameraTargetGoal);
            {
                float distance = farPoint.Length();
                float reach = distance + PULL_WALL_CLEARANCE;
                farPoint.Normalized3();
                farPoint.Copy(farPoint * reach + g_cameraTargetGoal);
            }
            if (g_mapRootNode != 0)
            {
                if (g_mapRootNode->CastSegment("CA", PULL_MASK_CAMERA, &farPoint, &target, &hit, 0))
                {
                    float clearance = (hit - target).Length();
                    if (clearance > PULL_WALL_NEAR)
                        clearance -= PULL_WALL_CLEARANCE;
                    else
                        clearance *= PULL_WALL_NEAR_SHRINK;
                    hit.Copy(farPoint - target);
                    hit.Normalized3();
                    g_cameraPositionGoal.Copy(target + hit * clearance);
                    farPoint.Copy(target + hit * (clearance + PULL_WALL_CLEARANCE));
                }
                if (g_mapRootNode != 0)
                {
                    if (g_mapRootNode->CastSegment("CS", 0, &farPoint, &target, &hit, 0))
                    {
                        float clearance = (hit - target).Length();
                        if (clearance > PULL_WALL_NEAR)
                            clearance -= PULL_WALL_CLEARANCE;
                        else
                            clearance *= PULL_WALL_NEAR_SHRINK;
                        hit.Copy(farPoint - target);
                        hit.Normalized3();
                        farPoint.Copy(target + hit * clearance);
                        g_cameraPositionGoal.Copy(farPoint);
                    }
                }
            }
        }
        g_cameraTargetNow.Copy(g_cameraTargetNow + (g_cameraTargetGoal - g_cameraTargetNow) * PULL_EASE_GOAL +
                               (g_cameraPreviousTarget - g_cameraTargetNow) * PULL_EASE_PREVIOUS);
        g_cameraPositionNow.Copy(g_cameraPositionNow + (g_cameraPositionGoal - g_cameraPositionNow) * PULL_EASE_GOAL +
                                 (g_cameraPreviousPosition - g_cameraPositionNow) * PULL_EASE_PREVIOUS);
        g_cameraTargetCopy.Copy(g_cameraTargetNow);
        g_cameraPositionCopy.Copy(g_cameraPositionNow);
        g_cameraTarget.Copy(g_cameraTargetNow);
        g_cameraEye.Copy(g_cameraPositionNow);
        g_cameraPreviousPosition.Copy(g_cameraPositionNow);
        g_cameraPreviousTarget.Copy(g_cameraTargetNow);
    }
    else if (g_cameraShiftFrames == PULL_STATE_ENDING)
    {
        Camera_SetModeFollow();
        g_cameraShiftFrames = PULL_STATE_ENDED;
    }
    if (g_mapRootNode != 0)
    {
        Node *stand = g_mapRootNode->Find("@kagudai", 0);
        if (stand != 0)
        {
            Vec4 playerPosition;
            Vec4 toStand;
            playerPosition.Copy(g_characterModels[0].matrix.GetTranslation());
            toStand.Copy(stand->worldMatrix.GetTranslation());
            toStand.Copy(toStand - playerPosition);
            toStand.z = 0;
            if (toStand.Length2D() > PULL_MAX_STEP)
            {
                toStand.Normalized3();
                float step = PULL_MAX_STEP;
                toStand.Copy(toStand * step);
            }
            if (g_characterModels[0].IsClearOfMapLimited(&toStand))
            {
                g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] += toStand.x;
                g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] += toStand.y;
            }
            g_cameraTargetGoal.Copy(g_characterModels[0].matrix.GetTranslation());
            g_cameraTargetGoal.z += PULL_TARGET_LIFT;
        }
    }
    return 0;
}
