// A hovering watcher (character behaviour run from 0x100f9670): it keeps facing the player, drifts back to the height
// it came in at (DAT_104c4438) and turns smoothly instead of snapping to its target heading.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "char_slot.h"
#include "char_state.h"

#include "char_tables.h"
#include "world.h"
#include "char_vars.h"

#include "char_ground.h"

#define WATCHER_BUSY 1000              // DAT_10210d30 value of a watcher in a scripted sequence: it only aims
#define WATCHER_CALLED_STATE 60        // state given to a watcher that was called (DAT_10210b30)
#define WATCHER_IDLE_STATES 0x79       // below this state the idle motion is played
#define WATCHER_GROUND_PERIOD 30       // the shadow is put on the ground every 30 frames (staggered by slot)
#define WATCHER_MAP 0x262              // on this map the shadow is shown at full opacity
#define WATCHER_NO_HOME -200000.0f     // DAT_104c4438 x at or below this: no home height
#define WATCHER_HEIGHT_TOLERANCE 10.0f // the watcher climbs or sinks 10 units a frame outside +-10 of its home height
#define WATCHER_TURN_DIVISOR 2.5f      // each frame the heading covers 1/2.5 of the turn left

// atan2 of float arguments, rounded back to float (the arguments are rounded to float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10026730 (Watcher_Update). One frame of watcher `index`: aims its target heading at the player and returns at
// once while DAT_10210d30 is WATCHER_BUSY. Otherwise a living watcher (health > 0) is handed back to the game
// (mode 0), it idles, the shadow is put on the ground every WATCHER_GROUND_PERIOD frames (the ground height kept in
// DAT_10255160, reused in between), its height follows its home height and its target heading is pulled back towards
// its heading so that it turns gradually. The distances to the player are computed and dropped. Returns 0.
extern "C" int FUN_10026730(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    (player - self).Length2D();  // computed and dropped
    (player - self).Length();    // computed and dropped
    DAT_10db6ee8[index].targetHeading = Atan2(player.x - self.x, player.y - self.y) + 3.1415f;
    if (DAT_10210d30[index] == WATCHER_BUSY)
        return 0;
    if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
        DAT_10db6ee8[index].mode = 0;
    DAT_10db6ee8[index].m_0267 = 1;
    if (DAT_10c62ed0 == WATCHER_MAP)
        DAT_10db6ee8[index].shadowOpacity = 15;
    if (DAT_10210b30[index] != 0 && DAT_1020b430[index].state > 0)
        DAT_1020b430[index].state = WATCHER_CALLED_STATE;
    if (DAT_1020b430[index].state < WATCHER_IDLE_STATES)
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
    if (DAT_1020b430[index].groundTimer % WATCHER_GROUND_PERIOD == index % WATCHER_GROUND_PERIOD)
    {
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        DAT_10db6ee8[index].m_0012 = 2;
        FUN_1001c090(index, 1);
        DAT_11b002c0 = 0;
        DAT_10255160[index][0] = DAT_10db6ee8[index].shadowMatrix._43;
    }
    else
        DAT_10db6ee8[index].shadowMatrix._43 = DAT_10255160[index][0];
    DAT_1020b430[index].groundTimer++;
    if (DAT_104c4438[index].x > WATCHER_NO_HOME)
    {
        if (DAT_104c4438[index].z + WATCHER_HEIGHT_TOLERANCE < self.z)
            DAT_10db6ee8[index].matrix._43 -= WATCHER_HEIGHT_TOLERANCE;
        else if (DAT_104c4438[index].z - WATCHER_HEIGHT_TOLERANCE > self.z)
            DAT_10db6ee8[index].matrix._43 += WATCHER_HEIGHT_TOLERANCE;
    }
    float heading = DAT_10db6ee8[index].heading;
    float target = DAT_10db6ee8[index].targetHeading;
    while (heading > 3.1415f)
        heading -= 6.283f;
    while (target > 3.1415f)
        target -= 6.283f;
    while (heading < -3.1415f)
        heading += 6.283f;
    while (target < -3.1415f)
        target += 6.283f;
    float turn = heading - target;
    while (turn > 3.1415f)
        turn -= 6.283f;
    while (turn < -3.1415f)
        turn += 6.283f;
    if (turn != 0.0f)
    {
        turn /= WATCHER_TURN_DIVISOR;
        DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading - turn;
    }
    return 0;
}
