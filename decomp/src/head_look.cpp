// Head tracking of the player character: the head bone's parent node gets a saved-state block whose rotation limits
// bound how far the head may turn towards its target (Node::M_101948f0 enters the tracking state 0x400000).

#include "vec3.h"

#include "node_data.h"

#include "node.h"

#include "char_vars.h"

// mode 1: starts head tracking, limited to +-1.22 rad around x, +-0.27 around y and +-0.17 around z;
// mode 0: lets the head return to rest; mode -5: removes the tracking block. Returns 0.
extern "C" int FUN_100165e0(int mode)
{
    if (DAT_10db91dc != 0)
    {
        if (mode == 1)
        {
            DAT_10db91dc->parent->AllocSaved();
            DAT_10db91dc->parent->saved->limitMin.x = -1.22f;
            DAT_10db91dc->parent->saved->limitMin.y = -0.27f;
            DAT_10db91dc->parent->saved->limitMin.z = -0.17f;
            DAT_10db91dc->parent->saved->limitMax.x = 1.22f;
            DAT_10db91dc->parent->saved->limitMax.y = 0.27f;
            DAT_10db91dc->parent->saved->limitMax.z = 0.17f;
            DAT_10db91dc->parent->EnterTargetState(0);
            return 0;
        }
        if (mode == 0)
        {
            if (DAT_10db91dc->parent->saved != 0)
            {
                DAT_10db91dc->parent->saved->state = SAVED_RETURNING;
                return 0;
            }
        }
        else if (mode == -5 && DAT_10db91dc->parent->saved != 0)
            DAT_10db91dc->parent->FreeSaved();
    }
    return 0;
}

// The look-around target point and the player's position and heading.
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

// 0x10019210 (Player_LookAround). Moves the head-look target to a point 1000 units to one side of the player and
// 500 units up, switching sides every ten calls, and (re)starts head tracking. Returns 0.
extern "C" int FUN_10019210(void)
{
    DAT_10802ac8.z = 500.0f;
    DAT_10802ac8.y = -1000.0f;
    if (DAT_10c6358c >= 0)
    {
        DAT_10802ac8.RotateZ(1.5708f - DAT_10db9364);
        DAT_10802ac8.Copy(DAT_10802ac8 + DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
        DAT_10c6358c++;
        if (DAT_10c6358c >= 10)
        {
            DAT_10c6358c = -1;
            FUN_100165e0(1);
            return 0;
        }
    }
    else
    {
        DAT_10802ac8.RotateZ(-DAT_10db9364 - 1.5708f);
        DAT_10802ac8.Copy(DAT_10802ac8 + DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
        DAT_10c6358c--;
        if (DAT_10c6358c <= -10)
            DAT_10c6358c = 1;
    }
    FUN_100165e0(1);
    return 0;
}
