// Ground height from the height maps of the map's ground objects (called by 0x1011d810).

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "ground_height.h"


#include "anim_model.h"
#include "map_vars.h"

// 0x10016700 (Ground_HeightAt). Picks the ground object nearest to the player and averages its height samples
// around (x, y): the samples within 72 units of the point on the 25-unit grid whose weight (60 minus a fifth of the
// x and y distances) is above 1, ignoring missing samples (-200 and below). Returns the average plus the object's
// base height, or -5000 when there is no object or no sample.
extern "C" float FUN_10016700(int x, int y)
{
    Vec4 player;
    Vec4 position;
    player.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
    int found = 0;
    DAT_104bfdb4 = -1;
    float best = 99999.0f;
    for (int i = 0; i < 0x60; i++)
    {
        if (DAT_1020d860[i] == 0)
            continue;
        position.Copy(((GroundObject *)DAT_1020d860[i])->matrix.GetTranslation());
        float distance = position.Distance(player);
        if (found == 0 || distance < best)
        {
            found = 1;
            DAT_104bfdb4 = i;
            best = distance;
        }
    }
    if (DAT_104bfdb4 < 0)
        return -5000.0f;
    {
        GroundOrigin *ground = DAT_1020d860[DAT_104bfdb4];
        x -= (int)ground->x;
        y -= (int)ground->y;
        int count = 0;
        float sum = 0.0f;
        for (int row = -5; row < 8; row++)
        {
            for (int column = -5; column < 6; column++)
            {
                int dx = column * 25 - x;
                int dy = row * 25 - y;
                if (sqrt((float)(dx * dx + dy * dy)) < 72.0)
                {
                    float spread = (float)fabs((float)row * 25.0f - (float)y) + (float)fabs((float)column * 25.0f - (float)x);
                    float weight = 60.0f - spread * 0.2f;
                    if (0.0f < weight && weight > 1.0f && DAT_1049f3dc[DAT_104bfdb4][column][row] > -200.0f)
                    {
                        sum += DAT_1049f3dc[DAT_104bfdb4][column][row];
                        count++;
                    }
                }
            }
        }
        if (count == 0)
            return -5000.0f;
        return sum / (float)count + DAT_1020d860[DAT_104bfdb4]->base;
    }
}

#include "game_vars.h"
#include "script_vars.h"

int FUN_1014b1c0(short value);                   // 0x1014b1c0: seeds the random table position
extern "C" int FUN_1013f880(void);               // the game's random number

#define HEIGHT_GRID 14                           // samples per side of a ground object's height map
#define HEIGHT_STEP 25                           // distance between samples
#define HEIGHT_MIN_OFFSET (-5 * HEIGHT_STEP)     // the first sample is placed here relative to the object
#define HEIGHT_SOURCES 5                         // hills: one in the middle and four around it
#define HEIGHT_SOURCE_SLOTS 16
#define HEIGHT_ROUGHNESS 20                      // random bumps of 0..19 on top of the shape
#define HEIGHT_BASE 100.0f                       // the shape is lifted by 20 and lowered by this

// 0x100169c0 (Ground_BuildHeights). Fills the 14x14 height samples of ground object `slot` (and copies them to
// DAT_1042e3f0): the shape is the sum of five hills (one in the middle, four 80 units around it) for hill style
// `style` (0, 2, 3 or 4; other styles stay flat) plus a random 0..19 bump per sample. Re-seeds the random table first.
extern "C" void FUN_100169c0(int slot, int style)
{
    FUN_1014b1c0(0);
    Vec4 sample;
    Vec4 sources[HEIGHT_SOURCE_SLOTS];
    sources[0].Set(0.0f, 0.0f, 50.0f, 0.0f);
    sources[1].Set(80.0f, 0.0f, 40.0f, 0.0f);
    sources[2].Set(0.0f, 80.0f, 40.0f, 0.0f);
    sources[3].Set(-80.0f, 0.0f, 40.0f, 0.0f);
    sources[4].Set(0.0f, -80.0f, 40.0f, 0.0f);
    for (int i = 0; i < HEIGHT_GRID; i++)
        for (int j = 0; j < HEIGHT_GRID; j++)
            DAT_1049f200[slot][j][i] = 0.0f;
    int y = HEIGHT_MIN_OFFSET;
    float *rowStart = &DAT_1049f200[slot][2][2];
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
            *cell = (float)(FUN_1013f880() % HEIGHT_ROUGHNESS) + height + 20.0f - HEIGHT_BASE;
            cell += 16;
        }
        rowStart++;
    }
    for (int i = 0; i < HEIGHT_GRID; i++)
        for (int j = 0; j < HEIGHT_GRID; j++)
            DAT_1042e3f0[slot][j][i] = DAT_1049f200[slot][j][i];
}
