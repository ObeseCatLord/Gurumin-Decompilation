// Ground height from the height maps of the map's ground objects (called by 0x1011d810).

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "ground_height.h"


#include "anim_model.h"
extern "C" GroundOrigin *DAT_1020d860[0x60];         // the ground objects (0 = free)
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
