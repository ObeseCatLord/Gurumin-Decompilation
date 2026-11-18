// The face settings of the helmets: each helmet item has an upgrade level kept in a script flag (the item's flag
// plus 3205), and the level picks a bit mask for the face meshes (FUN_100154c0 stores it in DAT_104c02c8).

// Item table entry (inventory.cpp ItemInfo).
struct ItemInfo
{
    int type;
    int levelFlag;            // 0x04: script flag holding the item's level (minus 3205)
    char pad_0008[0xb8];
};

extern "C" ItemInfo DAT_1088e6b0[];
extern "C" int DAT_10c62edc[0x1000];   // script flags

// Script flag `index` (as Game::TestScriptFlag does it): a negative index tests that the flag is clear, flag 0 is
// always set and indexes past 3999 read as clear (or as set for the negated test).
inline int ScriptFlag(int index)
{
    if (index < 0)
    {
        index = -index;
        if (index > 3999)
            return 1;
        return DAT_10c62edc[index] == 0;
    }
    if (index > 3999)
        return 0;
    if (index == 0)
        return 1;
    return DAT_10c62edc[index];
}

// 0x10014d30 (Helmet_FaceSettings). The face mask of helmet `item` at its current level (0 when it is not a helmet
// with face settings): one value for level 0 or less, one each for levels 1, 2 and 3 and one for higher levels.
extern "C" int FUN_10014d30(int item)
{
    char unused[8];   // never used; the original's /GS cookie shows it declared a character buffer
    int result = 0;
    int level = ScriptFlag(DAT_1088e6b0[item].levelFlag + 3205);
    if (item == 0x29)
    {
        if (level <= 0)
            result = 1;
        if (level == 1)
            return 2;
        if (level == 2)
            return 0x12;
        if (level == 3)
            return 0x112;
        if (level >= 4)
            return 0x4223;
    }
    else if (item == 0x2a)
    {
        if (level <= 0)
            result = 0x1000;
        if (level == 1)
            return 0x2000;
        if (level == 2)
            return 0x2010;
        if (level == 3)
            return 0x2110;
        if (level >= 4)
            return 0x7220;
    }
    else if (item == 0x2c)
    {
        if (level <= 0)
            result = 0x100;
        if (level == 1)
            return 0x200;
        if (level == 2)
            return 0x210;
        if (level == 3)
            return 0x310;
        if (level >= 4)
            return 0x422;
    }
    else if (item == 0x2b)
    {
        if (level <= 0)
            result = 0x10;
        if (level == 1)
            return 0x20;
        if (level == 2)
            return 0x120;
        if (level == 3)
            return 0x130;
        if (level >= 4)
            return 0x2240;
    }
    else if (item == 0x28)
    {
        if (level <= 0)
            result = 4;
        if (level == 1)
            return 0xc;
        if (level == 2)
            return 0x1c;
        if (level == 3)
            return 0x11c;
        if (level >= 4)
            return 0x22e;
    }
    else if (item == 0x2d)
    {
        if (level <= 0)
            result = 0x10000;
        if (level == 1)
            return 0x10010;
        if (level == 2)
            return 0x20010;
        if (level == 3)
            return 0x20020;
        if (level >= 4)
            return 0x30222;
    }
    else if (item == 0x15)
    {
        if (level <= 0)
            result = 0x80;
        if (level == 1)
            return 0x90;
        if (level == 2)
            return 0x190;
        if (level == 3)
            return 0x1a0;
        if (level >= 4)
            return 0x4a20;
    }
    else if (item == 0x3e)
    {
        if (level <= 0)
            result = 0x100000;
        if (level == 1)
            return 0x200000;
        if (level == 2)
            return 0x200010;
        if (level == 3)
            return 0x200020;
        if (level >= 4)
            return 0x300220;
    }
    else if (item == 0x25)
    {
        if (level <= 0)
            result = 0x4000;
        if (level == 1)
            return 0x4010;
        if (level == 2)
            return 0x4020;
        if (level == 3)
            return 0x4120;
        if (level >= 4)
            return 0x8222;
    }
    else if (item == 0x3d)
    {
        if (level <= 0)
            result = 0x11001;
        if (level == 1)
            return 0x12002;
        if (level == 2)
            return 0x1200e;
        if (level == 3)
            return 0x2201e;
        if (level >= 4)
            return 0x2611e;
    }
    else if (item == 0x39)
    {
        if (level <= 0)
            result = 0x400000;
        if (level == 1)
            return 0x400010;
        if (level == 2)
            return 0x400112;
        if (level == 3)
            return 0x402122;
        if (level >= 4)
            return 0x822222;
    }
    else if (item == 0x38)
    {
        if (level <= 0)
            result = 0x10000000;
        if (level == 1)
            return 0x10000010;
        if (level == 2)
            return 0x10000020;
        if (level == 3)
            return 0x20000120;
        if (level >= 4)
            return 0x30002222;
    }
    else if (item == 0x2f)
    {
        if (level <= 0)
            result = 0x1000000;
        if (level == 1)
            return 0x1000010;
        if (level == 2)
            return 0x1000110;
        if (level == 3)
            return 0x2000110;
        if (level >= 4)
            return 0x3000220;
    }
    else if (item == 0x2e)
    {
        if (level <= 0)
            result = 0x4000000;
        if (level == 1)
            return 0x4000010;
        if (level == 2)
            return 0x4000022;
        if (level == 3)
            return 0x4002122;
        if (level >= 4)
            return 0x8022232;
    }
    else if (item == 0x30)
    {
        if (level <= 0)
            result = 0x40000;
        if (level == 1)
            return 0x40010;
        if (level == 2)
            return 0x40110;
        if (level == 3)
            return 0x40120;
        if (level >= 4)
            result = 0x82222;
    }
    return result;
}
