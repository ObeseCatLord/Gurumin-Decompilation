// Footstep effects and sounds of the player (EN build, no JP twin).
#include "char.h"
#include "effect_manager.h"
#include "footstep.h"
#include "hud_fade.h"
#include "sound_effects.h"
#include "switches.h"
#include "vec4.h"

// Frames after the start of the game before steps are heard.
const unsigned int FOOTSTEP_FIRST_FRAME = 15;
// Scene timer range [first, last) in which steps are silent.
const int FOOTSTEP_SILENT_TIMER_FIRST = 30000;
const int FOOTSTEP_SILENT_TIMER_END = 31000;
// The game mode in which steps are played.
const int FOOTSTEP_PLAY_MODE = 1;
// The step counter picks one of four sounds.
const int FOOTSTEP_SOUND_VARIANTS = 4;
// Effect ids of the steps.
const int FOOTSTEP_EFFECT_DUST = 0x2da;
const int FOOTSTEP_EFFECT_SPLASH = 0x2dc;
const int FOOTSTEP_EFFECT_SPLASH_RING = 0x88bd;
const int FOOTSTEP_EFFECT_SMALL = 0x2e0;
// First sound id of each surface (plus the step variant).
const int FOOTSTEP_SOUND_PLAIN = 0;
const int FOOTSTEP_SOUND_A = 0x227;
const int FOOTSTEP_SOUND_SPLASH = 0x258;
const int FOOTSTEP_SOUND_SMALL = 0x25c;
const int FOOTSTEP_SOUND_E = 0x3c0;
// Height of the ring effect under the footstep position.
const float FOOTSTEP_RING_Z = -5.0f;

// Footstep_Play: plays the effect and sound of one footstep at `*position` (see footstep.h). Called by the player's
// movement when a foot lands.
void Footstep_Play(int unused1, int unused2, const Vec4 *position)
{
    if (g_gameFrames < FOOTSTEP_FIRST_FRAME)
        return;
    if (g_sceneTimer >= FOOTSTEP_SILENT_TIMER_FIRST && g_sceneTimer < FOOTSTEP_SILENT_TIMER_END)
        return;
    if (g_stepsSilenced != 0)
        return;
    if (g_sceneSoundMode != 0)
        return;
    if (g_gameMode != FOOTSTEP_PLAY_MODE)
        return;
    g_stepCounter++;
    if (g_stepSurfaceA != 0)
    {
        Sound_PlayEffect((g_stepCounter & (FOOTSTEP_SOUND_VARIANTS - 1)) + FOOTSTEP_SOUND_A, position, 1.0f);
    }
    else if (g_stepSurfaceB != 0)
    {
        Sound_PlayEffect(g_stepCounter & (FOOTSTEP_SOUND_VARIANTS - 1), position, 1.0f);
    }
    else if (g_stepSurfaceC != 0)
    {
        Vec4 where;
        where.Copy(*position);
        g_effectManager.Start(FOOTSTEP_EFFECT_SMALL, where, -1, 0, 0, 0, 0, 0, 0, 0);
        Sound_PlayEffect((g_stepCounter & (FOOTSTEP_SOUND_VARIANTS - 1)) + FOOTSTEP_SOUND_SMALL, position, 1.0f);
    }
    else if (g_stepSurfaceD != 0)
    {
        Vec4 where;
        where.Copy(*position);
        g_effectManager.Start(FOOTSTEP_EFFECT_SPLASH, where, -1, 0, 0, 0, 0, 0, 0, 0);
        Matrix4 ring;
        ring = RotationZ(g_characterModels[0].heading + ANGLE_HALF_TURN);
        ring.Translate(where.x, where.y, where.z);
        float zero = 0;
        where.Fill(zero);
        where.z = FOOTSTEP_RING_Z;
        g_effectManager.Start(FOOTSTEP_EFFECT_SPLASH_RING, where, -1, 0, 0, 0, 0, &ring, 0, 0);
        Sound_PlayEffect((g_stepCounter & (FOOTSTEP_SOUND_VARIANTS - 1)) + FOOTSTEP_SOUND_SPLASH, position, 1.0f);
    }
    else if (g_stepSurfaceE != 0)
    {
        Vec4 where;
        where.Copy(*position);
        g_effectManager.Start(FOOTSTEP_EFFECT_DUST, where, -1, 0, 0, 0, 0, 0, 0, 0);
        Sound_PlayEffect((g_stepCounter & (FOOTSTEP_SOUND_VARIANTS - 1)) + FOOTSTEP_SOUND_E, position, 1.0f);
    }
    else
    {
        Vec4 where;
        where.Copy(*position);
        g_effectManager.Start(FOOTSTEP_EFFECT_DUST, where, -1, 0, 0, 0, 0, 0, 0, 0);
        Sound_PlayEffect(g_stepCounter & (FOOTSTEP_SOUND_VARIANTS - 1), position, 1.0f);
    }
}
