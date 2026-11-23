// Character model loading and motion setup.

#include "world.h"
#include "anim_model.h"
#include "char_state.h"
#include "char_slot.h"

extern "C" const char *FUN_1013f8a0(const char *format, ...);

// Loads motion file `path` (folder 0) on character `index` and plays frames start..end; `once` = 0 loops.
extern "C" int FUN_10016500(int index, const char *path, int start, int end, int once)
{
    DAT_10db6ee8[index].LoadModel(FUN_1013f8a0(path, 0));
    DAT_10db6ee8[index].M_10161b60();
    DAT_10db6ee8[index].M_10161b90(0, start);
    DAT_10db6ee8[index].M_10161b90(end - start, end);
    DAT_10db6ee8[index].M_10161be0(1, once == 0);
    return 0;
}

// Model path table at 0x10b314c8: one 0x4b34-byte record per model id, starting with the file name.
struct ModelPath
{
    char path[0x4b34];
};

extern "C" ModelPath DAT_10b314c8[];
extern "C" int FUN_10012220(int index, int value);

// Reloads the model of every character slot 0..0x5b that is still in the world (slot 0, the player, always)
// and whose slot is not waiting on the previous slot, then resets its motion with FUN_10012220(i, -1).
extern "C" void FUN_100123d0(void)
{
    for (int i = 0; i < 0x5c; i++)
    {
        if (i != 0 && DAT_10db6ee8[i].matrix._41 < -50000.0f)
            continue;
        if (DAT_10daaaec[i].m_009c != 0 && DAT_10daaaec[i - 1].m_00a4 == 0)
            continue;
        DAT_10db6ee8[i].LoadModel(FUN_1013f8a0(DAT_10b314c8[DAT_10daaaec[i].model].path, 0));
        FUN_10012220(i, -1);
    }
}
