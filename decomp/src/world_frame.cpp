// The world object (global instance at 0x10c68000): the current map's models, textures and draw lists.
// M_1017cc90 (29,886 bytes, the largest function in APP.dll) builds the world for a frame.

// One scene node body (0x18a0 bytes, see node_dtor.cpp).
struct WorldNode
{
    char data[0x18a0];
    void Release();  // 0x1019e9a0
};

typedef void (*WorldCallback)(int code, int value);

struct WorldRenderer
{
    char pad_00000[0x975];
    char m_0975;                   // 0x975: non-zero = no screen fade
    char pad_00976[2];
    int fadeOnTop;                 // 0x978: 1 = the fade is drawn below the cinema bars, 0 = above them
    char pad_0097c[0x3014];
    int fadeColour;                // 0x3990: colour of the full-screen fade
    char pad_03994[0xc];
    float fadeAmount;              // 0x39a0: fade strength in percent (0 = off)
    char pad_039a4[0x4ec4];
    WorldNode nodes[0x20];         // 0x8868: map model nodes
    char pad_39c68[0x15e00];
    char loaded;                   // 0x4fa68: 1 while a map is loaded
    char pad_4fa69[3];
    WorldCallback callback;        // 0x4fa6c: host notification callback
    char pad_4fa70[0x973bc];
    int enabled;                   // 0xe6e2c: frame updates on
    int frame;                     // 0xe6e30: current world frame
    int lastFrame;                 // 0xe6e34: frame the draw lists were last built for
    char pad_e6e38[0x58fa0];
    char barsFull;                 // 0x13fdd8: non-zero = show the cinema bars fully at once
    char pad_13fdd9[3];
    int textures[0x20];            // 0x13fddc: texture slots used by the map (-1 = none)
    char pad_13fe5c[0x126c];
    int m_1410c8;                  // 0x1410c8
    int notifyPending;             // 0x1410cc: a 0x5e0 notification must be sent on release
    int M_1017cc90(int flags);
    int AdvanceFrame();
    int Release();
    int Seek(int target);
    void DrawOverlay();
};

extern "C" void FUN_10143e00(int left, int top, int right, int bottom, int colour, int mode, float alpha);  // fills a rectangle

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

extern "C" int FUN_10174c70(int slot, int force);  // releases one reference to a texture slot (texture_mgr.cpp)

// Unloads the current map: sends the pending 0x5e0 notification, releases the map's texture slots and the 32
// model nodes, and marks the world as not loaded. Returns 0.
int WorldRenderer::Release()
{
    m_1410c8 = 0;
    if (notifyPending != 0)
    {
        notifyPending = 0;
        callback(0x5e0, 0);
    }
    for (int i = 0; i < 0x20; i++)
    {
        if (textures[i] != -1)
        {
            FUN_10174c70(textures[i], 0);
            textures[i] = -1;
        }
    }
    for (int j = 0; j < 0x20; j++)
    {
        nodes[j].Release();
        loaded = 0;
    }
    return 0;
}

// Jumps the world to frame `target`, rebuilding every frame on the way so animated parts end in the right state:
// forwards from the current frame when the target is ahead, otherwise from frame 0. Enables frame updates.
int WorldRenderer::Seek(int target)
{
    int f = frame;
    if (f < target)
    {
        do
        {
            frame = f;
            lastFrame = f - 1;
            M_1017cc90(1);
            f++;
        } while (f <= target);
        enabled = 1;
        return 0;
    }
    frame = target;
    for (f = 0; f <= target; f++)
    {
        lastFrame = f - 1;
        frame = f;
        M_1017cc90(1);
    }
    enabled = 1;
    return 0;
}

// 0x101692c0. Draws the screen overlays of the world: the cinema bars at the top (34 pixels) and bottom (34 pixels)
// of the 640x480 screen, fading in over the first 30 frames (at once when barsFull is set), and the full-screen
// fade in fadeColour at fadeAmount percent, before or after the bars depending on fadeOnTop.
void WorldRenderer::DrawOverlay()
{
    if (fadeOnTop == 1 && m_0975 == 0 && fadeAmount != 0.0f)
        FUN_10143e00(0, 0, 640, 480, fadeColour, 0, fadeAmount / 100.0f);
    int bars = 30;
    if (frame <= 30)
        bars = frame;
    if (barsFull != 0)
        bars = 30;
    float barAlpha = (float)bars / 30.0f;
    FUN_10143e00(0, 0, 640, 34, 0, 0, barAlpha);
    FUN_10143e00(0, 446, 640, 480, 0, 0, barAlpha);
    if (fadeOnTop == 0 && m_0975 == 0 && fadeAmount != 0.0f)
        FUN_10143e00(0, 0, 640, 480, fadeColour, 0, fadeAmount / 100.0f);
}
