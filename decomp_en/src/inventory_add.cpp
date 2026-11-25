// Putting items into the inventory (EN build).
#include "game_vars.h"
#include "item_info.h"

// Item number that means "no item".
const int ITEM_NONE = 0x10;
// Item type that stacks to ITEM_STACK_MAX; every other item stops at ITEM_SINGLE_MAX.
const int ITEM_TYPE_STACKING = 4;
const int ITEM_STACK_MAX = 99;
const int ITEM_SINGLE_MAX = 3;

// Adds *amount of item *index to the stock. Items that do not stack stop at ITEM_SINGLE_MAX, stacking items at
// ITEM_STACK_MAX; the part that does not fit stays in *amount and 0 is returned. Returns 1 and clears *amount and
// *index when everything fit. Marks the item as changed and refreshes the anti-tamper copies. Called through the script
// command table. EN 0x5ec460 (JP 0x10012130).
extern "C" int Inventory_Add(int *index, int *amount)
{
    int item = *index;
    if (item != ITEM_NONE && g_itemTable[item].type != ITEM_TYPE_STACKING &&
        *amount + g_itemStock[item] > ITEM_SINGLE_MAX)
    {
        *amount = *amount + g_itemStock[item] - ITEM_SINGLE_MAX;
        g_itemStock[*index] = ITEM_SINGLE_MAX;
        g_itemChanged[*index] = 1;
        AntiTamper_StoreArrays();
        return 0;
    }
    if (*amount + g_itemStock[item] > ITEM_STACK_MAX)
    {
        *amount = *amount + g_itemStock[item] - ITEM_STACK_MAX;
        g_itemStock[*index] = ITEM_STACK_MAX;
        g_itemChanged[*index] = 1;
        AntiTamper_StoreArrays();
        return 0;
    }
    g_itemChanged[item] = 1;
    g_itemStock[*index] += (char)*amount;
    *amount = 0;
    *index = 0;
    AntiTamper_StoreArrays();
    return 1;
}
