// The zone roamer (zone_roamer.cpp, 0x10049ec0): a party character (slots 0xd, 0xf and 0x3a) that wanders between the
// zones of the field maps 2, 3, 4, 5 and 0xc by itself, until the player talks to it.
#ifndef GURUMIN_ZONE_ROAMER_H
#define GURUMIN_ZONE_ROAMER_H

// One zone of the field map (DAT_10216dc0, indexed through DAT_101d6310): where its centre is, what kind of place it
// is and how far from the centre a character counts as inside it.
struct ZonePoint
{
    int x;
    int y;
    int kind;                 // 1 or 3 = a place to rest at (the roamer may stay and look around), else it passes through
    int radius;
};

extern "C" ZonePoint DAT_10216dc0[];           // one record per zone place
extern "C" const char DAT_101d6310[];          // zone number -> record of DAT_10216dc0
extern "C" const char DAT_101d6330[];          // zone number -> the map (2, 3, 4, 5 or 0xc) the zone lies in (0 = none)
extern "C" const short DAT_101d6350[][8];      // zone number -> the (up to) eight zones next to it (0 = none)
extern "C" int DAT_10c635c4;                   // non-zero = the roamers stand still (a scene is running)

// 0x10049ec0 (Npc_RoamZones; C++ linkage). Per-frame behaviour of the roaming party character `index`. Returns 0.
int Npc_RoamZones(int index);

#endif
