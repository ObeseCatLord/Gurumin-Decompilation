// Per-character behaviour state: 0x40 bytes for each of the 0x80 character slots (DAT_1020b430). Every behaviour
// (NPC idling and talking, enemy chases, pendulums, route walkers, scripts, scripted shoves, ...) keeps its own
// variables here, so most words mean different things to different behaviours. The header gives each offset a
// union of the names the behaviours use; the comment on each name says which behaviour (source file) uses it.
// game_reset.cpp clears the whole array when a game starts; char_move.cpp clears one slot when a behaviour starts.
// This is the one declaration of this type: include this header, never redeclare it.
#ifndef GURUMIN_CHAR_STATE_H
#define GURUMIN_CHAR_STATE_H

// Number of character slots (DAT_1020b430, DAT_10db6ee8 and the other per-character arrays).
#define CHAR_SLOT_COUNT 0x80

// One scripted shove (char_shove.cpp): a character slides by a per-frame step for some frames, or turns to look at
// another character. A slot's state holds four of them while shoves run.
struct Shove
{
    int target;                   // character slot + 10000 * (non-zero = keep its heading), negative = look target
    int stepX;                    // per-frame step (for a look target: the character to look at)
    int stepY;
    int frames;                   // frames left + 10000 * (non-zero = keep its motion)
};

struct CharState
{
    union
    {
        struct
        {
            union                                 // 0x00
            {
                int state;                        // behaviour step: 0 = not started yet, then per behaviour
                int turn;                         // damage_zone.cpp: turn counter of the zone
                int counterValue;                 // digit_counter.cpp: the value shown (0..1000)
                int approachStarted;              // npc_approach.cpp: 1 once the approach has begun
                int walkingBack;                  // route_follow.cpp: 1 while walking the route backwards
                int gatePoint;                    // npc_gate_carrier.cpp: route point it comes from (the point it is at)
            };
            union                                 // 0x04..0x37: behaviour variables
            {
                struct
                {
                    union                         // 0x04
                    {
                        int wait;                 // frames in the current step (or until the next voice line)
                        int gatePrevious;         // npc_gate_carrier.cpp: the route point before gatePoint
                        int scriptRunning;        // char_script.cpp: 1 while the character's script runs
                        int cooldown;             // kind4a_turret.cpp: frames before the next random attack
                        int talkMotionStarted;    // npc_greet.cpp: 1 once the talk motion was started
                        int routeStopped;         // route_walk.cpp: non-zero = the character stays where it is
                        int layerCount;           // flame_layers.cpp: flame layers found (their timers are values[1..])
                        int counterPhase;         // digit_counter.cpp: 0 = idle, 1..10 = turning to count + 1
                        int shockFrames;          // npc_link_shock.cpp: 0 = standing, 1..60 = frames shaking (counted up)
                        int ballFrame;            // rolling_ball.cpp: 0 = not set up, 1 = waiting, then the frame count of the roll
                        int zoneRampStep;          // area_zone.cpp: how much the push grows each frame the player is inside
                        // (route_follow.cpp uses `wait` for the frames left at a pause point)
                    };
                    union                         // 0x08
                    {
                        int talk;                 // talk id (script number * 100 + line) or the behaviour's
                                                  // parameter from the map (distance, speed limit, ...)
                        int pausing;              // enemy_chase.cpp: 1 = pausing instead of chasing
                        int facing;               // npc_facing.cpp: idle facing in steps of pi/6 (100+ = never
                                                  // turns to the player while talking)
                        int scriptStep;           // char_script.cpp: current script step
                        int attacking;            // kind4a_turret.cpp: 1 while an attack motion plays
                        int approachCode;         // npc_approach.cpp: stop distance / 10 (% 100) and release flag
                                                  // (/ 100)
                        int targetPoint;          // route_walk.cpp: route point the character heads for
                        int hideCount;            // companion_update.cpp (companion slot): frames the companion
                                                  // stays hidden (2 in the areas 0x13 and 0x30), counted down
                        int keepCompanionUp;      // companion_update.cpp (owner slot): 0 = a hidden companion
                                                  // sinks 50 units
                        int weaponMarkA;          // weapon_attach.cpp: -2 after an unknown weapon (no model)
                        int stuckFrames;          // enemy_slash.cpp: frames counted without moving
                        int wanderState;          // enemy_wander.cpp: 0 = walking, 1 or 2 = looking for a heading
                        int spiritStep;           // char_spirit.cpp: motion step of the character's motion table row
                        int lookoutMode;          // npc_lookout.cpp: what the lookout watches for (1..5)
                        int hitFrames;            // map_cursor_walk.cpp: frames since it was hit (state 1), 0..0xaa
                        int ballSlot;             // rolling_ball.cpp: slot of the character dropped in (states 1..30)
                        int ballTimer;            // rolling_ball.cpp: frames counted in state 3000
                  int swingReady;           // swing_platform.cpp: 1 once the platform was set up
                        int holdFrames;           // mole_game.cpp: frames to wait before the next step (counted down; 5 after a hit)
                        int zoneRampMax;           // area_zone.cpp: the push stops growing at this strength
                    };
                    union                         // 0x0c
                    {
                        int range;                // char_move.cpp: distance at which a waiting character reacts
                        int count;                // enemy_react.cpp: frames counted towards the next action
                        int pauseFrames;          // enemy_chase.cpp
                        int angle;                // enemy_pendulum.cpp: swing angle in tenths of a degree
                        int idleStarted;          // npc_facing.cpp: 1 once the idle motion was started
                        int saved;                // npc_frozen.cpp: the slot's m_0140 while held
                        int marker;               // npc_ride.cpp: the marker character ridden behind
                        int push;                 // npc_wheel.cpp: push strength (speed = push / 4 - 20)
                        int route;                // route_carrier.cpp: the route walked
                        int registered;           // char_script.cpp: 1 once the position is in the camera list
                        int attackTimer;          // kind4a_turret.cpp: frames left of the attack
                        int stopDistance;         // npc_approach.cpp
                        int placed;               // route_walk.cpp: 1 once the character was put on its route
                        int cameraFrames;         // camera_follow.cpp (slot 0): frames since the closing camera
                                                  // started (effect every 65, sinks after 90)
                        int pullVariant;          // camera_item_stand_pull.cpp (slot 0): random bits picked when the pull shot starts
                        int downTimer;            // companion_update.cpp (companion slot): frames counted in
                                                  // the down / revive cycle
                        int companionHealthCap;   // companion_update.cpp (owner slot): a revived companion gets
                                                  // at most 2000 + this
                        int weaponMarkB;          // weapon_attach.cpp: -2 after an unknown weapon (no model)
                        int fleeSide;             // enemy_flee.cpp: 1 = veers left, 2 = veers right (picked at random)
                        int hikariSpeed;          // char_hikari.cpp: speed counter, 0..90 (10 while it waits)
                        int spiritReady;          // char_spirit.cpp: 1 once the spirit was set up
                        int promptFrames;         // npc_lookout.cpp: frames the key prompt stays (counted down)
                        int watchPhase;           // npc_group_watch.cpp: 0 = idle, 1 = turning to a groupmate, 2 = showing an emote
                        int neighbour;            // npc_gesture.cpp: slot of the character it reacts to (own slot - talk / 100)
                        int gazeTimer;            // npc_greet.cpp: frames left in the current gaze step (counted down)
                        int shakeX;               // npc_link_shock.cpp: shake steps of the three axes, -/+ (x)
                        int ballSavedKind;        // rolling_ball.cpp: the dropped-in character's kind, restored after it landed
                  int swingStatus;          // swing_platform.cpp: 7 = the swing is running, -1 = the character was told to leave
                  int hoverLimits;          // npc_hover_carrier.cpp: talk % 100 = bits that tie it to its home position (1/2 x/y pinned to home, 4/8 x/y kept from passing home upwards, 16/32 x/y kept from passing home downwards)
                        int holeIndex;            // mole_game.cpp: the hole ("@p1".. "@p9" = 0..8) the mole heads for or dives into
                        int zoneRamp;              // area_zone.cpp: the push strength now (1/100 of the force)
                    };
                    union                         // 0x10
                    {
                        int action;               // char_move.cpp: marker to run to (Npc_RunToMarker), alarm
                                                  // raised (Npc_RideAlarm), attacks left to chain
                                                  // (Enemy_ShoveAndBlast)
                        int running;              // enemy_chase.cpp: 1 = running (faster) after the player was far
                        int swingingBack;         // enemy_pendulum.cpp: 1 while swinging back
                        int nextPoint;            // route_carrier.cpp: the route point headed for
                        int downMode;             // companion_update.cpp (companion slot): 0 = active,
                        int passenger;            // npc_gate_carrier.cpp: slot of the enemy it carries (0 = none)
                                                  // 1 = reviving, 2 = recovering, 10 and above = going down
                        int spiritSize;           // char_spirit.cpp: 100 (with an "@O_047b" node) or 200; sets the swarm
                        int hikariPoint;          // char_hikari.cpp: the route point it last reached (route step output)
                        int eyeOpen;              // npc_lookout.cpp: 1 = the lamp's eye is lit
                        int groupSize;            // npc_group_watch.cpp: groupmates counted (same group digit, behaviour 3)
                        int pitPhase;             // map_cursor_walk.cpp: 0 = outside the pit area, 1 = entering, 2 = inside,
                                                  // 3 = leaving
                        int greeted;              // npc_greet.cpp: 0 = has not greeted yet
                        int shakeY;               // npc_link_shock.cpp: shake step of the y axis
                  int swingFlip;            // swing_platform.cpp: non-zero = the acceleration flips when the angle passes 0 or +-180
                  int hoverGround;          // npc_hover_carrier.cpp: ground height under the carrier (-5000 = none)
                        int lift;                 // mole_game.cpp: upward kick after a hit (0x30), 3 less each step it moves
                        int zoneType;              // area_zone.cpp: 0..4 push, 5 / 7 / 8 camera setting, 6 = other zone
                        int perchMode;            // npc_perch.cpp: talk % 100 (0 = copies the neighbour's heading, 1 and 2 = turn to the player and spit, 3 = carried)
                    };
                    union                         // 0x14
                    {
                        int alarmFlashes;         // char_move.cpp: alarm flashes so far (Npc_RideAlarm)
                        int swing;                // enemy_pendulum.cpp: angle change per frame
                        int reversed;             // route_carrier.cpp, route_follow.cpp: 1 while walking the route
                                                  // backwards
                        int messageDelay;         // tutorial_page_next.cpp (slot 0): frames before the "last page"
                                                  // message may show again
                        int reviveHealth;         // companion_update.cpp (companion slot): health it revives
                                                  // with (0 = its maximum + 2000)
                        int scriptValue;          // char_script.cpp: value carried between steps (opacity in
                                                  // tenths)
                        int chaseFrames;          // char_spirit.cpp: frames spent chasing (swarm every 4th)
                        int hikariReady;          // char_hikari.cpp: 2 once the light was set up on its route
                        int reactDelay;           // npc_lookout.cpp: frames until the event is raised (0 = idle)
                        int watchTarget;          // npc_group_watch.cpp: slot of the groupmate it turns to
                        int cheerPending;         // npc_gesture.cpp: 1 = a cheer starts this frame
                        int pickupBlocked;        // npc_gate_carrier.cpp: 1 = the player is outside the pick-up height band
                        int pitFrames;            // map_cursor_walk.cpp: frames in the current pit phase
                        int greetPending;         // npc_greet.cpp: 1 = greeted, the event is raised once greetings are off
                        int shakeZ;               // npc_link_shock.cpp: shake step of the z axis
                        int ballRollFrame;        // rolling_ball.cpp: frames the ball has rolled (the ball node's frame source)
                  int swingLength;          // swing_platform.cpp: length of the arm in units (distance from the pivot + 50)
                  int hoverLowered;         // npc_hover_carrier.cpp: non-zero = the carrier floats 10 units lower
                        int panicked;             // mole_game.cpp: 1 after a hit (the mole runs faster), 0 once it came up again
                        int zoneValue;             // area_zone.cpp: camera value (types 5 and 8) or 1 = the push also tests the height
                        int shotCount;            // npc_perch.cpp: spits so far in the current round (a mode 3 rider sets its neighbour's to 1)
                    };
                    union                         // 0x18
                    {
                        int fall;                 // enemy_pendulum.cpp: drop speed (more negative each frame)
                        int speed;                // route_carrier.cpp: units per frame (from the route's first
                                                  // point)
                        int keepAfterScript;      // char_script.cpp: non-zero = stay in the world after the script
                        int shotSound;            // enemy_strafe.cpp: SHOT_SOUND_PENDING = play the shot sound with
                                                  // the next shot
                        int spiritTimer;          // char_spirit.cpp: frames left of the spirit's visit (counts down)
                        int hikariHold;           // char_hikari.cpp: 15 after the route ended
                        int watchedCode;          // npc_lookout.cpp: item stock digits seen (-1 = none yet)
                        int groupDigit;           // npc_group_watch.cpp: talk % 10, the group it belongs to
                        int cheerReady;           // npc_gesture.cpp: 0 = the cheer is queued instead of played at once
                        int pickupHeight;         // npc_gate_carrier.cpp: lowest player height (below the carrier) for a pick-up
                        int shakeLevel;           // npc_link_shock.cpp: three digits (x, y, z) = how strongly each axis shakes
                  int swingMirror;          // swing_platform.cpp: non-zero = the swing turns the other way round
                        int motionSaved;          // mole_game.cpp: 1 while the motion below is put back after a hit (1 while it dives)
                        int zoneActive;            // area_zone.cpp: 1 while the player is inside and the zone acts
                    };
                    union                         // 0x1c
                    {
                        int rider;                // route_carrier.cpp, npc_hover_carrier.cpp: character carried (0 = none)
                        int scriptResult;         // char_script.cpp: result of the last step (5 = run the next
                                                  // step at once, 2 = waiting)
                        int forceStep;            // route_step.cpp: non-zero = take the point at once, whether
                                                  // or not the character is near it (cleared by the step)
                        int spiritHaunting;       // char_spirit.cpp: 1 while the visit counts down (150+ frames left)
                        int hikariStopped;        // char_hikari.cpp: 1 = stopped by an obstacle, waits for the way to clear
                        int bestValue;            // npc_lookout.cpp: the largest value seen
                        int sightScale;           // npc_group_watch.cpp: talk / 10, its sight range in units of 20
                        int cheerMotion;          // npc_gesture.cpp: motion of the cheer (0x5a + random 0..3)
                        int linkIndex;            // npc_link_shock.cpp: the link (DAT_1049f0c8) the character stands at, -1 = none
                        int savedLayer;           // mole_game.cpp: motion layer playing at the end of the last frame (-1 while blending)
                        int zoneFrame;             // area_zone.cpp: frame of the zone's cycle
                    };
                    union                         // 0x20
                    {
                        int pointPause;           // route_carrier.cpp: frames to wait at a route point
                        int hopRequests;          // enemy_strafe.cpp (Enemy_HopAndStrafe): frames counted
                                                  // towards the next hop
                        int bestAmount;           // npc_lookout.cpp: the amount that came with it
                        int routeMode;            // spirit_route.cpp: 1..3 = put the spirit on a fixed route point
                        int gesturing;            // npc_gesture.cpp: 1 while the gesture shows (neighbours react to it)
                        int hikariSound;          // char_hikari.cpp: 1 while its "anime" sound plays
                        int releaseDistance;      // npc_link_shock.cpp: player distance (talk / 10 * 10) below which it is released
                  int swingGround;          // swing_platform.cpp: highest ground under the swing so far (-3500 = none)
                  int playerGround;         // npc_hover_carrier.cpp: height of the ground under the player
                        int savedFrame;           // mole_game.cpp: motion frame at the end of the last frame
                        int zonePeriod;            // area_zone.cpp: frames in the cycle
                    };
                    union                         // 0x24
                    {
                        int m_0024;
                        int zoneActiveFrom;        // area_zone.cpp: first frame of the cycle in which the zone acts
                        int sinkLimit;            // map_cursor_walk.cpp: set to -40 while the character is not sinking
                        int restFrames;           // enemy_strafe.cpp (Enemy_HopAndStrafe): frames standing still
                                                  // in the idle motion
                    };
                    union                         // 0x28
                    {
                        int fadeActions;          // route_carrier.cpp: bit n set = fade out at a point with
                                                  // action n
                        int skipGroundProbe;      // non-zero = skip the ground probe of a shot (0x1001a590)
                        int hopBack;              // enemy_strafe.cpp (Enemy_HopAndStrafe): 1 = the next hop
                                                  // ends the strafe
                        int fadeOut;              // enemy_busy.cpp: 0 = stays, 1.. = frames of fading out as it
                                                  // leaves the queue (QUEUE_FADE_FRAMES = gone)
                        int groundTimer;          // hover_watcher.cpp: frames counted; the shadow is put on the
                                                  // ground when it matches the slot (mod 30)
                        int headingFrom;          // route_step.cpp: heading (degrees) at the route's first point
                        int markerSlot;           // marker_rider.cpp: slot of the marker character ridden
                        int lastHealth;           // map_cursor_walk.cpp: the health of the slot, copied every frame
                        int zoneActiveTo;          // area_zone.cpp: last frame of the cycle in which the zone acts
                  int swingMaxAngle;        // swing_platform.cpp: largest swing angle seen over ground
                    };
                    union                         // 0x2c
                    {
                        int m_002c;
                        int cursorNode;           // map_cursor_walk.cpp: the route node the character heads for
                        int headingTo;            // route_step.cpp: heading (degrees) at the route's last point
                        int vanishFrames;         // marker_rider.cpp: frames left of the vanishing once the marker fell
                        int hopFrame;             // enemy_strafe.cpp (Enemy_HopAndStrafe): frame of the hop
                                                  // under way (0 = not hopping)
                  int swingMinAngle;        // swing_platform.cpp: smallest swing angle seen over ground
                        int zoneEffect;            // area_zone.cpp: effect started at one frame of the cycle (0 = none)
                    };
                    union                         // 0x30
                    {
                        int m_0030;
                        int zoneEffectFrame;       // area_zone.cpp: the frame of the cycle that starts the effect
                        int markerPhase;          // marker_rider.cpp: 0 = new, 1 = riding (flags set), 2 = vanishing
                        int arrivalHold;          // map_cursor.cpp: frames left in which the cursor counts as
                        int hikariScanned;        // char_hikari.cpp: 1 once the character to carry along was looked up
                                                  // having been carried by a rider (15 after one lands on it)
                    };
                    union                         // 0x34
                    {
                        int m_0034;
                        int zoneSound;             // area_zone.cpp: sound played with the effect (0 = none)
                        int cursorKind;           // map_cursor_walk.cpp: kind of the node last reached (MapCursor_Step)
                        int arrivalAction;        // npc_patrol.cpp: action of the last route point reached
                        int hopHistory;           // enemy_strafe.cpp (Enemy_HopAndStrafe): HopHistory state of
                        int hikariFollower;       // char_hikari.cpp: slot of the character carried along with the light (0 = none)
                        int gateStarted;          // npc_gate_carrier.cpp: 2 once the carrier was set up on its route
                                                  // the position history in LastPosition
                    };
                };
                int values[13];                   // the same variables by index (enemy_ai.cpp, enemy_moves.cpp,
                                                  // ai_pick.cpp and friends use values[0..5])
            };
            union                                 // 0x38
            {
                int sawPlayer;                    // 1 once the character saw the player (or was told to react)
                int zoneSoundDelay;               // area_zone.cpp: frames before that sound plays (0 = at once)
                int cursorPrevious;               // map_cursor_walk.cpp: the node reached before (MapCursor_Step)
                int releasable;                   // npc_approach.cpp: non-zero = the release flag ends the approach
            };
            union                                 // 0x3c
            {
                int behaviour;                    // behaviour number (0 = not unpacked from the script parameter yet);
                                                  // setting it switches behaviour
                int zoneTracking;                 // area_zone.cpp (type 8): 1 once the follow height was taken
            };
        };
        int words[0x10];                          // the whole block by index (resets and copies)
        Shove shoves[4];                          // char_shove.cpp: the slot's scripted shoves
    };
};

// The behaviour state of every character slot.
extern "C" CharState DAT_1020b430[CHAR_SLOT_COUNT];

#endif
