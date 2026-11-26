// Seasonal events and packed script variables of the EN build (JP: app_0014.cpp).
#ifndef GURUMIN_EN_APP_0014_H
#define GURUMIN_EN_APP_0014_H

// Values returned by SeasonalEvent.
enum SeasonalEventKind
{
    SEASON_NONE = 0,
    SEASON_CHRISTMAS = 1,   // December 25, or December 24 after 18:00
    SEASON_NEW_YEAR = 2     // January 1 to 3
};

// Reads the PC clock and returns the SeasonalEventKind of today. EN 0x5c0aa0 (JP 0x1001d850).
int SeasonalEvent();

// Unpacks script variable `index` when it holds a packed pair; see the definition. EN 0x5ee4d0 (JP 0x10017b10).
extern "C" int Script_UnpackVar(int index, int *high, int *low);

// Packs the pair (`high`, `low`) into script variable `index` when it is 0 or `overwrite` is set. Returns 0. EN 0x646630.
extern "C" int Script_PackVar(int index, int high, int low, int overwrite);

#endif
