// Commands and modes of Render_Command, the renderer's state switch (EN build; JP: decomp/include/render_state.h).
// RENDER_BLEND_MODE, RENDER_TEXTURE_ADDRESS, RENDER_CLEAR, RENDER_FLUSH_SCENE, RENDER_DEPTH_WRITE, BLEND_ALPHA and
// BLEND_ADD are declared in texture_mgr.h; this header adds the rest. This is the one declaration of them in decomp_en.
#ifndef GURUMIN_EN_RENDER_COMMAND_H
#define GURUMIN_EN_RENDER_COMMAND_H

#include "texture_mgr.h"

// More `command` values of Render_Command.
const int RENDER_ALPHA_TEST = 3;            // value 0/1: alpha test off/on; extra = reference alpha
const int RENDER_CLEAR_RECT_128 = 5;        // clears the top left 128x128 of the target to colour `value`
const int RENDER_TEXTURE_FILTER = 7;        // value 0 = point, else linear (or anisotropic) filtering on samplers 0 and 1
const int RENDER_CULL_MODE = 10;            // value = RenderCullMode
const int RENDER_CLEAR_DEPTH = 11;          // clears the depth buffer to value / 65536
const int RENDER_DEPTH_TEST = 12;           // value 0/1: depth test off/on
const int RENDER_CLEAR_RECT_512 = 13;       // clears the top left 512x512 of the target to colour `value`
const int RENDER_DEPTH_FUNC = 14;           // value 0 = less or equal, 2 = greater or equal
const int RENDER_FOG = 15;                  // value 0/1: fog off/on; extra = fog colour
const int RENDER_END_SCENE = 16;            // ends the current scene
const int RENDER_BEGIN_SCENE = 17;          // begins a new scene
const int RENDER_DEPTH_FUNC_ALWAYS = 19;    // value 0 = less or equal, else always pass
const int RENDER_CLEAR_STRIP_48 = 20;       // clears a 380x48 strip of the target to colour `value`
const int RENDER_CLEAR_RECT_3 = 21;         // clears a 3x3 square of the target to colour `value`
const int RENDER_CLEAR_STRIP_128 = 22;      // clears a 380x128 strip of the target to colour `value`
const int RENDER_CLEAR_STRIP_256 = 23;      // clears a 380x256 strip of the target to colour `value`
const int RENDER_CLEAR_STRIP_64 = 24;       // clears a 380x64 strip of the target to colour `value`
const int RENDER_ALPHA_TEST_OFF = 32;       // value 0 = alpha test on, else off

// More blend modes of RENDER_BLEND_MODE (the source / destination blend pairs they set).
const int BLEND_MULTIPLY = 2;               // zero / source colour: the target is multiplied by the source (shadows)
const int BLEND_BLACK = 3;                  // zero / zero: whatever is drawn writes black (shadow silhouettes)
const int BLEND_OPAQUE = 4;                 // blending off: the source replaces the target
const int BLEND_KEEP_TARGET = 5;            // zero / one: the target is left unchanged
const int BLEND_COLOUR_MIX = 6;             // source colour / inverse source colour
const int BLEND_MULTIPLY_COPY = 7;          // the same states as BLEND_MULTIPLY
const int BLEND_DARKEN_COPY = 8;            // the same states as BLEND_DARKEN
const int BLEND_ADD_COLOUR = 9;             // one / one: the source colour added to the target
const int BLEND_DARKEN = 10;                // zero / inverse source colour: the target darkened by the source colour
const int BLEND_ADD_TARGET = 11;            // target colour / one
const int BLEND_INVERT = 12;                // inverse target colour / inverse source colour
const int BLEND_TARGET_ALPHA = 13;          // target colour / inverse source alpha

// Texture addressing of RENDER_TEXTURE_ADDRESS (stage 0).
const int TEXTURE_ADDRESS_WRAP = 0;         // repeats (the default)
const int TEXTURE_ADDRESS_CLAMP = 1;        // edge texels stretched outwards
const int TEXTURE_ADDRESS_MIRROR = 2;       // repeats mirrored

// Cull modes of RENDER_CULL_MODE.
const int CULL_DEFAULT = 0;                 // counter-clockwise (or clockwise while the map pass runs)
const int CULL_NONE = 1;                    // both faces drawn
const int CULL_REVERSED = 2;                // clockwise (or counter-clockwise while the map pass runs)

// RENDER_DEPTH_FUNC values: less or equal (0) and greater or equal (2).
const int DEPTH_FUNC_LESS_EQUAL = 0;
const int DEPTH_FUNC_GREATER_EQUAL = 2;

// Size of the rectangles the clear commands cover (the strips are 380 wide).
const int CLEAR_STRIP_WIDTH = 380;
const int CLEAR_STRIP_HEIGHT_48 = 48;
const int CLEAR_STRIP_HEIGHT_64 = 64;
const int CLEAR_STRIP_HEIGHT_128 = 128;
const int CLEAR_STRIP_HEIGHT_256 = 256;
const int CLEAR_SQUARE_SMALL = 3;
const int CLEAR_SQUARE_128 = 128;
const int CLEAR_SQUARE_512 = 512;
// RENDER_CLEAR_DEPTH takes the depth as a fixed-point number with this many units for 1.0.
const float DEPTH_FIXED_ONE = 65536.0f;
// Texture filter setting below this value means plain linear filtering; from it on, anisotropic filtering with
// (setting - ANISOTROPY_SETTING_BASE) levels.
const int ANISOTROPY_SETTING_FIRST = 2;
const int ANISOTROPY_SETTING_BASE = 2;

// The render device's current blend mode (a BLEND_* value). EN 0x00908b28 (JP DAT_10202b14).
extern "C" int g_blendMode;
// The texture filter setting (see ANISOTROPY_SETTING_FIRST). EN 0x021e4820.
extern "C" int g_textureFilterSetting;

#endif
