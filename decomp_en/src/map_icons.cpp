// Map icon textures (JP twin: map_icons.cpp).

#include <string.h>

#include "map_records.h"
#include "texture_mgr.h"
#include "render.h"

// Extension of an icon file name (".bmp"): the length cut off before the texture name lookup.
static const int ICON_EXTENSION_LENGTH = 4;
// Icon loaded when the entry's own icon is missing.
static const char ICON_FALLBACK[] = "maruf00a.bmp";

// Returns the texture of the icon of the current map: the icon of the first table entry with the current entry's map
// id, found by its name without the extension or loaded (with "maruf00a.bmp" as the fallback; the frame timer is
// reset afterwards because loading takes long). Returns 0 when no entry matches. Called by the map screen.
extern "C" int Map_IconTexture(void)
{
    char name[MAP_ICON_NAME_SIZE];
    int i;
    for (i = 0; i < g_mapEntryCount; i++)
    {
        if (g_mapRecords[g_currentMapEntry].entry.id == g_mapRecords[i].entry.id)
            break;
    }
    if (i == g_mapEntryCount)
        return 0;
    strcpy(name, g_mapIconNames[g_mapRecords[i].entry.icon]);
    size_t length = strlen(name) - ICON_EXTENSION_LENGTH;
    name[length > 0 ? length : 0] = 0;
    int texture = Texture_FindByName(name);
    if (texture < 0)
    {
        texture = Texture_Load(g_mapIconNames[g_mapRecords[i].entry.icon], ICON_FALLBACK);
        g_textureManager.ResetFrameTimer();
    }
    return texture;
}
