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
    char m_0004[ITEM_INFO_SIZE - 4];           // 0x04: not yet named
};

extern "C" ItemInfo g_itemTable[];                          // EN 0x00f5e458 (JP DAT_1088e6b0): the item table, by item number
extern "C" char g_itemModelPaths[][ITEM_MODEL_PATH_SIZE];   // EN 0x00fc6534 (JP DAT_108f678c): model path of each item

#endif
