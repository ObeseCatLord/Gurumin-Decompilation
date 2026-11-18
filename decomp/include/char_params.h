#pragma once

// Per-character parameter record, 0x188 bytes: the table at 0x10daaae8 (one record per character; world.h's CharSlot
// views the same table from 0x10daab84) and its template at 0x10db4408. The implicit copy assignment of this struct is
// 0x100357a0: it is emitted only in a file that copies a record, so it matches once one of its callers
// (FUN_1005def0, FUN_1003e710, Game_LoadData 0x100cb0c0) is decompiled with this header (see open.tsv).

struct Vec4;

struct ParamVector
{
    float x;
    float y;
    float z;
    float w;
    ParamVector &operator=(const ParamVector &other);  // 0x101acc20 (Vec4::Copy)
};

struct CharParams
{
    int m_0000;
    int m_0004[16];
    int m_0044[16];
    // 0x84..0xf0: 27 single fields (copied one by one, so not an array)
    int m_0084, m_0088, m_008c, m_0090, m_0094, m_0098, m_009c, m_00a0, m_00a4, m_00a8, m_00ac, m_00b0, m_00b4,
        m_00b8, m_00bc, m_00c0, m_00c4, m_00c8, m_00cc, m_00d0, m_00d4, m_00d8, m_00dc, m_00e0, m_00e4, m_00e8,
        m_00ec;
    int m_00f0[16];
    ParamVector m_0130;
    char m_0140;
    char m_0141;              // 0x141: CharSlot::m_00a5
    char m_0142;
    char m_0143;
    char m_0144;
    char m_0145[0x20];
    char m_0165[0x20];
};

extern "C" CharParams DAT_10daaae8[0x80];
extern "C" CharParams DAT_10db4408;   // template record
