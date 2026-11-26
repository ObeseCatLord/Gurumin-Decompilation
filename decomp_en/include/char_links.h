// Links between pairs of characters (EN build; JP: decomp/include/char_links.h): up to CHAR_LINK_COUNT pairs that are
// joined by beam effects every CHAR_LINK_PERIOD frames (char_links.cpp). Declared once here.
#ifndef GURUMIN_EN_CHAR_LINKS_H
#define GURUMIN_EN_CHAR_LINKS_H

const int CHAR_LINK_COUNT = 5;
const int CHAR_LINK_PERIOD = 10;               // frames between two rounds of link effects
const int CHAR_LINK_END_EFFECT = 0x778;        // started at each end of a link, turned along it
const int CHAR_LINK_BEAM_EFFECT = 0x77a;       // one beam between node matrix n of both characters (n = 0..2)
const int CHAR_LINK_BEAMS = 3;
const char CHAR_LINK_EFFECT_LIST = 4;          // EffectOptions::m_000e of the link effects
const int CHAR_LINK_EFFECT_COLOUR = 15;        // EffectOptions::colour of the link effects
const char CHAR_KIND_DESTROYED = (char)0xaf;   // g_charKind value of a destroyed character

// One link: the two character slots it joins, 0 = unused.
struct CharLink
{
    int first;    // 0x00: slot of the first character, 0 = link unused
    int second;   // 0x04: slot of the second character, 0 = not joined yet
};

extern "C" CharLink g_charLinks[CHAR_LINK_COUNT];     // EN 0x0091f5d8 (JP DAT_1049f0c8): the links
extern "C" int g_charLinkTimer[CHAR_LINK_COUNT];      // EN 0x0091f600 (JP DAT_1021b928): frames until each link's next round of effects

// Draws the beam effects between linked characters (see char_links.cpp). EN 0x5ca520 (JP 0x1001bb20). Called by the scene frame.
int Char_DrawLinks();

#endif
