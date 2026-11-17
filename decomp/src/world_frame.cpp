struct WorldRenderer
{
    char pad_00000[0xe6e2c];
    int enabled;
    int frame;
    int lastFrame;
    int M_1017cc90(int flags);
    int AdvanceFrame();
};

extern "C" int DAT_1101de08;
extern "C" int DAT_1101de0c;
extern "C" int DAT_1101de10;
extern "C" int DAT_1101de14;

int WorldRenderer::AdvanceFrame()
{
    if (enabled != 0)
    {
        if (frame != lastFrame)
            M_1017cc90(0);
        lastFrame = frame;
        DAT_1101de08 = frame;
        DAT_1101de0c = frame;
        DAT_1101de10 = frame;
        DAT_1101de14 = frame;
        frame++;
    }
    return 0;
}
