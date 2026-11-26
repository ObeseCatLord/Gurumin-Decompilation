// Face (expression) animation of characters (EN build; JP twin: decomp/src/face_anim.cpp, app_0009.cpp).
#include <string.h>
#include "anti_tamper.h"
#include "engine.h"
#include "face_set.h"
#include "texture_mgr.h"

// Loads the .iti face texture `name` from the data folder.
inline int LoadFaceTexture(const char *name)
{
    return Texture_LoadIti(Path_Data(name, 0), 0, 0);
}

// Char_SetFace: shows face frame `frame` on character `index` and schedules the next frame of its face set. The player in costume
// FACE_COSTUME_PLAYER uses the costume's own face textures: the names of character 0 with "14" in positions 2 and 3. Does
// nothing for a character without a face set. Returns 0. Called by the script commands (and expanded in place by Faces_Update).
extern "C" int Char_SetFace(int index, int frame)
{
    if (g_charRecords[index].faceSet < 0)
        return 0;
    g_lastFaceFrame = (short)frame;
    if (index == 0 && g_guardedValue9 == FACE_COSTUME_PLAYER)
    {
        char name[256];
        strcpy(name, g_faceSets[0].names[frame]);
        name[FACE_COSTUME_DIGIT_FIRST] = '1';
        name[FACE_COSTUME_DIGIT_SECOND] = '4';
        g_characterModels[index].ApplyMaterialSwap(LoadFaceTexture(name));
    }
    else
        g_characterModels[index].ApplyMaterialSwap(LoadFaceTexture(g_faceSets[index].names[frame]));
    int next = g_faceSets[g_charRecords[index].faceSet].next[frame];
    if (next == frame)
        g_faceDelay[index] = 0;
    else
    {
        g_faceDelay[index] = g_faceSets[g_charRecords[index].faceSet].delay[frame] + 1;
        g_faceNext[index] = next;
    }
    return 0;
}

// Faces_Update: counts down g_faceDelay of every character slot; when a delay runs out the character shows its g_faceNext
// frame. Returns 0. Called by the script commands that change faces.
extern "C" int Faces_Update(void)
{
    for (int index = 0; index < CHAR_SLOT_COUNT; index++)
    {
        if (g_faceDelay[index] != 0 && --g_faceDelay[index] == 0)
            Char_SetFace(index, g_faceNext[index]);
    }
    return 0;
}
