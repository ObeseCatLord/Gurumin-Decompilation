// Drawing the running effects of one render pass (EN build). Ported from the JP draft decomp/src/char_effects.cpp
// (EffectManager::Draw, 0x10190e30, not matched there) and the EN bytes.
#include <stdio.h>

#include "effect_manager.h"
#include "map_view.h"
#include "texture_mgr.h"

// Render_Command codes the pass uses (the other codes are in texture_mgr.h).
const int RENDER_ALPHA_TEST = 3;       // value 1 = alpha test on, extra = reference alpha
const int RENDER_TEXTURE_FILTER = 7;   // value 1 = linear filtering
const int RENDER_CULL_MODE = 10;       // value = CULL_NONE or CULL_DEFAULT
const int RENDER_DEPTH_TEST = 12;      // value 1 = depth test on
// Cull modes of RENDER_CULL_MODE.
const int CULL_DEFAULT = 0;            // the default face culling
const int CULL_NONE = 1;               // both faces drawn
// Reference alpha of the effect pass.
const int EFFECT_ALPHA_REFERENCE = 0xf;
// Render passes: 0 = opaque pass (list 0, then the list-1 effects not drawn yet), 1 = overlay pass (list 1).
const int EFFECT_PASS_OPAQUE = 0;
const int EFFECT_PASS_OVERLAY = 1;
// A list-1 effect is drawn in the overlay pass when it is in front of the visible area's depth limit (the z of its upper
// corner) plus this margin.
const float EFFECT_DEPTH_MARGIN = 20.0f;
// Passes with their own group of records, and the entries of the group table (the start and the end of each group).
const int EFFECT_PASS_COUNT = 5;
const int EFFECT_PASS_TABLE_SIZE = 10;

// A record that is in a list but free means the lists are corrupt: the original crashes on purpose by reading a null file.
static void Effect_CrashOnFreeRecord(int index)
{
    if (g_effectRecords[index].id == EFFECT_NONE)
        fread(0, 1, 1, 0);
}

// Draws the effects of render pass `pass`: pass 1 draws the list-1 effects whose current part is inside the visible area
// and in front of its depth limit (+20), marking them drawn; the other passes draw their own list, pass 0 also the list-1
// effects not drawn yet. Then the sprites of both tables in the pass's group of records, and in pass 0 the effect lights.
// The effect batch is flushed at the end. Returns 0. Called by the map and effect passes of the scene drawing.
int EffectManager::Draw(const Matrix4 &view, const Matrix4 *camera, unsigned char pass)
{
    Render_Command(RENDER_ALPHA_TEST, 1, EFFECT_ALPHA_REFERENCE);
    Render_Command(RENDER_TEXTURE_FILTER, 1, 0);
    Render_Command(RENDER_DEPTH_WRITE, 0, 0);
    // The records each pass draws sprites for: the first (index = pass) and the end (index = pass + 5).
    int ranges[EFFECT_PASS_TABLE_SIZE] = {0,
                                          EFFECT_GROUP_START_1,
                                          EFFECT_GROUP_START_2,
                                          EFFECT_GROUP_START_3,
                                          EFFECT_GROUP_START_4,
                                          EFFECT_GROUP_START_1,
                                          EFFECT_GROUP_START_2,
                                          EFFECT_GROUP_START_3,
                                          EFFECT_GROUP_START_4,
                                          EFFECT_RECORD_COUNT};
    g_effectBatch.count = 0;
    g_effectBatch.blendMode = BLEND_MODE_UNSET;
    g_effectBatch.texture = 0;
    int i;
    if (pass == EFFECT_PASS_OVERLAY)
    {
        for (i = g_effectListFirst[1]; i > -1; i = g_effectRecords[i].next)
        {
            if (g_effectRecords[i].id == EFFECT_NONE)
                fread(0, 1, 1, 0);
            else
            {
                if (g_effectRecords[i].partPositions[g_effectRecords[i].part].InRect(&g_mapVisibleArea) != 0 &&
                    g_mapVisibleArea.max.z + EFFECT_DEPTH_MARGIN > g_effectRecords[i].partPositions[g_effectRecords[i].part].z)
                {
                    Render_Command(RENDER_CULL_MODE, CULL_NONE, 0);
                    DrawRecord(i, view, camera, 1);
                    g_effectRecords[i].options.settings[EFFECT_SETTING_DRAWN] = 1;
                }
                else
                    g_effectRecords[i].options.settings[EFFECT_SETTING_DRAWN] = 0;
                Effect_CrashOnFreeRecord(i);
            }
        }
    }
    else
    {
        for (i = g_effectListFirst[pass]; i > -1; i = g_effectRecords[i].next)
        {
            if (g_effectRecords[i].id == EFFECT_NONE)
                fread(0, 1, 1, 0);
            else
            {
                Render_Command(RENDER_CULL_MODE, CULL_NONE, 0);
                DrawRecord(i, view, camera, pass);
                Effect_CrashOnFreeRecord(i);
            }
        }
        if (pass == EFFECT_PASS_OPAQUE)
        {
            for (i = g_effectListFirst[1]; i > -1; i = g_effectRecords[i].next)
            {
                if (g_effectRecords[i].id == EFFECT_NONE)
                    fread(0, 1, 1, 0);
                else
                {
                    if (g_effectRecords[i].options.settings[EFFECT_SETTING_DRAWN] == 0)
                    {
                        Render_Command(RENDER_CULL_MODE, CULL_NONE, 0);
                        DrawRecord(i, view, camera, 0);
                        g_effectRecords[i].options.settings[EFFECT_SETTING_DRAWN] = 1;
                    }
                    Effect_CrashOnFreeRecord(i);
                }
            }
        }
    }
    Render_Command(RENDER_ALPHA_TEST, 1, EFFECT_ALPHA_REFERENCE);
    Render_Command(RENDER_TEXTURE_FILTER, 1, 0);
    Render_Command(RENDER_CULL_MODE, CULL_NONE, 0);
    int first = ranges[pass];
    int end = ranges[pass + EFFECT_PASS_COUNT];
    int sprite;
    EffectSprite *spriteA = &g_effectSpritesA[first];
    for (sprite = first; sprite < end; sprite++, spriteA++)
    {
        if (spriteA->type != EFFECT_NONE)
            DrawSpriteA(sprite, view, camera);
    }
    Render_Command(RENDER_CULL_MODE, CULL_NONE, 0);
    for (sprite = first; sprite < end; sprite++)
    {
        if (g_effectSpritesB[sprite].type != EFFECT_NONE)
            DrawSpriteB(sprite, view, camera);
    }
    if (pass == EFFECT_PASS_OPAQUE)
    {
        for (i = 0; i < EFFECT_LIGHT_COUNT; i++)
        {
            if (g_effectLights[i].active != 0)
                DrawLight(i, view, camera);
        }
    }
    // TriangleBatch::Flush, inline in the original.
    if (g_effectBatch.count >= TRIANGLE_VERTICES)
    {
        if (g_effectBatch.texture != 0)
        {
            g_pendingBatches--;
            Render_Command(RENDER_BLEND_MODE, g_effectBatch.blendMode, 0);
            g_effectBatch.texture->DrawTriangles(g_effectBatch.vertices, g_effectBatch.count / TRIANGLE_VERTICES);
            Render_Command(RENDER_BLEND_MODE, 0, 0);
        }
        g_effectBatch.count = 0;
    }
    Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    Render_Command(RENDER_DEPTH_WRITE, 1, 0);
    Render_Command(RENDER_DEPTH_TEST, 1, 0);
    Render_Command(RENDER_CULL_MODE, CULL_DEFAULT, 0);
    return 0;
}
