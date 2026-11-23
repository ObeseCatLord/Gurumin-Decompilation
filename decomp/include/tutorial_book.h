// The tutorial book (the pair of pages the main menu opens): its functions, declared once. The globals are in
// ui_vars.h and render_vars.h.
#ifndef GURUMIN_TUTORIAL_BOOK_H
#define GURUMIN_TUTORIAL_BOOK_H

extern "C" int FUN_1004ed60(int book);   // TutorialBook_Open (tutorial_open.cpp)
extern "C" int FUN_1004f250(void);       // TutorialBook_Close (tutorial_page_next.cpp)
extern "C" int FUN_1004f4f0(void);       // TutorialBook_NextPage (tutorial_page_next.cpp)
extern "C" int FUN_1004f860(void);       // TutorialBook_PrevPage (tutorial_page_next.cpp)
int TutorialBook_Update();               // 0x10065450 (tutorial_update.cpp)

#endif
