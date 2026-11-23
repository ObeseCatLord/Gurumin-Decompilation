// Removing an enemy from play.

#include "world.h"
#include "anim_model.h"
#include "char_state.h"

// 16-byte per-character marker records at 0x10867e18 (only the y value is used here).
struct Marker
{
    float y;
    char pad_0004[0xc];
};

extern "C" char DAT_1020d540[0x80];
extern "C" int DAT_10c635a8;
extern "C" char DAT_1049efb0[10][10];
extern "C" Marker DAT_10867e18[0x80];

// Retires character `index` once: moves it out of the world (matrix._41 = -100000 - matrix._41), records its two-digit
// formation number (m_0004[1]: tens = row, ones = column) in the 10x10 grid DAT_1049efb0, marks the state as
// retired (m_0000 = 1) and hides its marker (y = -200000). Not byte-identical yet (see open.tsv).
extern "C" int FUN_1001b7b0(int index)
{
    if (DAT_1020b430[index].state == 0)
    {
        DAT_10db6ee8[index].matrix._41 = -100000.0f - DAT_10db6ee8[index].matrix._41 * 1.0f;
        int value = DAT_1020b430[index].values[1];
        DAT_1020d540[index] = 0;
        DAT_1020b430[index].values[3] = value % 10;
        DAT_1020b430[index].state = 1;
        DAT_10c635a8 = 1;
        DAT_1049efb0[value / 10 % 10][value % 10] = (char)index;
        DAT_10867e18[index].y = -200000.0f;
    }
    return 0;
}
