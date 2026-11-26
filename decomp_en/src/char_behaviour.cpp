// Running the behaviour of a character (EN build). Ported from the matched JP source decomp/src/npc_dispatch.cpp.
#include "anim_model.h"
#include "char.h"
#include "char_behaviours.h"
#include "char_move.h"
#include "char_proximity.h"
#include "char_dispatch.h"
#include "collide_query.h"
#include "enemy_pendulum.h"
#include "game_vars.h"
#include "marker_rider.h"
#include "npc_facing.h"
#include "npc_greet.h"
#include "npc_hover_carrier.h"
#include "npc_hurl.h"
#include "npc_ride.h"

// How often (in frames) a character that stands idle moves its shadow to the ground, and the AnimModel::m_0012 value it sets.
const int SHADOW_PLACE_PERIOD = 30;
const int HOVER_MARK = 2;
// AnimModel::mode of a character that is not active, and AnimModel::idling of one that idles.
const int MODEL_MODE_INACTIVE = 0;
const int MODEL_IDLING = 1;
// Maps on which some behaviours keep running during an event.
const int EVENT_MAP_A = 0x25c;
const int EVENT_MAP_B = 0x12d;
// Behaviour numbers the event lets run.
const int BEHAVIOUR_EVENT_HOVER_CARRIER = 0x21;
const int BEHAVIOUR_EVENT_PERCH = 10;
const int BEHAVIOUR_EVENT_DROP_IN = 0x24;
// Behaviour numbers of the dispatch table.
const int BEHAVIOUR_UNPACK = 0;
const int BEHAVIOUR_WATCH_PLAYER = 1;
const int BEHAVIOUR_MODE_A = 2;
const int BEHAVIOUR_GROUP_WATCH = 3;
const int BEHAVIOUR_MODE_B = 4;
const int BEHAVIOUR_PENDULUM = 5;
const int BEHAVIOUR_NONE = 6;
const int BEHAVIOUR_RISE_WHEN_SEEN = 7;
const int BEHAVIOUR_PATROL_ROUTE = 8;
const int BEHAVIOUR_WALK_ROUTE = 9;
const int BEHAVIOUR_PERCH_ON_NEIGHBOUR = 10;
const int BEHAVIOUR_RAISE_ALARM = 0xb;
const int BEHAVIOUR_RIDE_ALARM = 0xc;
const int BEHAVIOUR_GATED_CARRIER = 0xd;
const int BEHAVIOUR_FACE_AND_TALK = 0xe;
const int BEHAVIOUR_CHILD_FOLLOW = 0xf;
const int BEHAVIOUR_RIDE_MARKER = 0x10;
const int BEHAVIOUR_SLEEP = 0x11;
const int BEHAVIOUR_IDLE_WATCH = 0x12;
const int BEHAVIOUR_POP_UP = 0x13;
const int BEHAVIOUR_GREET_WHEN_NEAR = 0x14;
const int BEHAVIOUR_MODE_C = 0x15;
const int BEHAVIOUR_WAIT_FACING_AWAY = 0x16;
const int BEHAVIOUR_SHOCK_BY_LINK = 0x17;
const int BEHAVIOUR_HELD = 0x18;
const int BEHAVIOUR_TALK_WHEN_NEAR_ENEMIES = 0x19;
const int BEHAVIOUR_RETIRE_WAVE = 0x1a;
const int BEHAVIOUR_APPEAR_NEARBY = 0x1b;
const int BEHAVIOUR_UPDATE_GROUP = 0x1c;
const int BEHAVIOUR_RANDOM_IDLE = 0x1d;
const int BEHAVIOUR_PUSH_CART = 0x1e;
const int BEHAVIOUR_RIDE_BEHIND_MARKER = 0x1f;
const int BEHAVIOUR_IDLE = 0x20;
const int BEHAVIOUR_HOVER_CARRIER = 0x21;
const int BEHAVIOUR_SWING_DOOR = 0x22;
const int BEHAVIOUR_TURN_WHEN_SEEN = 0x23;
const int BEHAVIOUR_DROP_IN = 0x24;
const int BEHAVIOUR_RUN_TO_MARKER = 0x25;
const int BEHAVIOUR_HURL = 0x26;
const int BEHAVIOUR_DROP_IN_WHEN_NEAR = 0x27;
const int BEHAVIOUR_TALK_FACING_SIMPLE = 0x28;
const int BEHAVIOUR_GREET_WHEN_FACED = 0x29;
const int BEHAVIOUR_LOOKOUT = 0x2a;
const int BEHAVIOUR_APPROACH_PLAYER = 0x2b;
const int BEHAVIOUR_ALERT_WHEN_SEEN = 0x2c;
const int BEHAVIOUR_PERFORM_TRICKS = 0x2d;
const int BEHAVIOUR_GESTURE_WITH_NEIGHBOUR = 0x2e;
const int BEHAVIOUR_TALK_WHEN_NEAR = 0x2f;
const int BEHAVIOUR_DRAG_TO_PIT = 0x31;
// Argument of Npc_ModeBehaviour for the behaviours 2 and 0x15 (0) and 4 (1), and of Npc_TalkWhenNear (wake the enemies).
const int MODE_PLAIN = 0;
const int MODE_VARIANT = 1;

// Char_RunBehaviour: every 30 frames (staggered by index), for a character that is not active (mode 0) or idles, it marks
// m_0012 and puts the shadow on the ground (Char_PlaceOnGround). Then it unpacks the behaviour number on first use and
// runs that behaviour; while an event plays only behaviours 0x21, 10 and 0x24 run, on maps 0x25c and 0x12d with
// g_eventAllowsBehaviours set. Unknown numbers reset the character's state. Returns 0. Called every frame for each
// character that has a behaviour.
extern "C" int Char_RunBehaviour(int index)
{
    if ((g_characterModels[index].mode == MODEL_MODE_INACTIVE || g_characterModels[index].idling == MODEL_IDLING) &&
        g_frameCounter % SHADOW_PLACE_PERIOD == index % SHADOW_PLACE_PERIOD)
    {
        g_characterModels[index].m_0012 = HOVER_MARK;
        Char_PlaceOnGround(index, 0);
    }
    if (g_charState[index].behaviour == BEHAVIOUR_UNPACK)
        Char_UnpackParameter(index);
    if (g_eventPlaying == 0 ||
        ((g_currentMap == EVENT_MAP_A || g_currentMap == EVENT_MAP_B) && g_eventAllowsBehaviours != 0 &&
         (g_charState[index].behaviour == BEHAVIOUR_EVENT_HOVER_CARRIER ||
          g_charState[index].behaviour == BEHAVIOUR_EVENT_PERCH || g_charState[index].behaviour == BEHAVIOUR_EVENT_DROP_IN)))
    {
        switch (g_charState[index].behaviour)
        {
        case BEHAVIOUR_WATCH_PLAYER:
            Npc_WatchForPlayer(index);
            return 0;
        case BEHAVIOUR_MODE_A:
        case BEHAVIOUR_MODE_C:
            Npc_ModeBehaviour(index, MODE_PLAIN);
            return 0;
        case BEHAVIOUR_GROUP_WATCH:
            Npc_GroupWatch(index);
            return 0;
        case BEHAVIOUR_MODE_B:
            Npc_ModeBehaviour(index, MODE_VARIANT);
            return 0;
        case BEHAVIOUR_PENDULUM:
            Enemy_Pendulum(index);
            return 0;
        case BEHAVIOUR_NONE:
            break;
        case BEHAVIOUR_RISE_WHEN_SEEN:
            Npc_RiseWhenSeen(index);
            return 0;
        case BEHAVIOUR_PATROL_ROUTE:
            Npc_PatrolRoute(index);
            return 0;
        case BEHAVIOUR_WALK_ROUTE:
            Npc_WalkRoute(index);
            return 0;
        case BEHAVIOUR_PERCH_ON_NEIGHBOUR:
            Npc_PerchOnNeighbour(index);
            return 0;
        case BEHAVIOUR_RAISE_ALARM:
            Npc_RaiseAlarm(index);
            return 0;
        case BEHAVIOUR_RIDE_ALARM:
            Npc_RideAlarm(index);
            return 0;
        case BEHAVIOUR_GATED_CARRIER:
            Npc_GatedRouteCarrier(index);
            return 0;
        case BEHAVIOUR_FACE_AND_TALK:
            Npc_FaceAndTalk(index);
            return 0;
        case BEHAVIOUR_CHILD_FOLLOW:
            Npc_ChildFollow(index);
            return 0;
        case BEHAVIOUR_RIDE_MARKER:
            Npc_RideMarker(index);
            return 0;
        case BEHAVIOUR_SLEEP:
            Npc_SleepUntilWoken(index);
            return 0;
        case BEHAVIOUR_IDLE_WATCH:
            Char_IdleWatchPlayer(index);
            return 0;
        case BEHAVIOUR_POP_UP:
            Npc_PopUp(index);
            return 0;
        case BEHAVIOUR_GREET_WHEN_NEAR:
            Npc_GreetWhenNear(index);
            return 0;
        case BEHAVIOUR_WAIT_FACING_AWAY:
            Npc_WaitFacingAway(index);
            return 0;
        case BEHAVIOUR_SHOCK_BY_LINK:
            Npc_ShockByLink(index);
            return 0;
        case BEHAVIOUR_HELD:
            Npc_Held(index);
            return 0;
        case BEHAVIOUR_TALK_WHEN_NEAR_ENEMIES:
            Npc_TalkWhenNear(index, MODE_VARIANT);
            return 0;
        case BEHAVIOUR_RETIRE_WAVE:
            Char_RetireWaveEnemy(index);
            return 0;
        case BEHAVIOUR_APPEAR_NEARBY:
            Enemy_AppearNearby(index);
            return 0;
        case BEHAVIOUR_UPDATE_GROUP:
            Char_UpdateGroup(index);
            return 0;
        case BEHAVIOUR_RANDOM_IDLE:
            Char_RandomIdle(index);
            return 0;
        case BEHAVIOUR_PUSH_CART:
            Npc_PushCart(index);
            return 0;
        case BEHAVIOUR_RIDE_BEHIND_MARKER:
            Npc_RideBehindMarker(index);
            return 0;
        case BEHAVIOUR_IDLE:
            Npc_Idle(index);
            return 0;
        case BEHAVIOUR_HOVER_CARRIER:
            Npc_HoverCarrier(index);
            return 0;
        case BEHAVIOUR_SWING_DOOR:
            Char_SwingDoor(index);
            return 0;
        case BEHAVIOUR_TURN_WHEN_SEEN:
            Npc_TurnWhenSeen(index);
            return 0;
        case BEHAVIOUR_DROP_IN:
            Enemy_DropIn(index);
            return 0;
        case BEHAVIOUR_RUN_TO_MARKER:
            Npc_RunToMarker(index);
            return 0;
        case BEHAVIOUR_HURL:
            Npc_HurlAtPlayer(index);
            return 0;
        case BEHAVIOUR_DROP_IN_WHEN_NEAR:
            Enemy_DropInWhenNear(index);
            return 0;
        case BEHAVIOUR_TALK_FACING_SIMPLE:
            Npc_TalkFacingSimple(index);
            return 0;
        case BEHAVIOUR_GREET_WHEN_FACED:
            Npc_GreetWhenFaced(index);
            return 0;
        case BEHAVIOUR_LOOKOUT:
            Npc_Lookout(index);
            return 0;
        case BEHAVIOUR_APPROACH_PLAYER:
            Npc_ApproachPlayer(index);
            return 0;
        case BEHAVIOUR_ALERT_WHEN_SEEN:
            Npc_AlertWhenSeen(index);
            return 0;
        case BEHAVIOUR_PERFORM_TRICKS:
            Npc_PerformTricks(index);
            return 0;
        case BEHAVIOUR_GESTURE_WITH_NEIGHBOUR:
            Npc_GestureWithNeighbour(index);
            return 0;
        case BEHAVIOUR_TALK_WHEN_NEAR:
            Npc_TalkWhenNear(index, MODE_PLAIN);
            return 0;
        case BEHAVIOUR_DRAG_TO_PIT:
            Enemy_DragToPit(index);
            return 0;
        default:
            Char_ResetState(index);
            break;
        }
    }
    return 0;
}
