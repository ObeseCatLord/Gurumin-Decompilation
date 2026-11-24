// Head tracking of the player character (EN build). JP twin: head_look.cpp.
#include "head_look.h"
#include "node.h"
#include "node_data.h"

// Moves the head-look target to a point 1000 units to one side of the player and 500 units up, switching sides every
// ten calls, and (re)starts head tracking. Returns 0. Called every frame while the player looks around.
extern "C" int Player_LookAround(void)
{
    g_headLookTarget.y = LOOK_TARGET_SIDE;
    g_headLookTarget.z = LOOK_TARGET_UP;
    if (g_lookAroundCounter >= 0)
    {
        g_headLookTarget.RotateZ(LOOK_QUARTER_TURN - g_characterModels[PLAYER_SLOT].heading);
        g_headLookTarget.Copy(g_headLookTarget + g_characterModels[PLAYER_SLOT].matrix.GetTranslation());
        int next = g_lookAroundCounter + 1;
        if (next >= LOOK_SWITCH_COUNT)
            next = -1;
        g_lookAroundCounter = next;
    }
    else
    {
        g_headLookTarget.RotateZ(-g_characterModels[PLAYER_SLOT].heading - LOOK_QUARTER_TURN);
        g_headLookTarget.Copy(g_headLookTarget + g_characterModels[PLAYER_SLOT].matrix.GetTranslation());
        int next = g_lookAroundCounter - 1;
        if (next <= -LOOK_SWITCH_COUNT)
            next = 1;
        g_lookAroundCounter = next;
    }
    Player_SetHeadTracking(HEAD_TRACK_START);
    return 0;
}

// Player_SetHeadTracking: mode 1 starts head tracking, limited to +-1.22 rad around x, +-0.27 around y and +-0.17
// around z; mode 0 lets the head return to rest; mode -5 removes the tracking block. Returns 0.
extern "C" int Player_SetHeadTracking(int mode)
{
    if (g_characterModels[PLAYER_SLOT].headNode != 0)
    {
        if (mode == HEAD_TRACK_START)
        {
            g_characterModels[PLAYER_SLOT].headNode->parent->AllocSaved();
            g_characterModels[PLAYER_SLOT].headNode->parent->saved->limitMin[0] = -HEAD_LIMIT_X;
            g_characterModels[PLAYER_SLOT].headNode->parent->saved->limitMin[1] = -HEAD_LIMIT_Y;
            g_characterModels[PLAYER_SLOT].headNode->parent->saved->limitMin[2] = -HEAD_LIMIT_Z;
            g_characterModels[PLAYER_SLOT].headNode->parent->saved->limitMax[0] = HEAD_LIMIT_X;
            g_characterModels[PLAYER_SLOT].headNode->parent->saved->limitMax[1] = HEAD_LIMIT_Y;
            g_characterModels[PLAYER_SLOT].headNode->parent->saved->limitMax[2] = HEAD_LIMIT_Z;
            g_characterModels[PLAYER_SLOT].headNode->parent->EnterTargetState(0);
            return 0;
        }
        if (mode == HEAD_TRACK_RETURN)
        {
            if (g_characterModels[PLAYER_SLOT].headNode->parent->saved != 0)
            {
                g_characterModels[PLAYER_SLOT].headNode->parent->saved->state = SAVED_RETURNING;
                return 0;
            }
        }
        else if (mode == HEAD_TRACK_REMOVE && g_characterModels[PLAYER_SLOT].headNode->parent->saved != 0)
            g_characterModels[PLAYER_SLOT].headNode->parent->FreeSaved();
    }
    return 0;
}
