// Running the tutorial book one frame at a time (the book mode's per-frame function): the state machine of the
// book (closed, turning forward, turning back, closing, opening), the mouse and key input that starts a turn, and
// the drawing of the book model from the overlay camera while it opens and closes.

#include <stdlib.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "tracker.h"
#include "engine_memory.h"
#include "render_vars.h"
#include "game_vars.h"
#include "ui_vars.h"
#include "tutorial_book.h"

extern "C" int FUN_1000ed70(const char *text, int style, int frames);  // shows a message
extern "C" int FUN_10174c70(int slot, int force);                      // Texture_Release
extern "C" void FUN_10194360(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, int a);  // Camera_SetMatrices
Matrix4 RotationY(float angle);                                        // 0x101b1bb0
Matrix4 Scaling(float x, float y, float z);                            // 0x101b13c0

// Book states (DAT_104c03e4).
#define BOOK_STATE_IDLE 0
#define BOOK_STATE_TURN_FORWARD 1
#define BOOK_STATE_TURN_BACK 2
#define BOOK_STATE_CLOSING 5
#define BOOK_STATE_OPENING 6
#define BOOK_STATE_OPENING_THEN_CLOSE 7

#define BOOK_FADE_FRAMES 20.0f
#define BOOK_OPEN_END_FRAME 0x14
#define BOOK_TURN_END_FRAME 0x10
#define BOOK_TURN_HALF_FRAME 0xe
#define BOOK_COOLDOWN_FRAMES 0x3c
#define BOOK_FRAME_LIMIT 0xfffffffu
#define BOOK_PAGE_SLOT_COUNT 0x22
#define BOOK_PI 3.14159f
#define BOOK_FLY_DISTANCE 100.0f
#define BOOK_STICK_DEAD_ZONE 3
#define BOOK_CENTRE_X 0x140
#define BOOK_CENTRE_Y 0xf0
#define BOOK_CLICK_RADIUS_X 0x112
#define BOOK_CLICK_RADIUS_Y 0xb6
#define BOOK_TURN_NONE 0
#define BOOK_TURN_NEXT 1
#define BOOK_TURN_PREV 2
#define BOOK_TURN_CLOSE 5

// Shift-JIS "I do not remember anything." Shown when the book closes after the second opening.
#define BOOK_NO_MEMORY_TEXT "\x89\xbd\x82\xe0\x8a\x6f\x82\xa6\x82\xc4\x82\xa2\x82\xdc\x82\xb9\x82\xf1"

// 0x10065450 (TutorialBook_Update). One frame of the tutorial book. Counts down the message delay and advances the
// frame counter DAT_102239c8; returns 0 with no book. Steps the book state by the frame DAT_104c03e0: pages turning
// forward (1) or back (2) restart the half colour plugs on frames 1 and 0xe and end on 0x10; closing (5) fades the
// book out and frees it on frame 0x14; opening (6, or 7 = open then close) fades it in and restarts the sheet and
// half plugs on frames 5, 6 and 0x14. When the book is still, reads the pad (stick left/up = next, right/down =
// previous, DAT_1065cb31 = close) or the mouse (a click on the right half = next, the left half = previous, the
// right button or a click far from the book = close) and starts the turn. Finally draws the book from the overlay
// camera, scaled and turned while it fades. A C++ function: its temporary matrices need an EH frame. Returns 1.
int TutorialBook_Update()
{
    if (DAT_1020b444 > 0)
        DAT_1020b444--;
    DAT_102239c8++;
    if ((unsigned int)DAT_102239c8 >= BOOK_FRAME_LIMIT)
        DAT_102239c8 = 0;
    if (DAT_104c03c8 == 0)
        return 0;
    if (DAT_104c03e4 != BOOK_STATE_IDLE)
        DAT_104c03e0++;   // the frame counter runs in every state but idle
    if (DAT_104c03e4 == BOOK_STATE_IDLE)
    {
        if (DAT_104c03e0 > 0)
        {
            DAT_104c03e0++;
            if (DAT_104c03e0 > BOOK_COOLDOWN_FRAMES)
                DAT_104c03e0 = 0;
        }
    }
    else if (DAT_104c03e4 == BOOK_STATE_TURN_FORWARD)
    {
        if (DAT_104c03e0 == 1)
        {
            if (DAT_104c03d0 == 0)
                goto draw;
            DAT_104c03d0->RestartColourPlugs("stop1", DAT_102239c8 * 2);
        }
        if (DAT_104c03e0 == BOOK_TURN_HALF_FRAME)
        {
            if (DAT_104c03d4 == 0)
                goto draw;
            DAT_104c03d4->RestartColourPlugs("stop1", DAT_102239c8 * 2);
        }
        if (DAT_104c03e0 == BOOK_TURN_END_FRAME)
        {
            DAT_104c03e0 = 0;
            DAT_104c03e4 = BOOK_STATE_IDLE;
            return 0;
        }
    }
    else if (DAT_104c03e4 == BOOK_STATE_TURN_BACK)
    {
        if (DAT_104c03e0 == 1)
        {
            if (DAT_104c03d4 == 0)
                goto draw;
            DAT_104c03d4->RestartColourPlugs("stop0", DAT_102239c8 * 2);
        }
        if (DAT_104c03e0 == BOOK_TURN_HALF_FRAME)
        {
            if (DAT_104c03d0 == 0)
                goto draw;
            DAT_104c03d0->RestartColourPlugs("stop0", DAT_102239c8 * 2);
        }
        if (DAT_104c03e0 == BOOK_TURN_END_FRAME)
        {
            DAT_104c03e0 = 0;
            DAT_104c03e4 = BOOK_STATE_IDLE;
            return 0;
        }
    }
    else if (DAT_104c03e4 == BOOK_STATE_CLOSING)
    {
        DAT_104c03ec = 1.0f - (float)DAT_104c03e0 / BOOK_FADE_FRAMES;
        if (DAT_104c03e0 == BOOK_TURN_HALF_FRAME)
        {
            if (DAT_104c03d0 == 0)
                goto draw;
            DAT_104c03d0->RestartColourPlugs("stop0", DAT_102239c8 * 2);
        }
        if (DAT_104c03e0 == BOOK_OPEN_END_FRAME)
        {
            DAT_104c03dc = 0;
            if (DAT_104c03c8 != 0)
            {
                DAT_104c03c8->Release();
                DAT_11b1ba40(DAT_104c03c8);
                DAT_104c03c8 = 0;
            }
            DAT_104c03cc = 0;
            DAT_104c03d0 = 0;
            DAT_104c03d4 = 0;
            DAT_104c03d8 = 0;
            DAT_104c03e0 = 0;
            DAT_104c03e4 = BOOK_STATE_IDLE;
            for (int page = 0; page < BOOK_PAGE_SLOT_COUNT; page++)
            {
                if (DAT_1030b870[page] > -1)
                {
                    FUN_10174c70(DAT_1030b870[page], 1);
                    DAT_1030b870[page] = -1;
                }
            }
            return 0;
        }
    }
    else if (DAT_104c03e4 == BOOK_STATE_OPENING || DAT_104c03e4 == BOOK_STATE_OPENING_THEN_CLOSE)
    {
        DAT_104c03ec = (float)DAT_104c03e0 / BOOK_FADE_FRAMES;
        if (DAT_104c03e0 == 5)
        {
            if (DAT_104c03cc == 0)
                goto draw;
            DAT_104c03cc->RestartColourPlugs("pageup", DAT_102239c8 * 2);
        }
        else if (DAT_104c03e0 == 6)
        {
            if (DAT_104c03d0 == 0)
                goto draw;
            DAT_104c03d0->RestartColourPlugs("stop1", DAT_102239c8 * 2);
        }
        else if (DAT_104c03e0 == BOOK_OPEN_END_FRAME)
        {
            if (DAT_104c03d4 != 0)
                DAT_104c03d4->RestartColourPlugs("stop1", DAT_102239c8 * 2);
            if (DAT_104c03cc != 0)
                DAT_104c03cc->RestartColourPlugs("stop1", DAT_102239c8 * 2);
            if (DAT_104c03d8 != 0)
                DAT_104c03d8->RestartColourPlugs("stop0", DAT_102239c8 * 2);
            if (DAT_104c03e4 == BOOK_STATE_OPENING_THEN_CLOSE)
            {
                FUN_1000ed70(BOOK_NO_MEMORY_TEXT, 0, 0x28);
                FUN_1004f250();
                DAT_104c03e0 = 0;
                goto draw;
            }
            DAT_104c03e4 = BOOK_STATE_IDLE;
            DAT_104c03e0 = 0;
            goto read_input;
        }
    }
    if (DAT_104c03e0 != 0)
        goto draw;
read_input:
    {
        int turn = BOOK_TURN_NONE;
        char left = 0;
        char right = 0;
        int stickX = DAT_1065cb48;
        int stickY = DAT_1065cb4c;
        if (stickX > BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_NEXT;
        else if (stickX < -BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_PREV;
        else if (stickY > BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_PREV;
        else if (stickY < -BOOK_STICK_DEAD_ZONE)
            turn = BOOK_TURN_NEXT;
        if (DAT_1065cb31 != 0)
            turn = BOOK_TURN_CLOSE;
        if (DAT_1065cb90 == 0)
        {
            int mouse[2];
            int wheel;
            if (DAT_1065cb3e != 0 || DAT_1065cb3f != 0 || turn != BOOK_TURN_NONE)
                DAT_1020d9e0.M_100014a0();
            DAT_1020d9e0.M_10001450();
            DAT_1020d9e0.M_10001110(mouse, &left, 0, &right, &wheel, 0);
            if (((abs(mouse[0] - BOOK_CENTRE_X) > BOOK_CLICK_RADIUS_X ||
                  abs(mouse[1] - BOOK_CENTRE_Y) > BOOK_CLICK_RADIUS_Y) &&
                 left == 2) ||
                right == 2)
            {
                FUN_1004f250();
                goto draw;
            }
            if (left == 1)
            {
                if (BOOK_CENTRE_X - mouse[0] <= 0)
                    FUN_1004f4f0();
                else
                    FUN_1004f860();
                goto draw;
            }
        }
        if (turn == BOOK_TURN_NEXT)
            FUN_1004f4f0();
        else if (turn == BOOK_TURN_PREV)
            FUN_1004f860();
        else if (turn == BOOK_TURN_CLOSE)
            FUN_1004f250();
    }
draw:
    {
    Matrix4 placement;
    Matrix4 world;
    Matrix4 view;
    Matrix4 projection;
    Matrix4 camera;
    world.SetIdentity();
    FUN_10194360(&DAT_10c66edc, &DAT_10edbdf8, &camera, &projection, 0);
    view = camera * projection;
    if (DAT_104c03ec < 1.0f)
    {
        float opacity = DAT_104c03ec;
        float rest = 1.0f - opacity;
        placement = Scaling(opacity, opacity, opacity) * RotationY(rest * BOOK_PI);
        placement.m[MATRIX_TRANSLATION_Z] = rest * BOOK_FLY_DISTANCE;
    }
    else
        placement.SetIdentity();
    DAT_104c03c8->StartProximityPass(placement.m, &view, &view, DAT_102239c8 * 2, 0);
    return 1;
    }
}
