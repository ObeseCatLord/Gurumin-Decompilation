// The map cursor: a marker that moves from node to node along a map record's route and follows a rider that lands on
// it (EN build; JP: decomp/include/map_cursor.h). Declared once here.
#ifndef GURUMIN_EN_MAP_CURSOR_H
#define GURUMIN_EN_MAP_CURSOR_H

// Route node kinds (RoutePoint::z of the cursor's route).
const int CURSOR_NODE_JUMP = 1;            // code = the node to jump to
const int CURSOR_NODE_ENTRY = 2;           // code = the node to start from
const int CURSOR_NODE_RIDER_STOP = 4;      // stops the cursor; a rider lands here
const int CURSOR_NODE_BACK = 5;            // the cursor goes back one node
const int CURSOR_NODE_PORTAL = 6;          // code = route * 10000 + node: continues there, looking back
const int CURSOR_NODE_PORTAL_FORWARD = 7;  // the same, looking forward

// Cursor modes.
const int CURSOR_MODE_JUMP = 1;            // follow the node's jump, or go to the requested node
const int CURSOR_MODE_ENTRY = 2;           // follow the node's entry, or go to the requested node
const int CURSOR_TARGET_NONE = -1;         // no requested node: use the node's own jump or entry

// Indexes into CharState::values used by the cursor.
const int CURSOR_VALUE_HOLD = 11;          // 0x30: frames left in which the cursor counts as ridden (15 after a rider landed)
// CharRecord::stats entry the cursor sets to -1 at a rider stop (index 2, not named elsewhere yet).
const int CURSOR_STAT_RELEASE = 2;

// The cursor is on a node when it is within this many units of it on both axes.
const float CURSOR_NODE_RANGE = 10.0f;
// Frames the cursor keeps counting as ridden after a rider landed on it.
const int CURSOR_ARRIVAL_HOLD_FRAMES = 15;
// Rider detection: the rider must stand up to 100 units above, within 200 units (squared 40000).
const int CURSOR_RIDER_HEIGHT = 100;
const int CURSOR_RIDER_DISTANCE_SQUARED = 40000;
// The hold must be longer than this for a rider to speed the cursor up.
const int CURSOR_RIDER_SPEED_HOLD = 10;
// Factor that stopping at a rider node applies to the cursor's speed scale.
const float CURSOR_RIDER_SPEED_FACTOR = 2.5f;
// Digits of a portal code: the node is the low four digits, the route the rest.
const int CURSOR_PORTAL_DIGITS = 10000;

extern "C" int g_mapCursorRidden;   // EN 0x013332c8 (JP DAT_10c63520): 1 while a rider stands on a cursor

// Moves cursor `index` (standing on node `step` of its route record) one step: notes whether a rider landed on it
// (the hold counts the frames since), doubles its speed scale while the hold lasts, adjusts the node to head for when
// the rider's arrival or departure changed (*kind, *previousStep, *wasRidden keep the previous node's kind, the node
// before and the rider flag between calls) and, when the cursor at (*x, *y) reached the node, picks the next one:
// `target` (-1 = the node's own jump or entry) in modes 1 and 2, otherwise by the node's kind (a rider node reverses
// the cursor and changes its speed, a back node goes back, a portal switches to its route). Writes the node to *outStep
// and its position to *x and *y. Returns 0; does nothing without `wasRidden`. EN 0x602730 (JP 0x10051190). Called by the
// map cursor behaviour.
extern "C" int MapCursor_Step(int index, int step, int *outStep, float *x, float *y, int mode, int target, int unused,
                              int *kind, int *previousStep, int *wasRidden);

#endif
