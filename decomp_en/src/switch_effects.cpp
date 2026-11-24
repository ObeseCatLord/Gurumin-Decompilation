// Effects of the switch objects (EN build). Ported from the matched JP source decomp/src/switch_effects.cpp.
#include "camera.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "node.h"
#include "switches.h"
#include "visibility.h"

// Objects farther than this from the camera target start no effects.
const float SWITCH_EFFECT_RANGE = 3000.0f;
// Frames between two effect starts of a slot.
const unsigned int SWITCH_EFFECT_PERIOD = 12;
// Effect ids of the three effect kinds (kind 0 follows the object, kinds 1 and 2 play at its position).
const int EFFECT_SWITCH_ATTACHED = 0x65;
const int EFFECT_SWITCH_AT_POSITION_A = 0x6a;
const int EFFECT_SWITCH_AT_POSITION_B = 0x86b;
// Value of EffectOptions::flags that makes an effect follow what it is attached to.
const unsigned char EFFECT_OPTION_FOLLOW_MATRIX = 1;
// Effect parent and unused arguments meaning "none".
const int EFFECT_NO_PARENT = -1;

// Switch_UpdateEffects: for each hold slot in use whose object is near the camera target and visible, the first time
// and then every 12 frames, an object with the hidden flag releases its holder; the other objects start effect kind
// 0 attached to the object, 1 or 2 at the object's position. Returns 1. Called every frame by the game update.
extern "C" int Switch_UpdateEffects(void)
{
    for (int slot = 0; slot < SWITCH_HOLD_SLOTS; slot++)
    {
        Node *object = (Node *)g_switchHeldObject[slot];
        if (object != 0)
        {
            Vec4 at = object->worldMatrix.GetTranslation();
            if (at.Distance(g_cameraTargetNow) < SWITCH_EFFECT_RANGE && Vis_IsVisible(at))
            {
                Vec4 position;
                if (g_switchEffectStarted[slot] == 0 || g_gameFrames % SWITCH_EFFECT_PERIOD == 0)
                {
                    g_switchEffectStarted[slot] = 1;
                    if (((Node *)g_switchHeldObject[slot])->flags & NODE_FLAG_HIDDEN)
                        Switch_Release(g_switchHolder[slot]);
                    else
                    {
                        if (g_switchEffectKind[slot] == 0)
                        {
                            position.Fill(0.0f);
                            EffectOptions options;
                            options.flags = EFFECT_OPTION_FOLLOW_MATRIX;
                            g_effectManager.Start(EFFECT_SWITCH_ATTACHED, position, EFFECT_NO_PARENT, EFFECT_NO_PARENT,
                                                  &((Node *)g_switchHeldObject[slot])->worldMatrix, 0, 0, 0, &options, 0);
                        }
                        if (g_switchEffectKind[slot] == 1)
                        {
                            EffectOptions options;
                            options.flags = EFFECT_OPTION_FOLLOW_MATRIX;
                            position.Copy(((Node *)g_switchHeldObject[slot])->worldMatrix.GetTranslation());
                            g_effectManager.Start(EFFECT_SWITCH_AT_POSITION_A, position, EFFECT_NO_PARENT,
                                                  EFFECT_NO_PARENT, 0, 0, 0, 0, &options, 0);
                        }
                        if (g_switchEffectKind[slot] == 2)
                        {
                            EffectOptions options;
                            options.flags = EFFECT_OPTION_FOLLOW_MATRIX;
                            position.Copy(((Node *)g_switchHeldObject[slot])->worldMatrix.GetTranslation());
                            g_effectManager.Start(EFFECT_SWITCH_AT_POSITION_B, position, EFFECT_NO_PARENT,
                                                  EFFECT_NO_PARENT, 0, 0, 0, 0, &options, 0);
                        }
                    }
                }
            }
        }
    }
    return 1;
}
