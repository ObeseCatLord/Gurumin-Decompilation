// Scene node pool and node helpers.

struct Node
{
    void Release();
    int M_10191920(void *info, int *count);
    int M_10191990(void *info, int count);
    int M_1019eca0();
};

extern "C" Node *DAT_11b002d0[0x80];
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

// Queries the node's 32-byte info block (M_10191920) and applies it again when it reports a count.
int Node::M_1019eca0()
{
    int count = 0;
    int info[8];
    M_10191920(info, &count);
    if (count != 0)
        M_10191990(info, count);
    return 1;
}
