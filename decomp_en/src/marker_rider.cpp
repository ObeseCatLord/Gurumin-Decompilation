// A character riding a marker and vanishing when it falls away (EN build). Ported from the matched JP source
// decomp/src/marker_rider.cpp.
#include <math.h>
#include "anim_model.h"
#include "anti_tamper.h"
#include "char.h"
#include "effect_manager.h"
#include "marker_rider.h"
#include "node.h"
#include "vec4.h"

// Marker slots searched for the marker to ride.
const int MARKER_FIRST_SLOT = 0x5c;
const int MARKER_SLOT_END = 0x7c;
// g_charKind value of a taken marker.
const char MARKER_KIND_TAKEN = 'Q';
// The marker must lie within this distance on every axis.
const float MARKER_REACH = 75.0f;
// A marker below this height has fallen out of the world.
const float MARKER_GONE_Z = -20000.0f;
// Phases kept in CharState::values[MARKER_VALUE_PHASE].
const int MARKER_PHASE_NEW = 0;
const int MARKER_PHASE_RIDING = 1;
const int MARKER_PHASE_VANISHING = 2;
// CharState::values indices: wait, talk id, the marker slot ridden (-1 = gone, 0 = none), the frames left while
// vanishing, the phase; MARKER_VALUE_COUNT is the number of values the state reset clears.
const int MARKER_VALUE_WAIT = 0;
const int MARKER_VALUE_TALK = 1;
const int MARKER_VALUE_SLOT = 9;
const int MARKER_VALUE_VANISH_FRAMES = 10;
const int MARKER_VALUE_PHASE = 11;
const int MARKER_VALUE_COUNT = 13;
// Raw words of the CharState the reset clears (everything between the talk id and the behaviour).
const int MARKER_FIRST_CLEARED_WORD = 3;
const int MARKER_LAST_CLEARED_WORD = 15;
// Behaviour given to a character that found no marker, and its talk id.
const int MARKER_NO_MARKER_BEHAVIOUR = 0x11;
const int MARKER_NO_MARKER_TALK = 0x32;
// Vanishing frames with g_guardedValue8 below 2, and equal to 2 (none above).
const int MARKER_VANISH_EASY = 30;
const int MARKER_VANISH_HARD = 15;
// Node kind masks of the riding and of the vanishing nodes.
const unsigned int MARKER_KIND_MASK = 0x400;
const unsigned int MARKER_VANISH_KIND_MASK = 0x200;
// Display state of a vanishing character, and the effect started every second frame.
const unsigned char MARKER_VANISH_DISPLAY_STATE = 0x1e;
const int MARKER_PUFF_EFFECT = 0x2da;
// Motion layer, blend frames and the half turn added to the heading facing the player.
const int MARKER_MOTION_LAYER = 1;
const int MARKER_MOTION_FRAMES = 5;
const float MARKER_TURN_HALF = 3.1415f;

// Npc_RideMarker: first frame: finds the taken marker (slot MARKER_FIRST_SLOT up) within MARKER_REACH of the character
// on all three axes; with none the slot's state is cleared and it becomes behaviour 0x11. While the marker exists the
// character is shown (its shadow hidden, its model locked) and its node flags are set once; when the marker falls out
// of the world the character starts vanishing (phase 2, 30 frames, 15 with g_guardedValue8 == 2, none above) with
// display state 0x1e. While vanishing it faces the player, plays motion 1, and every second frame starts effect 0x2da;
// at the end of the countdown its state is reset. Returns 1 while it is riding or vanishing, else 0. Called by the
// behaviour dispatch.
int Npc_RideMarker(int index)
{
    if (g_charState[index].state == 0)
    {
        int marker = MARKER_FIRST_SLOT;
        do
        {
            if (g_charKind[marker] == MARKER_KIND_TAKEN &&
                g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] - MARKER_REACH < g_characterModels[marker].matrix.m[MATRIX_TRANSLATION_X] + MARKER_REACH &&
                g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] + MARKER_REACH > g_characterModels[marker].matrix.m[MATRIX_TRANSLATION_X] - MARKER_REACH &&
                g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] - MARKER_REACH < g_characterModels[marker].matrix.m[MATRIX_TRANSLATION_Y] + MARKER_REACH &&
                g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y] + MARKER_REACH > g_characterModels[marker].matrix.m[MATRIX_TRANSLATION_Y] - MARKER_REACH &&
                g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] - MARKER_REACH < g_characterModels[marker].matrix.m[MATRIX_TRANSLATION_Z] + MARKER_REACH &&
                g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] + MARKER_REACH > g_characterModels[marker].matrix.m[MATRIX_TRANSLATION_Z] - MARKER_REACH)
            {
                g_charState[index].values[MARKER_VALUE_SLOT] = marker;
                break;
            }
            marker++;
        } while (marker < MARKER_SLOT_END);
        if (marker == MARKER_SLOT_END)
        {
            g_charState[index].values[MARKER_VALUE_WAIT] = 0;
            g_charState[index].state = 0;
            g_charState[index].values[MARKER_VALUE_TALK] = MARKER_NO_MARKER_TALK;
            for (int word = MARKER_FIRST_CLEARED_WORD; word < MARKER_LAST_CLEARED_WORD; word++)
                g_charState[index].words[word] = 0;
            g_charState[index].behaviour = MARKER_NO_MARKER_BEHAVIOUR;
            return 0;
        }
        g_charState[index].state = 1;
    }
    if (g_charState[index].values[MARKER_VALUE_SLOT] > 0)
    {
        g_charRecords[index].defeated = 1;
        g_characterModels[index].shadowHidden = 1;
        g_characterModels[index].m_17e8 = 1;
        if (g_charState[index].values[MARKER_VALUE_PHASE] == MARKER_PHASE_NEW)
        {
            g_characterModels[index].root->SetColourA(0, -1);
            g_charState[index].values[MARKER_VALUE_PHASE] = MARKER_PHASE_RIDING;
        }
        if (g_characterModels[g_charState[index].values[MARKER_VALUE_SLOT]].matrix.m[MATRIX_TRANSLATION_X] < MARKER_GONE_Z)
        {
            g_charState[index].values[MARKER_VALUE_SLOT] = -1;
            g_charState[index].state = MARKER_PHASE_VANISHING;
            g_charPoseState[index] = MARKER_VANISH_DISPLAY_STATE;
            if (g_guardedValue8 >= 0)
            {
                switch (g_guardedValue8)
                {
                case 0:
                case 1:
                    g_charState[index].values[MARKER_VALUE_VANISH_FRAMES] = MARKER_VANISH_EASY;
                    break;
                case 2:
                    g_charState[index].values[MARKER_VALUE_VANISH_FRAMES] = MARKER_VANISH_HARD;
                    break;
                default:
                    g_charState[index].values[MARKER_VALUE_VANISH_FRAMES] = 0;
                }
            }
            else
                g_charState[index].values[MARKER_VALUE_VANISH_FRAMES] = 0;
            if (g_charState[index].values[MARKER_VALUE_PHASE] == MARKER_PHASE_RIDING)
            {
                g_characterModels[index].root->SetColourC(index, MARKER_KIND_MASK);
                g_characterModels[index].root->SetColourA(0xfe, MARKER_KIND_MASK);
                g_characterModels[index].root->SetColourB(0x2f, MARKER_KIND_MASK);
                g_characterModels[index].root->SetColourC(0, MARKER_VANISH_KIND_MASK);
                g_characterModels[index].root->SetColourA(0xff, MARKER_VANISH_KIND_MASK);
                g_characterModels[index].root->SetColourB(0x2f, MARKER_VANISH_KIND_MASK);
                g_charState[index].values[MARKER_VALUE_PHASE] = MARKER_PHASE_VANISHING;
                g_characterModels[index].shadowHidden = 0;
            }
            g_characterModels[index].m_17e8 = 0;
            g_charRecords[index].defeated = 0;
        }
        return 1;
    }
    if (g_charState[index].values[MARKER_VALUE_SLOT] == 0)
    {
        g_characterModels[index].m_17e8 = 0;
        g_charRecords[index].defeated = 0;
        if (g_charState[index].values[MARKER_VALUE_PHASE] == MARKER_PHASE_RIDING)
        {
            g_characterModels[index].root->SetColourC(index, MARKER_KIND_MASK);
            g_characterModels[index].root->SetColourA(0xfe, MARKER_KIND_MASK);
            g_characterModels[index].root->SetColourB(0x2f, MARKER_KIND_MASK);
            g_characterModels[index].root->SetColourC(0, MARKER_VANISH_KIND_MASK);
            g_characterModels[index].root->SetColourA(0xff, MARKER_VANISH_KIND_MASK);
            g_characterModels[index].root->SetColourB(0x2f, MARKER_VANISH_KIND_MASK);
            g_charState[index].values[MARKER_VALUE_PHASE] = MARKER_PHASE_VANISHING;
            g_characterModels[index].shadowHidden = 0;
        }
        Char_ResetState(index);
    }
    if (g_charState[index].state == MARKER_PHASE_VANISHING)
    {
        if (g_charState[index].values[MARKER_VALUE_VANISH_FRAMES] > 0)
        {
            g_charState[index].values[MARKER_VALUE_VANISH_FRAMES]--;
            g_characterModels[index].PlayMotion(MARKER_MOTION_LAYER, 0, MARKER_MOTION_FRAMES, 1.0f);
            g_characterModels[index].heading =
                (float)atan2(g_characterModels[0].matrix.GetTranslation().x - g_characterModels[index].matrix.GetTranslation().x,
                      g_characterModels[0].matrix.GetTranslation().y - g_characterModels[index].matrix.GetTranslation().y) +
                MARKER_TURN_HALF;
            if (g_charState[index].values[MARKER_VALUE_VANISH_FRAMES] % 2 == 0)
            {
                Vec4 offset;
                Matrix4 matrix;
                matrix.SetIdentity();
                offset.Fill(0.0f);
                matrix = g_characterModels[index].matrix;
                g_effectManager.Start(MARKER_PUFF_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, &g_charEffectHandle[index]);
            }
            return 1;
        }
        Char_ResetState(index);
    }
    return 0;
}
