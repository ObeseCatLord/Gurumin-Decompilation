// Scene node pool and node helpers.

#include "node.h"

#include "char_tables.h"

extern "C" void (*DAT_11b1ba40)(void *);

// Releases and frees every node in the 0x80-entry node pool DAT_11b002d0.
extern "C" int FUN_101a5860(void)
{
    for (int i = 0; i < 0x80; i++)
    {
        if (DAT_11b002d0[i] != 0)
        {
            DAT_11b002d0[i]->Release();
            DAT_11b1ba40(DAT_11b002d0[i]);
            DAT_11b002d0[i] = 0;
        }
    }
    return 0;
}

// Collects the subtree's nodes in state 0x4000 (up to 8, M_10191920) and links them to its state-0x1000 nodes
// (M_10191990) when there are any.
int Node::M_1019eca0()
{
    int count = 0;
    Node *list[8];
    M_10191920(list, &count);
    if (count != 0)
        M_10191990(list, count);
    return 1;
}
