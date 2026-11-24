// Model bank, per-frame preparation: which lights shine on which models and which models cast shadows this frame.
#include <stdlib.h>

#include "node.h"
#include "vec4.h"
#include "matrix4.h"
#include "anim_model.h"
#include "model_bank.h"
#include "camera_vars.h"
#include "render_vars.h"

extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *matrix, float margin);  // 0x101ae4d0 (vec4.cpp)
#include "depth_sort.h"
extern "C" void FUN_1014b480(void);                                                   // 0x1014b480 (render_reset.cpp)

// Node::flags of a light node: it makes a lens flare / it lights models.
#define LIGHT_NODE_LENS_FLARE 0x8
#define LIGHT_NODE_LIGHTS_MODELS 0x10000
// A light point inside the view volume widened by this is a flare point.
#define FLARE_VISIBLE_MARGIN 24.0f
// A lit model is kept when its position is inside the view volume widened by this.
#define MODEL_VISIBLE_MARGIN 24.0f
// Models further left than this have not been placed yet.
#define MODEL_UNPLACED_X -100000.0f
// A flare brightens over the last 1/FLARE_FADE_RANGE of the depth range.
#define FLARE_FADE_RANGE 70.0f
// Lights: number of the nearest ones kept per model, the most the camera or spare entry takes, and the limits.
#define LIGHTS_PER_MODEL 3
#define LIGHTS_PER_SPARE_ENTRY 6
#define SHADOW_CASTER_LIMIT 10
#define LIGHT_ENTRY_CAMERA 0
// Vectors of a lit model's entry in `lighting`: (direction, colour) for each of three lights, then one fade
// value (in w) for each of six.
#define LIGHT_VECTOR_FADE 8
// The default caster (no light in the scene): the light stands above and beside the model; the caster's size.
#define DEFAULT_LIGHT_RAISE 500.0f
#define DEFAULT_LIGHT_SIDE 100.0f
#define DEFAULT_LIGHT_BACK 200.0f
#define DEFAULT_CASTER_SIZE 600.0f
#define DEFAULT_LIGHT_STRENGTH 0.71f
#define LIGHT_RANGE_MARGIN 200.0f
#define LIGHT_FADE_DIVISOR 50.0f
// Value of AnimModel::m_17e8 of a model that casts no shadow texture (1), or only when shadow targets exist (2).
#define MODEL_NO_SHADOW 1
#define MODEL_TARGET_SHADOW 2

// 0x10165210. qsort comparison of two lit model entries by DAT_10fa6bb8 (nearest first).
extern "C" int FUN_10165210(const void *first, const void *second)
{
    if (DAT_10fa6bb8[*(const int *)first] < DAT_10fa6bb8[*(const int *)second])
        return -1;
    return 1;
}

// 0x10165240 (ModelBank::PrepareFrame). Returns 0 when the bank is off. Otherwise, each frame: collects the lens
// flare points of the lights in view (flareScreen/flareAlpha); keeps the lit models that are in view, placed and
// have collision (and resets the placed-at-a-fixed-position ones to their position), sorted by their distance to the
// camera target; and for each of them takes the three nearest lights (direction, colour and fade into `lighting`)
// and makes it a shadow caster entry (m_0484/m_0c84/casterSize/casters), drawing its shadow texture when
// `renderShadowTextures` is set. The camera entry and the spare entry take up to six lights and up to three caster
// entries. Returns 1.
int ModelBank::PrepareFrame(const Matrix4 *world, const Matrix4 *view, int a3, int renderShadowTextures)
{
    if (enabled == 0)
        return 0;
    int casterCount = 0;
    Vec4 position;
    Vec4 origin;
    Vec4 lightPos[0x20];
    int tileY[15] = {0, 0, 0, 0, 0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x100, 0x100, 0x100, 0x100, 0x100};
    int tileX[15] = {0, 0x80, 0x100, 0x180, 0x200, 0, 0x80, 0x100, 0x180, 0x200, 0, 0x80, 0x100, 0x180, 0x200};
    flareCount = 0;
    m_1884 = 0;
    for (int i = 0; i < 0x20; i++)
    {
        if (lights[i] != 0)
        {
            lightPos[i].Copy(lights[i]->worldMatrix.GetTranslation());
            if ((lights[i]->flags & LIGHT_NODE_LENS_FLARE) != 0 && FUN_101ae4d0(&lightPos[i], view, FLARE_VISIBLE_MARGIN) != 0)
            {
                flareScreen[flareCount].Copy(Project(lightPos[i], *view));
                if (flareScreen[flareCount].z > 0.0f && flareScreen[flareCount].z < 1.0f)
                {
                    float alpha = (1.0f - flareScreen[flareCount].z) * FLARE_FADE_RANGE;
                    flareAlpha[flareCount] = 1.0f > alpha ? alpha : 1.0f;
                    flareCount++;
                }
            }
        }
    }
    int visible[0x80];
    int visibleCount = 0;
    for (int s = 0; s < 0x80; s++)
    {
        if (litModels[s] == 0)
        {
            if (s == LIGHT_SPARE_ENTRY)
                visible[visibleCount++] = s;
        }
        else if (s == LIGHT_SPARE_ENTRY)
            visible[visibleCount++] = s;
        else
        {
            if (litModels[s]->m_2390 != MODEL_BANK_NO_TIMEOUT &&
                FUN_101ae4d0(&litModels[s]->matrix.GetTranslation(), view, MODEL_VISIBLE_MARGIN) == 0)
                continue;
            if (litModels[s]->m_2390 == MODEL_BANK_NO_TIMEOUT)
            {
                litModels[s]->m_17e8 = 0;
                litModels[s]->matrix.SetIdentity();
                litModels[s]->matrix.SetTranslation(*params[litModels[s]->bankIndex]);
                if (litModels[s]->root == 0)
                {
                    litModels[s] = 0;
                    continue;
                }
                if (litModels[s]->matrix._41 < MODEL_UNPLACED_X)
                    continue;
            }
            else
            {
                if (litModels[s]->root == 0)
                {
                    litModels[s] = 0;
                    continue;
                }
                if (litModels[s]->matrix._41 < MODEL_UNPLACED_X || litModels[s]->shadowHidden != 0 ||
                    litModels[s]->shadowOpacity == 0)
                    continue;
            }
            position.Copy(litModels[s]->matrix.GetTranslation());
            if (litModels[s]->loadedRoot->hasCollision != 0)
            {
                DAT_10fa6bb8[s] = position.DistanceSquared(DAT_10f668e8);
                visible[visibleCount++] = s;
            }
        }
    }
    if (visibleCount - 2 > 0)
        qsort(&visible[1], visibleCount - 2, sizeof(int), FUN_10165210);
    for (int v = 0; v < visibleCount; v++)
    {
        int entry = visible[v];
        if (entry == LIGHT_SPARE_ENTRY)
        {
            position.Copy(DAT_10f668e8);
            origin.Copy(DAT_10f668e8);
        }
        else
        {
            position.Copy(litModels[entry]->matrix.GetTranslation());
            origin.Copy(litModels[entry]->matrix.GetTranslation());
        }
        float distances[0x20];
        int nearLights[0x20];
        float ranges[0x20];
        float keys[0x20];
        int nearCount = 0;
        for (int i = 0; i < 0x20; i++)
        {
            if (lights[i] != 0 && (lights[i]->flags & LIGHT_NODE_LIGHTS_MODELS) != 0)
            {
                float distance = position.Distance(lightPos[i]);
                float range = lights[i]->fogFar;
                distances[nearCount] = distance;
                nearLights[nearCount] = i;
                ranges[nearCount] = range;
                nearCount++;
                keys[nearCount - 1] = (distance + distance) / range;
            }
        }
        if (nearCount == 0)
        {
            Vec4 unusedTint(1.0f, 1.0f, 1.0f, 0.0f);
            Vec4 *entryLights = &lighting[entry * LIGHT_VECTORS];
            m_0484[casterCount].Copy(position);
            m_0c84[casterCount].Copy(position);
            casterSize[casterCount] = DEFAULT_CASTER_SIZE;
            m_0c84[casterCount].z += DEFAULT_LIGHT_RAISE;
            m_0c84[casterCount].x += DEFAULT_LIGHT_SIDE;
            m_0c84[casterCount].y -= DEFAULT_LIGHT_BACK;
            casters[casterCount] = entry;
            Vec4 direction;
            direction.Copy(m_0c84[casterCount] - m_0484[casterCount]);
            float strength = DEFAULT_LIGHT_STRENGTH;
            entryLights[0].Copy(Vec3_Normalized(&direction) * strength);
            entryLights[1].Set(1.0f, 1.0f, 1.0f, 1.0f);
            entryLights[2].Set(0, 0, 0, 0);
            entryLights[3].Set(0, 0, 0, 0);
            entryLights[4].Set(0, 0, 0, 0);
            entryLights[5].Set(0, 0, 0, 0);
            entryLights[8].Set(0, 0, 0, 0);
            entryLights[9].Set(0, 0, 0, 0);
            entryLights[10].Set(0, 0, 0, 0);
            entryLights[11].Set(0, 0, 0, 0);
            entryLights[12].Set(0, 0, 0, 0);
            entryLights[13].Set(0, 0, 0, 0);
            entryLights[14].Set(0, 0, 0, 0);
            entryLights[15].Set(0, 0, 0, 0);
            entryLights[16].Set(0, 0, 0, 0);
            entryLights[17].Set(0, 0, 0, 0);
            entryLights[18].Set(0, 0, 0, 0);
            entryLights[19].Set(0, 0, 0, 0);
            if (entry != LIGHT_SPARE_ENTRY && litModels[entry]->m_17e8 != MODEL_NO_SHADOW &&
                (litModels[entry]->m_17e8 != MODEL_TARGET_SHADOW || DAT_10203434 != 0))
            {
                if (renderShadowTextures != 0 && casterCount < SHADOW_CASTER_LIMIT && DAT_10203434 != 0)
                    litModels[entry]->root->RenderShadowTexture(&litModels[entry]->matrix, view, a3 / 2,
                                                                &m_0484[casterCount], &m_0c84[casterCount],
                                                                shadows[casterCount], litModels[entry]->loadedRoot->bounds.w,
                                                                tileX[casterCount], tileY[casterCount]);
                casterCount++;
            }
        }
        else
        {
            FUN_10165020(keys, distances, ranges, nearLights, nearCount);
            Vec4 *entryLights = &lighting[entry * LIGHT_VECTORS];
            for (int j = 0; j < LIGHTS_PER_MODEL; j++)
            {
                if (j < nearCount)
                {
                    Vec4 offset;
                    offset.Copy(lightPos[nearLights[j]] - origin);
                    float reach = ranges[j] - offset.Length();
                    float falloff;
                    if (reach <= 0.0f)
                        falloff = 0.0f;
                    else
                        falloff = reach;
                    float scale = falloff / (ranges[j] + LIGHT_RANGE_MARGIN);
                    entryLights[j * 2].Copy(Vec3_Normalized(&offset) * scale);
                    entryLights[j * 2 + 1].Copy(lights[nearLights[j]]->fogColour);
                }
                else
                {
                    entryLights[j * 2].Set(0, 0, 0, 0);
                    entryLights[j * 2 + 1].Set(0, 0, 0, 0);
                }
            }
            for (int j = 0; j < (nearCount > LIGHTS_PER_SPARE_ENTRY - 1 ? LIGHTS_PER_SPARE_ENTRY : nearCount); j++)
            {
                if (j != 0 && entry != LIGHT_ENTRY_CAMERA && entry != LIGHT_SPARE_ENTRY)
                    break;
                if (j < LIGHTS_PER_MODEL && entry != LIGHT_SPARE_ENTRY)
                {
                    int light = nearLights[j];
                    m_0484[casterCount].Copy(position);
                    m_0c84[casterCount].Copy(lightPos[light]);
                    casters[casterCount] = entry;
                    casterSize[casterCount] = distances[j] * 2 * DEFAULT_CASTER_SIZE / ranges[j];
                    entryLights[j * 2].Copy(m_0c84[casterCount]);
                    entryLights[j * 2 + 1].Copy(lights[light]->fogColour);
                    entryLights[LIGHT_VECTOR_FADE + j * 2].w = lights[light]->fogDensity * ranges[j] / LIGHT_FADE_DIVISOR;
                    if (litModels[entry]->m_17e8 == MODEL_NO_SHADOW ||
                        (litModels[entry]->m_17e8 == MODEL_TARGET_SHADOW && DAT_10203434 == 0))
                        continue;
                    if (renderShadowTextures != 0 && casterCount < SHADOW_CASTER_LIMIT && DAT_10203434 != 0)
                        litModels[entry]->root->RenderShadowTexture(&litModels[entry]->matrix, view, a3 / 2,
                                                                    &m_0484[casterCount], &m_0c84[casterCount],
                                                                    shadows[casterCount], litModels[entry]->loadedRoot->bounds.w,
                                                                    tileX[casterCount], tileY[casterCount]);
                    casterCount++;
                }
                else
                {
                    int light = nearLights[j];
                    entryLights[j * 2].Copy(lightPos[light]);
                    entryLights[j * 2 + 1].Copy(lights[light]->fogColour);
                    entryLights[LIGHT_VECTOR_FADE + j * 2].w = ranges[j] * lights[light]->fogDensity / LIGHT_FADE_DIVISOR;
                }
            }
        }
    }
    if (DAT_10203434 != 0)
        FUN_1014b480();
    m_1884 = casterCount;
    return 1;
}
