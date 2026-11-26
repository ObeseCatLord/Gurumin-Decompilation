// Enemy waves (EN build). Ported from the matched JP source decomp/src/enemy_spawn.cpp.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "effect_manager.h"
#include "hover_watcher.h"
#include "vec4.h"
#include "wave_update.h"

// Wave groups, slots per group, and the waiting-enemy limits.
const int WAVE_GROUP_COUNT = 4;
const int WAVE_NO_SLOT = 99;
const int WAVE_PRESENT_MAX = 2;
// An enemy whose x is at or below this is not in the world (waiting).
const float WAVE_NOT_PLACED_X = -20000.0f;
// Talk ids with hundreds mark enemies that never come in by themselves.
const int WAVE_TALK_HUNDREDS = 100;
// How the enemy comes in: flash effect, idle motion layer, blend frames.
const int WAVE_FLASH_EFFECT = 0x3f4;
const int WAVE_IDLE_LAYER = 2;
const int WAVE_IDLE_BLEND = 5;
// The slot's group timer value after it came in, and the variants of a slot.
const int WAVE_GROUP_TIMER = 2;
const int WAVE_VARIANT_COUNT = 5;

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Wave_Update: see wave_update.h. Called by the scene frame.
int Wave_Update(void)
{
    int alive = 0;
    Vec4 position;
    for (int group = 0; group < WAVE_GROUP_COUNT; group++)
    {
        int chosen = -1;
        int earliest = WAVE_NO_SLOT;
        int present = 0;
        for (int slot = 0; slot < WAVE_GRID_SIZE; slot++)
        {
            if (g_waveGrid[group][slot] == 0)
                continue;
            int enemy = g_waveGrid[group][slot];
            if (g_charPhase[enemy] != 0)
            {
                g_waveGrid[group][slot] = 0;
                continue;
            }
            alive++;
            if (g_characterModels[enemy].matrix.m[MATRIX_TRANSLATION_X] > WAVE_NOT_PLACED_X ||
                g_charMarkers[enemy].x > WAVE_NOT_PLACED_X)
                present++;
            else if (earliest > slot)
            {
                earliest = slot;
                if (g_charState[enemy].values[1] / WAVE_TALK_HUNDREDS != 0)
                    chosen = -1;
                else
                    chosen = enemy;
            }
            if (present < WAVE_PRESENT_MAX && chosen > 0)
            {
                position.Copy(g_characterModels[chosen].matrix.GetTranslation());
                g_charScriptValue[chosen] = -1;
                position.x = position.x * -1.0f - 100000.0f;
                g_characterModels[chosen].SetPosition(position.x, position.y, position.z);
                g_characterModels[chosen].PlayMotion(WAVE_IDLE_LAYER, 0, WAVE_IDLE_BLEND, 1.0f);
                g_charHomes[chosen].Copy(g_characterModels[chosen].matrix.GetTranslation());
                g_charGroupTimer[chosen] = WAVE_GROUP_TIMER;
                g_charToldToReact[chosen] = 0;
                if (g_charInPlay[chosen] == 0)
                {
                    g_charInPlay[chosen] = 1;
                    g_charVariant[chosen] = (char)(chosen % WAVE_VARIANT_COUNT);
                    g_charMarkers[chosen].Copy(g_characterModels[chosen].matrix.GetTranslation());
                }
                Matrix4 matrix;
                Vec4 origin;
                matrix.SetIdentity();
                matrix.SetTranslation(position);
                g_effectManager.Start(WAVE_FLASH_EFFECT, origin, -1, -1, 0, 0, 0, &matrix, 0, 0);
                float heading = Atan2(g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] -
                                          g_characterModels[chosen].matrix.m[MATRIX_TRANSLATION_X],
                                      g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] -
                                          g_characterModels[chosen].matrix.m[MATRIX_TRANSLATION_Y]) +
                                ANGLE_HALF_TURN;
                g_characterModels[chosen].targetHeading = heading;
                g_characterModels[chosen].heading = heading;
                Char_ResetState(chosen);
            }
        }
    }
    g_waveEnemiesAlive = alive;
    return 0;
}
