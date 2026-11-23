// Movement limits: a character can be kept within a range of its start position along x and/or y.
#include <math.h>
#include "world.h"

#include "vec4.h"
#include "anim_model.h"
#include "char_state.h"

#include "slot_position.h"
#include "move_limit.h"


// Absolute value of a float (an inline helper: its argument is rounded to float first).
inline float AbsFloat(float value)
{
    return (float)fabs(value);
}

#include "char_tables.h"

extern "C" SlotPosition DAT_10802bf8[0x80];

// 0x10024e80 (Char_OutsideLimit). 1 when moving character `index` by (dx, dy) would take it further from its start
// position than its x or y range allows, else 0.
extern "C" int FUN_10024e80(int index, float dx, float dy)
{
    float rangeX = (float)DAT_102110b0[index].rangeX;
    float rangeY = (float)DAT_102110b0[index].rangeY;
    if (DAT_102110b0[index].limitX == 1 && rangeX != 0.0f &&
        AbsFloat(dx + DAT_10db6ee8[index].matrix._41 - DAT_10802bf8[index].position.x) > rangeX)
        return 1;
    if (DAT_102110b0[index].limitY == 1 && rangeY != 0.0f &&
        AbsFloat(dy + DAT_10db6ee8[index].matrix._42 - DAT_10802bf8[index].position.y) > rangeY)
        return 1;
    return 0;
}
