// The world renderer (the map scene object): its constructor and Bind, which resets it for a new scene (EN build).
// Ported from the matched JP source decomp/src/world_ctor.cpp.
#include <stdio.h>

#include "anim_model.h"
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
        m_00980[i] = WORLD_ID_EMPTY;
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
