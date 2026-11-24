// The frame's 3D scene (Scene_DrawFrame): the radar markers and the music bars, the character outline, the overhead
// pass, the map, its shadows, the characters, the effects, the crossfade pass and the key prompts. C++ rather than
// extern "C": the function keeps an exception frame for its Matrix4 locals.

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "model_bank.h"
#include "effect_manager.h"
#include "texture_object.h"
#include "visibility.h"
#include "world_renderer.h"
#include "debug_shape.h"
#include "bgm_spectrum.h"

#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_load_data.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_state.h"
#include "render_vars.h"
#include "ui_vars.h"
#include "menu_scene.h"
#include "scene_draw.h"
#include "scene_draw_vars.h"

// Fog colour components are floats from 0 to 1.
#define FOG_COLOUR_SCALE 255.0f
// Name of the root node that carries a fog colour.
#define FOG_ROOT_NAME "fogset"
// DAT_104bfe6c: 3 = only the screen is cleared; 1 = the world pictures are drawn.
#define SCENE_MODE_CLEAR_ONLY 3
#define SCENE_MODE_PICTURES 1
// The music bars are on when this bit of the game state DAT_104c003c is set; the overhead pass when this one is.
#define STATE_MUSIC_BARS 1
#define STATE_OVERHEAD_PASS 2
#define CLEAR_DEPTH_FULL 0xffff
#define PASS_MASK_ALL (-1)
#define ALPHA_TEST_REFERENCE 0xf
// The radar: every character slot below this one may have a marker.
#define MARKER_SLOT_COUNT 0x7c
#define MARKER_KIND_HELPER 0
#define MARKER_KIND_FRIEND 2
#define MARKER_KIND_ENEMY 3
#define MARKER_KIND_PICKUP_B 0x0f
#define MARKER_KIND_PICKUP_A 0x50
#define MARKER_KIND_FRIEND_ALT 0x63
#define MARKER_SCREEN_CENTRE_X 320.0f
#define MARKER_SCREEN_CENTRE_Y 240.0f
#define MARKER_PLAYER_HALF_SIZE 20
#define MARKER_HALF_SIZE 8
#define MARKER_HELPER_HALF_SIZE 9
#define MARKER_PLAYER_CELL_MIN 0.25f
#define MARKER_PLAYER_CELL_MAX 0.3125f
#define MARKER_PLAYER_TURN 3.1415f
#define MARKER_COLOUR_ENEMY 0xff0000
#define MARKER_COLOUR_FRIEND 0xff00
#define MARKER_COLOUR_HELPER 0xffffff
#define MARKER_COLOUR_PICKUP_A 0xffff00
#define MARKER_COLOUR_PICKUP_B 0xffff
#define MARKER_FADE_PEOPLE 5.0e-6f
#define MARKER_FADE_PICKUP 2.0e-6f
#define COSTUME_SEES_ALL_PICKUPS 0xb
#define HELMET_FACE_PICKUP_A 8
#define HELMET_FACE_PICKUP_B 4
// The minimap: seen from above the map's camera target, with a 45000 units far plane.
#define MINIMAP_EYE_RAISE 5000.0f
#define MINIMAP_EYE_BACK 50.0f
#define MINIMAP_NEAR 20.0f
#define MINIMAP_FAR 45000.0f
#define MINIMAP_FIELD_OF_VIEW 0.785375f
#define MINIMAP_SQUASH 1.45f
#define MINIMAP_NODE_FLAGS 0x40
#define MINIMAP_NODE_DRAW_MODE 0x2b
// The character outline: the model is drawn thin and flat into one of two textures, the textures blend into each
// other and a jittered copy is added on top.
#define OUTLINE_MODEL_RENDER_MODE 0x3b
#define OUTLINE_SCALE_XY 0.1f
#define OUTLINE_SCALE_Z 0.9f
#define OUTLINE_BLEND_ALPHA 0.8f
#define OUTLINE_GHOST_ALPHA 0.2f
#define OUTLINE_SHIFT 12
#define OUTLINE_JITTER_DIVISOR 1000.0f
#define OUTLINE_JITTER_SPAN 2.0f
#define OUTLINE_TIME_DIVISOR 100.0f
// The crossfade pass clears its target to white before drawing the map into it.
#define CROSSFADE_CLEAR_COLOUR 0xffffff
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
// The music bars along the screen edges: 10 pixels wide, 1.5 pixels per level.
#define BAR_WIDTH 10
#define BAR_RIGHT_START 0x276
#define BAR_RIGHT_STOP 0x136
#define BAR_PIXELS_PER_LEVEL 1.5f
#define BAR_COLOUR 0x8888ff
// The map's visibility cells have a box each.
#define VISIBILITY_CELL_COUNT 0x40
// Characters below this one are lit as themselves, the others as the player.
#define CHARACTER_OWN_LIGHT_COUNT 0x5c
#define CHARACTER_AREA_COLOUR_FIRST 0x3a
#define SHADOW_OPACITY_PLACED 0xf

// The opaque ARGB colour of a fog colour vector.
static __forceinline unsigned int FogColourArgb(const Vec4 &fog)
{
    return ((((unsigned char)(fog.x * FOG_COLOUR_SCALE) | 0xffffff00) << 8 | (unsigned char)(fog.y * FOG_COLOUR_SCALE)) << 8) |
           (unsigned char)(fog.z * FOG_COLOUR_SCALE);
}

// Clears the target and the depth buffer to the fog colour of a "fogset" map root, black otherwise.
static __forceinline void ClearToFog()
{
    if (stricmp(DAT_10255f08->name, FOG_ROOT_NAME) == 0)
        FUN_1013f070(RENDER_CLEAR, FogColourArgb(DAT_10255f08->fogColour), 0);
    else
        FUN_1013f070(RENDER_CLEAR, 0, 0);
}

// 0x10038c10 (Scene_DrawFrame). Draws the whole in-game 3D frame; `unused` is pushed by the callers and never
// read. Does nothing without a map root (DAT_10255f08) or while DAT_104bfd50 is 0, and only clears the screen
// while DAT_104bfe6c is 3. Otherwise: the minimap and the radar markers of the characters (the player's turning
// arrow, red for enemies, green for living friends, white for helpers, yellow and cyan for pickups while the helmet
// or costume allows it); the swap target preview of the scene; the outline of character DAT_104c02c5 (drawn thin
// into two alternating textures that blend with a jittered copy); the music bars along the screen edges
// (DAT_104c003c bit 0) and the overhead pass (bit 1); then the main pass: the fog clear, the map and its shadows,
// the debris, the world pictures, the effects, the characters nearest the camera last and the crossfade pass; and
// finally the lens flares, the swap target preview, the draw list pass and the key prompts. The effect manager's
// Draw passes are 1 (before the map's draw list), 0 (after it), 2 (after the characters) and 3 (the last).
// DAT_10836100[0][1] (DAT_10836104) is the texture the outline model is drawn into.
void Scene_DrawFrame(int unused)
{
    Vec4 unusedColour;   // constructed and never read in the original
    if (DAT_10255f08 == 0)
        return;
    if (DAT_104bfd50 == 0)
        return;
    if (DAT_104c0220 == 0)
    {
        if (DAT_10f65484 != 0)
            FUN_101439e0();
        else
            FUN_1014b480();
    }
    if (DAT_104bfe6c == SCENE_MODE_CLEAR_ONLY)
    {
        FUN_1013f070(RENDER_CLEAR, 0, 0);
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        return;
    }
    if (DAT_104c0220 == 0 && DAT_10f654cc != 0 && DAT_104c008c != 0)
    {
        Matrix4 world;
        world.SetIdentity();
        Matrix4 viewProjection;
        Vec4 eye;
        Vec4 target;
        Vec4 up;
        target.Copy(DAT_10f668e8);
        eye.Copy(target);
        eye.z = eye.z + MINIMAP_EYE_RAISE;
        eye.y = eye.y - MINIMAP_EYE_BACK;
        up.Set(0, 1.0f, 0, 0);
        viewProjection = LookAt(eye, target, &up, 0) *
                         Multiply(Scaling(1.0f, MINIMAP_SQUASH, 1.0f),
                                  Perspective(MINIMAP_NEAR, MINIMAP_FAR, MINIMAP_FIELD_OF_VIEW));
        DAT_118d0ba8[DAT_108360fc].UpdateImage(0);
        Vec4 marker;
        FUN_1013f070(RENDER_CLEAR, 0, 0);
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        for (int slot = 0; slot < MARKER_SLOT_COUNT; slot++)
        {
            if (slot == 0 || DAT_1021f3c0[slot] == MARKER_KIND_ENEMY || DAT_1021f3c0[slot] == MARKER_KIND_FRIEND ||
                DAT_1021f3c0[slot] == MARKER_KIND_FRIEND_ALT)
            {
                marker.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                float height = marker.z;
                marker.Copy(Project(marker, viewProjection));
                marker.z = 0;
                marker.x = (marker.x + 1.0f) * MARKER_SCREEN_CENTRE_X;
                marker.y = (1.0f - marker.y) * MARKER_SCREEN_CENTRE_Y;
                if (slot == 0)
                {
                    DAT_118d0ba8[DAT_104b7208].DrawTinted(
                        (int)(marker.x - MARKER_PLAYER_HALF_SIZE), (int)(marker.y - MARKER_PLAYER_HALF_SIZE),
                        (int)(marker.x + MARKER_PLAYER_HALF_SIZE), (int)(marker.y + MARKER_PLAYER_HALF_SIZE),
                        MARKER_PLAYER_CELL_MIN, MARKER_PLAYER_CELL_MIN, MARKER_PLAYER_CELL_MAX,
                        MARKER_PLAYER_CELL_MAX, 0, 1.0f, DAT_10db6ee8[0].heading + MARKER_PLAYER_TURN, 0);
                }
                else
                {
                    float fade;
                    if (DAT_1021f3c0[slot] == MARKER_KIND_ENEMY)
                    {
                        fade = DAT_10f66910 - height;
                        fade = 1.0f - fade * fade * MARKER_FADE_PEOPLE;
                        FUN_10143e00((int)(marker.x - MARKER_HALF_SIZE), (int)(marker.y - MARKER_HALF_SIZE),
                                     (int)(marker.x + MARKER_HALF_SIZE), (int)(marker.y + MARKER_HALF_SIZE),
                                     MARKER_COLOUR_ENEMY, 0, 0.0f < fade ? fade : 0.0f);
                    }
                    else if ((DAT_1021f3c0[slot] == MARKER_KIND_FRIEND || DAT_1021f3c0[slot] == MARKER_KIND_FRIEND_ALT) &&
                             DAT_10db6ee8[slot].alpha > 0.0f)
                    {
                        fade = DAT_10f66910 - height;
                        fade = 1.0f - fade * fade * MARKER_FADE_PEOPLE;
                        FUN_10143e00((int)(marker.x - MARKER_HALF_SIZE), (int)(marker.y - MARKER_HALF_SIZE),
                                     (int)(marker.x + MARKER_HALF_SIZE), (int)(marker.y + MARKER_HALF_SIZE),
                                     MARKER_COLOUR_FRIEND, 0, 0.0f < fade ? fade : 0.0f);
                    }
                    else if (DAT_1021f3c0[slot] == MARKER_KIND_HELPER)
                    {
                        FUN_10143e00((int)(marker.x - MARKER_HELPER_HALF_SIZE), (int)(marker.y - MARKER_HELPER_HALF_SIZE),
                                     (int)(marker.x + MARKER_HELPER_HALF_SIZE), (int)(marker.y + MARKER_HELPER_HALF_SIZE),
                                     MARKER_COLOUR_HELPER, 0, 1.0f);
                    }
                }
            }
            if (DAT_1021f3c0[slot] == MARKER_KIND_PICKUP_A || DAT_1021f3c0[slot] == MARKER_KIND_PICKUP_B)
            {
                marker.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                float height = marker.z;
                marker.Copy(Project(marker, viewProjection));
                marker.z = 0;
                marker.x = (marker.x + 1.0f) * MARKER_SCREEN_CENTRE_X;
                marker.y = (1.0f - marker.y) * MARKER_SCREEN_CENTRE_Y;
                if (((DAT_104c02c8 & HELMET_FACE_PICKUP_A) != 0 || DAT_10c655f0 == COSTUME_SEES_ALL_PICKUPS) &&
                    DAT_1021f3c0[slot] == MARKER_KIND_PICKUP_A)
                {
                    float fade = DAT_10f66910 - height;
                    fade = 1.0f - fade * fade * MARKER_FADE_PICKUP;
                    FUN_10143e00((int)(marker.x - MARKER_HALF_SIZE), (int)(marker.y - MARKER_HALF_SIZE),
                                 (int)(marker.x + MARKER_HALF_SIZE), (int)(marker.y + MARKER_HALF_SIZE),
                                 MARKER_COLOUR_PICKUP_A, 0, 0.0f < fade ? fade : 0.0f);
                }
                if (((DAT_104c02c8 & HELMET_FACE_PICKUP_B) != 0 || DAT_10c655f0 == COSTUME_SEES_ALL_PICKUPS) &&
                    DAT_1021f3c0[slot] == MARKER_KIND_PICKUP_B)
                {
                    float fade = DAT_10f66910 - height;
                    fade = 1.0f - fade * fade * MARKER_FADE_PICKUP;
                    FUN_10143e00((int)(marker.x - MARKER_HALF_SIZE), (int)(marker.y - MARKER_HALF_SIZE),
                                 (int)(marker.x + MARKER_HALF_SIZE), (int)(marker.y + MARKER_HALF_SIZE),
                                 MARKER_COLOUR_PICKUP_B, 0, 0.0f < fade ? fade : 0.0f);
                }
            }
        }
        unsigned int savedFlags = DAT_104c008c->flags;
        int savedDrawMode = DAT_104c008c->drawMode;
        DAT_104c008c->flags = MINIMAP_NODE_FLAGS;
        DAT_104c008c->drawMode = MINIMAP_NODE_DRAW_MODE;
        DAT_104c008c->QueueDraw(&world, &viewProjection, 0, PASS_MASK_ALL, 0, 0);
        DAT_104c008c->flags = savedFlags;
        DAT_104c008c->drawMode = savedDrawMode;
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        if (DAT_10f65484 != 0)
            FUN_101439e0();
        else
            FUN_1014b480();
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
    }
    unsigned int outlineSlot = DAT_104c02c5;
    if (DAT_104c0220 == 0)
    {
        if (DAT_102027f0 != 0 && DAT_10f654cc != 0 && DAT_10f65484 != 0)
        {
            DAT_10f655a0 = DAT_108360f4;
            DAT_118d0ba8[DAT_108360f4].UpdateImage(0);
            DAT_10f6548c = 1;
            ClearToFog();
            FUN_10199860();
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            FUN_1013f070(RENDER_DEPTH_FUNC, 0, 0);
            FUN_1013f070(RENDER_CULL_MODE, 0, 0);
            if (DAT_10255f0c != 0)
                DAT_10255f0c->QueueDraw(&DAT_10836ec8, &DAT_10836cd8,
                                        DAT_10804bf8.LightDirection(DAT_10db6ee8, &DAT_118e69e8, &DAT_118e6cb8),
                                        PASS_MASK_ALL, &DAT_10da90d0, 0);
            FUN_1013f070(RENDER_CLEAR_DEPTH, CLEAR_DEPTH_FULL, 0);
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            FUN_1013f070(RENDER_DEPTH_FUNC, 0, 0);
            DAT_10255f08->QueueDraw(&DAT_10836ec8, &DAT_108392d8,
                                    DAT_10804bf8.LightDirection(DAT_10db6ee8, &DAT_118e69e8, &DAT_118e6cb8),
                                    PASS_MASK_ALL, &DAT_10da90d0, 0);
            FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
            if (DAT_10f65484 != 0)
                FUN_101439e0();
            else
                FUN_1014b480();
            FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
            FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
            if (DAT_104c0220 != 0)
                goto passesDone;
        }
        {
            if (DAT_104c02c5 != 0 && DAT_10f654cc != 0 && DAT_10db6ee8[outlineSlot].root != 0 &&
                DAT_104bf890 != DAT_103088b0)
            {
                DAT_103088b0 = (unsigned char)DAT_104bf890;
                FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
                FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
                FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
                FUN_1013f070(RENDER_DEPTH_TEST, 0, 0);
                FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
                DAT_104c02c4 = DAT_104c02c4 + 1;
                DAT_118d0ba8[DAT_10836100[DAT_104c02c4 % 2][0]].UpdateImage(0);
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                FUN_1013f070(RENDER_CLEAR, 0, 0);
                DAT_118d0ba8[DAT_10836100[(DAT_104c02c4 + 1) % 2][0]].DrawTinted(
                    -1, -2, SCREEN_WIDTH + 1, SCREEN_HEIGHT - 2, 0, 0, 1.0f, 1.0f, 0, OUTLINE_BLEND_ALPHA, 0, 0);
                FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
                DAT_118d0ba8[DAT_10836100[0][1]].UpdateImage(0);
                FUN_1013f070(RENDER_CLEAR, 0, 0);
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_BLACK, 0);
                DAT_10db6ee8[outlineSlot].root->M_10191dd0();
                DAT_10db6ee8[outlineSlot].root->M_1019b050(OUTLINE_MODEL_RENDER_MODE);
                Vec4 savedScale = DAT_10db6ee8[outlineSlot].scale;
                DAT_10db6ee8[outlineSlot].scale.x = OUTLINE_SCALE_XY;
                DAT_10db6ee8[outlineSlot].scale.y = OUTLINE_SCALE_XY;
                DAT_10db6ee8[outlineSlot].scale.z = OUTLINE_SCALE_Z;
                DAT_10db6ee8[outlineSlot].Draw(&DAT_108392d8,
                                               DAT_10804bf8.LightDirection(0, &DAT_118e69e8, &DAT_118e6cb8), 0);
                DAT_10db6ee8[outlineSlot].root->M_10191e10();
                DAT_10db6ee8[outlineSlot].scale.Copy(savedScale);
                sin((float)DAT_104bf890 / OUTLINE_TIME_DIVISOR);   // the original discards the result
                if (DAT_102027fc > 0)
                {
                    float jitterX = (float)rand() / OUTLINE_JITTER_DIVISOR;
                    float jitterY = (float)rand() / OUTLINE_JITTER_DIVISOR;
                    FUN_1013f070(RENDER_BLEND_MODE, BLEND_MULTIPLY, 0);
                    DAT_118d0ba8[DAT_102027fc].DrawTinted(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, jitterX, jitterY,
                                                          jitterX + OUTLINE_JITTER_SPAN, jitterY + OUTLINE_JITTER_SPAN,
                                                          0, 1.0f, 0, 0);
                }
                FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
                DAT_118d0ba8[DAT_10836100[DAT_104c02c4 % 2][0]].UpdateImage(0);
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
                if (DAT_1020d4c0[outlineSlot] == 0)
                {
                    DAT_118d0ba8[DAT_10836100[0][1]].DrawTinted(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1.0f, 1.0f,
                                                                0, OUTLINE_GHOST_ALPHA, 0, 0);
                    DAT_118d0ba8[DAT_10836100[0][1]].DrawTinted(OUTLINE_SHIFT, 0, SCREEN_WIDTH + OUTLINE_SHIFT,
                                                                SCREEN_HEIGHT, 0, 0, 1.0f, 1.0f, 0, 1.0f, 0, 0);
                    DAT_118d0ba8[DAT_10836100[0][1]].DrawTinted(-OUTLINE_SHIFT, 0, SCREEN_WIDTH - OUTLINE_SHIFT,
                                                                SCREEN_HEIGHT, 0, 0, 1.0f, 1.0f, 0, 1.0f, 0, 0);
                    DAT_118d0ba8[DAT_10836100[0][1]].DrawTinted(0, OUTLINE_SHIFT, SCREEN_WIDTH,
                                                                SCREEN_HEIGHT + OUTLINE_SHIFT, 0, 0, 1.0f, 1.0f, 0,
                                                                1.0f, 0, 0);
                    DAT_118d0ba8[DAT_10836100[0][1]].DrawTinted(0, -OUTLINE_SHIFT, SCREEN_WIDTH,
                                                                SCREEN_HEIGHT - OUTLINE_SHIFT, 0, 0, 1.0f, 1.0f, 0,
                                                                1.0f, 0, 0);
                }
                else if (DAT_102027fc > 0)
                {
                    float jitterX = (float)rand() / OUTLINE_JITTER_DIVISOR;
                    float jitterY = (float)rand() / OUTLINE_JITTER_DIVISOR;
                    FUN_1013f070(RENDER_BLEND_MODE, BLEND_MULTIPLY, 0);
                    DAT_118d0ba8[DAT_102027fc].DrawTinted(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, jitterX, jitterY,
                                                          jitterX + OUTLINE_JITTER_SPAN, jitterY + OUTLINE_JITTER_SPAN,
                                                          0, 1.0f, 0, 0);
                }
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                if (DAT_10f65484 != 0)
                    FUN_101439e0();
                else
                    FUN_1014b480();
                FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
                FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
                if (DAT_104c0220 != 0)
                    goto passesDone;
            }
            if (DAT_10f654cc != 0 && DAT_104c003c != 0)
            {
                if ((DAT_104c003c & STATE_MUSIC_BARS) != 0)
                {
                    DAT_118d0ba8[DAT_108360f8].UpdateImage(0);
                    int left = 0;
                    const float *level = DAT_104bff20;
                    for (int right = BAR_RIGHT_START; right > BAR_RIGHT_STOP; right -= BAR_WIDTH)
                    {
                        int next = left + BAR_WIDTH;
                        DAT_118d0ba8[DAT_104b7208].DrawGradient(
                            left, (int)(SCREEN_HEIGHT - level[BGM_SPECTRUM_BANDS] * BAR_PIXELS_PER_LEVEL), next,
                            SCREEN_HEIGHT, BAR_COLOUR, 0, 0, 1.0f);
                        DAT_118d0ba8[DAT_104b7208].DrawGradient(
                            right, (int)(SCREEN_HEIGHT - level[0] * BAR_PIXELS_PER_LEVEL), right + BAR_WIDTH,
                            SCREEN_HEIGHT, BAR_COLOUR, 0, 0, 1.0f);
                        DAT_118d0ba8[DAT_104b7208].DrawGradient(
                            left, 0, next, (int)(SCREEN_HEIGHT - level[BGM_SPECTRUM_BANDS] * BAR_PIXELS_PER_LEVEL), 0,
                            BAR_COLOUR, 0, 1.0f);
                        DAT_118d0ba8[DAT_104b7208].DrawGradient(
                            right, 0, right + BAR_WIDTH, (int)(SCREEN_HEIGHT - level[0] * BAR_PIXELS_PER_LEVEL), 0,
                            BAR_COLOUR, 0, 1.0f);
                        level++;
                        left = next;
                    }
                    if (DAT_10f65484 != 0)
                        FUN_101439e0();
                    else
                        FUN_1014b480();
                }
                if ((DAT_104c003c & STATE_OVERHEAD_PASS) != 0)
                {
                    DAT_104bfe28 = 1;
                    DAT_10203418 = DAT_102022c4;
                    if (DAT_102022c4 == 0 || DAT_10f654cc == 0)
                        FUN_1000f7f0(DAT_10315140, DAT_104c0054);
                    else
                    {
                        DAT_104c007c->flags &= ~NODE_FLAG_HIDDEN;
                        DAT_118d0ba8[DAT_108360f4].UpdateImage(0);
                        DAT_10f6548c = 1;
                        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
                        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                        FUN_1013f070(RENDER_CULL_MODE, 0, 0);
                        FUN_1013f070(RENDER_FOG, 0, 0);
                        ClearToFog();
                        FUN_1000f7f0(DAT_10315140, DAT_104c0054);
                        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
                        if (DAT_10f65484 != 0)
                            FUN_101439e0();
                        else
                            FUN_1014b480();
                    }
                }
            }
        }
    }
passesDone:
    if ((DAT_104c003c & STATE_OVERHEAD_PASS) == 0)
        DAT_104bfe28 = 0;
    FUN_10199860();
    if (DAT_104c0220 == 0)
    {
        if (DAT_10f654e4 != 0)
            DAT_118d0ba8[DAT_108360f0].UpdateImage(0);
        else if (DAT_10f65484 != 0)
            FUN_101439e0();
        else
            FUN_1014b480();
    }
    ClearToFog();
    FUN_10199860();
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    FUN_1013f070(RENDER_DEPTH_FUNC, 0, 0);
    FUN_1013f070(RENDER_CULL_MODE, 0, 0);
    if (DAT_10255f0c != 0)
        DAT_10255f0c->QueueDraw(&DAT_10836ec8, &DAT_10836cd8,
                                DAT_10804bf8.LightDirection(DAT_10db6ee8, &DAT_118e69e8, &DAT_118e6cb8),
                                PASS_MASK_ALL, &DAT_10da90d0, 0);
    FUN_1013f070(RENDER_CLEAR_DEPTH, CLEAR_DEPTH_FULL, 0);
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    FUN_1013f070(RENDER_DEPTH_FUNC, 0, 0);
    DAT_10255f08->QueueDraw(&DAT_10836ec8, &DAT_108392d8,
                            DAT_10804bf8.LightDirection(DAT_10db6ee8, &DAT_118e69e8, &DAT_118e6cb8),
                            PASS_MASK_ALL, &DAT_10da90d0, 0);
    FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
    if (DAT_102022b8 != 0)
        DAT_10804bf8.DrawShadowsInto(DAT_108654f8.FindByFlags(NODE_FLAG_DRAWN, 0, 0), &DAT_10836ec8,
                                     &DAT_108392d8, 0);
    else
        DAT_10804bf8.DrawShadows(DAT_10255f08, &DAT_10836ec8, &DAT_108392d8, 0);
    FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
    if (DAT_104c0238 != 0 && DAT_10255f08->visibilityCells != 0)
    {
        Box2 *box = DAT_10255f08->visibilityCells->boxes;
        Matrix4 cellFrame;
        cellFrame.SetIdentity();
        for (int cell = 0; cell < VISIBILITY_CELL_COUNT; cell++)
        {
            if (DAT_118e6cb8.InBox(box))
                Debug_DrawShape(DEBUG_SHAPE_SOLID_BOX, &box->min, &box->max, &cellFrame, &DAT_108392d8);
            box++;
        }
    }
    FUN_1000e340();
    // The redundant "!= 0" test is in the original (dropping it changes the compare sequence).
    if (DAT_104bfe6c != 0 && DAT_104bfe6c == SCENE_MODE_PICTURES)
        DAT_10c68000.DrawWorldPictures(&DAT_108392d8, &DAT_10836330);
    FUN_1000c340();
    DAT_10daaa80.Draw(DAT_108392d8, &DAT_10836330, 1);
    FUN_1013f070(RENDER_ALPHA_TEST, 1, ALPHA_TEST_REFERENCE);
    FUN_101999a0(&DAT_10836ec8, &DAT_108392d8, &DAT_118e69e8, 0);
    DAT_1065ca08[0] = DAT_10836ec8;
    DAT_1065ca08[1] = DAT_108392d8;
    DAT_1065ca08[2] = DAT_10836330;
    DAT_10c67ef0[0].Copy(DAT_118e69e8);
    DAT_10daaa80.Draw(DAT_108392d8, &DAT_10836330, 0);
    if (DAT_104bfd78 == 0)
        ResetMeshSlots();
    for (int drawn = DAT_104c0054 - 1; drawn >= 0; drawn--)
    {
        int character = DAT_10315140[drawn];
        if (DAT_10db6ee8[character].shadowOpacity < SHADOW_OPACITY_PLACED)
        {
            Vec4 position = DAT_10db6ee8[character].matrix.GetTranslation();
            DAT_10f66948.Copy(DAT_10f08080[character] + position);
            if (character >= CHARACTER_AREA_COLOUR_FIRST)
                DAT_10f655c4 = DAT_10447bd0[character];
            if (DAT_1020d438[character] != 0)
                DAT_10f655c4 = 0;
            if (character < CHARACTER_OWN_LIGHT_COUNT)
                DAT_10db6ee8[character].Draw(
                    &DAT_108392d8,
                    DAT_10804bf8.LightDirection(&DAT_10db6ee8[character], &DAT_118e69e8, &DAT_118e6cb8), 0);
            else
                DAT_10db6ee8[character].Draw(
                    &DAT_108392d8, DAT_10804bf8.LightDirection(&DAT_10db6ee8[0], &DAT_118e69e8, &DAT_118e6cb8), 0);
            DAT_10f655c4 = 0;
        }
    }
    DAT_10daaa80.Draw(DAT_108392d8, &DAT_10836330, 2);
    FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
    if (DAT_104c0220 == 0 && DAT_10f654e4 != 0)
    {
        FUN_1013f070(RENDER_DEPTH_TEST, 0, 0);
        DAT_118d0ba8[DAT_108360ec].UpdateImage(0);
        DAT_118d0ba8[DAT_108360f0].DrawBlurred();
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        DAT_118d0ba8[DAT_108360e8].UpdateImage(0);
        DAT_118d0ba8[DAT_108360ec].DrawBlurred();
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
        FUN_101446b0();
        FUN_10199860();
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
        FUN_1013f070(RENDER_DEPTH_FUNC, 0, 0);
        FUN_1013f070(RENDER_CULL_MODE, 0, 0);
        DAT_118d0ba8[DAT_108360ec].UpdateImage(0);
        DAT_10f65580 = 1;
        FUN_1013f070(RENDER_CLEAR, CROSSFADE_CLEAR_COLOUR, 0);
        DAT_10255f08->QueueDraw(&DAT_10836ec8, &DAT_108392d8,
                                DAT_10804bf8.LightDirection(DAT_10db6ee8, &DAT_118e69e8, &DAT_118e6cb8),
                                PASS_MASK_ALL, 0, 0);
        if (DAT_104bfe6c != 0 && DAT_104bfe6c == SCENE_MODE_PICTURES)
            DAT_10c68000.DrawWorldPictures(&DAT_108392d8, &DAT_10836330);
        FUN_1013f070(RENDER_ALPHA_TEST, 1, ALPHA_TEST_REFERENCE);
        FUN_101999a0(&DAT_10836ec8, &DAT_108392d8, &DAT_118e69e8, 0);
        FUN_101999a0(&DAT_1065ca08[0], &DAT_1065ca08[1], DAT_10c67ef0, 1);
        DAT_10daaa80.Draw(DAT_1065ca08[1], &DAT_1065ca08[2], 3);
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        DAT_10f65580 = 0;
        if (DAT_10f65484 != 0)
        {
            FUN_101439e0();
            ClearToFog();
            FUN_10199860();
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            FUN_1013f070(RENDER_DEPTH_FUNC, 0, 0);
            FUN_1013f070(RENDER_CULL_MODE, 0, 0);
            if (DAT_10255f0c != 0)
                DAT_10255f0c->QueueDraw(&DAT_10836ec8, &DAT_10836cd8,
                                        DAT_10804bf8.LightDirection(DAT_10db6ee8, &DAT_118e69e8, &DAT_118e6cb8),
                                        PASS_MASK_ALL, &DAT_10da90d0, 0);
            FUN_1013f070(RENDER_CLEAR_DEPTH, CLEAR_DEPTH_FULL, 0);
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            FUN_1013f070(RENDER_DEPTH_FUNC, 0, 0);
            DAT_10255f08->QueueDraw(&DAT_10836ec8, &DAT_108392d8,
                                    DAT_10804bf8.LightDirection(DAT_10db6ee8, &DAT_118e69e8, &DAT_118e6cb8),
                                    PASS_MASK_ALL, &DAT_10da90d0, 0);
            FUN_101999a0(&DAT_10836ec8, &DAT_108392d8, &DAT_118e69e8, 0);
            FUN_101999a0(&DAT_1065ca08[0], &DAT_1065ca08[1], DAT_10c67ef0, 1);
        }
        else
            FUN_1014b480();
        FUN_1013f070(RENDER_DEPTH_TEST, 0, 0);
        FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
        DAT_118d0ba8[DAT_108360ec].DrawThroughShader(&DAT_118d0ba8[DAT_108360f0], &DAT_118d0ba8[DAT_108360e8]);
        FUN_101446b0();
        FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
    }
    if (DAT_104c0121 == 4)
        DAT_118d0ba8[DAT_10f6559c].DrawTinted(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1.0f, 1.0f, 0, 1.0f, 0, 0);
    FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
    DAT_10804bf8.DrawLensFlares();
    if (DAT_104c0220 == 0 && DAT_102027f0 != 0 && DAT_10f654cc != 0 && DAT_10f65484 == 0)
    {
        DAT_10f655a0 = DAT_108360f4;
        DAT_118d0ba8[DAT_108360f4].UpdateImage(0);
        DAT_10f6548c = 1;
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
        FUN_1013f070(RENDER_DEPTH_TEST, 0, 0);
        FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
        FUN_10149100();
        FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
        if (DAT_10f65484 != 0)
            FUN_101439e0();
        else
            FUN_1014b480();
        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
        FUN_1013f070(RENDER_DEPTH_TEST, 1, 0);
    }
    FUN_101999a0(&DAT_1065ca08[0], &DAT_1065ca08[1], DAT_10c67ef0, 1);
    DAT_10daaa80.Draw(DAT_1065ca08[1], &DAT_1065ca08[2], 3);
    FUN_1000d6b0(&DAT_1065ca08[1]);
}
