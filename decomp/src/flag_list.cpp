// Registry of ids kept inside a large game object (offsets 0x3eda08.. of the object passed as `this`).
// `marked` is a list of 0x100 ids; `pairs` records ids together with whether they were marked when registered.

struct FlagPair
{
    int id;
    int flag;
};

struct FlagOwner
{
    char pad_0000[0x3eda08];
    int marked[0x100];
    FlagPair pairs[0x100];
    int M_100020d0(int id);
};

// Registers `id` once: returns 1 if it is already in `pairs` or was added to the first free pair (with flag = 1
// when `id` is also in `marked`), 0 when the pair table is full.
int FlagOwner::M_100020d0(int id)
{
    int flag = 0;
    int i;
    for (i = 0; i < 0x100; i++)
    {
        if (marked[i] == id)
            flag = 1;
    }
    for (i = 0; i < 0x100; i++)
    {
        if (pairs[i].id == id)
            return 1;
    }
    for (i = 0; i < 0x100; i++)
    {
        if (pairs[i].id == 0)
        {
            pairs[i].id = id;
            pairs[i].flag = flag;
            return 1;
        }
    }
    return 0;
}
