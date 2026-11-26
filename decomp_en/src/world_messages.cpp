// Message lines of the WorldRenderer (EN build, EN-only: no JP twin found).
#include <string.h>

#include "world_renderer.h"

// Stores line `index` of the character's .itm file: its id and its text (copied without a length check, the lines are
// at most 0xff characters). Returns 0. Called while the .itm file is read (no direct caller).
int WorldRenderer::SetMessage(int index, int id, const char *text)
{
    messageIds[index] = (short)id;
    strcpy(messages[index], text);
    return 0;
}
