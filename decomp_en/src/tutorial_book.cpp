// The tutorial book (EN build; JP twin APP.dll 1004f250 and neighbours).
#include <stdlib.h>
#include <windows.h>
#include "camera.h"
#include "char.h"
#include "engine_memory.h"
#include "game.h"
#include "game_vars.h"
#include "mouse.h"
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

// Index in CharState::values of slot 0 of the countdown the book mode ticks down every frame (a message delay).
const int BOOK_MESSAGE_DELAY_VALUE = 4;
// Book states (g_overlayModelValueG).
const int BOOK_STATE_IDLE = 0;
const int BOOK_STATE_TURN_FORWARD = 1;
const int BOOK_STATE_TURN_BACK = 2;
const int BOOK_STATE_OPENING = 6;
const int BOOK_STATE_OPENING_THEN_CLOSE = 7;
// Frames of the fade, of the opening and closing, of a page turn and of its half, and the frames after which the
// idle book accepts input again.
#define BOOK_FADE_FRAMES 20.0f
const int BOOK_OPEN_END_FRAME = 0x14;
const int BOOK_TURN_END_FRAME = 0x10;
const int BOOK_TURN_HALF_FRAME = 0xe;
const int BOOK_COOLDOWN_FRAMES = 0x3c;
// The page-turn frame counter wraps to 0 at this value.
const unsigned int BOOK_FRAME_LIMIT = 0xfffffffu;
// Frame the sheet and the half plugs restart on while the book opens.
const int BOOK_OPEN_SHEET_FRAME = 5;
const int BOOK_OPEN_HALF_FRAME = 6;
// Angle (about half a turn) the closed book is rotated by, and how far it flies away (z), while it fades.
#define BOOK_PI 3.14159f
#define BOOK_FLY_DISTANCE 100.0f
// Stick values beyond this count as a push; the mouse screen centre and the click distances that mean "outside the book".
const int BOOK_STICK_DEAD_ZONE = 3;
const int BOOK_CENTRE_X = 0x140;
const int BOOK_CENTRE_Y = 0xf0;
const int BOOK_CLICK_RADIUS_X = 0x112;
const int BOOK_CLICK_RADIUS_Y = 0xb6;
// Mouse button states: a fresh click.
const char BOOK_CLICK_PRESSED = 1;
const char BOOK_CLICK_RELEASED = 2;
// What the pad or mouse asks for.
const int BOOK_TURN_NONE = 0;
const int BOOK_TURN_NEXT = 1;
const int BOOK_TURN_PREV = 2;
const int BOOK_TURN_CLOSE = 5;
// Message frames of the "No special moves yet." message.
const int BOOK_MESSAGE_FRAMES = 0x28;
// Text shown when the second opening of the book ends.
const char BOOK_NO_MOVES_TEXT[] = "No special moves yet.";
// Virtual key that closes the book.
const int BOOK_CLOSE_VIRTUAL_KEY = VK_ESCAPE;
// Bits of the key state that say the key is down.
const int BOOK_KEY_DOWN_BITS = 0xf000;

// TutorialBook_Update: one frame of the tutorial book. Counts down the message delay and advances the turn frame counter;
// returns 0 with no book. Steps the book state by the frame counter: pages turning forward (1) or back (2) restart the
// half colour plugs on frames 1 and 0xe and end on 0x10; closing (5) fades the book out and frees it on frame 0x14; opening
// (6, or 7 = open then close) fades it in and restarts the sheet and half plugs on frames 5, 6 and 0x14. When the book is
// still, reads the pad (stick left/up = next, right/down = previous, the close key or Escape = close) or the mouse (a click
// on the right half = next, the left half = previous, the right button or a click far from the book = close) and starts
// the turn. Finally draws the book from the overlay camera, scaled and turned while it fades. Returns 1. Called every
// frame while the tutorial mode runs.
int TutorialBook_Update()
{
    if (g_charState[0].values[BOOK_MESSAGE_DELAY_VALUE] > 0)
        g_charState[0].values[BOOK_MESSAGE_DELAY_VALUE]--;
    g_tutorialTurnFrames++;
    if ((unsigned int)g_tutorialTurnFrames >= BOOK_FRAME_LIMIT)
        g_tutorialTurnFrames = 0;
    if (g_overlayModel == 0)
        return 0;
    if (g_overlayModelValueG != BOOK_STATE_IDLE)
        g_overlayModelValueF++;
    if (g_overlayModelValueG == BOOK_STATE_IDLE)
    {
        if (g_overlayModelValueF > 0)
        {
            g_overlayModelValueF++;
            if (g_overlayModelValueF > BOOK_COOLDOWN_FRAMES)
                g_overlayModelValueF = 0;
        }
    }
    else if (g_overlayModelValueG == BOOK_STATE_TURN_FORWARD)
    {
        if (g_overlayModelValueF == 1)
        {
            if (g_overlayModelValueB == 0)
                goto draw;
            g_overlayModelValueB->RestartColourPlugs("stop1", g_tutorialTurnFrames * 2);
        }
        if (g_overlayModelValueF == BOOK_TURN_HALF_FRAME)
        {
            if (g_overlayModelValueC == 0)
                goto draw;
            g_overlayModelValueC->RestartColourPlugs("stop1", g_tutorialTurnFrames * 2);
        }
        if (g_overlayModelValueF == BOOK_TURN_END_FRAME)
        {
            g_overlayModelValueF = 0;
            g_overlayModelValueG = BOOK_STATE_IDLE;
            return 0;
        }
    }
    else if (g_overlayModelValueG == BOOK_STATE_TURN_BACK)
    {
        if (g_overlayModelValueF == 1)
        {
            if (g_overlayModelValueC == 0)
                goto draw;
            g_overlayModelValueC->RestartColourPlugs("stop0", g_tutorialTurnFrames * 2);
        }
        if (g_overlayModelValueF == BOOK_TURN_HALF_FRAME)
        {
            if (g_overlayModelValueB == 0)
                goto draw;
            g_overlayModelValueB->RestartColourPlugs("stop0", g_tutorialTurnFrames * 2);
        }
        if (g_overlayModelValueF == BOOK_TURN_END_FRAME)
        {
            g_overlayModelValueF = 0;
            g_overlayModelValueG = BOOK_STATE_IDLE;
            return 0;
        }
    }
    else if (g_overlayModelValueG == TUTORIAL_STATE_CLOSING)
    {
        g_overlayProgress = 1.0f - (float)g_overlayModelValueF / BOOK_FADE_FRAMES;
        if (g_overlayModelValueF == BOOK_TURN_HALF_FRAME)
        {
            if (g_overlayModelValueB == 0)
                goto draw;
            g_overlayModelValueB->RestartColourPlugs("stop0", g_tutorialTurnFrames * 2);
        }
        if (g_overlayModelValueF == BOOK_OPEN_END_FRAME)
        {
            g_overlayModelValueD = 0;
            if (g_overlayModel != 0)
            {
                g_overlayModel->Release();
                g_engineFree(g_overlayModel);
                g_overlayModel = 0;
            }
            g_overlayModelValueA = 0;
            g_overlayModelValueB = 0;
            g_overlayModelValueC = 0;
            g_overlayModelValueE = 0;
            g_overlayModelValueF = 0;
            g_overlayModelValueG = BOOK_STATE_IDLE;
            for (int page = 0; page < OVERLAY_TEXTURE_COUNT; page++)
            {
                if (g_overlayTextures[page] > -1)
                {
                    Texture_Release(g_overlayTextures[page], 1);
                    g_overlayTextures[page] = -1;
                }
            }
            return 0;
        }
    }
    else if (g_overlayModelValueG == BOOK_STATE_OPENING || g_overlayModelValueG == BOOK_STATE_OPENING_THEN_CLOSE)
    {
        g_overlayProgress = (float)g_overlayModelValueF / BOOK_FADE_FRAMES;
        if (g_overlayModelValueF == BOOK_OPEN_SHEET_FRAME)
        {
            if (g_overlayModelValueA == 0)
                goto draw;
            g_overlayModelValueA->RestartColourPlugs("pageup", g_tutorialTurnFrames * 2);
        }
        else if (g_overlayModelValueF == BOOK_OPEN_HALF_FRAME)
        {
            if (g_overlayModelValueB == 0)
                goto draw;
            g_overlayModelValueB->RestartColourPlugs("stop1", g_tutorialTurnFrames * 2);
        }
        else if (g_overlayModelValueF == BOOK_OPEN_END_FRAME)
        {
            if (g_overlayModelValueC != 0)
                g_overlayModelValueC->RestartColourPlugs("stop1", g_tutorialTurnFrames * 2);
            if (g_overlayModelValueA != 0)
                g_overlayModelValueA->RestartColourPlugs("stop1", g_tutorialTurnFrames * 2);
            if (g_overlayModelValueE != 0)
                g_overlayModelValueE->RestartColourPlugs("stop0", g_tutorialTurnFrames * 2);
            if (g_overlayModelValueG == BOOK_STATE_OPENING_THEN_CLOSE)
            {
                Message_Show(BOOK_NO_MOVES_TEXT, 0, BOOK_MESSAGE_FRAMES);
                TutorialBook_Close();
                g_overlayModelValueF = 0;
                goto draw;
            }
            g_overlayModelValueG = BOOK_STATE_IDLE;
            g_overlayModelValueF = 0;
            goto read_input;
        }
    }
    if (g_overlayModelValueF != 0)
        goto draw;
read_input:
    {
        int turn = BOOK_TURN_NONE;
        char left = 0;
        char right = 0;
        int stickX = g_inputStickX;
        int stickY = g_inputStickY;
        if (stickX > BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_NEXT;
        else if (stickX < -BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_PREV;
        else if (stickY > BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_PREV;
        else if (stickY < -BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_NEXT;
        if (g_inputCloseButton != 0 || (GetKeyState(BOOK_CLOSE_VIRTUAL_KEY) & BOOK_KEY_DOWN_BITS) != 0)
            turn = BOOK_TURN_CLOSE;
        if (g_inputMouseDisabled == 0)
        {
            MousePoint mouse;
            int wheel;
            if (g_inputMouseButtonA != 0 || g_inputMouseButtonB != 0 || turn != BOOK_TURN_NONE)
                g_mouse.ClearChanged();
            g_mouse.UpdateMoved();
            g_mouse.GetState(&mouse, &left, 0, &right, &wheel, 0, 0);
            if (((abs(mouse.x - BOOK_CENTRE_X) > BOOK_CLICK_RADIUS_X || abs(mouse.y - BOOK_CENTRE_Y) > BOOK_CLICK_RADIUS_Y) &&
                 left == BOOK_CLICK_RELEASED) ||
                right == BOOK_CLICK_RELEASED)
            {
                TutorialBook_Close();
                goto draw;
            }
            if (left == BOOK_CLICK_PRESSED)
            {
                if (BOOK_CENTRE_X - mouse.x <= 0)
                    TutorialBook_NextPage();
                else
                    TutorialBook_PreviousPage();
                goto draw;
            }
        }
        if (turn == BOOK_TURN_NEXT)
            TutorialBook_NextPage();
        else if (turn == BOOK_TURN_PREV)
            TutorialBook_PreviousPage();
        else if (turn == BOOK_TURN_CLOSE)
            TutorialBook_Close();
    }
draw:
{
    Matrix4 placement;
    Matrix4 world;
    Matrix4 view;
    Matrix4 projection;
    Matrix4 camera;
    world.SetIdentity();
    Camera_SetMatrices(&g_overlayCameraEye, &g_overlayCameraTarget, &camera, &projection, 0);
    view = camera * projection;
    if (g_overlayProgress < 1.0f)
    {
        float opacity = g_overlayProgress;
        float rest = 1.0f - opacity;
        placement = ScaleMatrix(opacity, opacity, opacity) * RotationY(rest * BOOK_PI);
        placement.m[MATRIX_TRANSLATION_Z] = rest * BOOK_FLY_DISTANCE;
    }
    else
        placement.SetIdentity();
    g_overlayModel->StartProximityPass(placement.m, &view, &view, g_tutorialTurnFrames * 2, 0);
    return 1;
}
}
