// "shaga" effect slots: models built from EEQR.it3 by FUN_100425f0 (node "shaga_000"), one per slot of
// DAT_10446e00. Loading takes long, so the frame timer is reset before and after (no frame skip afterwards).

struct Node
{
    int M_1019b300(const char *motion, int value);  // restarts the two colour plugs (scene_node.cpp)
};

// Engine object at 0x10836c70; M_1013eee0 is FrameTimer_ResetNext (engine_1013e.cpp).
struct Engine
{
    int M_1013eee0();
};

extern "C" Engine DAT_10836c70;
extern "C" Node *DAT_10446e00[];
extern "C" int DAT_1042db08[];
extern "C" int DAT_1021d018[];
extern "C" int DAT_10254798[];
extern "C" int DAT_104bf8a4;
extern "C" int FUN_100425f0(void *a, int b, int c, float d, float e);

// Creates a shaga effect (FUN_100425f0 with the same arguments), clears the slot's two targets (-1) and its
// counter, and restarts the model's colour plugs with "stop". Returns 0.
extern "C" int FUN_10062750(void *a, int b, int c, float d, float e)
{
    int slot = FUN_100425f0(a, b, c, d, e);
    DAT_10836c70.M_1013eee0();
    if (slot > -1)
    {
        DAT_1042db08[slot] = -1;
        DAT_1021d018[slot] = -1;
        DAT_10254798[slot] = 0;
        DAT_10446e00[slot]->M_1019b300("stop", DAT_104bf8a4);
    }
    DAT_10836c70.M_1013eee0();
    return 0;
}
