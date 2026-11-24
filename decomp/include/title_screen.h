// The title screen (game mode 0, run by the frame step Screenshot_Capture 0x10137b10): the scrolling background, the
// flickering title pieces, the music and the main menu on top. Globals and routines only it uses.
#ifndef GURUMIN_TITLE_SCREEN_H
#define GURUMIN_TITLE_SCREEN_H

// Pieces of the title logo: DAT_10202808[0..12] are their texture slots (tit001.bmp..tit013.bmp). Each one flickers:
// DAT_10202848[i] is set when piece i starts to flicker, DAT_10202888[i] is the current flicker height in pixels.
#define TITLE_PIECE_COUNT 13

extern "C" int DAT_10202848[TITLE_PIECE_COUNT];           // non-zero while the piece's flicker rises
extern "C" int DAT_10202888[TITLE_PIECE_COUNT];           // flicker height of the piece (0 = at rest, at most 10)
extern "C" int DAT_10202840;                              // texture slot of the title frame (menu7.bmp)
extern "C" int DAT_104b7214;                              // texture slot of the scrolling background
extern "C" int DAT_10d4ee30;                              // frame counter copied to DAT_104bf8a4
extern "C" float DAT_10db9240;                            // x of the player (character 0): kept at 0 on the title screen
extern "C" float DAT_10db9244;                            // y of the player: kept at 0

// A key of the current input frame (DemoRecording::buttons_now[n], see DAT_1065cb30 in ui_vars.h).
extern "C" char DAT_1065cb32;                             // buttons_now[6]
extern "C" char DAT_1065cb33;                             // buttons_now[7]
extern "C" char DAT_1065cb34;                             // buttons_now[8]
extern "C" char DAT_1065cb35;                             // buttons_now[9]
extern "C" char DAT_1065cb36;                             // buttons_now[0xa]
extern "C" char DAT_1065cb37;                             // buttons_now[0xb]
extern "C" char DAT_1065cb38;                             // buttons_now[0xc]

extern "C" void FUN_10199860(void);                       // 0x10199860 (RenderQueue_Clear)
extern "C" int FUN_1000ee60(void);                        // 0x1000ee60 (Message_Update, app_0009.cpp)

// 0x10114950 (TitleScreen_Update, title_screen.cpp): one frame of the title screen. Returns 0 when the mode ends
// (a menu entry changed DAT_104bf8c0) or the window is closing, 1 otherwise.
int TitleScreen_Update();

#endif
