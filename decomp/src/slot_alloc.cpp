// Small allocators over fixed tables.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Copy(const Vec4 &source);
};

extern "C" unsigned int DAT_104c013c;
extern "C" char DAT_10254c40[0x400];
extern "C" Vec4 DAT_104c0438[0x400];

// Takes the next free entry of the 0x400-entry position ring (DAT_10254c40 = in use) after the last one used,
// stores `position` there and returns 1; gives up after 100 tries and returns 0.
extern "C" int FUN_1000c5c0(const Vec4 &position)
{
    unsigned int i = DAT_104c013c;
    for (int tries = 0; tries < 100; tries++)
    {
        if (++i >= 0x400)
            i = 0;
        if (DAT_10254c40[i] == 0)
        {
            DAT_104c013c = i;
            DAT_10254c40[i] = 1;
            DAT_104c0438[i].Copy(position);
            return 1;
        }
    }
    DAT_104c013c = i;
    return 0;
}

struct Record3d8
{
    int id;
    char pad_0004[0x3d4];
};

struct RecordC4
{
    int id;
    char pad_0004[0xc0];
};

extern "C" Record3d8 DAT_1101e0d8[1150];
extern "C" RecordC4 DAT_11134cc8[840];
extern "C" RecordC4 DAT_118a8888[840];

// Marks all entries of three object tables as free (id = -1): 1150 records of 0x3d8 bytes and two tables of
// 840 records of 0xc4 bytes.
extern "C" int FUN_1016fb10(void)
{
    int i;
    for (i = 0; i < 1150; i++)
        DAT_1101e0d8[i].id = -1;
    for (i = 0; i < 840; i++)
        DAT_11134cc8[i].id = -1;
    for (i = 0; i < 840; i++)
        DAT_118a8888[i].id = -1;
    return 0;
}
