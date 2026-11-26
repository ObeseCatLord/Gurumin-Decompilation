// The tutorial book (the pair of pages the main menu opens), EN build (JP: decomp/include/tutorial_book.h). Declared
// once here. The overlay mode globals it shares are in dialogs.h.
#ifndef GURUMIN_EN_TUTORIAL_BOOK_H
#define GURUMIN_EN_TUTORIAL_BOOK_H

#include "dialogs.h"

// Pages of the book, bytes of a page's texture name, and the slot of the placeholder page.
const int TUTORIAL_PAGE_COUNT = OVERLAY_TEXTURE_COUNT;
const int TUTORIAL_NAME_SIZE = 0x10;
const int TUTORIAL_LOCKED_PAGE = 0x16;
// Value of g_overlayModelValueG while the book closes.
const int TUTORIAL_STATE_CLOSING = 5;
// Script flag value of a page that is always readable.
const int TUTORIAL_PAGE_ALWAYS = -1;

extern "C" char g_tutorialPageNames[TUTORIAL_PAGE_COUNT][TUTORIAL_NAME_SIZE];   // EN 0x00908160 (JP DAT_102028e8): texture name of each page; entry 0x16 is the locked page placeholder
extern "C" int g_tutorialLeftPage;       // EN 0x00920988 (JP DAT_102194c0): the left page shown
extern "C" int g_tutorialRightPage;      // EN 0x0092098c (JP DAT_102194c4): the right page shown
extern "C" int g_tutorialSecondBook;     // EN 0x00920b1c (JP DAT_104c03e8): 1 in the second book (pages are locked until their flag is set)
extern "C" int g_tutorialTurnFrames;     // EN 0x00920e40 (JP DAT_102239c8): half the frames of a page turn

// Starts closing the book (state TUTORIAL_STATE_CLOSING): loads the cover and the first two readable pages from the left
// one on, turns the sheet back, stops the halves and closes the cover; plays the book sound. Returns 0. EN 0x5c2c40 (JP 0x1004f250).
// Called when the tutorial mode is left.
extern "C" int TutorialBook_Close(void);

// Input of the current frame the book reads (JP: the DemoRecording's buttons_now / values_now of the input frame).
extern "C" char g_inputCloseButton;     // EN 0x00bce1b1 (JP DAT_1065cb31): button 5 of the input frame; closes the book
extern "C" char g_inputMouseButtonA;    // EN 0x00bce1be (JP DAT_1065cb3e): button 0x12 of the input frame
extern "C" char g_inputMouseButtonB;    // EN 0x00bce1bf (JP DAT_1065cb3f): button 0x13 of the input frame
extern "C" int g_inputStickX;           // EN 0x00bce1c8 (JP DAT_1065cb48): first analogue value of the input frame (left/right)
extern "C" int g_inputStickY;           // EN 0x00bce1cc (JP DAT_1065cb4c): second analogue value (up/down)
extern "C" int g_inputMouseDisabled;    // EN 0x00bce210 (JP DAT_1065cb90): non-zero = the book does not read the mouse

// Turns to the next page (JP 0x1004f4f0). EN 0x63e180.
extern "C" int TutorialBook_NextPage(void);
// Turns to the previous page (JP 0x1004f860). EN 0x63dc20.
extern "C" int TutorialBook_PreviousPage(void);

// One frame of the tutorial book (see tutorial_book.cpp). Returns 1 when the book was drawn, else 0. EN 0x5b8590 (JP 0x10065450).
int TutorialBook_Update();

#endif
