// Host hooks of the effect system (EN build, EN-only: no JP twin found).
#include "effect_host_hooks.h"
#include "camera.h"
#include "char.h"
#include "depth_sort.h"
#include "game_vars.h"
#include "map_view.h"
#include "marker_rider.h"
#include "sound_effects.h"
#include <string.h>

// EffectHook_HostCommand: runs the host command `command` (see effect_host_hooks.h). Returns 0. Called through the
// effect hook slot.
extern "C" int EffectHook_HostCommand(float command, float a, float b, float c, const Vec4 *position)
{
    if (command == HOST_COMMAND_SOUND)
    {
        Sound_PlayEffect((int)c, position, 1.0f);
        return 0;
    }
    if (command == HOST_COMMAND_CAPTION)
    {
        Caption_SetTitle(HOST_COMMAND_CAPTION_TEXT);
        return 0;
    }
    if (command == HOST_COMMAND_FADE)
    {
        ColourFade_StartInline(HOST_COMMAND_FADE_FRAMES, a, b, c, 1.0f);
        return 0;
    }
    if (command == HOST_COMMAND_KNOCK_BACK)
        Player_KnockBack();
    return 0;
}

// EffectHook_PickTarget: see effect_host_hooks.h. Keeps the current target of the effect while it lives, else collects
// the enemies (and markers) in range, sorts them by distance and picks one by `cycle`.
extern "C" int EffectHook_PickTarget(float x, float y, float z, float cycle, EffectOptions *options)
{
    Vec4 origin;
    origin.Set(x, y, z, 0);
    int count = 0;
    int target = options->m_0014;
    if (target != 0)
    {
        if (g_charRecords[target].stats[STAT_HEALTH] >= 0 && !(g_characterModels[target].matrix.m[MATRIX_TRANSLATION_X] < CHAR_OFFWORLD_X))
        {
            Vec4 aim = g_characterModels[target].matrix.GetTranslation();
            aim.z += HOOK_TARGET_HEIGHT;
            options->steering->Copy(aim);
            return options->m_0014;
        }
        options->m_0014 = 0;
    }
    int slots[HOOK_TARGET_MAX];
    float keys[HOOK_TARGET_MAX];
    for (int slot = ENEMY_SLOT_FIRST; slot < ENEMY_SLOT_END; slot++)
    {
        if (g_characterModels[slot].root != 0 &&
            (g_charKind[slot] == CHAR_KIND_ENEMY_SLOT ||
             (options->m_000e == HOOK_TARGET_ALLOW_MARKERS && g_charKind[slot] == HOOK_TARGET_KIND_MARKER)) &&
            g_charPoseState[slot] == 0 && g_charRecords[slot].m_0142 == 0)
        {
            Vec4 position = g_characterModels[slot].matrix.GetTranslation();
            position.z += HOOK_TARGET_HEIGHT;
            if ((options->m_000e == HOOK_TARGET_ALLOW_MARKERS && g_charKind[slot] == HOOK_TARGET_KIND_MARKER) ||
                (position.x > CHAR_OFFWORLD_X && g_charRecords[slot].stats[STAT_HEALTH] > 0))
            {
                float distance = origin.DistanceSquared(position);
                keys[count] = distance;
                if (distance < HOOK_TARGET_RANGE_SQUARED)
                {
                    if (!View_ContainsPoint(&g_characterModels[slot].matrix.GetTranslation(), &g_cameraViewProjection,
                                            HOOK_TARGET_VIEW_MARGIN))
                        keys[count] += HOOK_TARGET_OFFSCREEN_PENALTY;
                    slots[count] = slot;
                    count++;
                }
            }
        }
    }
    if (count == 0)
        return -1;
    SortByDepth(keys, 0, 0, slots, count);
    options->m_0014 = slots[(int)cycle % count];
    Vec4 aim = g_characterModels[options->m_0014].matrix.GetTranslation();
    aim.z += HOOK_TARGET_HEIGHT;
    options->steering->Copy(aim);
    return 1;
}
