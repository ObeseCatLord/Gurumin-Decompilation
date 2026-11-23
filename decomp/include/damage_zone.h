// DamageZone, SavedPosition: declared here once (first used by src/damage_zone.cpp).
#ifndef GURUMIN_DAMAGE_ZONE_H
#define GURUMIN_DAMAGE_ZONE_H

// A zone (0x100 bytes per entry): an integer box and how it hurts.
struct DamageZone
{
    int left;                     // 0x00: x range
    int right;                    // 0x04
    int pad_0008[2];
    int front;                    // 0x10: y range
    int back;                     // 0x14
    int pad_0018[2];
    int top;                      // 0x20: z range (top first)
    int bottom;                   // 0x24
    int pad_0028[2];
    int damage;                   // 0x30
    int step;                     // 0x34: enemies checked per frame are every step-th one
    int onFlag;                   // 0x38: script flag that must be set (0 = always on)
    int offFlag;                  // 0x3c: script flag that turns the zone off (0 = none)
    char pad_0040[0xc0];
};
// The saved position of each character slot (16 bytes per slot).
struct SavedPosition
{
    float x;
    float y;
    float z;
    float w;
};

#endif
