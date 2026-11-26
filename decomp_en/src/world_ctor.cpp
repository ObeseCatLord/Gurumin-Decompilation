// The world renderer (the map scene object): its constructor and Bind, which resets it for a new scene (EN build).
// Ported from the matched JP source decomp/src/world_ctor.cpp.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anim_model.h"
#include "game_file.h"
#include "itm_scramble.h"
#include "world_renderer.h"

// Number of entries of the two small tables cleared by the constructor.
const int WORLD_SMALL_TABLE_SIZE = 0x14;
// Entries of the three message id tables.
const int WORLD_ID_TABLE_SIZE = 1000;
// Marker of an empty entry of the id tables.
const int WORLD_ID_EMPTY = -9999;
// Pictures (and lip names, tweens) per world.
const int WORLD_LIP_TEXTURES = 0x10;
// Model records the scene resets (the rest of the character table is unused).
const int WORLD_BOUND_MODELS = 0x60;
// Frames of a scene before the script ends, unless the script says otherwise.
const int WORLD_DEFAULT_FRAME_LIMIT = 900;

// Constructs the members, clears the two 0x14-entry tables, the names and flags of the 0x20 map slots and sets
// their texture slots to -1. Runs once at start-up for the global world object.
WorldRenderer::WorldRenderer()
{
    for (int i = 0; i < WORLD_SMALL_TABLE_SIZE; i++)
    {
        m_e6d34[i] = 0;
        m_e6d84[i] = 0;
    }
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        faceTextureNames[i][0] = 0;
        pictureKind[i] = 0;
        textures[i] = -1;
        lipActive[i] = 0;
        nodeFiles[i][0] = 0;
    }
}

// Resets the scene for a new map: the character slots get their default names (the katakana word for "character" plus the slot number, Shift-JIS), the three
// 1000-entry tables -9999, the map slots, textures, node tables and fade state are cleared, StartFade(0, 0, 0) runs,
// and the model records, callback and callback data are stored. mapRoot and state are not used. Returns 1. Called
// when a scene is set up.
int WorldRenderer::Bind(AnimModel *models, void *mapRoot, int *state, WorldCallback callback, void *data)
{
    bgm = -1;
    transitionsOff = 0;
    m_00975 = 0;
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        m_eb058[i] = -1;
        sprintf(characterNames[i], "\x83\x4c\x83\x83\x83\x89%d", i);
    }
    effectLevel = 0;
    g_scriptedCameraSlot = 0;
    m_00974 = 0;
    fadeOnTop = 1;
    barsFull = 0;
    g_lightScale = 1.0f;
    for (int i = 0; i < WORLD_ID_TABLE_SIZE; i++)
    {
        lowestIdCache[i] = WORLD_ID_EMPTY;
        m_01920[i] = WORLD_ID_EMPTY;
        m_028c0[i] = WORLD_ID_EMPTY;
    }
    m_03860 = WORLD_ID_EMPTY;
    moviePicture = 0;
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        for (int j = 0; j < WORLD_LIP_TEXTURES; j++)
            lipNames[i][j][0] = 0;
        faceTextureNames[i][0] = 0;
        pictureKind[i] = 0;
        textures[i] = -1;
        lipActive[i] = 0;
        nodeFiles[i][0] = 0;
    }
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        lipActive[i] = 0;
        nodeAngleX[i] = 0;
        nodeAngleY[i] = 0;
        nodeAngleZ[i] = 0;
        nodeAngleXFrames[i] = 0;
        nodeAngleYFrames[i] = 0;
        nodeAngleZFrames[i] = 0;
        nodeA[i] = 0;
        nodeScale[i] = 1.0f;
        nodeT[i] = 1.0f;
        nodeAFrames[i] = 0;
        nodeScaleFrames[i] = 0;
        nodeTFrames[i] = 0;
        textures[i] = -1;
        nodeFiles[i][0] = 0;
    }
    movieActive = 0;
    notifyPending = 0;
    lastScriptFrame = -1;
    frameLimit = WORLD_DEFAULT_FRAME_LIMIT;
    font = (FontRenderer *)data;
    captionVisible = 0;
    fadeTarget = 0;
    fadeAmount = 0;
    StartFade(0, 0, 0);
    this->callback = callback;
    this->models = models;
    eventsMuted = 0;
    for (int i = 0; i < WORLD_BOUND_MODELS; i++)
    {
        modelPartner[i] = -1;
        modelAlias[i] = -1;
        costumeNames[i][0] = 0;
        modelFileNames[i][0] = 0;
    }
    mapModelName[0] = 0;
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        modelAlphaFrames[i] = 0;
        modelAlphaStep[i] = 0;
    }
    return 1;
}

// Fades the screen to `target` percent over `frames` frames (0 = at once), remembering `mode`. TickFade advances it
// each frame. Returns 0. Called by the scene script (CFD) and when a scene is set up.
int WorldRenderer::StartFade(int mode, int target, int frames)
{
    float value = (float)target;
    fadeMode = mode;
    fadeTarget = value;
    if (frames == 0)
    {
        fadeStep = 0;
        fadeAmount = value;
        fadeTarget = value;
        fadeFrames = 0;
        return 0;
    }
    fadeFrames = frames;
    fadeStep = (value - fadeAmount) / frames;
    return 0;
}

// Bytes at the end of an .itm file: the body size, the byte sum and the xor check.
// Host callback code sent when a character is loaded.
const int WORLD_CALLBACK_LOAD_CHARACTER = 9999;
// Motion key banks of the world.
const int WORLD_MOTION_BANKS = 0xc;
// Size of the path buffer.
const int MAX_PATH_LENGTH = 0x104;
// Size of the base name buffer.
const int ITM_NAME_BUFFER = 0x100;
const int ITM_TRAILER_SIZE = 0xc;
// Size of the buffer that holds the five digits of a message id after the "##" marker.
const int ITM_ID_BUFFER = 8;
// Characters of the "itm" file extension.
const int ITM_EXTENSION_LENGTH = 3;

// Resets the scene like Bind (and tells the host with callback code 9999), clears the key banks, copies two flags of
// each of the 0x60 model records, then reads the character's message file: `path` with its extension replaced by
// "itm", descrambled with its base name. Each line "##nnnnn text" adds message id nnnnn and its text (up to the end
// of the line). Returns 0 when the file does not pass the check, else 1 (also when there is no file). Called when a
// scene is set up (scene_start).
int WorldRenderer::LoadCharacter(const char *path)
{
    bgm = -1;
    transitionsOff = 0;
    m_00975 = 0;
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        m_eb058[i] = -1;
        sprintf(characterNames[i], "\x83\x4c\x83\x83\x83\x89%d", i);
    }
    effectLevel = 0;
    g_scriptedCameraSlot = 0;
    m_00974 = 0;
    fadeOnTop = 1;
    barsFull = 0;
    g_lightScale = 1.0f;
    for (int i = 0; i < WORLD_ID_TABLE_SIZE; i++)
    {
        lowestIdCache[i] = WORLD_ID_EMPTY;
        m_01920[i] = WORLD_ID_EMPTY;
        m_028c0[i] = WORLD_ID_EMPTY;
    }
    m_03860 = WORLD_ID_EMPTY;
    moviePicture = 0;
    if (callback != 0)
        callback(WORLD_CALLBACK_LOAD_CHARACTER, 0);
    lastScriptFrame = -1;
    movieActive = 0;
    notifyPending = 0;
    postStrength = 0;
    postStep = 0;
    postFrames = 0;
    frame = 0;
    levelKeys[0].Clear();
    levelKeys[1].Clear();
    levelKeys[2].Clear();
    levelKeys[3].Clear();
    levelKeyStart[0] = 0;
    levelKeyStart[1] = 0;
    levelKeyStart[2] = 0;
    levelKeyStart[3] = 0;
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        for (int j = 0; j < WORLD_LIP_TEXTURES; j++)
            lipNames[i][j][0] = 0;
        faceTextureNames[i][0] = 0;
        lipActive[i] = 0;
        nodeAngleX[i] = 0;
        nodeAngleY[i] = 0;
        nodeAngleZ[i] = 0;
        nodeAngleXFrames[i] = 0;
        nodeAngleYFrames[i] = 0;
        nodeAngleZFrames[i] = 0;
        nodeA[i] = 0;
        nodeScale[i] = 1.0f;
        nodeT[i] = 1.0f;
        nodeAFrames[i] = 0;
        nodeScaleFrames[i] = 0;
        nodeTFrames[i] = 0;
        textures[i] = -1;
        pictureKind[i] = 0;
        nodeFiles[i][0] = 0;
    }
    for (int i = 0; i < WORLD_MODEL_SLOTS; i++)
    {
        scaleMode[i] = 0;
        scaleValue[i] = 0;
        scaleStep[i] = 0;
        scaleFrames[i] = 0;
    }
    int model;
    for (model = 0; model < WORLD_BOUND_MODELS; model++)
    {
        if (models != 0)
        {
            m_00844[model] = models[model].important;
            m_008c4[model] = models[model].idling;
        }
        modelPartner[model] = -1;
        modelAlias[model] = -1;
        costumeNames[model][0] = 0;
        modelFileNames[model][0] = 0;
    }
    mapModelName[model] = 0;
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        nodePositionKeys[i].Clear();
        modelAlphaFrames[i] = 0;
        modelAlphaStep[i] = 0;
    }
    for (int i = 0; i < WORLD_MOTION_BANKS; i++)
        motionKeys[i].Clear();
    levelValue[0] = 0;
    levelFrames[0] = 0;
    levelStep[0] = 0;
    levelValue[1] = 0;
    levelFrames[1] = 0;
    levelStep[1] = 0;
    levelValue[2] = 0;
    levelFrames[2] = 0;
    levelStep[2] = 0;
    levelValue[3] = 0;
    levelFrames[3] = 0;
    levelStep[3] = 0;
    enabled = 0;
    GameFile file;
    messageCount = 0;
    char itmPath[MAX_PATH_LENGTH];
    strcpy(itmPath, path);
    strcpy(&itmPath[strlen(itmPath) - ITM_EXTENSION_LENGTH], "itm");
    FILE *opened = file.Open(itmPath, "rb");
    if (opened != 0)
    {
        char baseName[ITM_NAME_BUFFER];
        Path_GetBaseNameWorldCopy(baseName, path);
        file.Seek(0, SEEK_END);
        int size = file.Size() - ITM_TRAILER_SIZE;
        file.Seek(0, SEEK_SET);
        char *text = (char *)operator new(size);
        int bodySize;
        int sum;
        unsigned int check;
        file.Read(text, 1, size);
        file.Read(&bodySize, 1, 4);
        file.Read(&sum, 1, 4);
        file.Read(&check, 1, 4);
        file.Close();
        if (Itm_Descramble((unsigned char *)text, size, baseName, sum, check) == 0)
        {
            free(text);
            return 0;
        }
        messageCount = 0;
        char *p = text;
        char c = *p++;
        int position = 1;
        while (c != 0 && position < size)
        {
            if (c == '#')
            {
                c = *p++;
                position++;
                if (c == '#')
                {
                    char number[ITM_ID_BUFFER];  // the five digits of the id; not terminated (atol stops at what follows)
                    number[0] = *p++;
                    number[1] = *p++;
                    number[2] = *p++;
                    number[3] = *p++;
                    number[4] = *p++;
                    messageIds[messageCount] = (short)atol(number);
                    int length = 0;
                    c = *p++;
                    position += 6;
                    while (c != 0 && c != '\n' && c != '\r' && position < size)
                    {
                        messages[messageCount][length] = c;
                        messages[messageCount][length + 1] = 0;
                        length++;
                        c = *p++;
                        position++;
                    }
                    messageCount++;
                }
            }
            c = *p++;
            position++;
        }
        free(text);
    }
    return 1;
}
