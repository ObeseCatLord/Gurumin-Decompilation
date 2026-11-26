// The lava hazard of a map (the "@yogan" nodes: yogan = lava) and its glowing floor effects (EN build; JP twin:
// lava_hazard.cpp).
#include "camera.h"
#include "char.h"
#include "char_attach.h"
#include "effect_holds.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "engine.h"
#include "event_slots.h"
#include "anti_tamper.h"
#include "marker_rider.h"
#include "listbank.h"
#include "player_damage.h"
#include "visibility.h"

extern "C" int Camera_SetMode1(void);

// Effect ids the map holds in g_effectHolds for the lava and the floor lights.
const int EFFECT_LAVA_SPARK = 0x740;
const int EFFECT_FLOOR_LIGHT = 0x36c3;
// The player is in the lava when its height is under this and the "@yogan_atari" surface is more than
// LAVA_SURFACE_MARGIN above it.
const float LAVA_DEPTH = -600.0f;
const float LAVA_SURFACE_MARGIN = 40.0f;
// Trap helmet bits of g_helmetFaceSettings: both = heals, 0x20000 = no damage, 0x10000 = half damage.
const int HELMET_TRAP_HALF = 0x10000;
const int HELMET_TRAP_NONE = 0x20000;
const int HELMET_TRAP_HEAL = 0x30000;
// Damage of a frame in the lava without a trap helmet, and with the half damage helmet.
const int LAVA_DAMAGE = 5;
const int LAVA_DAMAGE_HALF = 2;
// Frames the player is protected after a frame in lava healed or was cancelled.
const int LAVA_PROTECT_FRAMES = 0x3c;
// Frames a hold slot must have waited before its effect may start, and the chance (of 10000) per frame.
const int LAVA_SPARK_AGE = 0x5a;
const int LAVA_SPARK_CHANCE = 500;
const int FLOOR_LIGHT_AGE = 0x3c;
const int FLOOR_LIGHT_CHANCE = 1000;
const int LAVA_SPARK_NODES = 10;
const int FLOOR_LIGHT_NODES = 4;
// Range of the random draw the chances are tested against.
const int CHANCE_RANGE = 10000;
// g_healthGaugeChange value that makes the health gauge show a gain.
const int GAUGE_GAIN = 8;
// Effect list the floor lights are started in (EffectOptions::m_000e).
const char FLOOR_LIGHT_LIST = 1;

// Lava_Update: runs every frame on maps with a "@yogan_null" lava node: the player under the surface of "@yogan_atari"
// takes lava damage (camera mode 1; the trap helmet heals, cancels or halves it), otherwise the camera returns to the
// follow mode. Each "@yogan_efc" node then starts a spark effect from a waiting hold slot (effect 0x740), and each
// "@fb_null" node a floor light with a random turn (effect 0x36c3). Returns 0. Called by the scene frame.
int Lava_Update(void)
{
    int slot;   // next hold slot to look at; like the original, only set once a "@yogan_null" node exists
    float tiltX[4] = {-0.7f, -0.0518f, 0, 0};
    float turnZ[4] = {0.39141f, 5.54959f, 5.09459f, 2.36341f};
    int i;
    if (g_mapRootNode != 0)
    {
        Node *lava = g_mapRootNode->Find("@yogan_null", 0);
        if (lava != 0)
        {
            Node *surface = lava->Find("@yogan_atari", 0);
            if (surface != 0 && g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Z] < LAVA_DEPTH &&
                g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Z] + LAVA_SURFACE_MARGIN <
                    surface->worldMatrix.m[MATRIX_TRANSLATION_Z])
            {
                if (g_gameBuiltinsRegistered != 0)
                    Camera_SetMode1();
                if (g_charPoseState[0] <= 0 && g_charProtectFrames[0] <= 0)
                {
                    if ((g_helmetFaceSettings & HELMET_TRAP_HEAL) == HELMET_TRAP_HEAL)
                    {
                        EventSlot_AddWaiting(0, 1);
                        g_charProtectFrames[0] = LAVA_PROTECT_FRAMES;
                        int health = g_charRecords[0].stats[STAT_HEALTH] + 1;
                        if (health > g_charRecords[0].stats[STAT_MAX_HEALTH])
                            health = g_charRecords[0].stats[STAT_MAX_HEALTH];
                        if (g_charRecords[0].stats[STAT_HEALTH] != health)
                            g_healthGaugeChange = GAUGE_GAIN;
                        g_charRecords[0].stats[STAT_HEALTH] = health;
                        AntiTamper_StoreValues();
                    }
                    else if (g_helmetFaceSettings & HELMET_TRAP_NONE)
                    {
                        EventSlot_AddWaiting(0, 1);
                        g_charProtectFrames[0] = LAVA_PROTECT_FRAMES;
                    }
                    else if (g_helmetFaceSettings & HELMET_TRAP_HALF)
                        Player_TakeDamage(LAVA_DAMAGE_HALF);
                    else
                        Player_TakeDamage(LAVA_DAMAGE);
                }
            }
            else if (g_gameBuiltinsRegistered != 0)
                Camera_SetModeFollow();
            slot = 0;
            for (i = 0; i < LAVA_SPARK_NODES; i++)
            {
                Node *spark = lava->Find("@yogan_efc", i);
                if (spark == 0)
                    break;
                for (int j = slot; j < EFFECT_HOLD_COUNT; j++)
                {
                    slot = j + 1;
                    if (g_effectHolds[j].effect == EFFECT_LAVA_SPARK)
                    {
                        if (g_effectHolds[j].age > LAVA_SPARK_AGE && Random() % CHANCE_RANGE < LAVA_SPARK_CHANCE)
                        {
                            Vec4 position;
                            g_effectManager.Start(EFFECT_LAVA_SPARK, position, -1, -1, 0, 0, 0, &spark->worldMatrix,
                                                  0, 0);
                            g_effectHolds[j].age = 0;
                        }
                        break;
                    }
                }
            }
        }
        for (i = 0; i < FLOOR_LIGHT_NODES; i++)
        {
            Node *light = g_mapRootNode->Find("@fb_null", i);
            if (light == 0)
                return 0;
            for (int j = slot; j < EFFECT_HOLD_COUNT; j++)
            {
                slot = j + 1;
                if (g_effectHolds[j].effect == EFFECT_FLOOR_LIGHT)
                {
                    if (g_effectHolds[j].age > FLOOR_LIGHT_AGE && Random() % CHANCE_RANGE < FLOOR_LIGHT_CHANCE)
                    {
                        Vec4 position;
                        Matrix4 matrix;
                        EffectOptions options;
                        options.m_000e = FLOOR_LIGHT_LIST;
                        options.colour = g_effectHolds[j].value;
                        matrix = RotationX(tiltX[i]) * RotationZ(turnZ[i]);
                        matrix.SetTranslation(light->worldMatrix.GetTranslation());
                        g_effectManager.Start(EFFECT_FLOOR_LIGHT, position, -1, -1, 0, 0, 0, &matrix, &options, 0);
                        g_effectHolds[j].age = 0;
                    }
                    break;
                }
            }
        }
    }
    return 0;
}
