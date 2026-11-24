// Seconds timer shared by the mini game HUDs of the maps 0x17 and 0x18: digits cut from the "minigm" sheet's grid of
// 32 texel cells, a dot or colon quad, and the frame to hundredths conversion.
#ifndef GURUMIN_MINIGAME_TIMER_H
#define GURUMIN_MINIGAME_TIMER_H

#include "map_vars.h"
#include "minigame_banner.h"
#include "texture_object.h"

// The map timer runs at 30 frames per second; the hundredths digits are frames * 100 / 30.
#define TIMER_FRAMES_PER_SECOND 30
#define TIMER_HUNDREDTHS_PER_FRAME 3.333333f
#define TIMER_COUNT_DOWN_SECONDS 3

// Timer digits are 32 texel cells, four to a row, on the 128 texel sheet (inset by one texel).
#define DIGIT_COLUMNS 4
#define DIGIT_CELL 32
#define DIGIT_CELL_INSET 1
#define DIGIT_CELL_END 31

// Screen rectangles of the timer digits and the dot between the seconds and the hundredths.
#define TIMER_TENS_LEFT 0x1de
#define TIMER_ONES_LEFT 0x1fe
#define TIMER_DOT_LEFT 0x21e
#define TIMER_HUNDREDTHS_LEFT 0x22e
#define TIMER_HUNDREDTHS_ONES_LEFT 0x23e
#define TIMER_HUNDREDTHS_END 0x24e
#define TIMER_DIGIT_TOP 0x19a
#define TIMER_DIGIT_BOTTOM 0x1ba
#define TIMER_SMALL_DIGIT_TOP 0x1aa
#define TIMER_DOT_U0 (130.0f / 256.0f)
#define TIMER_DOT_V0 (130.0f / 256.0f)
#define TIMER_DOT_U1 (190.0f / 256.0f)
#define TIMER_DOT_V1 (190.0f / 256.0f)

// Draws the digit `digit` (0..9) cut from the sheet's grid of 32 texel cells into the given screen rectangle.
static __forceinline void DrawTimerDigit(int left, int top, int right, int bottom, int digit)
{
    int row = (digit / DIGIT_COLUMNS) * DIGIT_CELL;
    int column = (digit % DIGIT_COLUMNS) * DIGIT_CELL;
    DAT_118d0ba8[DAT_102028c8].DrawColoured(left, top, right, bottom, (float)(column + DIGIT_CELL_INSET) / SHEET_WIDTH,
                                            (float)(row + DIGIT_CELL_INSET) / SHEET_WIDTH,
                                            (float)(column + DIGIT_CELL_END) / SHEET_WIDTH,
                                            (float)(row + DIGIT_CELL_END) / SHEET_WIDTH, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                                            1.0f);
}
#define DRAW_TIMER_DIGIT DrawTimerDigit

#endif
