// MenuText: declared here once (first used by src/menu_list.cpp).
#ifndef GURUMIN_MENU_TEXT_H
#define GURUMIN_MENU_TEXT_H

struct MenuText
{
    char text[0x80];
};

extern "C" int FUN_1015d520(char *text);   // text_fullwidth.cpp

#endif
