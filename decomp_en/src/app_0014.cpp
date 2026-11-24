// Seasonal events and packed script variables (EN build). JP twin: app_0014.cpp.
#include <windows.h>

#include "app_0014.h"
#include "engine.h"
#include "listbank.h"
#include "mmv_player.h"
#include "scene_start.h"
#include "texture_mgr.h"

// Month and day numbers of the seasonal events.
const int MONTH_JANUARY = 1;
const int MONTH_DECEMBER = 12;
const int DAY_CHRISTMAS = 25;
const int DAY_CHRISTMAS_EVE = 24;
const int HOUR_CHRISTMAS_EVE_START = 18;   // Christmas eve counts after this hour
const int DAY_NEW_YEAR_END = 4;            // January 1-3 (days before the 4th)

// Reads the PC clock. Returns SEASON_CHRISTMAS on Christmas (December 25, or December 24 after 18:00),
// SEASON_NEW_YEAR on January 1-3, else SEASON_NONE. Seasonal content depends on it.
int SeasonalEvent()
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    if (now.wMonth == MONTH_DECEMBER)
    {
        if (now.wDay == DAY_CHRISTMAS || (now.wDay == DAY_CHRISTMAS_EVE && now.wHour > HOUR_CHRISTMAS_EVE_START))
            return SEASON_CHRISTMAS;
        return SEASON_NONE;
    }
    if (now.wMonth == MONTH_JANUARY && now.wDay < DAY_NEW_YEAR_END)
        return SEASON_NEW_YEAR;
    return SEASON_NONE;
}

// Layout of a packed script variable: low nibble = type, then two 8-bit fields.
const int PACKED_TYPE_MASK = 0xf;
const int PACKED_TYPE_PAIR = 1;       // the variable holds a packed pair
const int PACKED_LOW_SHIFT = 5;       // first field: bits 5..12
const int PACKED_HIGH_SHIFT = 13;     // second field: bits 13..20
const int PACKED_FIELD_MASK = 0xff;
const int PACKED_LOW_MAX = 0x80;      // largest valid first field
const int PACKED_HIGH_MAX = 0xff;     // largest valid second field

// Script_UnpackVar: when script variable `index` holds a packed pair, returns 1 and stores its first field (bits 5..12,
// valid 0..0x80) in *low and its second field (bits 13..20, valid 0..0xff) in *high; out-of-range fields become 0.
// Returns 0 when the variable holds something else. Called through the script command table.
extern "C" int Script_UnpackVar(int index, int *high, int *low)
{
    int value = g_scriptFlags[index];
    if ((value & PACKED_TYPE_MASK) == PACKED_TYPE_PAIR)
    {
        *low = (value >> PACKED_LOW_SHIFT) & PACKED_FIELD_MASK;
        *high = (g_scriptFlags[index] >> PACKED_HIGH_SHIFT) & PACKED_FIELD_MASK;
        if (*low < 0 || *low > PACKED_LOW_MAX)
            *low = 0;
        if (*high < 0 || *high > PACKED_HIGH_MAX)
            *high = 0;
        return 1;
    }
    return 0;
}

// Folder argument of Path_Data for the movie files.
const int PATH_FOLDER_MOVIE = 0;

// Opens movie `name` in the movie player, decodes its first frame into the movie texture slot and records the movie's
// frame count. `secondPalette` picks the palette the frame is drawn with. Returns 1. Called by the scene host callback
// when a scene movie starts.
extern "C" int Movie_PlayToTexture(const char *name, int secondPalette)
{
    g_moviePlayer.Open(Path_Data(name, PATH_FOLDER_MOVIE));
    g_moviePlayer.DecodeNextFrame();
    g_moviePlayer.DrawToTexture(&g_textures[g_movieTextureSlot], secondPalette);
    g_movieFrameCount = g_moviePlayer.size;
    g_movieFramesDecoded = 1;
    return 1;
}
