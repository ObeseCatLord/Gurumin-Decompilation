// The lava hazard of a map (the "@yogan" nodes: yogan = lava) and its glowing floor effects.

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "effect_holds.h"

#include "char_slot.h"

#include "char_vars.h"
#include "char_tables.h"
#include "game_vars.h"
#include "map_vars.h"

extern "C" int FUN_1013f880(void);                    // the game's random number
extern "C" int FUN_1000e750(void);                    // AntiTamper_StoreValues
extern "C" int FUN_10011aa0(int id, int value);       // EventSlots_AddWaiting
extern "C" int FUN_1005dbf0(int damage);              // Player_TakeDamage
extern "C" int FUN_1016a8c0(void);                    // camera mode 1
extern "C" int FUN_1016a8e0(void);                    // camera mode 0

// Effect ids the map holds in DAT_1020dcf8 for the lava and the floor lights.
#define EFFECT_LAVA_SPARK 0x740
#define EFFECT_FLOOR_LIGHT 0x36c3
// The player is in the lava when its height is under this and the "@yogan_atari" surface is more than
// LAVA_SURFACE_MARGIN above it.
#define LAVA_DEPTH (-600.0f)
#define LAVA_SURFACE_MARGIN 40.0f
// Trap helmet bits of DAT_104c02c8 (ability_text.cpp): both = heals, 0x20000 = no damage, 0x10000 = half damage.
#define HELMET_TRAP_HALF 0x10000
#define HELMET_TRAP_NONE 0x20000
#define HELMET_TRAP_HEAL 0x30000
// Damage of a frame in the lava without a trap helmet, and with the half damage helmet.
#define LAVA_DAMAGE 5
#define LAVA_DAMAGE_HALF 2
// Frames the player is protected after a frame in lava healed or was cancelled.
#define LAVA_PROTECT_FRAMES 0x3c
// Frames a hold slot must have waited before its effect may start, and the chance (of 10000) per frame.
#define LAVA_SPARK_AGE 0x5a
#define LAVA_SPARK_CHANCE 500
#define FLOOR_LIGHT_AGE 0x3c
#define FLOOR_LIGHT_CHANCE 1000
#define LAVA_SPARK_NODES 10
#define FLOOR_LIGHT_NODES 4
// DAT_104c011d value that makes the health gauge show a gain.
#define GAUGE_GAIN 8

// 0x1005f750 (Lava_Update). A C++ function (its temporary matrices need an EH frame). Runs every frame on maps with a "@yogan_null" lava node: the player under the surface
// of "@yogan_atari" takes lava damage (camera mode 1; the trap helmet heals, cancels or halves it), otherwise
// the camera returns to mode 0. Each "@yogan_efc" node then starts a spark effect from a waiting hold slot
// (effect 0x740), and each "@fb_null" node a floor light with a random turn (effect 0x36c3). Returns 0.
int Lava_Update(void)
{
    int slot;   // next hold slot to look at; like the original, only set once a "@yogan_null" node exists
    float tiltX[4] = {-0.7f, -0.0518f, 0, 0};
    float turnZ[4] = {0.39141f, 5.54959f, 5.09459f, 2.36341f};
    int i;
    if (DAT_10255f08 != 0)
    {
        Node *lava = DAT_10255f08->Find("@yogan_null", 0);
        if (lava != 0)
        {
            slot = 0;
            Node *surface = lava->Find("@yogan_atari", 0);
            if (surface != 0 && DAT_10db9248 < LAVA_DEPTH && DAT_10db9248 + LAVA_SURFACE_MARGIN < surface->worldMatrix.m[14])
            {
                if (DAT_1032d69c != 0)
                    FUN_1016a8c0();
                if (DAT_104686b0[0] <= 0 && DAT_10253e98[0] <= 0)
                {
                    if ((DAT_104c02c8 & HELMET_TRAP_HEAL) == HELMET_TRAP_HEAL)
                    {
                        FUN_10011aa0(0, 1);
                        DAT_10253e98[0] = LAVA_PROTECT_FRAMES;
                        int health = DAT_10daaae8[0].stats[STAT_HEALTH] + 1;
                        if (health > DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
                            health = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
                        if (DAT_10daaae8[0].stats[STAT_HEALTH] != health)
                            DAT_104c011d = GAUGE_GAIN;
                        DAT_10daaae8[0].stats[STAT_HEALTH] = health;
                        FUN_1000e750();
                    }
                    else if (DAT_104c02c8 & HELMET_TRAP_NONE)
                    {
                        FUN_10011aa0(0, 1);
                        DAT_10253e98[0] = LAVA_PROTECT_FRAMES;
                    }
                    else if (DAT_104c02c8 & HELMET_TRAP_HALF)
                        FUN_1005dbf0(LAVA_DAMAGE_HALF);
                    else
                        FUN_1005dbf0(LAVA_DAMAGE);
                }
            }
            else if (DAT_1032d69c != 0)
                FUN_1016a8e0();
            for (i = 0; i < LAVA_SPARK_NODES; i++)
            {
                Node *spark = lava->Find("@yogan_efc", i);
                if (spark == 0)
                    break;
                for (int j = slot; j < EFFECT_HOLD_COUNT; j++)
                {
                    slot = j + 1;
                    if (DAT_1020dcf8[j].effect == EFFECT_LAVA_SPARK)
                    {
                        if (DAT_1020dcf8[j].age > LAVA_SPARK_AGE && FUN_1013f880() % 10000 < LAVA_SPARK_CHANCE)
                        {
                            Vec4 position;
                            DAT_10daaa80.Start(EFFECT_LAVA_SPARK, position, -1, -1, 0, 0, 0, &spark->worldMatrix, 0, 0);
                            DAT_1020dcf8[j].age = 0;
                        }
                        break;
                    }
                }
            }
        }
        for (i = 0; i < FLOOR_LIGHT_NODES; i++)
        {
            Node *light = DAT_10255f08->Find("@fb_null", i);
            if (light == 0)
                return 0;
            for (int j = slot; j < EFFECT_HOLD_COUNT; j++)
            {
                slot = j + 1;
                if (DAT_1020dcf8[j].effect == EFFECT_FLOOR_LIGHT)
                {
                    if (DAT_1020dcf8[j].age > FLOOR_LIGHT_AGE && FUN_1013f880() % 10000 < FLOOR_LIGHT_CHANCE)
                    {
                        Vec4 position;
                        Matrix4 matrix;
                        EffectOptions options;
                        options.m_000e = 1;
                        options.m_0004 = DAT_1020dcf8[j].value;
                        matrix = RotationX(tiltX[i]) * RotationZ(turnZ[i]);
                        matrix.SetTranslation(light->worldMatrix.GetTranslation());
                        DAT_10daaa80.Start(EFFECT_FLOOR_LIGHT, position, -1, -1, 0, 0, 0, &matrix, &options, 0);
                        DAT_1020dcf8[j].age = 0;
                    }
                    break;
                }
            }
        }
    }
    return 0;
}
