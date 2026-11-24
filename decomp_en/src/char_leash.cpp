// Character leash (JP twin: char_leash.cpp).

#include <math.h>

#include "char_shots.h"
#include "enemy_flee.h"
#include "move_limit.h"

// 1 when moving character `index` by `move` (x and y used) would take it further from its start position than its x
// or y range allows, else 0. Called by the enemy moves before they step.
extern "C" int Char_OutsideLimit(int index, Vec4 move)
{
    float rangeX = (float)g_moveLimits[index].rangeX;
    float rangeY = (float)g_moveLimits[index].rangeY;
    if (g_moveLimits[index].limitX == MOVE_LIMIT_ENABLED && rangeX != 0.0f &&
        (float)fabs(move.x + g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] - g_shotSlots[index].position.x) > rangeX)
        return 1;
    if (g_moveLimits[index].limitY == MOVE_LIMIT_ENABLED && rangeY != 0.0f &&
        (float)fabs(move.y + g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] - g_shotSlots[index].position.y) > rangeY)
        return 1;
    return 0;
}
