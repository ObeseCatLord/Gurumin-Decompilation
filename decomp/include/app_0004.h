// Callee, Latch: declared here once (first used by src/app_0004.cpp). Latch is the head of a GimmickSlot.
#ifndef GURUMIN_APP_0004_H
#define GURUMIN_APP_0004_H

struct Callee
{
    void Method();
};
struct Latch
{
    int frame;                // 0x00: animation frame; 0 = rest, then 1 while running
    int targetFrame;          // 0x04: frame it runs to
    int id;                   // 0x08: script variable / lever or chest number (-1 = invalid)
    int m_000c;
    int flag;                 // 0x10: script flag set while the gimmick is on (0 = none)
    int type;                 // 0x14: 10 = chest, 0xb and 0xf = levers
    int M_1000b9c0();
};

#endif
