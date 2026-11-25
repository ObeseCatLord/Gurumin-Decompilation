// A hovering watcher (EN build): it keeps facing the player, drifts back to the height it came in at and turns smoothly
// instead of snapping to its target heading. Ported from the matched JP source decomp/src/hover_watcher.cpp.
#include <math.h>
#include "hover_watcher.h"
#include "char.h"
#include "char_shots.h"
#include "game_vars.h"

// Value of g_charPhase of a watcher in a scripted sequence: it only aims.
const int WATCHER_BUSY = 1000;
// State given to a watcher that was told to react.
const int WATCHER_CALLED_STATE = 60;
// Below this state the idle motion is played.
const int WATCHER_IDLE_STATES = 0x79;
// The shadow is put on the ground every this many frames (staggered by slot).
const int WATCHER_GROUND_PERIOD = 30;
// On this map the shadow is shown at full opacity.
const int WATCHER_MAP = 0x262;
const unsigned char WATCHER_SHADOW_OPACITY = 15;
// Index in CharState::values of the frame counter that times the ground probe.
const int WATCHER_GROUND_TIMER_VALUE = 9;
// Idle motion (layer, first, last) the watcher plays.
const int WATCHER_IDLE_LAYER = 2;
const int WATCHER_IDLE_FIRST = 0;
const int WATCHER_IDLE_LAST = 5;
// Collision mode written every frame.
const char WATCHER_COLLISION_MODE = 2;
// g_charHomes x at or below this: no home height.
const float WATCHER_NO_HOME = -200000.0f;
// The watcher climbs or sinks this much a frame outside this margin of its home height.
const float WATCHER_HEIGHT_TOLERANCE = 10.0f;
// Each frame the heading covers 1 / this of the turn left.
const float WATCHER_TURN_DIVISOR = 2.5f;

// Watcher_Update: one frame of watcher `index`: aims its target heading at the player and returns at once while it is
// WATCHER_BUSY. Otherwise a living watcher (health > 0) is handed back to the game (mode 0), it idles, the shadow is put
// on the ground every WATCHER_GROUND_PERIOD frames (the ground height kept in g_shotParams, reused in between), its height
// follows its home height and its target heading is pulled back towards its heading so that it turns gradually. The
// distances to the player are computed and dropped. Returns 0.
extern "C" int Watcher_Update(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    (player - self).Length2D();
    (player - self).Length();
    g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
    if (g_charPhase[index] == WATCHER_BUSY)
        return 0;
    if (g_charRecords[index].stats[STAT_HEALTH] > 0)
        g_characterModels[index].mode = 0;
    g_characterModels[index].idling = 1;
    if (g_currentMap == WATCHER_MAP)
        g_characterModels[index].shadowOpacity = WATCHER_SHADOW_OPACITY;
    if (g_charToldToReact[index] != 0 && g_charState[index].state > 0)
        g_charState[index].state = WATCHER_CALLED_STATE;
    if (g_charState[index].state < WATCHER_IDLE_STATES)
        g_characterModels[index].PlayMotion(WATCHER_IDLE_LAYER, WATCHER_IDLE_FIRST, WATCHER_IDLE_LAST, 1.0f);
    if (g_charState[index].values[WATCHER_GROUND_TIMER_VALUE] % WATCHER_GROUND_PERIOD == index % WATCHER_GROUND_PERIOD)
    {
        g_collisionFilter = g_characterModels[index].collisionFilter;
        g_characterModels[index].m_0012 = WATCHER_COLLISION_MODE;
        Char_PlaceOnGround(index, 1);
        g_collisionFilter = 0;
        g_shotParams[index][0] = g_characterModels[index].shadowMatrix.m[MATRIX_TRANSLATION_Z];
    }
    else
        g_characterModels[index].shadowMatrix.m[MATRIX_TRANSLATION_Z] = g_shotParams[index][0];
    g_charState[index].values[WATCHER_GROUND_TIMER_VALUE]++;
    if (g_charHomes[index].x > WATCHER_NO_HOME)
    {
        if (g_charHomes[index].z + WATCHER_HEIGHT_TOLERANCE < self.z)
            g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] -= WATCHER_HEIGHT_TOLERANCE;
        else if (g_charHomes[index].z - WATCHER_HEIGHT_TOLERANCE > self.z)
            g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] += WATCHER_HEIGHT_TOLERANCE;
    }
    float heading = g_characterModels[index].heading;
    float target = g_characterModels[index].targetHeading;
    while (heading > ANGLE_HALF_TURN)
        heading -= ANGLE_FULL_TURN;
    while (target > ANGLE_HALF_TURN)
        target -= ANGLE_FULL_TURN;
    while (heading < -ANGLE_HALF_TURN)
        heading += ANGLE_FULL_TURN;
    while (target < -ANGLE_HALF_TURN)
        target += ANGLE_FULL_TURN;
    float turn = heading - target;
    while (turn > ANGLE_HALF_TURN)
        turn -= ANGLE_FULL_TURN;
    while (turn < -ANGLE_HALF_TURN)
        turn += ANGLE_FULL_TURN;
    if (turn != 0.0f)
    {
        turn /= WATCHER_TURN_DIVISOR;
        g_characterModels[index].targetHeading = g_characterModels[index].heading - turn;
    }
    return 0;
}
