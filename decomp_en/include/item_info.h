// The item table of the EN build (JP: decomp/include/item_info.h): one 0xc0-byte record per item kind.
#ifndef GURUMIN_EN_ITEM_INFO_H
#define GURUMIN_EN_ITEM_INFO_H

// Bytes of an item model path, and of an ItemInfo record.
const int ITEM_MODEL_PATH_SIZE = 0x240;
const int ITEM_INFO_SIZE = 0xc0;

// ItemInfo::type value of equipment with a level and abilities (a helmet).
const int ITEM_TYPE_EQUIPMENT = 1;

// One item kind (only the members EN code has used are named).
struct ItemInfo
{
    int type;                                  // 0x00: 1 = equipment with a level and abilities (a helmet), 4 = stacks to 99
    int sortKey;                               // 0x04: the key the item list of the status screen is sorted by (ascending)
    char m_0008[0x18 - 8];                     // 0x08: not yet named
    int icon;                                  // 0x18: cell of the item icon sheet (8 icons per row)
    char m_001c[ITEM_INFO_SIZE - 0x1c];        // 0x1c: not yet named
};

// Inventory slots held by the player (JP DAT_1042e2a8).
const int INVENTORY_SLOT_COUNT = 0x40;
extern "C" int g_inventory[INVENTORY_SLOT_COUNT];           // EN 0x00a36468 (JP DAT_1042e2a8): item number held in each inventory slot

// Draws the icon of the item in inventory slot `slot` as a 16x16 sprite at (x, y).
extern "C" int Item_DrawIcon(int x, int y, int slot, float alpha);          // EN 0x5e28b0 (JP 0x1000c200)

extern "C" ItemInfo g_itemTable[];                          // EN 0x00f5e458 (JP DAT_1088e6b0): the item table, by item number

// qsort comparator over item numbers: orders them by ItemInfo::sortKey, ascending (1 when `a` sorts after or equal to `b`,
// else -1). EN 0x5c9480.
extern "C" int ItemIndex_CompareBySortKey(const int *a, const int *b);
extern "C" char g_itemModelPaths[][ITEM_MODEL_PATH_SIZE];   // EN 0x00fc6534 (JP DAT_108f678c): model path of each item

#endif
