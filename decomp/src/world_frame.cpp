// The world renderer object (huge; only the frame counter fields at 0xe6e2c are named so far).
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

// Once per frame while enabled: rebuilds the world draw lists (M_1017cc90) when the frame changed since the last
// call, publishes the frame number to four globals read by the renderers and increments it.
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
