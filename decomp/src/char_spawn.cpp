// Spawning an enemy: finding a free enemy slot and starting it over with a fresh state.

#include "vec4.h"
#include "node.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"
#include "last_position.h"
#include "char_tables.h"
#include "world.h"
#include "game_vars.h"
#include "script_vars.h"
#include "char_vars.h"

// Slot range of the enemies and the slot kind (DAT_1021f3c0) that marks one.
#define ENEMY_SLOT_FIRST 0x3a
#define ENEMY_SLOT_END 0x5c
#define SLOT_KIND_ENEMY 3
#define ENEMY_GONE_HEIGHT -50000.0f
// Pending motion nodes of a character: the kind and the colour masks its model nodes are tagged with.
#define NODE_MASK_BODY 0x400
#define NODE_MASK_SHADOW 0x200
#define NODE_STATE_FE 0xfe
#define NODE_STATE_2F 0x2f
#define NODE_STATE_FF 0xff
#define EVENT_SLOT_COUNT 0x60
#define DEFAULT_HEALTH 5
#define KIND_PACKED_DIVISOR 10000
// Base kinds that change how the packed value is read.
#define BASE_KIND_PACKED_GROUP 0x31
#define BASE_KIND_CLEAR_FLAGS 0x10
#define NODE_FLAGS_HIDE 0x20

extern "C" int FUN_10011d40(float a, float b, float c);
#include "char_equip_models.h"

// 0x10050510 (Char_SpawnEnemy). Starts an enemy at (a, b, c). Without `kind` and `route` it takes the first free
// enemy slot (FUN_10011d40). Otherwise it searches the enemy slots for a live enemy that is out of the world or
// finished (below -50000, or no health left in state 2) with that kind and route record (0 matches any), tags its
// model nodes, restores its health and places the model; -1 when there is none. Then it clears the slot's state
// tables, the motion and event slots of the character and the character's model flags, and runs FUN_100404e0 with the
// parts of its packed value (stats[9]). Returns the slot.
extern "C" int FUN_10050510(float a, float b, float c, int kind, int route)
{
    int slot;
    if (kind == 0 && route == 0)
    {
        slot = FUN_10011d40(a, b, c);
    }
    else
    {
        slot = ENEMY_SLOT_FIRST;
        do
        {
            if (DAT_1021f3c0[slot] == SLOT_KIND_ENEMY &&
                (DAT_10db6ee8[slot].matrix._41 < ENEMY_GONE_HEIGHT ||
                 (DAT_10daaae8[slot].stats[STAT_HEALTH] <= 0 && DAT_10210d30[slot] == 2)) &&
                DAT_10db6ee8[slot].root != 0)
            {
                if (kind != 0)
                {
                    if (route == 0)
                    {
                        if (DAT_10daaae8[slot].kind == kind)
                            break;
                    }
                    else if (DAT_10daaae8[slot].kind == kind && DAT_10daaae8[slot].routeRecord == route)
                        break;
                }
                else if (route != 0 && DAT_10daaae8[slot].routeRecord == route)
                    break;
            }
            slot++;
        } while (slot < ENEMY_SLOT_END);
        if (slot == ENEMY_SLOT_END)
            return -1;
        DAT_10db6ee8[slot].root->M_10194630(slot, NODE_MASK_BODY);
        DAT_10db6ee8[slot].root->M_101946b0(NODE_STATE_FE, NODE_MASK_BODY);
        DAT_10db6ee8[slot].root->M_10194730(NODE_STATE_2F, NODE_MASK_BODY);
        DAT_10db6ee8[slot].root->M_10194630(0, NODE_MASK_SHADOW);
        DAT_10db6ee8[slot].root->M_101946b0(NODE_STATE_FF, NODE_MASK_SHADOW);
        DAT_10db6ee8[slot].root->M_10194730(NODE_STATE_2F, NODE_MASK_SHADOW);
        int maxHealth = DAT_10daaae8[slot].stats[STAT_MAX_HEALTH];
        DAT_10210d30[slot] = 0;
        DAT_10db6ee8[slot].alpha = 1.0f;
        DAT_10daaae8[slot].stats[STAT_HEALTH] = maxHealth;
        if (maxHealth < 1)
            DAT_10daaae8[slot].stats[STAT_HEALTH] = DEFAULT_HEALTH;
        DAT_10db6ee8[slot].SetPosition(a, b, c);
    }
    DAT_10453008[slot] = 0;
    DAT_10254090[slot] = 1;
    DAT_10254598[slot] = 0x5b;
    DAT_1030b8f8[slot] = 0;
    DAT_1020b430[slot].words[0] = 0;
    DAT_1020b430[slot].words[1] = 0;
    DAT_1020b430[slot].words[2] = 0;
    DAT_1020b430[slot].words[3] = 0;
    DAT_1020b430[slot].words[4] = 0;
    DAT_1020b430[slot].words[5] = 0;
    DAT_1021b4c8[slot] = (char)(slot % 5) - 1;
    DAT_1020b430[slot].words[6] = 0;
    DAT_1020b430[slot].words[7] = 0;
    DAT_1020b430[slot].words[8] = 0;
    DAT_1020b430[slot].words[9] = 0;
    DAT_1020b430[slot].words[10] = 0;
    DAT_1020b430[slot].words[11] = 0;
    DAT_1020b430[slot].words[12] = 0;
    DAT_1020b430[slot].words[13] = 0;
    DAT_1020b430[slot].words[14] = 0;
    DAT_1020b430[slot].words[15] = 0;
    DAT_103086b0[slot] = 1;
    DAT_1020d540[slot] = 0;
    DAT_10256420[slot] = 0;
    DAT_10db6ee8[slot].shadowOpacity = 1;
    DAT_10254390[slot] = 0;
    DAT_1021f940[slot] = 0;
    DAT_1020e138[slot] = -1;
    DAT_10db6ee8[slot].m_0294 = 0;
    DAT_10253a58[slot] = 0;
    DAT_104c4438[slot].Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
    for (int i = 0; i < EVENT_SLOT_COUNT; i++)
    {
        if (DAT_10447fd8[i] == slot)
        {
            DAT_1020d660[i] = 0;
            DAT_104463f8[i] = 0;
            DAT_10468230[i] = 0;
            DAT_10447fd8[i] = 0;
            DAT_1021f640[i] = 0;
        }
    }
    DAT_1030c480[slot] = 0;
    DAT_10467510[slot] = 0;
    DAT_10db6ee8[slot].m_02a8 = 0;
    DAT_10db6ee8[slot].holdState = HOLD_FREE;
    DAT_10db6ee8[slot].holdWeight = 0.0f;
    DAT_10db6ee8[slot].m_022c.Fill(0.0f);
    DAT_10467ba0[slot] = 0;
    DAT_102239d0[slot] = 0;
    DAT_10210d30[slot] = 0;
    DAT_104686b0[slot] = 0;
    DAT_1020d4c0[slot] = 0;
    DAT_102550e0[slot] = 0;
    DAT_10254090[slot] = 1;
    DAT_10218fc0[slot] = 0;
    DAT_10210b30[slot] = 0;
    DAT_10802bf8[slot].position.Fill(0.0f);
    DAT_10802bf8[slot].step.Fill(0.0f);
    int packed = DAT_10daaae8[slot].stats[9];
    DAT_10315350[slot] = 0;
    DAT_1021f6c0[slot] = 0;
    DAT_102548c0[slot] = 0;
    DAT_10219e40[slot] = 0;
    DAT_1042d9e8[slot] = 0;
    if (packed / KIND_PACKED_DIVISOR == 0 &&
        (((kind = DAT_10daaae8[slot].kind) != BASE_KIND_PACKED_GROUP && DAT_1030b668[slot] != BASE_KIND_PACKED_GROUP) ||
         packed % 100 == 0))
    {
        if ((kind == BASE_KIND_CLEAR_FLAGS || DAT_1030b668[slot] == BASE_KIND_CLEAR_FLAGS))
        {
            Node *node = DAT_10db6ee8[slot].root->Find("@EN_016ar.lwo", 0);
            if (node != 0)
            {
                node->ClearFlags(NODE_FLAGS_HIDE);
                return slot;
            }
        }
    }
    else
    {
        if (DAT_10daaae8[slot].kind == BASE_KIND_CLEAR_FLAGS || DAT_1030b668[slot] == BASE_KIND_CLEAR_FLAGS)
        {
            Node *node = DAT_10db6ee8[slot].root->Find("@EN_016ar.lwo", 0);
            if (node != 0)
                node->SetFlags(NODE_FLAGS_HIDE);
        }
        int low = 0;
        if (DAT_10daaae8[slot].kind == BASE_KIND_PACKED_GROUP || DAT_1030b668[slot] == BASE_KIND_PACKED_GROUP)
            low = DAT_10daaae8[slot].stats[9] % 100;
        int value = DAT_10daaae8[slot].stats[9];
        FUN_100404e0(slot, value / 10000000 % 100, value / 100000 % 100, value / 1000 % 100, low, 0, 0);
    }
    return slot;
}
