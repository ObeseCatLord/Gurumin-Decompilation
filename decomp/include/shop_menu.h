// The shop window drawn by ShopMenu_Draw (src/shop_menu.cpp).
// This is the one declaration of each of these: include this header, never redeclare one.
#ifndef GURUMIN_SHOP_MENU_H
#define GURUMIN_SHOP_MENU_H

#include "matrix4.h"

extern "C" int DAT_10c66cfc;                 // price percentage (0 = prices unchanged)
extern "C" Matrix4 DAT_10836430;             // the view matrix of the shop's item preview

void ShopMenu_Draw(void);                    // 0x10056b30

#endif
