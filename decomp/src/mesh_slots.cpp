// Attached mesh slots (see ai_pick.cpp for the slot table).

struct Matrix
{
    float m[16];
    Matrix();
    ~Matrix();
    void SetIdentity();
};

struct MeshSlot
{
    int pad_0000;
    void *mesh;
    char pad_0008[0xb00];
    void M_1000baf0(const void *matrix);
};

extern "C" MeshSlot DAT_10edbe08[0x40];
extern "C" char DAT_108392d8[];

// Updates every used mesh slot with the world matrix DAT_108392d8. The identity matrix is unused.
int ResetMeshSlots()
{
    Matrix identity;
    identity.SetIdentity();
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10edbe08[i].mesh != 0)
            DAT_10edbe08[i].M_1000baf0(DAT_108392d8);
    }
    return 0;
}

