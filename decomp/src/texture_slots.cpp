#include "d3d9_min.h"

extern "C" IUnknownLike *DAT_10f1a870[0x5a0];
extern "C" IUnknownLike *DAT_10f60d38[0x5a0];
extern "C" IUnknownLike *DAT_10f0d5e0[0x5a0];
extern "C" IUnknownLike *DAT_10f1c7f0[0x5a0];
extern "C" int DAT_10f1e670[0x5a0];
extern "C" int DAT_10f628e8[0x5a0];
extern "C" int DAT_10f1a86c;

extern "C" int FUN_101477c0(int slot)
{
    if (slot < 0)
        return 0;
    if (slot >= 0x5a0)
        return 0;
    if (DAT_10f1a870[slot] != 0)
        DAT_10f1a870[slot]->Release();
    if (DAT_10f60d38[slot] != 0)
        DAT_10f60d38[slot]->Release();
    if (DAT_10f0d5e0[slot] != 0)
    {
        DAT_10f0d5e0[slot]->Release();
        DAT_10f1a86c -= DAT_10f1e670[slot] + DAT_10f628e8[slot];
    }
    if (DAT_10f1c7f0[slot] != 0)
        DAT_10f1c7f0[slot]->Release();
    DAT_10f0d5e0[slot] = 0;
    DAT_10f1a870[slot] = 0;
    DAT_10f60d38[slot] = 0;
    DAT_10f1c7f0[slot] = 0;
    return 1;
}
