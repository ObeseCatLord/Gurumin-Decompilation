// A sinking block (EN build; JP twin: pushing_platform.cpp, unmatched there).
#include "anim_model.h"
#include "camera.h"
#include "char.h"
#include "char_shots.h"
#include "vec4.h"

// The character slot whose health falling below half drops the waiting blocks.
const int BLOCK_TRIGGER_SLOT = 59;
// CharState::state of a block: waiting above the map, waiting for the trigger, dropped.
const int BLOCK_STATE_NEW = 0;
const int BLOCK_STATE_WAITING = 1;
const int BLOCK_STATE_DROPPED = 2;
// Height a new block waits at, and the height it drops to.
const float BLOCK_WAIT_HEIGHT = 5000.0f;
const float BLOCK_DROP_HEIGHT = -2000.0f;
// Below this height a block rises by BLOCK_RISE_STEP a frame.
const float BLOCK_RISE_BELOW = -120.0f;
const float BLOCK_RISE_STEP = 5.0f;
// Node state set on the block's model (masks 0x200 and 0x400).
const unsigned char BLOCK_NODE_COLOUR = 0x41;
const unsigned int BLOCK_NODE_MASK_A = 0x200;
const unsigned int BLOCK_NODE_MASK_B = 0x400;
// Player on the block: lifted to block height plus this, moved outwards this far; the player must be above this height.
const float BLOCK_LIFT = 12.0f;
const float BLOCK_PUSH_OUT = 3.0f;
const float BLOCK_PLAYER_MIN_HEIGHT = -140.0f;
// Player blocked by the block: moved this far towards the block's centre.
const float BLOCK_PUSH_IN = 10.0f;
// Behaviour mode of a standing player.
const int MODE_STANDING = 2;

// Block_Update: one frame of block `index`. On the first call it is listed with the positions the camera keeps in
// view, placed 5000 units up, and node state 0x41 is set (masks 0x200 and 0x400). While waiting it drops to z -2000
// once the health of character slot 59 is below half of its maximum. Below z -120 it rises 5 units a frame; above it
// pushes the player away from its centre: while the player stands on something and the reversed player move is
// clear, the player is lifted onto the block and moved 3 units outwards, otherwise, when the block is blocked, it is
// moved 10 units towards the block's centre. The position is remembered for the next frame. Returns 0. Called by the
// character update (EN 0x6a0950) for block characters.
extern "C" int Block_Update(int index)
{
    if (g_charState[index].state == BLOCK_STATE_NEW)
    {
        g_trackedPositions[g_trackedCount] = &g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        g_trackedFlags[g_trackedCount] = 0;
        g_trackedCount++;
        g_charState[index].state = BLOCK_STATE_WAITING;
        g_characterModels[index].SetPosition(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                                             g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                                             BLOCK_WAIT_HEIGHT);
        g_characterModels[index].root->SetColourB(BLOCK_NODE_COLOUR, BLOCK_NODE_MASK_A);
        g_characterModels[index].root->SetColourB(BLOCK_NODE_COLOUR, BLOCK_NODE_MASK_B);
    }
    else if (g_charState[index].state == BLOCK_STATE_WAITING)
    {
        if (g_charRecords[BLOCK_TRIGGER_SLOT].stats[STAT_HEALTH] <
            g_charRecords[BLOCK_TRIGGER_SLOT].stats[STAT_MAX_HEALTH] / 2)
        {
            g_charState[index].state = BLOCK_STATE_DROPPED;
            g_characterModels[index].SetPosition(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                                                 g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                                                 BLOCK_DROP_HEIGHT);
        }
    }
    else if (g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] < BLOCK_RISE_BELOW)
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] += BLOCK_RISE_STEP;
    else
    {
        Vec4 player;
        Vec4 block;
        Vec4 away;
        player.Copy(g_characterModels[0].matrix.GetTranslation());
        block.Copy(g_characterModels[index].matrix.GetTranslation());
        away.Copy(block - player);
        if (away.Length2D() > 1.0f)
            away.Normalized3();
        else
            away.x = 1.0f;
        bool blocked = true;
        if (g_characterModels[0].mode == MODE_STANDING)
        {
            if (g_characterModels[index].IsClearAny(&(g_characterModels[0].move * -1.0f)) == 0)
            {
                if (g_characterModels[0].mode == MODE_STANDING &&
                    g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Z] > BLOCK_PLAYER_MIN_HEIGHT)
                {
                    g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Z] = block.z + BLOCK_LIFT;
                    g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] =
                        away.x * BLOCK_PUSH_OUT + g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X];
                    g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] =
                        away.y * BLOCK_PUSH_OUT + g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y];
                    blocked = false;
                }
            }
        }
        if (blocked && g_characterModels[index].IsClearAny(0) == 0)
        {
            g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] =
                g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] - away.x * BLOCK_PUSH_IN;
            g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] =
                g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] - away.y * BLOCK_PUSH_IN;
        }
    }
    g_shotSlots[index].position.Copy(g_characterModels[index].matrix.GetTranslation());
    return 0;
}
