// Constants of EffectManager::DrawRecord (effect_draw_record.cpp): the effect kinds it draws itself and the layout of
// the sprite tables its sprite kinds leave records in. The kinds 6, 7, 0x11, 0x16 and 0x17 draw lights (effect_manager.h).
#ifndef GURUMIN_EFFECT_DRAW_H
#define GURUMIN_EFFECT_DRAW_H

// EffectDef::kind values.
#define EFFECT_KIND_SPRITE 1             // textured square at the effect's position, whole texture
#define EFFECT_KIND_BURST_SQUARE 2       // two nested squares fading outwards (30 vertices)
#define EFFECT_KIND_BURST_HEXAGON 3      // two nested hexagons fading outwards (48 vertices)
#define EFFECT_KIND_MODEL_UNIFORM 4      // a model scaled evenly
#define EFFECT_KIND_QUAD 5               // textured quad turned about three axes
#define EFFECT_KIND_RIBBON_A 8           // ribbon from where the effect started to where it is (returns 1)
#define EFFECT_KIND_RIBBON_B 9           // the same
#define EFFECT_KIND_MODEL 10             // a model scaled on three axes
#define EFFECT_KIND_QUAD_STANDING 0xb    // quad standing on its bottom edge
#define EFFECT_KIND_DISC_NEAR 0xc        // fading disc of rings (visible from depth 0.5)
#define EFFECT_KIND_DISC 0xd             // fading disc of rings
#define EFFECT_KIND_FLARE 0xe            // flare stretched between the start and the position
#define EFFECT_KIND_DEBUG_BOX 0xf        // only shown as a debug shape
#define EFFECT_KIND_DEBUG_CYLINDER 0x10  // only shown as a debug shape
#define EFFECT_KIND_SPRITE_RECT 0x12     // textured square with its texture rectangle from tracks
#define EFFECT_KIND_QUAD_TEXTURED 0x13   // quad with its texture rectangle from tracks
#define EFFECT_KIND_MODEL_TINTED 0x14    // a model scaled on three axes and tinted
#define EFFECT_KIND_CONE 0x15            // cone of three rings drawn as an indexed mesh

// Where a draw pass keeps its sprites in the sprite tables: pass n uses the records [start, end).
#define EFFECT_SPRITE_PASS_COUNT 5

// One vertex of the cone mesh (kind 0x15), 0x70 bytes, read by the effect vertex shader.
struct EffectMeshVertex
{
    float position[4];                               // x, y on the ring, 0, 1
    float offset[4];                                 // the middle ring's offset track (zero on the outer rings), 0, 1
    float colour[4];                                 // the effect colour (alpha zero on the outer rings)
    float spare[4];
    float colour2[4];                                // the second colour
    float spare2[4];
    float one[4];                                    // 1, 0, 0, 0
};

#endif
