// Starting a character slot over when it is placed in the scene (called by the scene slot loop FUN_100e7280).

#include <string.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "last_position.h"
#include "char_tables.h"
#include "map_vars.h"
#include "sound_vars.h"
#include "camera_vars.h"
#include "world.h"

extern "C" int FUN_1013f880(void);   // the game random number

// Slot range of the enemies and the slot kind (DAT_1021f3c0) used here.
#define ENEMY_SLOT_FIRST 0x3a
#define SLOT_KIND_ENEMY 3
// Values of DAT_10254090 (state of the placement).
#define PLACE_STATE_ACTIVE 1
#define PLACE_STATE_DONE 2
// A model below this height has fallen out of the world.
#define WORLD_FLOOR -20000.0f
#define MODEL_PARKED_X -200000.0f
#define MODEL_SWAPPED_X -100000.0f
#define PLACE_RESULT_FALLEN -10000
#define PLACE_RESULT_SAFE -1
#define PLACE_RESULT_SWAP_OFFSET 10000
// Values of CharSlot::routeRecord: above 500 it carries a flag (500 is taken off), 101..500 and below 0 select a place.
#define ROUTE_FLAGGED 500
#define ROUTE_PLAIN_LIMIT 100
#define ROUTE_FLAG_CHANCE 100
#define ROUTE_FLAG_RANGE 1000
#define ROUTE_DIGITS 100
#define SHADOW_OPACITY_PLACED 15
#define KIND_CLEARS_BUILD_TABLE 0x19
#define BUILD_TABLE_WORDS 0x32
#define MARKER_SPREAD 5
#define TRAIL_CLEAR_PASSES 0x22
#define BUILD_CLEAR_PASSES 0x32
#define ATTACHED_MATRIX_COUNT 6
#define EXTRA_VECTOR_COUNT 6
#define SOUND_SLOT_COUNT 10

// 0x1001dc20 (Char_InitSlot). Starts character `index` when its placement is active (DAT_10254090 = 1) and it is an
// enemy (slot kind 3). A few kinds first turn the model mode to 0. The first enemy slot (0x3a) also clears the
// scene node pointers, the build table, the extra vectors, the attached matrices and the sound queue. The motion
// state, the behaviour state, the position records and the per-character tables are cleared, and the model position
// is stored as the entry point. A route record above 500 loses 500 and may draw a new route number from the random
// value; one above 100 only picks its route number; a negative one parks the model far away and, one time in five,
// takes over the place of an earlier enemy that holds a model and waits for the same route (swapping their
// per-character values). Afterwards a model below the world floor ends the placement with a result of -10000, else
// -1. Returns 0.
extern "C" int FUN_1001dc20(int index)
{
    if (DAT_10254090[index] == PLACE_STATE_ACTIVE)
    {
        if (DAT_1021f3c0[index] == SLOT_KIND_ENEMY)
        {
            int kind = DAT_10daaae8[index].kind;
            if (kind == 9 || kind == 0xb || kind == 0x28 || kind == 0x2a || kind == 0x21 || kind == 4 || kind == 0xe ||
                kind == 0x54 || kind == 0x48)
                DAT_10db6ee8[index].mode = 0;
            if (index == ENEMY_SLOT_FIRST)
            {
                for (int i = 0; i < TRAIL_CLEAR_PASSES; i++)
                {
                    DAT_1021c620 = 0;
                    DAT_1021c624 = 0;
                    DAT_1021c628 = 0;
                    DAT_1021c62c = 0;
                    DAT_1021c630 = 0;
                }
                for (int i = 0; i < BUILD_CLEAR_PASSES; i++)
                    DAT_10254598[index] = -1;
                for (int i = 0; i < EXTRA_VECTOR_COUNT; i++)
                    DAT_10edbd58[i].Fill(0);
                for (int i = 0; i < ATTACHED_MATRIX_COUNT; i++)
                {
                    DAT_10f08880[i].SetIdentity();
                    DAT_102108d0[i] = 0;
                }
                memset(DAT_10447310, 0, SOUND_SLOT_COUNT * sizeof(int));
                memset(DAT_1021a398, 0, SOUND_SLOT_COUNT * sizeof(int));
            }
            DAT_10254598[index] = 0x5b;
            DAT_1030b8f8[index] = 0;
            DAT_10db6ee8[index].m_02a8 = 0;
            DAT_10db6ee8[index].holdState = HOLD_FREE;
            DAT_10db6ee8[index].holdWeight = 0;
            DAT_10db6ee8[index].m_022c.Fill(0);
            DAT_1020b430[index].words[0] = 0;
            DAT_1020b430[index].words[1] = 0;
            DAT_1020b430[index].words[2] = 0;
            DAT_1020b430[index].words[3] = 0;
            DAT_1020b430[index].words[4] = 0;
            DAT_1020b430[index].words[5] = 0;
            DAT_1020b430[index].words[6] = 0;
            DAT_1020b430[index].words[7] = 0;
            DAT_1020b430[index].words[8] = 0;
            DAT_1020b430[index].words[9] = 0;
            DAT_1020b430[index].words[10] = 0;
            DAT_1020b430[index].words[11] = 0;
            DAT_1020b430[index].words[12] = 0;
            DAT_1020b430[index].words[13] = 0;
            DAT_1020b430[index].words[14] = 0;
            DAT_1020b430[index].words[15] = 0;
            DAT_10802bf8[index].position.Fill(0);
            DAT_10802bf8[index].step.Fill(0);
            DAT_103086b0[index] = 1;
            DAT_1020d540[index] = 0;
            DAT_1021b4c8[index] = (char)(index % MARKER_SPREAD) - 1;
            DAT_10867e18[index].Fill(0);
            DAT_10255f10[index] = DAT_10daaae8[index].stats[2];
            int slotKind = DAT_10daaae8[index].kind;
            DAT_10256420[index] = 0;
            DAT_10db6ee8[index].shadowOpacity = SHADOW_OPACITY_PLACED;
            DAT_1021f940[index] = 0;
            DAT_1020e138[index] = -1;
            DAT_10db6ee8[index].m_0294 = 0;
            DAT_10253a58[index] = 0;
            DAT_1042dba8[index] = 0;
            DAT_102199b8[index] = 0;
            DAT_1042dea8[index] = 0;
            if (slotKind == KIND_CLEARS_BUILD_TABLE)
                memset(DAT_10254598, 0, BUILD_TABLE_WORDS * sizeof(int));
            DAT_104c4438[index].Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            int route = DAT_10daaae8[index].routeRecord;
            DAT_1030b8f8[index] = 0;
            DAT_104483e8[index] = 0;
            DAT_10453008[index] = 0;
            if (route > ROUTE_FLAGGED)
            {
                DAT_10daaae8[index].routeRecord = route - ROUTE_FLAGGED;
                if (FUN_1013f880() % ROUTE_FLAG_RANGE < ROUTE_FLAG_CHANCE)
                {
                    DAT_10daaae8[index].m_00c0 = 0;
                    DAT_10daaae8[index].m_00c4 = 0;
                    DAT_10daaae8[index].m_00c8 = 0;
                    DAT_10daaae8[index].money = DAT_10daaae8[index].routeRecord % ROUTE_DIGITS;
                }
            }
            else if (route > ROUTE_PLAIN_LIMIT)
            {
                DAT_10daaae8[index].m_00c0 = 0;
                DAT_10daaae8[index].m_00c4 = 0;
                DAT_10daaae8[index].m_00c8 = 0;
                DAT_10daaae8[index].money = route % ROUTE_DIGITS;
            }
            else if (route < 0)
            {
                DAT_10210d30[index] = 2;
                DAT_10254090[index] = 0;
                DAT_104c4438[index].Copy(DAT_10db6ee8[index].matrix.GetTranslation());
                DAT_10db6ee8[index].SetPosition(MODEL_PARKED_X, 0, 0);
                int i;
                if (FUN_1013f880() % 5 == 0 && (i = ENEMY_SLOT_FIRST) < index)
                {
                    for (; i < index; i++)
                    {
                        if (DAT_10db6ee8[i].m_00b4[11].z == 0)  // 0x16c: the original tests this word as an int (open.tsv)
                            break;
                        if (DAT_10daaae8[i].m_0000 == -DAT_10daaae8[index].routeRecord && !(DAT_104c4438[i].x < WORLD_FLOOR) &&
                            FUN_1013f880() % 2 == 0)
                        {
                            DAT_10254090[index] = DAT_10254090[i];
                            DAT_10453008[i] = index + PLACE_RESULT_SWAP_OFFSET;
                            DAT_104c4438[index].Copy(DAT_104c4438[i]);
                            DAT_10db6ee8[index].SetPosition(DAT_10db6ee8[i].matrix.translation.x,
                                                            DAT_10db6ee8[i].matrix.translation.y,
                                                            DAT_10db6ee8[i].matrix.translation.z);
                            DAT_10db6ee8[i].SetPosition(MODEL_SWAPPED_X, DAT_10db6ee8[i].matrix.translation.y,
                                                        DAT_10db6ee8[i].matrix.translation.z);
                            DAT_1021b4c8[index] = DAT_1021b4c8[i];
                            DAT_10210d30[index] = 0;
                            DAT_10210d30[i] = 2;
                            DAT_1020d540[index] = DAT_1020d540[i];
                            DAT_10867e18[index].Copy(DAT_10867e18[i]);
                            DAT_10254390[index] = DAT_10254390[i];
                            DAT_10256420[index] = DAT_10256420[i];
                            break;
                        }
                    }
                }
            }
        }
        if (DAT_10254090[index] == PLACE_STATE_ACTIVE && DAT_1021f3c0[index] == SLOT_KIND_ENEMY)
        {
            if (DAT_10db6ee8[index].matrix.translation.x < WORLD_FLOOR)
            {
                DAT_10254090[index] = PLACE_STATE_DONE;
                DAT_10453008[index] = PLACE_RESULT_FALLEN;
                DAT_10254090[index] = PLACE_STATE_DONE;
                return 0;
            }
            else
            {
                DAT_10453008[index] = PLACE_RESULT_SAFE;
                DAT_10254090[index] = PLACE_STATE_DONE;
            }
        }
    }
    return 0;
}
