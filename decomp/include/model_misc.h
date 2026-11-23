// Collider, ModelObject, ViewLimits, SlotList, SlotLists, CharacterInfo, CharacterBank: declared here once (first used by src/model_misc.cpp).
#ifndef GURUMIN_MODEL_MISC_H
#define GURUMIN_MODEL_MISC_H

struct Collider
{
    int M_101ab3e0(int a, int b);
};
struct ModelObject
{
    char pad_0000[0x268];
    int m_0268;
    int m_026c;
    char pad_0270[0x207c];
    Collider *collider;
    char pad_22f0[0x2c];
    int m_231c;
    char pad_2320[0x1550];
    int m_3870;

    int M_10162f20(int value);
    int M_10163e70(int value);
    int M_10164090(int a, int b);
    void M_101687c0();
};
struct ViewLimits
{
    char pad_0000[0x20];
    int m_0020;
    char pad_0024[0x54];
    float minimum[3];
    int pad_0084;
    float maximum[3];
    char pad_0094[0x24];
    int m_00b8;
    void M_10166870();
};
struct SlotList
{
    char data[0x14];
    void Clear();
};
struct SlotLists
{
    SlotList lists[5];
    int m_0064;
    int m_0068;
    int m_006c;
    void M_10169620();
};
struct CharacterInfo
{
    char pad_0000[0x265];
    char m_0265;
    char pad_0266[0x2236];
};
struct CharacterBank
{
    char pad_0000[0x8860];
    CharacterInfo *characters;
    char pad_8864[0xe23f4];
    int m_eac58[0x80];
    int m_eae58[0x80];
    int M_10166910(int slot, int character, int value);
};

#endif
