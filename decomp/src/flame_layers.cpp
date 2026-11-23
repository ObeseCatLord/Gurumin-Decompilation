// Flame layers of a lit object (the "@O_060" model, run from the scene frame 0x100f9670): nine layer nodes of the map
// object glow with the "fire" colour plug motion and, when the object is near, throw sparks and a crackle.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "map_vars.h"

#include "char_state.h"

#include "effect_manager.h"

#include "char_slot.h"

#include "char_tables.h"

#include "char_vars.h"

#include "game_vars.h"

#include "render_vars.h"

extern "C" int FUN_1013f880(void);                                                 // the game's random number
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);        // Sound_PlayEffect
extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *viewProjection, float margin);   // InView

// atan2 on floats (an inline helper: both arguments are rounded to float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Number of layer nodes found for one flame object.
#define FLAME_LAYER_MAX 9
// Name of the layer nodes (the group index picks which of the object's copies).
#define FLAME_NODE_NAME "@O_060:Layer1"
#define FLAME_NODES_PER_GROUP 8
// A part of the flame code over this is negative (it is stored plus 10000).
#define FLAME_CODE_LIMIT 5000
#define FLAME_CODE_RADIX 10000
// Route variants: 0 flames near the player, 1 only when the player is far in x.
#define FLAME_VARIANT_NEAR 0
#define FLAME_VARIANT_FAR 1
#define FLAME_FAR_X 1500.0f
// Distance under which the layer counts as near the player.
#define FLAME_NEAR_DISTANCE 1000.0f
// Frames of a layer's cycle: the crackle plays on frame 8, the cycle ends on frame 0x5a (route variant 1: 0x4b).
#define FLAME_CRACKLE_FRAME 8
#define FLAME_CYCLE_END 0x5a
#define FLAME_CYCLE_END_FAR 0x4b
// Chance in thousandths a ready layer throws a spark.
#define FLAME_SPARK_CHANCE 50
#define FLAME_RANDOM_RANGE 1000
// The layer's heading is turned this far (a quarter turn).
#define FLAME_QUARTER_TURN 1.57075f
// Offsets in the layer's frame of the sparks and the crackle.
#define FLAME_OFFSET_Y (-40.0f)
#define FLAME_OFFSET_Z 60.0f
#define FLAME_SPARK_EFFECT 0x762
#define FLAME_SPARK_FRAME 8
#define FLAME_CRACKLE_SOUND 0x225

// Splits a flame code part: values over FLAME_CODE_LIMIT are negative ones plus FLAME_CODE_RADIX.
static inline int SignedPart(int part)
{
    if (part > FLAME_CODE_LIMIT)
        part -= FLAME_CODE_RADIX;
    return part;
}

// 0x10051e20 (Flames_Update). On the first call finds the nine "@O_060:Layer1" nodes of the object's group and keeps
// their grandparents' parents (the layer pivots); a missing one stops the search with the count found so far. Then,
// every frame, each layer runs its own timer: an idle layer (timer 0) that is on screen or within 1000 units of the
// player (route variant 0, or variant 1 with the player more than 1500 away in x) throws a spark with a 5 % chance:
// sparks 0x762 at its position turned with it, and the "fire" colour plug motion restarts. A running layer counts
// frames, plays the crackle (sound 0x225) at frame 8 and goes idle at frame 0x5b (variant 1: 0x4c). Returns 0.
int FUN_10051e20(int index)
{
    Vec4 player;
    Vec4 position;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    if (DAT_1020b430[index].state == 0)
    {
        for (int layer = 0; layer < FLAME_LAYER_MAX; layer++)
        {
            if (DAT_10255f08 != 0)
                DAT_1021bdf8[index][layer] = DAT_10255f08->Find(
                    FLAME_NODE_NAME, layer + SignedPart(DAT_10db6ee8[index].flameCode / FLAME_CODE_RADIX) * FLAME_NODES_PER_GROUP);
            if (DAT_1021bdf8[index][layer] == 0)
            {
                DAT_1020b430[index].layerCount = layer;
                DAT_1020b430[index].state = 1;
                return 0;
            }
            DAT_1021bdf8[index][layer] = DAT_1021bdf8[index][layer]->parent->parent->parent;
        }
        DAT_1020b430[index].state = 1;
    }
    else
    {
        CharState &state = DAT_1020b430[index];
        for (int layer = 0; layer < state.layerCount; layer++)
        {
            if (state.values[1 + layer] != 0)
            {
                int frame = ++state.values[1 + layer];
                if (frame <= FLAME_CYCLE_END)
                {
                    if (SignedPart(DAT_10db6ee8[index].flameCode % FLAME_CODE_RADIX) != FLAME_VARIANT_FAR ||
                        frame <= FLAME_CYCLE_END_FAR)
                    {
                        if (frame == FLAME_CRACKLE_FRAME)
                        {
                            Vec4 offset;
                            float heading = Atan2(DAT_1021bdf8[index][layer]->matrix._11, DAT_1021bdf8[index][layer]->matrix._12) - FLAME_QUARTER_TURN;
                            offset.Fill(0.0f);
                            offset.y = FLAME_OFFSET_Y;
                            offset.z = FLAME_OFFSET_Z;
                            offset.RotateZ(-heading);
                            position.x = DAT_1021bdf8[index][layer]->matrix.translation.x;
                            position.y = DAT_1021bdf8[index][layer]->matrix.translation.y;
                            position.z = DAT_1021bdf8[index][layer]->matrix.translation.z;
                            position.Copy(position + offset);
                            FUN_1003ad20(FLAME_CRACKLE_SOUND, &position, 1.0f);
                        }
                    }
                    else
                        state.values[1 + layer] = 0;
                }
                else
                    state.values[1 + layer] = 0;
            }
            else
            {
                int variant = SignedPart(DAT_10db6ee8[index].flameCode % FLAME_CODE_RADIX);
                if ((variant == FLAME_VARIANT_NEAR &&
                     (FUN_101ae4d0(&DAT_1021bdf8[index][layer]->matrix.GetTranslation(), &DAT_108392d8,
                                   DAT_10db6ee8[index].loadedRoot->cullMargin) != 0 ||
                      (player - DAT_1021bdf8[index][layer]->matrix.GetTranslation()).Length() < FLAME_NEAR_DISTANCE)) ||
                    (SignedPart(DAT_10db6ee8[index].flameCode % FLAME_CODE_RADIX) == FLAME_VARIANT_FAR &&
                     player.x > FLAME_FAR_X &&
                     (FUN_101ae4d0(&DAT_1021bdf8[index][layer]->matrix.GetTranslation(), &DAT_108392d8,
                                   DAT_10db6ee8[index].loadedRoot->cullMargin) != 0 ||
                      (player - DAT_1021bdf8[index][layer]->matrix.GetTranslation()).Length() < FLAME_NEAR_DISTANCE)))
                {
                    if (FUN_1013f880() % FLAME_RANDOM_RANGE < FLAME_SPARK_CHANCE)
                    {
                        float heading = Atan2(DAT_1021bdf8[index][layer]->matrix._11, DAT_1021bdf8[index][layer]->matrix._12) - FLAME_QUARTER_TURN;
                        Vec4 offset;
                        Matrix4 matrix;
                        EffectOptions options;
                        matrix = RotationZ(heading);
                        offset.Fill(0.0f);
                        offset.y = FLAME_OFFSET_Y;
                        offset.z = FLAME_OFFSET_Z;
                        offset.RotateZ(-heading);
                        matrix.SetTranslation(DAT_1021bdf8[index][layer]->matrix.GetTranslation());
                        matrix.translation.x += offset.x;
                        matrix.translation.y += offset.y;
                        matrix.translation.z += offset.z;
                        offset.Fill(0.0f);
                        options.m_0000 = 0;
                        options.m_0004 = 0xf;
                        options.m_000e = 5;
                        DAT_10daaa80.Start(FLAME_SPARK_EFFECT, offset, -1, -1, 0, 0, FLAME_SPARK_FRAME, &matrix, &options,
                                           &DAT_10219770[index]);
                        state.values[1 + layer] = 1;
                        DAT_1021bdf8[index][layer]->RestartColourPlugs("fire", DAT_104bf8a4);
                    }
                }
            }
        }
    }
    return 0;
}
