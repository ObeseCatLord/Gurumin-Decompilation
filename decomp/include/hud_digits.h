#ifndef HUD_DIGITS_H
#define HUD_DIGITS_H

// HUD numbers drawn from the digit strip of texWaku01 (hud_digits.cpp). style picks the digit row, grow
// enlarges every cell by grow * 8 pixels on each side, alpha and grey go to TextureObject::DrawTinted.
// Each returns the width of the number in pixels.
extern "C" int FUN_10012990(int x, int y, int value, float alpha, int style, float grow, float grey);  // Number_Draw
extern "C" int FUN_10012d90(int x, int y, int value, float alpha, int style, float grow,
                            float grey);  // Number_DrawRightAligned
extern "C" int FUN_100131b0(int x, int y, int value, float alpha, int style, float grow, float grey);  // Number_DrawBack

#endif
