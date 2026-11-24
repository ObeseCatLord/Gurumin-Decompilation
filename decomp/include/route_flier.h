// The route flier (behaviour 0x21 of the per-kind update 0x100f9670, route_flier.cpp): a character that flaps along a
// map route, bumps the player when it hits him and vanishes in a burst when it gets stuck or the route says so.
// CharState words: flierPoint, flierMode, flierRoute, flierFrames, flierGround, flierReady, flierLast[XYZ],
// flierStillFrames.
#ifndef GURUMIN_ROUTE_FLIER_H
#define GURUMIN_ROUTE_FLIER_H

#define FLIER_STATE_VANISHED 3            // DAT_10210d30 value of a slot whose flier was removed
#define FLIER_WING_NODE "@E_010:"
#define FLIER_BURST_EFFECT 0x164          // the effect started where it vanishes
#define FLIER_BURST_FRAMES 5              // frames it takes to vanish
#define FLIER_PROBE_PERIOD 15             // it probes the ground one frame in 15 (staggered by slot)
#define FLIER_PROBE_MASK 0x4000000
#define FLIER_PROBE_DEPTH 1000.0f
#define FLIER_NO_HIT -200000.0f           // hit.x of a cast that found nothing; also the x a removed model is parked at
#define FLIER_NO_GROUND (-5000)
#define FLIER_STUCK_DISTANCE 10.0f        // moving less than this in a frame counts as standing still
#define FLIER_STUCK_FRAMES 30
#define FLIER_GROUP_ROUTE 5               // the route record that is flown the other way
#define FLIER_START_SPEED 10.0f
#define FLIER_TOP_SPEED 30.0f
#define FLIER_SPEED_GAIN 0.05f
#define FLIER_FLAP_STEP 0.2f              // wing angle gained per frame
#define FLIER_DRIFT_STEP 0.01f            // sideways drift per unit of the random roll
#define FLIER_DRIFT_START_RANGE 300
#define FLIER_DRIFT_START_CENTRE 1.5f
#define FLIER_DRIFT_RANGE 150
#define FLIER_DRIFT_EDGE 40.0f            // distance from the route line at which it steers back
#define FLIER_DRIFT_PROBE 0.75f
#define FLIER_DRIFT_PROBE_FAR 0.95f
#define FLIER_PROBE_HEIGHT 5.0f
#define FLIER_TURN_DISTANCE 5.0f
#define FLIER_ROUTE_END_TYPE 2            // route point type that ends the route
#define FLIER_FADE_STEP 0.1f              // alpha lost per frame while vanishing
#define FLIER_HALF_TURN 3.1415f
#define FLIER_DEBRIS_LIFT 50.0f
#define FLIER_DEBRIS_TYPE 0x2e
#define FLIER_HELMET_SAFE 0x2e            // the helmet that protects the player from its bump
#define FLIER_COUNT_LIMIT 999

// Flapping speed, wing angle and sideways drift of a flier in DAT_10255160[slot].
#define FLIER_SPEED 0
#define FLIER_ANGLE 1
#define FLIER_DRIFT 2

extern "C" int DAT_10c655e4;              // fliers that have vanished (stops counting at FLIER_COUNT_LIMIT)
extern "C" int FUN_10055ee0(void);        // Player_KnockBack (player_knockback.cpp)

int FUN_1007c050(int slot);    // Char_RouteFlier

#endif
