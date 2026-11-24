// SlotTable of the EN build (JP: decomp/include/engine_10143.h): the materials of a model's mesh slots.
// This is the one declaration of SlotTable in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_SLOT_TABLE_H
#define GURUMIN_EN_SLOT_TABLE_H

struct SlotTable
{
    char m_0000[0x10];        // 0x00: not yet named
    int *originalMaterial;    // 0x10: material each entry was loaded with
    int *material;            // 0x14: material each entry currently uses
    char m_0018[0x3c - 0x18]; // 0x18: not yet named
    int first;                // 0x3c: first entry later operations touch
    int last;                 // 0x40: one past the last entry they touch
    int count;                // 0x44: number of entries
    int m_0048;               // 0x48: not yet named
    int bufferSlot;           // 0x4c: shared model file this mesh was loaded from (negative = none)

    int SetRange(int newFirst, int newLast);            // EN 0x77bf40 (JP 0x10146d90)
    int SetMaterial(int index, int value);              // EN 0x77cfa0 (JP 0x101476e0)
    int ReplaceMaterial(int key, int value);            // EN 0x77cf50 (JP 0x10147690)
};

// Value of the `key` argument of ReplaceMaterial that matches every entry.
const int MATERIAL_ANY = -1;

#endif
