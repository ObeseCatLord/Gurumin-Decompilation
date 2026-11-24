// Constants shared by the effect light draw functions (effect_light.cpp, effect_light_kind22.cpp): the trail is laid
// out on a 640x480 screen and each vertex is hidden when it lands too far outside it or behind the near plane.
#ifndef GURUMIN_EFFECT_TRAIL_H
#define GURUMIN_EFFECT_TRAIL_H

#define TRAIL_SCREEN_WIDTH 640.0f
#define TRAIL_SCREEN_HEIGHT 480.0f
#define TRAIL_HIDE_DISTANCE 1000.0f
// A projected depth in this range is in front of the camera; anything else hides the vertex.
#define TRAIL_NEAR_DEPTH 0.9
#define TRAIL_FAR_DEPTH 1.0
// How far outside the visible area (and in front of it) a point may be and still be drawn.
#define TRAIL_CULL_MARGIN 200.0f
#define TRAIL_CULL_MARGIN_NEAR 20.0f
#define TRAIL_CULL_MARGIN_FAR 1000.0f
// A colour channel of 1.0 is 255.
#define TRAIL_CHANNEL_SCALE 255.0f

#endif
