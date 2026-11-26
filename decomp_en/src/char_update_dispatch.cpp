// Choosing between the ordinary and the boss character update (EN build, EN-only: no JP twin found).
#include "char_dispatch.h"

// Runs the update of character `index`: kinds from CHAR_KIND_BS001 on (the bosses) go to Char_DispatchBossKind, every
// other kind to Char_UpdateByKind. Returns what the chosen update returns (0). Called by the character update loop.
extern "C" int Char_DispatchByKind(int index)
{
    if (g_charRecords[index].kind >= CHAR_KIND_BS001)
        return Char_DispatchBossKind(index);
    return Char_UpdateByKind(index);
}
