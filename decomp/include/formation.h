// Formation: declared here once (first used by src/group_tables.cpp).
#ifndef GURUMIN_FORMATION_H
#define GURUMIN_FORMATION_H

// Six 24-byte formation records; only the first field (the active flag) is reset here.
struct Formation
{
    int active;
    char pad_0004[0x14];
};

#endif
