// The name entry (game mode 10, run by the frame step): the screen the main menu opens after "new game". The first
// frame plays the sound and music and loads the player model and the face textures of the screen; every frame draws
// the scrolling background and the player model, and once a new engine tick has started reads the keys, runs the menu
// (which holds the name box) and starts the game when Enter confirms: a default name is picked from a table of 252
// katakana names when none was typed, the first save slot is reset and the registered player name set. C++ rather
// than extern "C": the function keeps an exception frame for its Matrix4 and Vec4 locals.

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "model_bank.h"

#include "tracker.h"

#include "bgm_track.h"
#include "char_hikari.h"
#include "sound_vars.h"
#include "camera_setup.h"
#include "char_slot.h"
#include "char_vars.h"
#include "dinput_min.h"
#include "doppler_loops.h"
#include "frame_vars.h"
#include "game.h"
#include "game_load_data.h"
#include "game_vars.h"
#include "input_state.h"
#include "item_dictionary.h"
#include "menu_main.h"
#include "name_entry.h"
#include "render_state.h"
#include "render_vars.h"
#include "script_system.h"
#include "script_vars.h"
#include "texture_load.h"
#include "texture_object.h"
#include "title_screen.h"
#include "ui_vars.h"

#define NAME_ENTRY_SOUND 0x357
#define NAME_ENTRY_MUSIC 3
#define TITLE_MUSIC 8
#define NAME_ENTRY_WINDOW_ID 0x218
#define FACE_NODE_KIND 0x1f
#define PLAYER_START_HEADING -0.9f
#define PLAYER_MOTION_IDLE 2
#define PLAYER_MOTION_BLEND 5
#define PLAYER_PLACE_Z -60.0f
#define BACKGROUND_SCROLL_LAST 0x137
#define BACKGROUND_SCROLL_SPEED 0.02f
#define BACKGROUND_WIDTH_UVS 5.0f
#define BACKGROUND_HEIGHT_UVS 3.75f
#define BACKGROUND_Z 0.991f
#define PROMPT_SIZE_MAX 10
#define BUTTON_PRESSED 2
#define EARLIEST_ENTER_TICK 5
#define STATE_OPENING 1
#define STATE_OPEN 2
#define STATE_FADE_OUT 0x3c
#define STATE_LEAVE 0x50
#define STATE_FADE_START 10
#define LIST_FADE_FULL 0x14
#define SCREEN_WIDTH 0x280
#define SCREEN_HEIGHT 0x1e0
#define CAMERA_Y -500.0f
#define CAMERA_Z 150.0f
#define DEFAULT_NAME_COUNT 0xfc
#define DEFAULT_NAME_SIZE 10
#define SLOT_START_HEALTH 0x32
#define SLOT_START_COINS 4
#define OPAQUE_FOG_COLOUR 0xedba67
#define QUEUE_MASK 7
#define LEAVE_FADE_AMOUNT 100.0f
#define LEAVE_FADE_STEP 6.6666665f
#define LEAVE_FADE_FRAMES 15
// Shift-JIS "parin" (the word of the messages that the player's name replaces).
#define NAME_WORD_PLAYER "\x83p\x83\x8a\x83\x93"

// 0x10116550 (NameEntry_Update). The first frame plays the name entry sound and music, loads the player model and the
// five face textures cf02_xx and turns the player towards the camera. Every frame draws the scrolling background
// (DAT_1020283c) and, once a new engine tick has started, reads the controls, advances the windows, the prompt size
// and the background and handles the keys: Escape or a right click returns to the title (state 1 or 2, after
// EARLIEST_ENTER_TICK ticks); Enter or a click confirms the name (a default one from the table when none was typed,
// and a new game's first save slot) and moves on to state 10; state 0x3c starts the fade out and state 0x50 leaves
// the mode for the game. Then it runs the menu (Menu_Main shows the name box), fades the list by state and draws the
// player model. Returns 0.
int NameEntry_Update()
{
    char left;
    char right;
    if (DAT_104bf8bc == 0)
    {
        FUN_1003ad20(NAME_ENTRY_SOUND, 0, 1.0f);
        DAT_104c012c = NAME_ENTRY_MUSIC;
        Bgm_Play(NAME_ENTRY_MUSIC, 0);
        DAT_10f6553c = FUN_10038520;
        FUN_1003b6e0(0, 0);
        if (DAT_10db6ee8[0].root != 0)
        {
            DAT_10db6ee8[0].root->M_101946b0(FACE_NODE_KIND, -1);
            DAT_10db6ee8[0].root->M_10194730(FACE_NODE_KIND, -1);
        }
        DAT_10db6ee8[0].m_0080 = 1;
        ((char *)DAT_10255a08)[0] = 0;
        DAT_104bf8bc = 1;
        DAT_10db6ee8[0].heading = PLAYER_START_HEADING;
        DAT_10db6ee8[0].targetHeading = PLAYER_START_HEADING;
        DAT_104bfd14 = STATE_OPENING;
        DAT_10db6ee8[0].PlayMotion(PLAYER_MOTION_IDLE, 0, PLAYER_MOTION_BLEND, 1.0f);
        DAT_1042d9d8[0] = FUN_10174ce0("cf02_00", 0, 0);
        DAT_1042d9d8[1] = FUN_10174ce0("cf02_07", 0, 0);
        DAT_1042d9d8[2] = FUN_10174ce0("cf02_27", 0, 0);
        DAT_1042d9d8[3] = FUN_10174ce0("cf02_03", 0, 0);
        DAT_1042d9d8[4] = FUN_10174ce0("cf02_04", 0, 0);
    }
    Matrix4 viewProjection;
    Matrix4 spareA;
    Matrix4 view;
    Matrix4 projection;
    Matrix4 spareB;
    float scroll = (float)DAT_104c0328 * BACKGROUND_SCROLL_SPEED;
    Vec4 eye;
    Vec4 target;
    FUN_1013f070(RENDER_CLEAR, OPAQUE_FOG_COLOUR, 0);
    FUN_1013f070(RENDER_BLEND_MODE, 0, 0);
    FUN_1013f070(RENDER_TEXTURE_ADDRESS, 0, 0);
    FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
    DAT_118d0ba8[DAT_1020283c].DrawTinted(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, scroll, scroll,
                                          scroll + BACKGROUND_WIDTH_UVS, scroll + BACKGROUND_HEIGHT_UVS,
                                          BACKGROUND_Z, 1.0f, 0, 0);
    eye.Set(0, CAMERA_Y, CAMERA_Z, 0);
    target.Set(0, 0, 0, 0);
    Camera_SetMatrices(&eye, &target, &view, &projection, 0);
    viewProjection = view * projection;
    if (DAT_10f65504 != DAT_104bf8b8)
    {
        DAT_104c0328++;
        DAT_1020d9e0.M_10001110(0, 0, 0, 0, 0, 0);
        DAT_10f08008.Update();
        DAT_10210f30 = DAT_10f08008.ReadControls(0);
        DAT_1020d9e0.M_10001550();
        DAT_1065cb28.SetButtons(DAT_10210f30);
        DAT_104bf8b8 = DAT_10f65504;
        DAT_104bf890++;
        DAT_104b7200.UpdateWindows();
        DAT_104b7200.MoveCursor();
        FUN_1003a9d0();
        DAT_1032d67c[0]++;
        if (DAT_1032d67c[0] > BACKGROUND_SCROLL_LAST)
            DAT_1032d67c[0] = 0;
        DAT_104bf88c += DAT_10202348;
        if (DAT_104bf88c == PROMPT_SIZE_MAX || DAT_104bf88c == 0)
            DAT_10202348 = -DAT_10202348;
        if (DAT_104bf8d4 != 0)
            FUN_1000cbe0();
        FUN_1015f4e0();
        DAT_1020d9e0.M_10001110(0, &left, 0, &right, 0, 0);
        if (((GetKeyState(VK_ESCAPE) & 0xf000) != 0 || right == BUTTON_PRESSED) &&
            (DAT_104bfd14 == STATE_OPENING || DAT_104bfd14 == STATE_OPEN) && DAT_10f6811c > EARLIEST_ENTER_TICK)
        {
            DAT_104c0308 = 0;
            DAT_104bfd14 = STATE_OPENING;
            DAT_104bf8c0 = 0;
            DAT_104bf890 = 0;
            DAT_1030c50c = 1;
            DAT_10f6553c = FUN_10038010;
            DAT_104c0324 = 0;
            DAT_104b7200.CloseWindow(NAME_ENTRY_WINDOW_ID);
            DAT_10db6ee8[0].Unload();
            DAT_10db6ee8[0].root = 0;
            FUN_1015fd30();
            DAT_104c012c = TITLE_MUSIC;
            Bgm_Play(TITLE_MUSIC, 0);
            return 0;
        }
        if (((GetKeyState(VK_RETURN) & 0xf000) != 0 || left == BUTTON_PRESSED) &&
            (DAT_104bfd14 == STATE_OPENING || DAT_104bfd14 == STATE_OPEN) && DAT_10f6811c > EARLIEST_ENTER_TICK)
        {
            FUN_1003ad20(NAME_ENTRY_SOUND, 0, 1.0f);
            if (((char *)DAT_10255a08)[0] == 0)
            {
                char names[DEFAULT_NAME_COUNT][DEFAULT_NAME_SIZE] = {
                    "\x83\x7e\x83\x47\x83\x52", "\x83\x41\x83\x84\x83\x52", "\x83\x7e\x83\x60\x83\x88", "\x83\x4a\x83\x59\x83\x7e",
                    "\x83\x67\x83\x82\x83\x7e", "\x83\x43\x83\x59\x83\x7e", "\x83\x7d\x83\x43\x83\x52", "\x83\x71\x83\x8d\x83\x52",
                    "\x83\x69\x83\x49", "\x83\x41\x83\x86\x83\x7e", "\x83\x69\x83\x63\x83\x7e", "\x83\x41\x83\x54\x83\x7e",
                    "\x83\x8c\x83\x69", "\x83\x8f\x83\x4a", "\x83\x7e\x83\x54\x83\x4c", "\x83\x7e\x83\x7a",
                    "\x83\x86\x83\x45\x83\x52", "\x83\x52\x83\x8a\x83\x93", "\x83\x6a\x83\x69", "\x83\x7e\x83\x69",
                    "\x83\x4a\x83\x49\x83\x8a", "\x83\x54\x83\x84\x83\x4a", "\x83\x71\x83\x67\x83\x7e", "\x83\x41\x83\x84\x83\x6c",
                    "\x83\x4a\x83\x88\x83\x52", "\x83\x5e\x83\x4a\x83\x52", "\x83\x7e\x83\x59\x83\x4c", "\x83\x60\x83\x67\x83\x5a",
                    "\x83\x60\x83\x69\x83\x63", "\x83\x60\x83\x69\x83\x7e", "\x83\x7d\x83\x88\x83\x43", "\x83\x6e\x83\x8b\x83\x7e",
                    "\x83\x6e\x83\x8b\x83\x4a", "\x83\x6e\x83\x8b\x83\x52", "\x83\x6e\x83\x69\x83\x52", "\x83\x6e\x83\x69\x83\x47",
                    "\x83\x41\x83\x43\x83\x52", "\x83\x41\x83\x86", "\x83\x41\x83\x86\x83\x52", "\x83\x41\x83\x43\x83\x4a",
                    "\x83\x45\x83\x89\x83\x7e", "\x83\x47\x83\x7e", "\x83\x47\x83\x7e\x83\x52", "\x83\x4a\x83\x58\x83\x7e",
                    "\x83\x4a\x83\x69", "\x83\x4a\x83\x88", "\x83\x4a\x83\x88\x83\x52", "\x83\x4e\x83\x7e",
                    "\x83\x4e\x83\x7e\x83\x52", "\x83\x50\x83\x43\x83\x52", "\x83\x50\x83\x43", "\x83\x54\x83\x88\x83\x52",
                    "\x83\x54\x83\x60\x83\x52", "\x83\x54\x83\x60\x83\x47", "\x83\x54\x83\x54\x83\x7e", "\x83\x54\x83\x5f\x83\x52",
                    "\x83\x54\x83\x4c", "\x83\x56\x83\x87\x83\x45\x83\x52", "\x83\x4a\x83\x59\x83\x52", "\x83\x58\x83\x7e\x83\x47",
                    "\x83\x5e\x83\x7e", "\x83\x65\x83\x8b\x83\x52", "\x83\x65\x83\x8b\x83\x7e", "\x83\x67\x83\x81",
                    "\x83\x67\x83\x4c\x83\x52", "\x83\x69\x83\x63\x83\x52", "\x83\x69\x83\x8b\x83\x7e", "\x83\x69\x83\x7e",
                    "\x83\x69\x83\x7e\x83\x52", "\x83\x69\x83\x49\x83\x52", "\x83\x6a\x81\x5b\x83\x69", "\x83\x6d\x83\x8a\x83\x52",
                    "\x83\x71\x83\x7e\x83\x52", "\x83\x74\x83\x7e\x83\x52", "\x83\x74\x83\x7e\x83\x47", "\x83\x74\x83\x7e",
                    "\x83\x7d\x83\x8a", "\x83\x7d\x83\x8a\x83\x52", "\x83\x7d\x83\x86\x83\x7e", "\x83\x7d\x83\x86\x83\x52",
                    "\x83\x7d\x83\x43", "\x83\x7d\x83\x7e", "\x83\x7d\x83\x7e\x83\x52", "\x83\x7d\x83\x84",
                    "\x83\x41\x83\x43", "\x83\x41\x83\x84", "\x83\x41\x83\x4c", "\x83\x7e\x83\x63\x83\x52",
                    "\x83\x7e\x83\x69\x83\x52", "\x83\x7e\x83\x63\x83\x52", "\x83\x84\x83\x43\x83\x52", "\x83\x86\x83\x7e",
                    "\x83\x86\x83\x7e\x83\x52", "\x83\x41\x83\x7e", "\x83\x88\x83\x45\x83\x52", "\x83\x57\x83\x85\x83\x93\x83\x52",
                    "\x83\x54\x83\x4c\x83\x52", "\x83\x54\x83\x60\x83\x47", "\x83\x54\x83\x69\x83\x47", "\x83\x8a\x83\x93\x83\x52",
                    "\x83\x7e\x83\x54", "\x83\x5a\x83\x43\x83\x52", "\x83\x60\x83\x54\x83\x67", "\x83\x41\x83\x4c\x83\x69",
                    "\x83\x7d\x83\x4c", "\x83\x7d\x83\x8a", "\x83\x69\x83\x49\x83\x7e", "\x83\x4c\x83\x87\x83\x45\x83\x52",
                    "\x83\x8a\x83\x87\x83\x45\x83\x52", "\x83\x41\x83\x8a\x83\x54", "\x83\x41\x83\x59\x83\x54", "\x83\x8c\x83\x43\x83\x69",
                    "\x83\x7e\x83\x58\x83\x59", "\x83\x7e\x83\x4a", "\x83\x7e\x83\x6c\x83\x52", "\x83\x74\x83\x57\x83\x52",
                    "\x83\x56\x83\x59\x83\x4a", "\x83\x41\x83\x4a\x83\x6c", "\x83\x4c\x83\x8a\x83\x52", "\x83\x4c\x83\x7e\x83\x52",
                    "\x83\x70\x83\x8a\x83\x93", "\x83\x54\x83\x8a\x83\x69", "\x83\x5a\x83\x8a\x83\x69", "\x83\x5a\x83\x8a\x83\x58",
                    "\x83\x86\x83\x45\x83\x69", "\x83\x73\x83\x73\x83\x8d", "\x83\x76\x83\x8a\x83\x93", "\x83\x7c\x83\x8d\x83\x93",
                    "\x83\x7c\x83\x8a\x83\x93", "\x83\x56\x83\x8b\x83\x72\x83\x41", "\x83\x5a\x83\x8c\x83\x58", "\x83\x4a\x83\x8a\x83\x69",
                    "\x83\x52\x83\x8d\x83\x69", "\x83\x5c\x83\x41", "\x83\x7d\x83\x8a\x83\x93", "\x83\x7e\x83\x7e",
                    "\x83\x7e\x81\x5b\x83\x69", "\x83\x4a\x83\x67\x83\x8c\x83\x41", "\x83\x4e\x83\x8a\x83\x58", "\x83\x7e\x83\x56\x83\x46\x83\x8b",
                    "\x83\x50\x83\x43\x83\x67", "\x83\x8c\x83\x7e", "\x83\x54\x83\x93\x83\x68\x83\x89", "\x83\x41\x83\x8a\x83\x56\x83\x41",
                    "\x83\x7e\x83\x88\x83\x52", "\x83\x7e\x83\x4a\x83\x51", "\x83\x41\x83\x7e\x83\x52", "\x83\x86\x83\x4c\x83\x4a",
                    "\x83\x54\x83\x4e\x83\x89", "\x83\x84\x83\x60\x83\x88", "\x83\x7e\x83\x43", "\x83\x82\x83\x82",
                    "\x83\x8a\x83\x93\x83\x53", "\x83\x58\x83\x82\x83\x82", "\x83\x7d\x83\x63\x83\x4c", "\x83\x45\x83\x4c",
                    "\x83\x53\x83\x57\x83\x52", "\x83\x4a\x83\x5b\x83\x52", "\x83\x74\x83\x45\x83\x52", "\x83\x88\x83\x7e",
                    "\x83\x6f\x83\x4e\x83\x5f\x83\x93", "\x83\x4e\x83\x7d\x83\x52", "\x83\x58\x83\x7e\x83\x8c", "\x83\x86\x83\x8a",
                    "\x83\x86\x83\x8a\x83\x52", "\x83\x86\x83\x7d", "\x83\x56\x83\x8d\x83\x45\x83\x67", "\x83\x7e\x83\x8f\x83\x52",
                    "\x83\x8f\x83\x4a\x83\x52", "\x83\x67\x83\x81\x83\x52", "\x83\x67\x83\x45\x83\x52", "\x83\x67\x83\x4e\x83\x52",
                    "\x83\x67\x83\x8f", "\x83\x67\x83\x82", "\x83\x41\x83\x93\x83\x69", "\x83\x67\x83\x82\x83\x88",
                    "\x83\x82\x83\x47", "\x83\x60\x83\x47\x83\x52", "\x83\x86\x83\x43", "\x83\x8d\x83\x55\x83\x93\x83\x69",
                    "\x83\x60\x83\x4a", "\x83\x60\x83\x71\x83\x8d", "\x83\x60\x83\x59\x83\x8b", "\x83\x60\x83\x47",
                    "\x83\x5c\x83\x89", "\x83\x7e\x83\x4a", "\x83\x8a\x83\x47", "\x83\x5c\x81\x5b\x83\x6a\x83\x93",
                    "\x83\x8a\x83\x54", "\x83\x58\x83\x57\x83\x93", "\x83\x77\x83\x58\x83\x93", "\x83\x80\x83\x63\x83\x52",
                    "\x83\x69\x83\x7e", "\x83\x6d\x83\x69\x83\x7e", "\x83\x8a\x83\x49", "\x83\x8c\x83\x43",
                    "\x83\x41\x83\x58\x83\x4a", "\x83\x71\x83\x4a\x83\x8b", "\x83\x71\x83\x4a\x83\x8a", "\x83\x7d\x83\x8a\x83\x41",
                    "\x83\x89\x83\x93", "\x83\x7d\x83\x54\x83\x7e", "\x83\x57\x83\x85\x83\x93", "\x83\x4a\x83\x8a\x83\x93",
                    "\x83\x86\x83\x43", "\x83\x54\x83\x4e\x83\x89", "\x83\x58\x83\x7e\x83\x8c", "\x83\x7e\x83\x88\x83\x52",
                    "\x83\x7e\x83\x54\x83\x47", "\x83\x54\x83\x55\x83\x47", "\x83\x7e\x83\x54\x83\x88", "\x83\x7e\x83\x69\x83\x47",
                    "\x83\x7e\x83\x69\x83\x7e", "\x83\x7e\x83\x52", "\x83\x7d\x83\x52", "\x83\x6d\x83\x8a\x83\x4a",
                    "\x83\x6d\x83\x8a\x83\x52", "\x83\x4c\x83\x87\x83\x45\x83\x4a", "\x83\x81\x83\x43", "\x83\x7e\x83\x89\x83\x43",
                    "\x83\x41\x83\x8a\x83\x54", "\x83\x41\x83\x59\x83\x4c", "\x83\x50\x83\x43", "\x83\x6d\x83\x5d\x83\x7e",
                    "\x83\x4a\x83\x63\x83\x7e", "\x83\x86\x83\x4c\x83\x47", "\x83\x86\x83\x4c\x83\x52", "\x83\x86\x83\x4c",
                    "\x83\x47\x83\x8a", "\x83\x47\x83\x8a\x83\x4a", "\x83\x47\x83\x7e\x83\x4a", "\x83\x47\x83\x7e\x83\x8a",
                    "\x83\x54\x83\x86\x83\x8a", "\x83\x54\x83\x8a\x83\x69", "\x83\x54\x83\x47\x83\x52", "\x83\x54\x83\x47",
                    "\x83\x54\x83\x4c", "\x83\x54\x83\x4c\x83\x47", "\x83\x88\x83\x45\x83\x52", "\x83\x88\x83\x8a\x83\x52",
                    "\x83\x71\x83\x67\x83\x47", "\x83\x88\x83\x8a\x83\x47", "\x83\x7e\x83\x8a\x83\x4a", "\x83\x41\x83\x59\x83\x7e",
                    "\x83\x41\x83\x50\x83\x7e", "\x83\x41\x83\x7e", "\x83\x7e\x83\x7a\x83\x56", "\x83\x7d\x83\x68\x83\x4a",
                    "\x83\x7d\x83\x43\x83\x4a", "\x83\x8f\x83\x4a\x83\x81", "\x83\x8a\x83\x4a", "\x83\x65\x83\x93\x83\x52",
                };
                strcpy((char *)DAT_10255a08, names[FUN_1013f880() % DEFAULT_NAME_COUNT]);
                strcpy(DAT_10f66df8, (char *)DAT_10255a08);
            }
            strcpy(DAT_10daaae8[0].name, (char *)DAT_10255a08);
            if (DAT_104c0308 == 0)
            {
                DAT_10daaae8[0].stats[STAT_HEALTH] = SLOT_START_HEALTH;
                DAT_10daaae8[0].stats[STAT_MAX_HEALTH] = SLOT_START_HEALTH;
                DAT_10daaae8[0].stats[7] = SLOT_START_HEALTH;
                memset(DAT_103150c0, 0, sizeof(DAT_103150c0));
                DAT_10daaae8[0].stats[2] = 0;
                DAT_10daaae8[0].stats[3] = 10;
                DAT_10daaae8[0].stats[4] = 5;
                DAT_10daaae8[0].stats[5] = 0;
                DAT_10daaae8[0].stats[6] = 0;
                DAT_10daaae8[0].stats[8] = 0;
                DAT_10daaae8[0].stats[9] = 0;
                DAT_10daaae8[0].stats[10] = 0;
                DAT_10daaae8[0].stats[11] = 0;
                DAT_10daaae8[0].stats[12] = 0;
                DAT_10daaae8[0].stats[13] = 0;
                DAT_10daaae8[0].stats[14] = 0;
                DAT_10daaae8[0].stats[15] = 0;
                DAT_10daaae8[0].coins = SLOT_START_COINS;
                memset(DAT_10c62edc, 0, sizeof(DAT_10c62edc));
            }
            FUN_1013e6f0(0, NAME_WORD_PLAYER, (char *)DAT_10255a08);
            DAT_104bfd14 = STATE_FADE_START;
            FUN_1015fd30();
            DAT_1030c53c = 1;
        }
        else if (DAT_104bfd14 == STATE_FADE_OUT)
        {
            DAT_104bf8f0 = 0;
            DAT_104bf8e8 = LEAVE_FADE_AMOUNT;
            DAT_104bf8ec = 0;
            DAT_1032d4b0 = LEAVE_FADE_STEP;
            DAT_104bf8e4 = LEAVE_FADE_FRAMES;
        }
        else if (DAT_104bfd14 == STATE_LEAVE)
        {
            FUN_100121e0(0);
            DAT_104bfdb8 = 1;
            DAT_10db6ee8[0].Unload();
            DAT_1020247c[DAT_104bfe88] = 0;
            DAT_104bfe88 = (DAT_104bfe88 + 1) & QUEUE_MASK;
            DAT_104bf8c0 = 1;
            DAT_104bf8bc = 0;
            DAT_104c0308 = 0;
            FUN_1000e750();
            FUN_1000e810();
            return 0;
        }
        FUN_10112140();
        if (DAT_104bfd14 > STATE_FADE_START)
            DAT_104bfd14++;
        DAT_10db6ee8[0].SetPosition(0, 0, PLAYER_PLACE_Z);
        DAT_10db6ee8[0].motionFrame = DAT_1032d67c[0];
        DAT_10db6ee8[0].UpdateFrame(&viewProjection, 1, 0, 0, 0);
        if (DAT_104bfd14 >= STATE_FADE_START + 1)
        {
            if (DAT_104c0232 < LIST_FADE_FULL)
                DAT_104c0232++;
        }
        else if (DAT_104bfd14 == STATE_FADE_START)
        {
            if (DAT_104c0232 > STATE_FADE_START)
                DAT_104c0232 = STATE_FADE_START;
            else if (DAT_104c0232 < STATE_FADE_START)
                DAT_104c0232++;
        }
        else if (DAT_104bfd14 < STATE_FADE_START && DAT_104c0232 > 0)
            DAT_104c0232--;
    }
    DAT_10db6ee8[0].Draw(&viewProjection, DAT_10804bf8.LightDirection(0, 0, 0), 0);
    return 0;
}
