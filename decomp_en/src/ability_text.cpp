// Ability names shown in the costume description (EN build). The original copies each name with an inlined strcpy.
#include <string.h>

#include "ability_text.h"

// Copies the name of the highest-priority ability set in `flags` to `out` and returns that ability's bits, or returns 0
// (leaving `out` alone) when none is set. Water, gas and trap protection come first, then the map and recovery
// abilities, then the critical, element, money, shop and damage multipliers.
extern "C" unsigned int Ability_GetDescription(unsigned int flags, char *out)
{
    if ((flags & 0x3) == 0x3)
    {
        strcpy(out, "Water Damage Restores HP");
        return 0x3;
    }
    if ((flags & 0x2) != 0)
    {
        strcpy(out, "Immune to Water Damage");
        return 0x2;
    }
    if ((flags & 0x1) != 0)
    {
        strcpy(out, "Water Damage Halved");
        return 0x1;
    }
    if ((flags & 0x3000) == 0x3000)
    {
        strcpy(out, "Gas Restores HP");
        return 0x3000;
    }
    if ((flags & 0x2000) != 0)
    {
        strcpy(out, "Immune to Gas Damage");
        return 0x2000;
    }
    if ((flags & 0x1000) != 0)
    {
        strcpy(out, "Gas Damage Halved");
        return 0x1000;
    }
    if ((flags & 0x30000) == 0x30000)
    {
        strcpy(out, "Traps Restore HP");
        return 0x30000;
    }
    if ((flags & 0x20000) != 0)
    {
        strcpy(out, "Immune to Trap Damage");
        return 0x20000;
    }
    if ((flags & 0x10000) != 0)
    {
        strcpy(out, "Trap Damage Halved");
        return 0x10000;
    }
    if ((flags & 0x8000000) != 0)
    {
        strcpy(out, "Immune to Falling Damage");
        return 0x8000000;
    }
    if ((flags & 0x4000000) != 0)
    {
        strcpy(out, "Falling Damage Halved");
        return 0x4000000;
    }
    if ((flags & 0x8) != 0)
    {
        strcpy(out, "Display Jars \x81\x95 Rocks on Mini-Map");
        return 0x8;
    }
    if ((flags & 0x4) != 0)
    {
        strcpy(out, "Display Chests on Mini-Map");
        return 0x4;
    }
    if ((flags & 0x800) != 0)
    {
        strcpy(out, "All Attacks Restore HP");
        return 0x800;
    }
    if ((flags & 0x80) == 0x80)
    {
        strcpy(out, "Critical Attacks Restore HP");
        return 0x80;
    }
    if ((flags & 0x8000) != 0)
    {
        strcpy(out, "Auto HP Restoration");
        return 0x8000;
    }
    if ((flags & 0x4000) != 0)
    {
        strcpy(out, "Rest regenerates HP");
        return 0x4000;
    }
    if ((flags & 0x80000) != 0)
    {
        strcpy(out, "Auto DP Restoration");
        return 0x80000;
    }
    if ((flags & 0x40000) != 0)
    {
        strcpy(out, "Rest regenerates DP");
        return 0x40000;
    }
    if ((flags & 0x800000) != 0)
    {
        strcpy(out, "Critical Amount x3");
        return 0x800000;
    }
    if ((flags & 0x400000) != 0)
    {
        strcpy(out, "Critical Amount x2");
        return 0x400000;
    }
    if ((flags & 0x3000000) == 0x3000000)
    {
        strcpy(out, "Elemental Damage x3");
        return 0x3000000;
    }
    if ((flags & 0x2000000) != 0)
    {
        strcpy(out, "Elemental Damage x2");
        return 0x2000000;
    }
    if ((flags & 0x1000000) != 0)
    {
        strcpy(out, "Elemental Damage x1.5");
        return 0x1000000;
    }
    if ((flags & 0x30000000) == 0x30000000)
    {
        strcpy(out, "Received Money x3");
        return 0x30000000;
    }
    if ((flags & 0x20000000) != 0)
    {
        strcpy(out, "Received Money x2");
        return 0x20000000;
    }
    if ((flags & 0x10000000) != 0)
    {
        strcpy(out, "Received Money x1.5");
        return 0x10000000;
    }
    if ((flags & 0x300000) == 0x300000)
    {
        strcpy(out, "50% Shop Discount");
        return 0x300000;
    }
    if ((flags & 0x200000) != 0)
    {
        strcpy(out, "10% Shop Discount");
        return 0x200000;
    }
    if ((flags & 0x100000) != 0)
    {
        strcpy(out, "5% Shop Discount");
        return 0x100000;
    }
    if ((flags & 0x40) != 0)
    {
        strcpy(out, "Damage Reduced by 75\x81\x93");
        return 0x40;
    }
    if ((flags & 0x30) == 0x30)
    {
        strcpy(out, "Damage Reduced by 65\x81\x93");
        return 0x30;
    }
    if ((flags & 0x20) != 0)
    {
        strcpy(out, "Damage Reduced by 50\x81\x93");
        return 0x20;
    }
    if ((flags & 0x10) != 0)
    {
        strcpy(out, "Damage Reduced by 25\x81\x93");
        return 0x10;
    }
    if ((flags & 0x400) != 0)
    {
        strcpy(out, "Attack Damage x2");
        return 0x400;
    }
    if ((flags & 0x300) == 0x300)
    {
        strcpy(out, "Attack Damage x1.7");
        return 0x300;
    }
    if ((flags & 0x200) != 0)
    {
        strcpy(out, "Attack Damage x1.5");
        return 0x200;
    }
    if ((flags & 0x100) != 0)
    {
        strcpy(out, "Attack Damage x1.2");
        return 0x100;
    }
    return 0;
}
