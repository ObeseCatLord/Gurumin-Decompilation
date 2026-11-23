// The renderer's state switch, FUN_1013f070 (render_device.cpp has not been written yet): one entry point that sets a
// group of Direct3D states selected by `command`. Only the commands decoded so far are named.
// This is the one declaration of these constants: include this header, never redeclare them.
#ifndef GURUMIN_RENDER_STATE_H
#define GURUMIN_RENDER_STATE_H

// FUN_1013f070's `command` values.
enum RenderCommand
{
    RENDER_BLEND_MODE = 0,    // value = blend mode (RenderBlendMode); returns the previous mode
    RENDER_TEXTURE_ADDRESS = 2,  // value = texture addressing of stage 0 (RenderTextureAddress)
    RENDER_CLEAR = 4,         // clears the target and the depth buffer to colour `value` (ARGB)
    RENDER_FLUSH_SCENE = 6,   // ends the current scene and begins a new one
    RENDER_CULL_MODE = 10     // value = RenderCullMode
};

// Blend modes of RENDER_BLEND_MODE (the D3DRS_SRCBLEND / D3DRS_DESTBLEND pairs they set).
enum RenderBlendMode
{
    BLEND_ALPHA = 0,          // source alpha / inverse source alpha (the default)
    BLEND_MULTIPLY = 2,       // zero / source colour: the target is multiplied by the source (shadows)
    BLEND_BLACK = 3           // zero / zero: whatever is drawn writes black (shadow silhouettes)
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
    CULL_NONE = 1             // both faces drawn
};

// The ARGB colour with every channel at 255: opaque white.
#define RENDER_COLOUR_WHITE (-1)

extern "C" int FUN_1013f070(int command, int value, int extra);  // 0x1013f070

#endif
