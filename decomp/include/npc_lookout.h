// Globals of the lookout NPC behaviour (npc_lookout.cpp).
#ifndef GURUMIN_NPC_LOOKOUT_H
#define GURUMIN_NPC_LOOKOUT_H

extern "C" int DAT_10c63390;                             // non-zero = a lookout NPC (npc_lookout.cpp) is already reacting
extern "C" int DAT_10c63394;                             // the kind of event the lookout reacted to (a map area, 0x2a)
extern "C" int DAT_10210e1c;                             // 2 = the state the lookout reacts in
extern "C" int DAT_10467798;                             // a counter the lookout compares with its best (npc_lookout.cpp)
extern "C" int DAT_10db0544;                             // the amount that came with it (npc_lookout.cpp)

#endif
