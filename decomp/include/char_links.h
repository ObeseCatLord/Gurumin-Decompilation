// Links between pairs of characters: up to CHAR_LINK_COUNT pairs (DAT_1049f0c8) that are joined by beam effects
// every CHAR_LINK_PERIOD frames (char_links.cpp). 0x100fb7b0 fills the table in; 0x100460d0 reads it.
#ifndef GURUMIN_CHAR_LINKS_H
#define GURUMIN_CHAR_LINKS_H

#define CHAR_LINK_COUNT 5
#define CHAR_LINK_PERIOD 10              // frames between two rounds of link effects
#define CHAR_LINK_END_EFFECT 0x778       // started at each end of a link, turned along it
#define CHAR_LINK_BEAM_EFFECT 0x77a      // one beam between ring point n of both characters (n = 0..2)
#define CHAR_LINK_BEAMS 3
#define CHAR_LINK_EFFECT_LIST 4          // EffectOptions::m_000e of the link effects
#define CHAR_LINK_EFFECT_VALUE 15        // EffectOptions::m_0004 of the link effects
#define SLOT_KIND_DESTROYED ((char)0xaf) // DAT_1021f3c0 value of a destroyed character (enemy_destroy.cpp)

// One link: the two character slots it joins, 0 = unused.
struct CharLink
{
    int first;
    int second;
};

extern "C" CharLink DAT_1049f0c8[CHAR_LINK_COUNT];   // the links
extern "C" int DAT_1021b928[CHAR_LINK_COUNT];        // frames until each link's next round of effects

int Char_DrawLinks();                                // 0x1001bb20 (char_links.cpp)
extern "C" int FUN_100fb7b0(void);                // 0x100fb7b0 (Scene_Reset, scene_reset.cpp): fills the table in

#endif
