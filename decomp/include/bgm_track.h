// BgmTrack: declared here once (first used by src/bgm_play.cpp).
#ifndef GURUMIN_BGM_TRACK_H
#define GURUMIN_BGM_TRACK_H

// A music track (0x30 bytes per entry).
struct BgmTrack
{
    char name[0x10];              // file name in bgm\ without ".de2"
    char mode;                    // 0x10: 'o' = played once (no loop)
    char pad_0011[0xf];
    int a;                        // 0x20: passed to soundarc (loop parameters)
    int b;                        // 0x24
    int start;                    // 0x28: start position (1000 = the beginning)
    int m_002c;                   // 0x2c: divisor of the beat length (the frame step: beat = (position - start) * 15 / (0x285ff0 / m_002c))
};

// 0x1003aef0 (Bgm_Play), bgm_play.cpp; C++ linkage. The callers pass a second argument that it does not read.
int Bgm_Play(int index, int unused);

#endif
