// The summary panel of a save game (EN build; JP: decomp/include/save_summary.h, decomp/src/save_summary.cpp). This is
// the one declaration of its tables, constants and function in decomp_en.
#ifndef GURUMIN_EN_SAVE_SUMMARY_H
#define GURUMIN_EN_SAVE_SUMMARY_H

// Number of map icon flags (and icons) of the panel.
const int MAP_ICON_FLAG_COUNT = 27;
// Bytes of a weekday name in g_weekdayNames (three full-width letters and the terminator).
const int WEEKDAY_NAME_SIZE = 7;
// Weekday names of the panel's date, Sunday first (full-width Shift-JIS text). EN 0x00848558 (JP DAT_101d6530).
extern "C" char g_weekdayNames[7][WEEKDAY_NAME_SIZE];
// Script flag of each map icon of the panel (negative = flag clear). EN 0x008485a0 (JP DAT_101d6578).
extern "C" short g_mapIconFlags[MAP_ICON_FLAG_COUNT];

// Draws the panel: the two label boxes of the second HUD sheet with the coin count (six digits, leading zeros left
// out) and the second counter (four digits), the two health labels with the health digits growing leftwards, the save
// date "yy/m/d (weekday)" and the time right-aligned in the text renderer, the percentage of the map clear points
// reached (measured over the 27 icon flags: 1 point for a flag that is set, 2 for a value above 1), and for every set
// flag the map icon in a 9-column grid, framed (and inset) with the fourth HUD sheet or, when the flag's value is above
// 1, with the fifth. Releases an icon texture again unless it is the map icon's. Returns 0. EN 0x5dc610 (JP 0x10021e70).
extern "C" int SaveSummary_Draw(void);

#endif
