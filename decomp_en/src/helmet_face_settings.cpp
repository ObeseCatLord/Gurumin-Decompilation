// Face settings (ability set) of the helmets (EN build; JP twin decomp 0x10014d30 has no matched source).
#include "ability_text.h"
#include "char_attach.h"
#include "game.h"
#include "item_info.h"

// Script flag that holds the level of an equipment item is its item-table sortKey plus this base.
const int ITEM_LEVEL_FLAG_BASE = 0xc85;
// Levels from this one up use the last (fully upgraded) ability set.
const int HELMET_MAX_LEVEL = 4;

// Item numbers of the helmets that carry abilities (the ones with a name in char_attach.h are costumes).
const int ITEM_HELMET_ABILITY_29 = 0x29;
const int ITEM_HELMET_ABILITY_2C = 0x2c;
const int ITEM_HELMET_ABILITY_2B = 0x2b;
const int ITEM_HELMET_ABILITY_2D = 0x2d;
const int ITEM_HELMET_ABILITY_15 = 0x15;
const int ITEM_HELMET_ABILITY_3E = 0x3e;
const int ITEM_HELMET_ABILITY_2F = 0x2f;
const int ITEM_HELMET_ABILITY_2E = 0x2e;
const int ITEM_HELMET_ABILITY_30 = 0x30;

// Helmet_FaceSettings: the ability set (bit flags, see Ability_BuildNameList) that helmet `item` gives at its current
// level. The level is a script flag (0 = not upgraded); each of the 15 helmets with abilities has one set for level 0,
// 1, 2, 3 and 4 or more. Other items give 0. Called when a helmet is put on and when an item description is built.
extern "C" int Helmet_FaceSettings(int item)
{
    int settings = 0;
    int level = g_game.TestScriptFlag(g_itemTable[item].sortKey + ITEM_LEVEL_FLAG_BASE);
    if (item == ITEM_HELMET_ABILITY_29)
    {
        if (level <= 0)
            settings = 0x1;
        if (level == 1)
            settings = 0x2;
        else if (level == 2)
            settings = 0x12;
        else if (level == 3)
            settings = 0x112;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x4223;
    }
    else if (item == ITEM_HELMET_NO_FACE)
    {
        if (level <= 0)
            settings = 0x1000;
        if (level == 1)
            settings = 0x2000;
        else if (level == 2)
            settings = 0x2010;
        else if (level == 3)
            settings = 0x2110;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x7220;
    }
    else if (item == ITEM_HELMET_ABILITY_2C)
    {
        if (level <= 0)
            settings = 0x100;
        if (level == 1)
            settings = 0x200;
        else if (level == 2)
            settings = 0x210;
        else if (level == 3)
            settings = 0x310;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x422;
    }
    else if (item == ITEM_HELMET_ABILITY_2B)
    {
        if (level <= 0)
            settings = 0x10;
        if (level == 1)
            settings = 0x20;
        else if (level == 2)
            settings = 0x120;
        else if (level == 3)
            settings = 0x130;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x2240;
    }
    else if (item == ITEM_HELMET_COSTUME_5)
    {
        if (level <= 0)
            settings = 0x4;
        if (level == 1)
            settings = 0xc;
        else if (level == 2)
            settings = 0x1c;
        else if (level == 3)
            settings = 0x11c;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x22e;
    }
    else if (item == ITEM_HELMET_ABILITY_2D)
    {
        if (level <= 0)
            settings = 0x10000;
        if (level == 1)
            settings = 0x10010;
        else if (level == 2)
            settings = 0x20010;
        else if (level == 3)
            settings = 0x20020;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x30222;
    }
    else if (item == ITEM_HELMET_ABILITY_15)
    {
        if (level <= 0)
            settings = 0x80;
        if (level == 1)
            settings = 0x90;
        else if (level == 2)
            settings = 0x190;
        else if (level == 3)
            settings = 0x1a0;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x4a20;
    }
    else if (item == ITEM_HELMET_ABILITY_3E)
    {
        if (level <= 0)
            settings = 0x100000;
        if (level == 1)
            settings = 0x200000;
        else if (level == 2)
            settings = 0x200010;
        else if (level == 3)
            settings = 0x200020;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x300220;
    }
    else if (item == ITEM_HELMET_COSTUME_4)
    {
        if (level <= 0)
            settings = 0x4000;
        if (level == 1)
            settings = 0x4010;
        else if (level == 2)
            settings = 0x4020;
        else if (level == 3)
            settings = 0x4120;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x8222;
    }
    else if (item == ITEM_HELMET_COSTUME_3)
    {
        if (level <= 0)
            settings = 0x11001;
        if (level == 1)
            settings = 0x12002;
        else if (level == 2)
            settings = 0x1200e;
        else if (level == 3)
            settings = 0x2201e;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x2611e;
    }
    else if (item == ITEM_HELMET_COSTUME_2)
    {
        if (level <= 0)
            settings = 0x400000;
        if (level == 1)
            settings = 0x400010;
        else if (level == 2)
            settings = 0x400112;
        else if (level == 3)
            settings = 0x402122;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x822222;
    }
    else if (item == ITEM_HELMET_COSTUME_1)
    {
        if (level <= 0)
            settings = 0x10000000;
        if (level == 1)
            settings = 0x10000010;
        else if (level == 2)
            settings = 0x10000020;
        else if (level == 3)
            settings = 0x20000120;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x30002222;
    }
    else if (item == ITEM_HELMET_ABILITY_2F)
    {
        if (level <= 0)
            settings = 0x1000000;
        if (level == 1)
            settings = 0x1000010;
        else if (level == 2)
            settings = 0x1000110;
        else if (level == 3)
            settings = 0x2000110;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x3000220;
    }
    else if (item == ITEM_HELMET_ABILITY_2E)
    {
        if (level <= 0)
            settings = 0x4000000;
        if (level == 1)
            settings = 0x4000010;
        else if (level == 2)
            settings = 0x4000022;
        else if (level == 3)
            settings = 0x4002122;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x8022232;
    }
    else if (item == ITEM_HELMET_ABILITY_30)
    {
        if (level <= 0)
            settings = 0x40000;
        if (level == 1)
            settings = 0x40010;
        else if (level == 2)
            settings = 0x40110;
        else if (level == 3)
            settings = 0x40120;
        else if (level >= HELMET_MAX_LEVEL)
            settings = 0x82222;
    }
    return settings;
}
