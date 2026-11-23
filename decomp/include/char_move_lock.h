// CharMoveLock: declared here once (first used by src/npc_approach.cpp).
#ifndef GURUMIN_CHAR_MOVE_LOCK_H
#define GURUMIN_CHAR_MOVE_LOCK_H

// Field 0x24a8 of each character record (it lies past the 0x249c bytes world.h gives a record, so it is declared
// on its own with the record stride).
struct CharMoveLock
{
    int locked;                   // 0 = the character may be moved by the game
    char pad_0004[0x2498];
};

#endif
