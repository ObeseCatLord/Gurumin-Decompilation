// Digging the ground: lowers the height samples of the ground object nearest to the player around a world point,
// uncovers buried treasure and throws up dirt (called by the script builtin 0x3299 on a grid of points).

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "mesh.h"

#include "anim_model.h"
#include "char_vars.h"
#include "debris.h"
#include "effect_holds.h"
#include "effect_manager.h"
#include "game_vars.h"
#include "ground_height.h"
#include "map_vars.h"

extern "C" int FUN_1013f880(void);                                    // the game's random number
extern "C" int FUN_1000e750(void);                                    // anti-tamper guard refresh
extern "C" int FUN_10161920(const char *name);                        // Texture_FindByName
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);  // Sound_PlayEffect

#define HEIGHT_GRID 14                  // samples per side of a ground object's height map
#define HEIGHT_STEP 25                  // distance between samples
#define HEIGHT_FIRST (-5)               // the first sample index (the map starts 5 steps before the object)
#define HEIGHT_LAST 8                   // one past the last sample index
#define DIG_REACH 6000.0                // samples nearer than this to the point are dug
#define DIG_NO_SAMPLE (-10000.0f)       // samples at or below this are holes already
#define DIG_FLOOR (-1000.0f)            // the lowest a sample is left at
#define DIG_BASE_DEPTH 20000.0f         // how far a dig lowers a sample
#define DIG_SLOPE 0.2f                  // plus this fraction of the height above the player
#define DIG_MAX_CELLS 30                // room for the dug rows
#define DIG_LIST_SIZE 128               // room for the dug depths and columns
#define TREASURE_LEVEL_BASE 9000        // treasure markers from here on hold a level
#define TREASURE_LEVEL_MODULUS 1000
#define TREASURE_MARKER DAT_1032d7a8[DAT_104bfdb4]   // the buried treasure marker of the nearest object
#define DIG_COPY_ROWS 10
#define DIG_COPY_COLUMNS 12
#define DIG_COPY_STRIDE 14
#define DIG_NEAR_HOLD_RANGE 500.0f      // hold requests nearer than this to the player are cancelled
#define HOLD_CANCELLED (-200000)
#define DIG_EFFECT 0x2de
#define TREASURE_SOUND 0x333
#define TREASURE_STAMINA 100
#define PLAYER_BUSY_STATE 0x41f
#define DIRT_PIECES 0x20
#define DIRT_TYPE_FIRST 0x6a
#define DIRT_SCATTER 100
#define NODE_HIDDEN_FLAG 0x20

// 0x1003bc60 (Ground_Dig). Picks the ground object nearest to the player and digs it around the world point (x, y):
// every height sample within DIG_REACH of the point drops by 20000 plus a fifth of its height above the player
// (never below -1000 afterwards), the dug-hole flag of the object is copied and its dig timer reset. When the
// sample under the point reaches the depth of the object's treasure marker, the treasure is found: sound, +100 stamina,
// nearby effect requests are cancelled and the object is hidden. A dig effect plays at the player and 32 pieces of
// dirt are thrown, coloured with the object's first texture. Does nothing when no ground object exists.
extern "C" void FUN_1003bc60(int x, int y)
{
    Vec4 player;
    Vec4 position;
    player.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
    int found = 0;
    DAT_104bfdb4 = -1;
    float best = 99999.0f;
    for (int i = 0; i < GROUND_OBJECT_SLOTS; i++)
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
        return;

    x -= (int)DAT_1020d860[DAT_104bfdb4]->x;
    y -= (int)DAT_1020d860[DAT_104bfdb4]->y;
    float heightCopy[DIG_COPY_ROWS][DIG_COPY_STRIDE];
    float dugDepth[DIG_LIST_SIZE];
    int dugColumn[DIG_LIST_SIZE];
    int dugRow[DIG_MAX_CELLS];
    float *from = &DAT_1049f200[DAT_104bfdb4][3][2];
    float *to = &heightCopy[1][0];
    for (int column = 0; column < DIG_COPY_COLUMNS; column++)
    {
        for (int row = -1; row < DIG_COPY_ROWS - 1; row++)
            to[row * DIG_COPY_STRIDE] = from[row * 16];
        from++;
        to++;
    }

    Vec4 offset;
    Vec4 effectPosition;
    offset.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
    effectPosition.Copy(offset);
    offset.x -= DAT_1020d860[DAT_104bfdb4]->x;
    offset.y -= DAT_1020d860[DAT_104bfdb4]->y;
    offset.z -= DAT_1020d860[DAT_104bfdb4]->base;

    int cellCount = 0;
    for (int row = HEIGHT_FIRST; row < HEIGHT_LAST; row++)
    {
        int dy = row * HEIGHT_STEP - y;
        for (int column = HEIGHT_FIRST; column < HEIGHT_LAST; column++)
        {
            int dx = column * HEIGHT_STEP - x;
            if (sqrt((float)(dx * dx + dy * dy)) < DIG_REACH)
            {
                float rowSpread = (float)fabs((float)row * (float)HEIGHT_STEP - (float)y);
                float columnSpread = (float)fabs((float)column * (float)HEIGHT_STEP - (float)x);
                float spread = rowSpread + columnSpread;
                float *sample = &DAT_1049f3dc[DAT_104bfdb4][column][row];
                if (*sample > DIG_NO_SAMPLE)
                {
                    float above = *sample - offset.z;
                    float lift = 0.0f < above ? above : 0.0f;
                    float level = *sample - (lift * DIG_SLOPE + DIG_BASE_DEPTH);
                    *sample = level;
                    if (DAT_104472b0[DAT_104bfdb4] < 2)
                        DAT_104c00be = DAT_104472b0[DAT_104bfdb4];
                    if (DAT_10468430[DAT_104bfdb4] != 0 && DAT_10468430[DAT_104bfdb4] < 0x1e)
                        DAT_10468430[DAT_104bfdb4] = 1;
                    if (column == 0 && row == 0)
                    {
                        if (TREASURE_MARKER < TREASURE_LEVEL_BASE && level < (float)-TREASURE_MARKER ||
                            TREASURE_MARKER >= TREASURE_LEVEL_BASE && level < (float)(TREASURE_MARKER % TREASURE_LEVEL_MODULUS))
                        {
                            if ((DAT_10c64ad0 != 0) + 2 <= DAT_10c64ad4)
                                FUN_1003ad20(TREASURE_SOUND, 0, 1.0f);
                            DAT_10c64ad8 += TREASURE_STAMINA;
                            FUN_1000e750();
                            for (int i = 0; i < DAT_1020b22c; i++)
                            {
                                Vec4 hold;
                                hold.x = (float)DAT_1020dcf8[i].position[0];
                                hold.y = (float)DAT_1020dcf8[i].position[1];
                                hold.z = (float)DAT_1020dcf8[i].position[2];
                                if (hold.Distance(player) < DIG_NEAR_HOLD_RANGE)
                                    DAT_1020dcf8[i].position[0] = HOLD_CANCELLED;
                            }
                            DAT_104c00d0 = PLAYER_BUSY_STATE;
                            ((Node *)DAT_1020d860[DAT_104bfdb4])->SetFlags(NODE_HIDDEN_FLAG);
                        }
                    }
                    if (DAT_1049f3dc[DAT_104bfdb4][column][row] < DIG_FLOOR)
                        DAT_1049f3dc[DAT_104bfdb4][column][row] = DIG_FLOOR;
                    dugDepth[cellCount] = DIG_BASE_DEPTH;
                    dugColumn[cellCount] = column;
                    dugRow[cellCount] = row;
                    cellCount++;
                }
            }
        }
    }
    if (cellCount == 0)
        return;

    DAT_10daaa80.Start(DIG_EFFECT, effectPosition, -1, -1, 0, 0, 0, 0, 0, 0);
    bool swapped;
    do
    {
        swapped = false;
        if (cellCount - 1 < 1)
            break;
        for (int i = 0; i < cellCount - 1; i++)
        {
            if (fabs(dugDepth[i]) < fabs(dugDepth[i + 1]))
            {
                int column = dugColumn[i + 1];
                int row = dugRow[i + 1];
                float depth = dugDepth[i + 1];
                dugColumn[i + 1] = dugColumn[i];
                dugRow[i + 1] = dugRow[i];
                dugDepth[i + 1] = dugDepth[i];
                dugColumn[i] = column;
                dugRow[i] = row;
                dugDepth[i] = depth;
                swapped = true;
            }
        }
    } while (swapped);

    int texture = FUN_10161920(((Node *)DAT_1020d860[DAT_104bfdb4])->materials->textures[0]);
    for (int piece = 0; piece < DIRT_PIECES; piece++)
    {
        Vec4 dirtPosition;
        Vec4 unused;
        Vec4 direction;
        direction.Set(0.0f, 0.0f, 15.0f, 0.0f);
        dirtPosition.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
        dirtPosition.x += (float)(FUN_1013f880() % DIRT_SCATTER - FUN_1013f880() % DIRT_SCATTER);
        dirtPosition.y += (float)(FUN_1013f880() % DIRT_SCATTER - FUN_1013f880() % DIRT_SCATTER);
        FUN_1000e130(dirtPosition.x, dirtPosition.y, dirtPosition.z, &direction, DIRT_TYPE_FIRST + FUN_1013f880() % 2,
                     texture);
    }
}
