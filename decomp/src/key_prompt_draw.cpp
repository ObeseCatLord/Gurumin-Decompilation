// Drawing the key prompts (FUN_1000d620 in app_0008.cpp shows one over a character): a 64x64 cell of a balloon
// texture above the character's head, projected to the screen.

#include "world.h"

#include "vec4.h"

#include "matrix4.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space

#include "node.h"
#include "anim_model.h"
#include "char_state.h"

#include "texture_object.h"

extern "C" int DAT_104bf8f8[0x7c];                   // 1 while a key prompt is shown over the character
extern "C" int DAT_1021d138[0x7c];                   // the prompt: cell (0..15) + 1000 or 2000 for the other sheets
#include "ui_vars.h"

// 0x1000d6b0 (KeyPrompt_DrawAll). For every character with a prompt (and in use, not removed below x -100000) the
// point 40 units above its head bone (or its origin without one, 20 units) is projected with `view` and the prompt
// cell is drawn there: prompts 0..999 from texBalloon5, 1000.. from texBalloon6, 2000.. from texBalloon7, 4x4 cells
// of 0.25 each. Only points in front of the camera (depth 0.9..1.0) are drawn. Returns 0.
extern "C" int FUN_1000d6b0(const Matrix4 *view)
{
    for (int i = 0; i < 0x7c; i++)
    {
        AnimModel *character = &DAT_10db6ee8[i];
        if (character->loadedRoot == 0 || DAT_104bf8f8[i] == 0 || !(character->matrix._41 > -100000.0f))
        {
            DAT_104bf8f8[i] = 0;
            continue;
        }
        Vec4 point;
        if (character->headNode != 0)
        {
            point.Copy(character->headNode->worldMatrix.GetTranslation());
            point.z += 20.0f;
        }
        else
            point.Copy(character->matrix.GetTranslation());
        point.z += 20.0f;
        int sheet = 0;
        int cell = DAT_1021d138[i];
        if (cell >= 2000)
        {
            sheet = 0x18;
            cell -= 2000;
        }
        else if (cell >= 1000)
        {
            sheet = 1;
            cell -= 1000;
        }
        point.Copy(Project(point, *view));
        point.x = (point.x + 1.0f) * 320.0f;
        point.y = (2.0f - (point.y + 1.0f)) * 240.0f;
        float u = (float)(cell % 4) / 4.0f;
        float v = (float)(cell % 16 / 4) / 4.0f;
        int half = (int)((float)DAT_104bf88c / 2.0f + 15.0f);
        if (point.z < 0.9 || point.w < 0.0f || point.z > 1.0 || point.z < 0.9)
            continue;
        DAT_118d0ba8[DAT_104b721c[sheet]].DrawTinted((int)(point.x - half), (int)(point.y - half),
                                                         (int)(point.x + half), (int)(point.y + half), u, v,
                                                         u + 0.25f, v + 0.25f, point.z, 1.0f, 0, 0);
    }
    return 0;
}
