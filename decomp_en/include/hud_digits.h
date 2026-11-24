// HUD numbers drawn from the digit strips of the HUD frame sheets (EN build; JP: decomp/include/hud_digits.h).
#ifndef GURUMIN_EN_HUD_DIGITS_H
#define GURUMIN_EN_HUD_DIGITS_H

// Draws `value` in decimal starting at (x, y). `style` picks the digit row of the sheet, `grow` enlarges every cell by
// grow * 8 pixels on each side, `alpha` and `grey` go to TextureObject::DrawTinted. Each returns the width of the
// number in pixels (the right aligned one returns the pen position after the number, relative to x).
extern "C" int Number_Draw(int x, int y, int value, float alpha, int style, float grow, float grey);         // EN 0x5cc630: styles 1-3, front depth
extern "C" int Number_DrawRightAligned(int x, int y, int value, float alpha, int style, float grow, float grey);  // EN 0x5cc110: styles 0-2, number ends at x
extern "C" int Number_DrawBack(int x, int y, int value, float alpha, int style, float grow, float grey);     // EN 0x5cc3b0: styles 0-2, back depth

#endif
