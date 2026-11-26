// Ground height maps of the map's ground objects (EN build). Ported from the matched JP source decomp/src/ground_height.cpp.
#include "engine.h"
#include "ground_height.h"
#include "vec4.h"

// Samples per side of a ground object's height map.
const int HEIGHT_GRID = 14;
// Distance between samples.
const int HEIGHT_STEP = 25;
// The first sample is placed here relative to the object.
const int HEIGHT_MIN_OFFSET = -5 * HEIGHT_STEP;
// Hills: one in the middle and four around it.
const int HEIGHT_SOURCES = 5;
// Hill records the function reserves (the first HEIGHT_SOURCES are used).
const int HEIGHT_SOURCE_SLOTS = 16;
// Random bumps of 0..19 on top of the shape.
const int HEIGHT_ROUGHNESS = 20;
// The shape is lifted by 20 and lowered by this.
const float HEIGHT_BASE = 100.0f;

// Ground_BuildHeights: fills the 14x14 height samples of ground object `slot` (and copies them to g_groundHeightsCopy):
// the shape is the sum of five hills (one in the middle, four 80 units around it) for hill style `style` (0, 2, 3 or 4;
// other styles stay flat) plus a random 0..19 bump per sample. Re-seeds the random table first. Called when a ground
// object is created.
extern "C" void Ground_BuildHeights(int slot, int style)
{
    FontRandom_Load(0);
    Vec4 sample;
    Vec4 sources[HEIGHT_SOURCE_SLOTS];
    sources[0].Set(0.0f, 0.0f, 50.0f, 0.0f);
    sources[1].Set(80.0f, 0.0f, 40.0f, 0.0f);
    sources[2].Set(0.0f, 80.0f, 40.0f, 0.0f);
    sources[3].Set(-80.0f, 0.0f, 40.0f, 0.0f);
    sources[4].Set(0.0f, -80.0f, 40.0f, 0.0f);
    for (int i = 0; i < HEIGHT_GRID; i++)
        for (int j = 0; j < HEIGHT_GRID; j++)
            g_groundHeights[slot][j][i] = 0.0f;
    int y = HEIGHT_MIN_OFFSET;
    float *rowStart = &g_groundHeights[slot][2][2];
    for (; y < 200; y += HEIGHT_STEP)
    {
        float *cell = rowStart;
        for (int x = HEIGHT_MIN_OFFSET; x < 200; x += HEIGHT_STEP)
        {
            sample.x = (float)x;
            sample.y = (float)y;
            sample.z = 0.0f;
            float height = 0.0f;
            if (style == 0)
            {
                for (int i = 0; i < HEIGHT_SOURCES; i++)
                {
                    if ((sample - sources[i]).Length2D() < 150.0f)
                    {
                        float away0;
                        if ((sample - sources[i]).Length2D() - 20.0f > 0.0f)
                            away0 = (sample - sources[i]).Length2D() - 20.0f;
                        else
                            away0 = 0.0f;
                        height += ((150.0f - away0) / 150.0f) * sources[i].z;
                    }
                }
            }
            else if (style == 2)
            {
                for (int i = 0; i < HEIGHT_SOURCES; i++)
                {
                    if ((sample - sources[i]).Length2D() < 150.0f)
                    {
                        float away2;
                        if ((sample - sources[i]).Length2D() - 150.0f > 0.0f)
                            away2 = (sample - sources[i]).Length2D() - 150.0f;
                        else
                            away2 = 0.0f;
                        height += ((200.0f - away2) / 200.0f) * sources[i].z;
                    }
                }
            }
            else if (style == 3)
            {
                for (int i = 0; i < HEIGHT_SOURCES; i++)
                {
                    if ((sample - sources[i]).Length2D() < 190.0f)
                    {
                        float away3;
                        if ((sample - sources[i]).Length2D() - 190.0f > 0.0f)
                            away3 = (sample - sources[i]).Length2D() - 190.0f;
                        else
                            away3 = 0.0f;
                        height += ((220.0f - away3) / 300.0f) * sources[i].z;
                    }
                }
            }
            else if (style == 4)
            {
                for (int i = 0; i < HEIGHT_SOURCES; i++)
                {
                    if ((sample - sources[i]).Length2D() < 280.0f)
                    {
                        float away4;
                        if ((sample - sources[i]).Length2D() - 280.0f > 0.0f)
                            away4 = (sample - sources[i]).Length2D() - 280.0f;
                        else
                            away4 = 0.0f;
                        height += ((320.0f - away4) / 300.0f) * sources[i].z;
                    }
                }
            }
            *cell = (float)(Random() % HEIGHT_ROUGHNESS) + height + 20.0f - HEIGHT_BASE;
            cell += 16;
        }
        rowStart++;
    }
    for (int i = 0; i < HEIGHT_GRID; i++)
        for (int j = 0; j < HEIGHT_GRID; j++)
            g_groundHeightsCopy[slot][j][i] = g_groundHeights[slot][j][i];
}
