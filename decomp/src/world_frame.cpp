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
    char pad_00000[0x8868];
    WorldNode nodes[0x20];         // 0x8868: map model nodes
    char pad_39c68[0x15e00];
    char loaded;                   // 0x4fa68: 1 while a map is loaded
    char pad_4fa69[3];
    WorldCallback callback;        // 0x4fa6c: host notification callback
    char pad_4fa70[0x973bc];
    int enabled;                   // 0xe6e2c: frame updates on
    int frame;                     // 0xe6e30: current world frame
    int lastFrame;                 // 0xe6e34: frame the draw lists were last built for
    char pad_e6e38[0x58fa4];
    int textures[0x20];            // 0x13fddc: texture slots used by the map (-1 = none)
    char pad_13fe5c[0x126c];
    int m_1410c8;                  // 0x1410c8
    int notifyPending;             // 0x1410cc: a 0x5e0 notification must be sent on release
    int M_1017cc90(int flags);
    int AdvanceFrame();
    int Release();
    int Seek(int target);
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
