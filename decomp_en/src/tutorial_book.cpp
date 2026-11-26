// The tutorial book (EN build; JP twin APP.dll 1004f250 and neighbours).
#include "game.h"
#include "node.h"
#include "sound_effects.h"
#include "texture_load.h"
#include "texture_mgr.h"
#include "tutorial_book.h"

// Sound effect of the book opening and closing.
const int TUTORIAL_BOOK_SOUND = 0x3bd;

// Starts closing the book (state 5): loads the cover texture and, from the left page on, the first two pages that exist
// (a page locked in the second book shows the placeholder); the sheet turns back ("pagedown") with the first page and
// the cover, the halves stop ("stop0" with the second page, "stop2") and the cover closes ("close"); the book sound
// plays. Returns 0. Called when the tutorial mode is left.
extern "C" int TutorialBook_Close(void)
{
    g_overlayModelValueG = TUTORIAL_STATE_CLOSING;
    g_overlayModelValueF = 0;
    int cover = LoadTexture(g_tutorialPageNames[0], 0);
    int found = 0;
    g_overlayTextures[0] = cover;
    int left = cover;
    int right = cover;
    for (int i = g_tutorialLeftPage; i < TUTORIAL_PAGE_COUNT; i++)
    {
        if (g_overlayPages[i] == 0)
            continue;
        if (found == 0)
        {
            if (g_tutorialSecondBook == 0 || g_overlayPages[i] == TUTORIAL_PAGE_ALWAYS)
            {
                left = LoadTexture(g_tutorialPageNames[i], 0);
                g_overlayTextures[i] = left;
            }
            else if (g_overlayPages[i] > 0 && g_game.TestScriptFlag(g_overlayPages[i]) != 0)
            {
                left = Texture_Load(g_tutorialPageNames[i], 0);
                g_overlayTextures[i] = left;
            }
            else
            {
                left = Texture_Load(g_tutorialPageNames[TUTORIAL_LOCKED_PAGE], 0);
                g_overlayTextures[TUTORIAL_LOCKED_PAGE] = left;
            }
            found = 1;
        }
        else
        {
            if (g_tutorialSecondBook == 0 || g_overlayPages[i] == TUTORIAL_PAGE_ALWAYS)
            {
                right = LoadTexture(g_tutorialPageNames[i], 0);
                g_overlayTextures[i] = right;
            }
            else if (g_overlayPages[i] > 0 && g_game.TestScriptFlag(g_overlayPages[i]) != 0)
            {
                right = Texture_Load(g_tutorialPageNames[i], 0);
                g_overlayTextures[i] = right;
            }
            else
            {
                right = Texture_Load(g_tutorialPageNames[TUTORIAL_LOCKED_PAGE], 0);
                g_overlayTextures[TUTORIAL_LOCKED_PAGE] = right;
            }
            break;
        }
    }
    if (g_overlayModelValueA != 0)
    {
        g_overlayModelValueA->SetMaterialValue("paper2", left);
        g_overlayModelValueA->SetMaterialValue("paper", cover);
        g_overlayModelValueA->RestartColourPlugs("pagedown", g_tutorialTurnFrames * 2);
    }
    if (g_overlayModelValueB != 0)
    {
        g_overlayModelValueB->SetMaterialValue("paper", right);
        g_overlayModelValueB->RestartColourPlugs("stop0", g_tutorialTurnFrames * 2);
    }
    if (g_overlayModelValueC != 0)
        g_overlayModelValueC->RestartColourPlugs("stop2", g_tutorialTurnFrames * 2);
    if (g_overlayModelValueE != 0)
        g_overlayModelValueE->RestartColourPlugs("close", g_tutorialTurnFrames * 2);
    g_textureManager.ResetFrameTimer();
    Sound_PlayEffect(TUTORIAL_BOOK_SOUND, 0, 1.0f);
    return 0;
}
