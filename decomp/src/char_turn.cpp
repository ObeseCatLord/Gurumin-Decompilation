// Turning characters towards a target heading.
#include "world.h"

// 0x1001bfb0 (Char_LimitTurn). Limits how far character `index` turns this step: the difference between its target
// heading and its heading, wrapped into -pi..pi (pi = 3.1415, a full turn = 6.283 here), is clamped to
// -maxTurn..maxTurn and the target heading is set to heading + that difference. Returns 0.
extern "C" int FUN_1001bfb0(int index, float maxTurn)
{
    float turn = DAT_10db6ed8[index].targetHeading - DAT_10db6ed8[index].heading;
    while (turn > 3.1415f)
        turn -= 6.283f;
    while (turn < -3.1415f)
        turn += 6.283f;
    if (turn > maxTurn)
        turn = maxTurn;
    else if (turn < -maxTurn)
        turn = -maxTurn;
    DAT_10db6ed8[index].targetHeading = turn + DAT_10db6ed8[index].heading;
    return 0;
}
