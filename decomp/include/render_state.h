// The renderer's state switch, FUN_1013f070 (render_command.cpp): one entry point that sets a
// group of Direct3D states selected by `command`. Only the commands decoded so far are named.
// This is the one declaration of these constants: include this header, never redeclare them.
#ifndef GURUMIN_RENDER_STATE_H
#define GURUMIN_RENDER_STATE_H

// FUN_1013f070's `command` values.
enum RenderCommand
{
    RENDER_BLEND_MODE = 0,    // value = blend mode (RenderBlendMode); returns the previous mode
    RENDER_TEXTURE_ADDRESS = 2,  // value = texture addressing of stage 0 (RenderTextureAddress)
    RENDER_ALPHA_TEST = 3,    // value 0/1: alpha test off/on; extra = reference alpha
    RENDER_CLEAR = 4,         // clears the target and the depth buffer to colour `value` (ARGB)
    RENDER_CLEAR_RECT_128 = 5,   // clears the top left 128x128 of the target to colour `value`
    RENDER_FLUSH_SCENE = 6,   // ends the current scene and begins a new one
    RENDER_TEXTURE_FILTER = 7,   // value 0 = point, else linear filtering on samplers 0 and 1
    RENDER_DEPTH_WRITE = 9,   // value 0/1: D3DRS_ZWRITEENABLE off/on
    RENDER_CULL_MODE = 10,    // value = RenderCullMode
    RENDER_CLEAR_DEPTH = 11,  // clears the depth buffer to value / 65536
    RENDER_DEPTH_TEST = 12,   // value 0/1: D3DRS_ZENABLE off/on
    RENDER_CLEAR_RECT_512 = 13,  // clears the top left 512x512 of the target to colour `value`
    RENDER_DEPTH_FUNC = 14,   // value 0 = less or equal, 2 = greater or equal
    RENDER_FOG = 15,          // value 0/1: fog off/on; extra = fog colour
    RENDER_END_SCENE = 16,    // ends the current scene
    RENDER_BEGIN_SCENE = 17,  // begins a new scene
    RENDER_DEPTH_FUNC_ALWAYS = 19,  // value 0 = less or equal, else always pass
    RENDER_CLEAR_STRIP_48 = 20,  // clears a 380x48 strip of the target to colour `value`
    RENDER_CLEAR_RECT_3 = 21,    // clears a 3x3 square of the target to colour `value`
    RENDER_CLEAR_STRIP_128 = 22, // clears a 380x128 strip of the target to colour `value`
    RENDER_CLEAR_STRIP_256 = 23, // clears a 380x256 strip of the target to colour `value`
    RENDER_CLEAR_STRIP_64 = 24,  // clears a 380x64 strip of the target to colour `value`
    RENDER_ALPHA_TEST_OFF = 32   // value 0 = alpha test on, else off
};

// Blend modes of RENDER_BLEND_MODE (the D3DRS_SRCBLEND / D3DRS_DESTBLEND pairs they set).
enum RenderBlendMode
{
    BLEND_ALPHA = 0,          // source alpha / inverse source alpha (the default)
    BLEND_ADD = 1,            // source alpha / one: added to the target
    BLEND_MULTIPLY = 2,       // zero / source colour: the target is multiplied by the source (shadows)
    BLEND_BLACK = 3,          // zero / zero: whatever is drawn writes black (shadow silhouettes)
    BLEND_OPAQUE = 4,         // blending off: the source replaces the target
    BLEND_KEEP_TARGET = 5,    // zero / one: the target is left unchanged
    BLEND_COLOUR_MIX = 6,     // source colour / inverse source colour
    BLEND_MULTIPLY_COPY = 7,  // the same states as BLEND_MULTIPLY
    BLEND_DARKEN_COPY = 8,    // the same states as BLEND_DARKEN
    BLEND_ADD_COLOUR = 9,     // one / one: the source colour added to the target
    BLEND_DARKEN = 10,        // zero / inverse source colour: the target darkened by the source colour
    BLEND_ADD_TARGET = 11,    // target colour / one
    BLEND_INVERT = 12,        // inverse target colour / inverse source colour
    BLEND_TARGET_ALPHA = 13   // target colour / inverse source alpha
};

// Texture addressing of RENDER_TEXTURE_ADDRESS (D3DSAMP_ADDRESSU/V of stage 0).
enum RenderTextureAddress
{
    TEXTURE_ADDRESS_WRAP = 0,    // repeats (the default)
    TEXTURE_ADDRESS_CLAMP = 1,   // edge texels stretched outwards
    TEXTURE_ADDRESS_MIRROR = 2   // repeats mirrored
};

// Cull modes of RENDER_CULL_MODE.
enum RenderCullMode
{
    CULL_DEFAULT = 0,         // counter-clockwise (or clockwise when the scene is mirrored)
    CULL_NONE = 1,            // both faces drawn
    CULL_REVERSED = 2         // clockwise (or counter-clockwise when the scene is mirrored)
};

// The ARGB colour with every channel at 255: opaque white.
#define RENDER_COLOUR_WHITE (-1)

extern "C" int DAT_10202b14;   // the render device's current blend mode (RenderBlendMode)

extern "C" int FUN_1013f070(int command, int value, int extra);  // 0x1013f070

#endif
