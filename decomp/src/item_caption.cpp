// The caption of an item in the item menus: its name and, for equipment, its level and abilities (scrolled when

// the ability list is longer than 17 characters).



#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "game.h"



// The start of each item record (the item table, ItemInfo in other files, is viewed from 0x40 bytes further on).

#include "item_caption.h"



// A 0x240-byte record per item holding its fallback name.



struct ItemInfo

{

    int type;                     // 1 = equipment with a level and abilities

    int levelFlag;                // 0x04: script flag holding the item's level (minus 3205)

    char pad_0008[0xb8];

};



struct TextRenderer

{

    // 0x101508d0 (text_outline.cpp DrawOutlined): text with a brown outline.

    int DrawOutlined(int x, int y, const char *text, int colour, int a, float alpha, float scale);

};





extern "C" ItemInfo DAT_1088e6b0[];

extern "C" TextRenderer DAT_1020e538;

#include "ui_vars.h"


extern "C" int FUN_10014d30(int item);           // Helmet_FaceSettings: the item's ability bits

extern "C" int FUN_1003bba0(unsigned int flags, char *out);   // the names of the abilities in `flags`



// 0x1003c370 (Item_DrawCaption). Draws the name of item `item` at x + 0x8c (y 0x18c) with opacity `alpha`; for

// equipment also its level ("Lv." with the level script flag + 1) and its abilities below (y 0x1a2), of which 17

// characters are shown, scrolling with DAT_104c02cc. Returns 0.

extern "C" int FUN_1003c370(int item, float alpha, int x)

{

    char text[0x100];

    char line[0x100];

    char name[0x100];

    name[0] = 0;

    strcpy(name, DAT_1088e670[item].name);

    if (name[0] == 0)

        strcpy(name, DAT_108f667c[item].name);

    sprintf(text, name);

    DAT_1020e538.DrawOutlined(x + 0x8c, 0x18c, text, 0xffffff, 0, alpha, 0.9f);

    if (DAT_1088e6b0[item].type == 1)

    {

        FUN_1003bba0(FUN_10014d30(item), text);

        int length = (int)strlen(text) / 2;

        if (length < 0x11)

        {

            sprintf(line, "\x82" "k\x82\x96\x81" "D%ds042\x8c\xf8\x89\xca\x81" "F%s",
                    DAT_10874848.TestScriptFlag(DAT_1088e6b0[item].levelFlag + 0xc85) + 1, text);

            DAT_1020e538.DrawOutlined(x + 0x8c, 0x1a2, line, 0xffffff, 0, alpha, 0.9f);

        }

        else

        {

            DAT_104c02cd = length - 0x10;

            // the scroll position (timer - 5, at least 0), at most the overflow
            int start = __max(0, DAT_104c02cc - 5) > DAT_104c02cd ? DAT_104c02cd : __max(0, DAT_104c02cc - 5);

            text[start * 2 + 0x22] = 0;

            sprintf(line, "\x82" "k\x82\x96\x81" "D%ds042\x8c\xf8\x89\xca\x81" "F%s",
                    DAT_10874848.TestScriptFlag(DAT_1088e6b0[item].levelFlag + 0xc85) + 1, &text[start * 2]);

            DAT_1020e538.DrawOutlined(x + 0x8c, 0x1a2, line, 0xffffff, 0, alpha, 0.9f);

        }

    }

    return 0;

}

