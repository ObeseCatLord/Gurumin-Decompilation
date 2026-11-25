// Drawing the key prompts above characters (EN build). Ported from decomp/src/key_prompt_draw.cpp.
#include "dialogs.h"
#include "engine.h"
#include "char.h"
#include "texture_mgr.h"

// Characters that can have a prompt: slots below this.
const int KEY_PROMPT_SLOT_COUNT = 0x7c;
// A character moved below this x has been removed from the world.
const float KEY_PROMPT_REMOVED_X = -100000.0f;
// Height above the head bone (or above the origin without one) the prompt is placed at, added once or twice.
const float KEY_PROMPT_HEIGHT = 20.0f;
// Prompt numbers from these start the second and third balloon sheets; the index of the sheet in g_dialogGameSlots.
const int KEY_PROMPT_SHEET_6_FIRST = 1000;
const int KEY_PROMPT_SHEET_7_FIRST = 2000;
const int KEY_PROMPT_SHEET_5 = 0;
const int KEY_PROMPT_SHEET_6 = 1;
const int KEY_PROMPT_SHEET_7 = 0x18;
// The sheets are 4x4 cells of 0.25.
const int KEY_PROMPT_CELLS_PER_ROW = 4;
const int KEY_PROMPT_CELLS_PER_SHEET = 16;
const float KEY_PROMPT_CELL_SIZE = 0.25f;
// 640x480 screen size halves and the half-size margin added to the balloon size.
const float KEY_PROMPT_SCREEN_HALF_WIDTH = 320.0f;
const float KEY_PROMPT_SCREEN_HALF_HEIGHT = 240.0f;
const float KEY_PROMPT_SIZE_MARGIN = 15.0f;
// Only points in front of the camera (projected depth 0.9 to 1.0) are drawn.
const double KEY_PROMPT_DEPTH_NEAR = 0.9;
const float KEY_PROMPT_DEPTH_FAR = 1.0f;

// KeyPrompt_DrawAll: for every character with a prompt (and in use, not removed below x -100000) the point 40 units
// above its head bone (or its origin without one, 20 units) is projected with `view` and the prompt cell is drawn
// there: prompts 0..999 from texBalloon5, 1000.. from texBalloon6, 2000.. from texBalloon7, 4x4 cells of 0.25 each. Only
// points in front of the camera are drawn. Returns 0. Called once per frame by the HUD.
extern "C" int KeyPrompt_DrawAll(const Matrix4 *view)
{
    for (int i = 0; i < KEY_PROMPT_SLOT_COUNT; i++)
    {
        AnimModel *character = &g_characterModels[i];
        if (character->loadedRoot == 0 || g_keyPromptShown[i] == 0 ||
            !(character->matrix.m[MATRIX_TRANSLATION_X] > KEY_PROMPT_REMOVED_X))
        {
            g_keyPromptShown[i] = 0;
            continue;
        }
        Vec4 point;
        if (character->headNode != 0)
        {
            point.Copy(character->headNode->worldMatrix.GetTranslation());
            point.z += KEY_PROMPT_HEIGHT;
        }
        else
            point.Copy(character->matrix.GetTranslation());
        point.z += KEY_PROMPT_HEIGHT;
        int sheet = KEY_PROMPT_SHEET_5;
        int cell = g_keyPromptKey[i];
        if (cell >= KEY_PROMPT_SHEET_7_FIRST)
        {
            sheet = KEY_PROMPT_SHEET_7;
            cell -= KEY_PROMPT_SHEET_7_FIRST;
        }
        else if (cell >= KEY_PROMPT_SHEET_6_FIRST)
        {
            sheet = KEY_PROMPT_SHEET_6;
            cell -= KEY_PROMPT_SHEET_6_FIRST;
        }
        point.Copy(Project(point, *view));
        point.x = (point.x + 1.0f) * KEY_PROMPT_SCREEN_HALF_WIDTH;
        point.y = (2.0f - (point.y + 1.0f)) * KEY_PROMPT_SCREEN_HALF_HEIGHT;
        float u = (float)(cell % KEY_PROMPT_CELLS_PER_ROW) / KEY_PROMPT_CELLS_PER_ROW;
        float v = (float)(cell % KEY_PROMPT_CELLS_PER_SHEET / KEY_PROMPT_CELLS_PER_ROW) / KEY_PROMPT_CELLS_PER_ROW;
        int half = (int)((float)g_keyPromptSize / 2.0f + KEY_PROMPT_SIZE_MARGIN);
        if (point.z < KEY_PROMPT_DEPTH_NEAR || point.w < 0.0f || point.z > KEY_PROMPT_DEPTH_FAR ||
            point.z < KEY_PROMPT_DEPTH_NEAR)
            continue;
        g_textures[g_dialogGameSlots[sheet]].DrawTinted((int)(point.x - half), (int)(point.y - half),
                                                          (int)(point.x + half), (int)(point.y + half), u, v,
                                                          u + KEY_PROMPT_CELL_SIZE, v + KEY_PROMPT_CELL_SIZE, point.z,
                                                          1.0f, 0, 0, ASPECT_CENTRE);
    }
    return 0;
}
