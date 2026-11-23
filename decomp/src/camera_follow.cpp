// Camera easing in the scene frame (FUN_100f5a80): the camera target (DAT_118e6cb8) and position (DAT_118e69e8) move
// towards their goals every frame, and the results are copied to the other camera vectors.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "map_vars.h"
#include "char_state.h"

#include "effect_manager.h"

#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"

extern "C" void FUN_1016a8d0(void);

extern "C" Node *DAT_1021bdf8[0x80][9];       // character 0: [0] = the stand's parent node

// 0x100184e0 (Camera_ItemStand). The shot of the item stand ("kagudai") rising: on the first call it finds the
// stand in the map (setting flag 0x20 on its parent and clearing it on its "itemkagu" item) and keeps its parent node. Every 65
// frames it starts sparkle 0x75f on a matrix 115 units above the target goal; the stand's parent follows the target
// goal 50 units lower. The position goal turns slowly about the target goal (-0.012 rad a frame) and after 90
// frames the target goal sinks (to 4 units a frame from frame 120). The target and position then move 20 %
// towards their goals plus 80 % towards their previous values, and the copies are refreshed. Returns 0.
extern "C" int FUN_100184e0(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_104c0358 = 1;
        DAT_1020b430[0].cameraFrames = 0;
        Node *stand;
        if (DAT_10255f08 != 0 && (stand = DAT_10255f08->Find("@kagudai", 0)) != 0)
        {
            DAT_1021bdf8[0][0] = stand->parent;
            DAT_1021bdf8[0][0]->SetFlags(0x20);
            Node *item = stand->Find("@itemkagu", 0);
            if (item != 0)
                item->ClearFlags(0x20);
        }
    }
    if (DAT_1020b430[0].cameraFrames % 65 == 0)
    {
        Vec4 position;
        DAT_10daaa80.Start(0x75f, position, -1, -1, &DAT_10868838[0][0], 0, 0, 0, 0, 0);
    }
    DAT_1020b430[0].cameraFrames++;
    if (DAT_1021bdf8[0][0] != 0)
    {
        DAT_1021bdf8[0][0]->matrix.SetTranslation(DAT_10edbd48);
        DAT_1021bdf8[0][0]->matrix.m[14] -= 50.0f;
    }
    DAT_10868838[0][0].SetIdentity();
    DAT_10868838[0][0].m[14] = DAT_10edbd48.z + 115.0f;
    DAT_10868838[0][0].m[12] = DAT_10edbd48.x;
    DAT_10868838[0][0].m[13] = DAT_10edbd48.y;
    DAT_10c6356c = (int)DAT_10edbd48.x;
    DAT_10c63570 = (int)DAT_10edbd48.y;
    DAT_10c63574 = (int)DAT_10edbd48.z + 115;
    Vec4 offset;
    offset.Copy(DAT_10866d98 - DAT_10edbd48);
    offset.RotateZ(-0.012f);
    DAT_10866d98.Copy(DAT_10edbd48 + offset);
    if (DAT_1020b430[0].cameraFrames > 120)
        DAT_10edbd48.z -= 4.0f;
    else if (DAT_1020b430[0].cameraFrames > 90)
        DAT_10edbd48.z -= (float)(DAT_1020b430[0].cameraFrames - 90) * 4.0f / 30.0f;
    DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8) * 0.2f +
                      (DAT_10f08aa0 - DAT_118e6cb8) * 0.8f);
    DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8) * 0.2f +
                      (DAT_104d29d8 - DAT_118e69e8) * 0.8f);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}

// 0x10018fb0 (Camera_Ease). On the first call sets the goals from the current camera (the target goal from the
// followed node when there is one) and every camera vector from them. Then each frame the target goal follows the
// node, the target moves 15 % of the way to its goal plus 85 % of the way to its previous value (likewise the
// position), and the copies are refreshed. Returns 0.
extern "C" int FUN_10018fb0(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_10866d98.Copy(DAT_118e69e8);
        if (DAT_104c039c != 0)
            DAT_10edbd48.Copy(DAT_104c039c->worldMatrix.GetTranslation());
        else
            DAT_10edbd48.Copy(DAT_118e6cb8);
        DAT_118e6cb8.Copy(DAT_10edbd48);
        DAT_118e69e8.Copy(DAT_10866d98);
        DAT_118e6c78.Copy(DAT_118e6cb8);
        DAT_11af3e28.Copy(DAT_118e69e8);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_10f65688.Copy(DAT_118e69e8);
        DAT_104d29d8.Copy(DAT_118e69e8);
        DAT_10f08aa0.Copy(DAT_118e6cb8);
        DAT_104c0358 = 1;
    }
    if (DAT_104c039c != 0)
        DAT_10edbd48.Copy(DAT_104c039c->worldMatrix.GetTranslation());
    DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8) * 0.15f +
                      (DAT_10f08aa0 - DAT_118e6cb8) * 0.85f);
    DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8) * 0.15f +
                      (DAT_104d29d8 - DAT_118e69e8) * 0.85f);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}


// 0x10019310 (Camera_Shift). Like Camera_Ease, with the goals offset: while a shift runs (DAT_104c035c counts down)
// the offsets move 15 % towards the shift goals relative to the current camera, divided by 1 + the frames left / 2;
// otherwise they shrink to 60 % each frame. The target and position then move halfway from their previous values
// by the offsets, and the copies are refreshed. Returns 0.
extern "C" int FUN_10019310(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_104c0358 = 1;
        DAT_108393d8.Fill(0.0f);
        DAT_10da93f0.Fill(0.0f);
    }
    DAT_10866d98.Copy(DAT_118e69e8);
    DAT_10edbd48.Copy(DAT_118e6cb8);
    if (DAT_104c035c == 0 || --DAT_104c035c == 0)
    {
        DAT_108393d8.Copy(DAT_108393d8 * 0.6f);
        DAT_10da93f0.Copy(DAT_10da93f0 * 0.6f);
    }
    else
    {
        Vec4 positionStep;
        Vec4 targetStep;
        int half = (int)((float)DAT_104c035c * -0.5f);
        positionStep.Copy((DAT_10802be8 - DAT_10866d98).Divided((float)(1 - half)));
        targetStep.Copy((DAT_10c67ff0 - DAT_10edbd48).Divided((float)(1 - half)));
        DAT_108393d8.Copy(DAT_108393d8 * 0.85f + positionStep * 0.15f);
        DAT_10da93f0.Copy(DAT_10da93f0 * 0.85f + targetStep * 0.15f);
    }
    DAT_10866d98.Copy(DAT_108393d8);
    DAT_10edbd48.Copy(DAT_10da93f0);
    DAT_118e6cb8.Copy(DAT_118e6cb8 + DAT_10edbd48 * 0.5f + (DAT_10f08aa0 - DAT_118e6cb8) * 0.5f);
    DAT_118e69e8.Copy(DAT_118e69e8 + DAT_10866d98 * 0.5f + (DAT_104d29d8 - DAT_118e69e8) * 0.5f);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];


// 0x10019d80 (Camera_Between). A two-character shot: the position goal is the head of character DAT_10da93e0
// (35 units above its head node, or 75 above it without one), the target goal the head of character DAT_10da93e4
// the same way, or 600 units ahead of and 60 below the first when that is negative. The target goal rises by
// DAT_10da93e8 per cent of the distance between them. The target and position then move 20 % towards their goals
// plus 80 % towards their previous values, and the copies are refreshed. Returns 0.
extern "C" int FUN_10019d80(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_104c0358 = 1;
    }
    int first = (int)(DAT_10da93e0 + 0.5f);
    first = first > 0 ? first : 0;
    float rounded = DAT_10da93e4 < 0.0f ? DAT_10da93e4 - 0.5f : DAT_10da93e4 + 0.5f;
    int second = (int)rounded;
    if (DAT_10db6ee8[first].headNode != 0)
    {
        DAT_10866d98.Copy(DAT_10db6ee8[first].headNode->worldMatrix.GetTranslation());
        DAT_10866d98.z += 35.0f;
    }
    else
    {
        DAT_10866d98.Copy(DAT_10db6ee8[first].matrix.GetTranslation());
        DAT_10866d98.z += 75.0f;
    }
    if (second < 0)
    {
        DAT_10edbd48.Fill(0.0f);
        DAT_10edbd48.y = -600.0f;
        DAT_10edbd48.z = -60.0f;
        DAT_10edbd48.RotateZ(-DAT_10db6ee8[first].heading);
        DAT_10edbd48.Copy(DAT_10edbd48 + DAT_10866d98);
    }
    else if (DAT_10db6ee8[second].headNode != 0)
    {
        DAT_10edbd48.Copy(DAT_10db6ee8[second].headNode->worldMatrix.GetTranslation());
        DAT_10edbd48.z += 35.0f;
    }
    else
    {
        DAT_10edbd48.Copy(DAT_10db6ee8[second].matrix.GetTranslation());
        DAT_10edbd48.z += 75.0f;
    }
    if (DAT_10866d98.x > -20000.0f)
        DAT_10edbd48.z += (DAT_10866d98 - DAT_10edbd48).Length() * DAT_10da93e8 * 0.01f;
    DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8) * 0.2f +
                      (DAT_10f08aa0 - DAT_118e6cb8) * 0.8f);
    DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8) * 0.2f +
                      (DAT_104d29d8 - DAT_118e69e8) * 0.8f);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}

// 0x1001a0d0 (Camera_Orbit). Like Camera_Ease with an orbit: the position goal keeps the camera's heading around
// the orbit centre at the distance of DAT_108393d8 from it, turned by that point's heading; the target goal is the
// centre (drifting 5 % towards DAT_1021c624 when set). The target and position then move 20 % towards their goals
// plus 80 % towards their previous values, and the copies are refreshed. Returns 0.
extern "C" int FUN_1001a0d0(void)
{
    if (DAT_104c0358 == 0)
    {
        FUN_1016a8d0();
        DAT_104c0358 = 1;
    }
    Vec4 offset;
    offset.Copy(DAT_118e69e8 - DAT_118e6cb8);
    float heading = (float)atan2(offset.x, offset.y) + 3.1415f;
    offset.x = 0;
    offset.y = -(DAT_108393d8 - DAT_10da93f0).Length2D();
    offset.RotateZ(DAT_108393d8.w - heading);
    offset.Copy(offset + DAT_10da93f0);
    DAT_10866d98.Copy(offset);
    DAT_10edbd48.Copy(DAT_10da93f0);
    if (DAT_1021c624 != 0)
        DAT_10edbd48.Copy(DAT_10edbd48 * 0.95f + DAT_1021c624->worldMatrix.GetTranslation() * 0.05f);
    DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8) * 0.2f +
                      (DAT_10f08aa0 - DAT_118e6cb8) * 0.8f);
    DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8) * 0.2f +
                      (DAT_104d29d8 - DAT_118e69e8) * 0.8f);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    DAT_104d29d8.Copy(DAT_118e69e8);
    DAT_10f08aa0.Copy(DAT_118e6cb8);
    return 0;
}
