// UI animation control (EN build). Ported from the matched JP source decomp/src/app_0013.cpp.
#include "ui_anim.h"
#include "anim_model.h"
#include "char.h"
#include "node.h"

// UiAnim_Play: plays `node`'s animation forwards or backwards (`reverse`) and remembers it as the current UI
// animation: forwards starts at frame 0 stepping +1, backwards at the last frame stepping -1. Called by the menus.
extern "C" int UiAnim_Play(Node *node, int reverse)
{
    g_uiAnimNode = node;
    if (reverse != 0)
    {
        g_uiAnimFrame = node->frameMatrixCount - 1;
        g_uiAnimStep = -1;
    }
    else
    {
        g_uiAnimFrame = 0;
        g_uiAnimStep = 1;
    }
    node->SetFlags(NODE_FLAG_UI_ANIM);
    g_characterModels[0].uiAnimStarted = 1;
    return 0;
}
