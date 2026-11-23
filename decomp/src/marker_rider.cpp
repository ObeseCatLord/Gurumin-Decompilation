// A character that rides a marker (behaviour 0x10 of the dispatch 0x100dfcf0): it looks for a taken marker among the
// marker slots on its first frame and is shown only while that marker character exists. Once the marker has fallen
// out of the world (below -20000) it vanishes with a short countdown and a puff effect.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"
#include "map_vars.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"
#include "char_vars.h"
#include "world.h"

#define MARKER_FIRST_SLOT 0x5c         // the marker slots searched for the marker to ride
#define MARKER_SLOT_END 0x7c
#define MARKER_KIND_TAKEN 'Q'          // DAT_1021f3c0 value of a taken marker
#define MARKER_REACH 75.0f             // the marker must lie within this distance on every axis
#define MARKER_GONE_Z -20000.0f        // a marker below this height has fallen out of the world
#define MARKER_PHASE_NEW 0
#define MARKER_PHASE_RIDING 1
#define MARKER_PHASE_VANISHING 2
#define MARKER_NO_MARKER_BEHAVIOUR 0x11  // behaviour given to a character that found no marker
#define MARKER_NO_MARKER_TALK 0x32       // ... and its talk id
#define MARKER_VANISH_EASY 30          // vanishing frames with DAT_10c635f8 below 2
#define MARKER_VANISH_HARD 15          // ... with DAT_10c635f8 equal to 2 (none above)
#define MARKER_KIND_MASK 0x400
#define MARKER_VANISH_KIND_MASK 0x200
#define MARKER_VANISH_DISPLAY_STATE 0x1e
#define MARKER_PUFF_EFFECT 0x2da

extern "C" int FUN_10043cc0(int index);       // Char_ResetState
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// atan2 of float arguments, rounded back to float (the arguments are rounded to float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10043fc0 (Npc_RideMarker). First frame: finds the taken marker (slot MARKER_FIRST_SLOT up) within MARKER_REACH of
// the character on all three axes; with none the slot's state is cleared and it becomes behaviour 0x11. While the
// marker exists the character is shown (its shadow hidden, its model locked) and its node flags are set once; when
// the marker falls out of the world the character starts vanishing (phase 2, 30 frames, 15 with DAT_10c635f8 == 2,
// none above) with display state 0x1e. While vanishing it faces the player, plays motion 1, and every second frame
// starts effect 0x2da; at the end of the countdown its state is reset. Returns 1 while it is riding or vanishing,
// else 0.
int FUN_10043fc0(int index)
{
    if (DAT_1020b430[index].state == 0)
    {
        int marker = MARKER_FIRST_SLOT;
        do
        {
            if (DAT_1021f3c0[marker] == MARKER_KIND_TAKEN &&
                DAT_10db6ee8[index].matrix.m[12] - MARKER_REACH < DAT_10db6ee8[marker].matrix.m[12] + MARKER_REACH &&
                DAT_10db6ee8[index].matrix.m[12] + MARKER_REACH > DAT_10db6ee8[marker].matrix.m[12] - MARKER_REACH &&
                DAT_10db6ee8[index].matrix.m[13] - MARKER_REACH < DAT_10db6ee8[marker].matrix.m[13] + MARKER_REACH &&
                DAT_10db6ee8[index].matrix.m[13] + MARKER_REACH > DAT_10db6ee8[marker].matrix.m[13] - MARKER_REACH &&
                DAT_10db6ee8[index].matrix.m[14] - MARKER_REACH < DAT_10db6ee8[marker].matrix.m[14] + MARKER_REACH &&
                DAT_10db6ee8[index].matrix.m[14] + MARKER_REACH > DAT_10db6ee8[marker].matrix.m[14] - MARKER_REACH)
            {
                DAT_1020b430[index].markerSlot = marker;
                break;
            }
            marker++;
        } while (marker < MARKER_SLOT_END);
        if (marker == MARKER_SLOT_END)
        {
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].talk = MARKER_NO_MARKER_TALK;
            DAT_1020b430[index].range = 0;
            DAT_1020b430[index].action = 0;
            DAT_1020b430[index].alarmFlashes = 0;
            DAT_1020b430[index].fall = 0;
            DAT_1020b430[index].rider = 0;
            DAT_1020b430[index].pointPause = 0;
            DAT_1020b430[index].m_0024 = 0;
            DAT_1020b430[index].fadeActions = 0;
            DAT_1020b430[index].markerSlot = 0;
            DAT_1020b430[index].vanishFrames = 0;
            DAT_1020b430[index].markerPhase = 0;
            DAT_1020b430[index].m_0034 = 0;
            DAT_1020b430[index].sawPlayer = 0;
            DAT_1020b430[index].behaviour = MARKER_NO_MARKER_BEHAVIOUR;
            return 0;
        }
        DAT_1020b430[index].state = 1;
    }
    if (DAT_1020b430[index].markerSlot > 0)
    {
        DAT_10daaae8[index].m_0141 = 1;
        DAT_10db6ee8[index].shadowHidden = 1;
        DAT_10db6ee8[index].m_17e8 = 1;
        if (DAT_1020b430[index].markerPhase == MARKER_PHASE_NEW)
        {
            DAT_10db6ee8[index].root->M_101946b0(0, -1);
            DAT_1020b430[index].markerPhase = MARKER_PHASE_RIDING;
        }
        if (DAT_10db6ee8[DAT_1020b430[index].markerSlot].matrix.m[12] < MARKER_GONE_Z)
        {
            DAT_1020b430[index].markerSlot = -1;
            DAT_1020b430[index].state = MARKER_PHASE_VANISHING;
            DAT_104686b0[index] = MARKER_VANISH_DISPLAY_STATE;
            if (DAT_10c635f8 >= 0)
            {
                switch (DAT_10c635f8)
                {
                case 0:
                case 1:
                    DAT_1020b430[index].vanishFrames = MARKER_VANISH_EASY;
                    break;
                case 2:
                    DAT_1020b430[index].vanishFrames = MARKER_VANISH_HARD;
                    break;
                default:
                    DAT_1020b430[index].vanishFrames = 0;
                }
            }
            else
                DAT_1020b430[index].vanishFrames = 0;
            if (DAT_1020b430[index].markerPhase == MARKER_PHASE_RIDING)
            {
                DAT_10db6ee8[index].root->M_10194630(index, MARKER_KIND_MASK);
                DAT_10db6ee8[index].root->M_101946b0(0xfe, MARKER_KIND_MASK);
                DAT_10db6ee8[index].root->M_10194730(0x2f, MARKER_KIND_MASK);
                DAT_10db6ee8[index].root->M_10194630(0, MARKER_VANISH_KIND_MASK);
                DAT_10db6ee8[index].root->M_101946b0(0xff, MARKER_VANISH_KIND_MASK);
                DAT_10db6ee8[index].root->M_10194730(0x2f, MARKER_VANISH_KIND_MASK);
                DAT_1020b430[index].markerPhase = MARKER_PHASE_VANISHING;
                DAT_10db6ee8[index].shadowHidden = 0;
            }
            DAT_10db6ee8[index].m_17e8 = 0;
            DAT_10daaae8[index].m_0141 = 0;
        }
        return 1;
    }
    if (DAT_1020b430[index].markerSlot == 0)
    {
        DAT_10db6ee8[index].m_17e8 = 0;
        DAT_10daaae8[index].m_0141 = 0;
        if (DAT_1020b430[index].markerPhase == MARKER_PHASE_RIDING)
        {
            DAT_10db6ee8[index].root->M_10194630(index, MARKER_KIND_MASK);
            DAT_10db6ee8[index].root->M_101946b0(0xfe, MARKER_KIND_MASK);
            DAT_10db6ee8[index].root->M_10194730(0x2f, MARKER_KIND_MASK);
            DAT_10db6ee8[index].root->M_10194630(0, MARKER_VANISH_KIND_MASK);
            DAT_10db6ee8[index].root->M_101946b0(0xff, MARKER_VANISH_KIND_MASK);
            DAT_10db6ee8[index].root->M_10194730(0x2f, MARKER_VANISH_KIND_MASK);
            DAT_1020b430[index].markerPhase = MARKER_PHASE_VANISHING;
            DAT_10db6ee8[index].shadowHidden = 0;
        }
        FUN_10043cc0(index);
    }
    if (DAT_1020b430[index].state == MARKER_PHASE_VANISHING)
    {
        if (DAT_1020b430[index].vanishFrames > 0)
        {
            DAT_1020b430[index].vanishFrames--;
            DAT_10db6ee8[index].PlayMotion(1, 0, 5, 1.0f);
            DAT_10db6ee8[index].heading =
                Atan2(DAT_10db6ee8[0].matrix.GetTranslation().x - DAT_10db6ee8[index].matrix.GetTranslation().x,
                      DAT_10db6ee8[0].matrix.GetTranslation().y - DAT_10db6ee8[index].matrix.GetTranslation().y) +
                3.1415f;
            if (DAT_1020b430[index].vanishFrames % 2 == 0)
            {
                Vec4 offset;
                Matrix4 matrix;
                matrix.SetIdentity();
                offset.Fill(0.0f);
                matrix = DAT_10db6ee8[index].matrix;
                FUN_1001d8b0(index, MARKER_PUFF_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0);
            }
            return 1;
        }
        FUN_10043cc0(index);
    }
    return 0;
}
