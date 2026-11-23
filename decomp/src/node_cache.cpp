// Loading models into scene nodes through a cache: each model file is read once into one of 128 cached nodes and
// copied from there into the nodes that use it.

#include <string.h>

#include "node.h"

extern "C" Node *DAT_11b002d0[0x80];          // the cached models
#include "char_tables.h"

extern "C" void *(*DAT_11b0b938)(int size, const char *tag);   // the engine's allocator
extern "C" int FUN_1013f880(void);            // the game's random number

// 0x101ac010 (Node::Load). Releases this node and gives it model `path`: from the cache when that file is cached
// (returns 1), else after loading it into a free cache slot or, with the cache full, into a random slot whose model
// is dropped (returns the slot). With `noCopy` the model is only cached, not copied into this node.
int Node::Load(const char *path, int noCopy, int a, int b)
{
    Release();
    for (int i = 0; i < 0x80; i++)
    {
        if (DAT_11b002d0[i] != 0 && _stricmp(path, DAT_11b004d0[i]) == 0)
        {
            if (noCopy == 0)
                Clone(DAT_11b002d0[i], a, b);
            return 1;
        }
    }
    int slot;
    for (slot = 0; slot < 0x80; slot++)
    {
        if (DAT_11b002d0[slot] == 0)
        {
            DAT_11b002d0[slot] = (Node *)DAT_11b0b938(sizeof(Node), "ccnode");
            DAT_11b002d0[slot]->Init();
            DAT_11b002d0[slot]->LoadIt3(path, a, b);
            strcpy(DAT_11b004d0[slot], path);
            if (noCopy == 0)
                Clone(DAT_11b002d0[slot], a, b);
            return slot;
        }
    }
    slot = FUN_1013f880() % 0x80;
    DAT_11b002d0[slot]->Release();
    DAT_11b002d0[slot] = (Node *)DAT_11b0b938(sizeof(Node), "ccnode");
    DAT_11b002d0[slot]->Init();
    DAT_11b002d0[slot]->LoadIt3(path, a, b);
    strcpy(DAT_11b004d0[slot], path);
    if (noCopy == 0)
        Clone(DAT_11b002d0[slot], a, b);
    return slot;
}
