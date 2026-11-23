// SmallRecord, EmptyObject: declared here once (first used by src/game_reset.cpp).
#ifndef GURUMIN_GAME_RESET_H
#define GURUMIN_GAME_RESET_H

// A 0xfc-byte record of which four fields are cleared here.
struct SmallRecord
{
    int m_0000;
    int m_0004;
    char pad_0008[0xec];
    int m_00f4;
    int m_00f8;
};
struct EmptyObject
{
    void M_1013edb0();                           // 0x1013edb0 (an empty method)
    void M_1013ef00();                           // 0x1013ef00
};

#endif
