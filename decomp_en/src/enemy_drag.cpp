// An enemy dragged towards the player and into a pit (EN build). Ported from the JP source (APP.dll 0x100b9930, open
// there) and the EN bytes.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_move.h"
#include "head_look.h"
#include "map_view.h"
#include "vec4.h"

// Distance of one drag step along the facing (the y of the move vector, turned by the heading).
const float DRAG_STEP = -180.0f;
// Motion played while dragged: layer, blend start and frames.
const int DRAG_MOTION_LAYER = 1;
const int DRAG_MOTION_FRAMES = 5;
// z of the move while the model is in the pit mode, and of the clear-way probe.
const float DRAG_PIT_LIFT = 5.0f;
const float DRAG_PROBE_LIFT = 10.0f;
// Frames after which the drag is over (the model is blocked or fades out), and the frame from which it is moved away.
const int DRAG_FADE_FRAME = 0x96;
const int DRAG_REMOVE_FRAME = 0xa5;
// Alpha lost per frame while fading (1/15).
const float DRAG_FADE_STEP = 0.066667f;
// x the removed model is moved to.
const float DRAG_REMOVED_X = -200000.0f;
// The model counts as in the pit when it is this far below the pit floor.
const float DRAG_PIT_DEPTH = 25.0f;
// AnimModel::mode values: outside the pit area and inside it.
const int DRAG_MODE_OUT = 1;
const int DRAG_MODE_IN_PIT = 2;

// Turns character `index` towards the player and moves it 180 units per step along its facing (lifted by 5 over the
// pit), counting the frames. After 150 frames (or once it is blocked) it fades out by 1/15 per frame and from 165 it
// is moved away (x -200000). While its height is 25 below the pit floor and its x/y lie inside the pit area its mode is
// 2, else 1. Returns 0. Called by the pit event behaviour at EN 0x6d1b40.
extern "C" int Enemy_DragToPit(int index)
{
    g_charRecords[index].stats[STAT_BUSY] = 1;
    g_charRecords[index].defeated = 1;
    g_charRespawned[index] = 0;
    Vec4 player;
    Vec4 position;
    Vec4 toPlayer;
    Vec4 move;
    player.Copy(g_characterModels[PLAYER_SLOT].matrix.GetTranslation());
    position.Copy(g_characterModels[index].matrix.GetTranslation());
    toPlayer.Copy(player - position);
    g_charState[index].values[0]++;
    g_characterModels[index].PlayMotion(DRAG_MOTION_LAYER, 0, DRAG_MOTION_FRAMES, 1.0f);
    g_characterModels[index].targetHeading = atan2f(toPlayer.x, toPlayer.y);
    move.y = DRAG_STEP;
    move.RotateZ(-g_characterModels[index].heading);
    if (g_characterModels[index].mode == DRAG_MODE_IN_PIT)
        move.z = DRAG_PIT_LIFT;
    Character_SetMove(index, move.x, move.y, move.z);
    move.z = DRAG_PROBE_LIFT;
    if (g_charState[index].values[0] < DRAG_FADE_FRAME && g_characterModels[index].IsClearOfMapLimited(&move) == 0)
        g_charState[index].values[0] = DRAG_FADE_FRAME;
    int frames = g_charState[index].values[0];
    if (frames > DRAG_FADE_FRAME)
    {
        g_characterModels[index].mode = DRAG_MODE_OUT;
        float alpha = g_characterModels[index].alpha - DRAG_FADE_STEP;
        g_characterModels[index].alpha = alpha;
        if (alpha < 0.0f)
            g_characterModels[index].alpha = 0;
    }
    if (frames >= DRAG_REMOVE_FRAME)
    {
        g_characterModels[index].SetPosition(DRAG_REMOVED_X, 0, 0);
        g_charMarkers[index].x = DRAG_REMOVED_X;
    }
    if (g_mapBottomValid != 0 && g_mapBottomZ - DRAG_PIT_DEPTH > g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z])
    {
        int y = (int)g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y];
        int x = (int)g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        if (x >= (int)g_mapPitMin.x && y >= (int)g_mapPitMin.y && x <= (int)g_mapPitMax.x && y <= (int)g_mapPitMax.y)
        {
            g_characterModels[index].mode = DRAG_MODE_IN_PIT;
            return 0;
        }
    }
    g_characterModels[index].mode = DRAG_MODE_OUT;
    return 0;
}
