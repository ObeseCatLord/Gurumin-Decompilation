// The world renderer (the map scene object DAT_10c68000, see world_frame.cpp): its constructor and Bind, which resets
// it for a new scene. Members are named where their role is known, the others by offset; the gaps are padding.

#include <stdio.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                          // 0x101ac990: zero
};

// Matrix4 with the engine's constructor and empty destructor (node_bones.cpp).
struct Matrix4
{
    float m[16];
    Matrix4();                                       // 0x101ae770
    ~Matrix4();                                      // 0x1013eda0 (empty)
};

// A fixed key-frame bank (vec4.cpp KeyBank), 0xa84 bytes.
struct KeyBank
{
    char data[0xa84];
    KeyBank();                                       // 0x101af420
};

// A scene node (scene_node.cpp), 0x18a0 bytes.
struct WorldNode
{
    char data[0x18a0];
    WorldNode();                                     // 0x1019ebc0
    ~WorldNode();                                    // 0x10191480
};

typedef int (*SceneCallback)(int message, int value, void *data);

struct WorldRenderer
{
    Matrix4 m_00000;
    int m_00040;
    char characterNames[0x20][0x20];                 // 0x44: "キャラ%d" until a character is loaded
    char pad_00444[0x530];
    char m_00974;                                    // 0x974: set for costume 6
    char m_00975;                                    // 0x975: non-zero = no screen fade
    char m_00976;
    char pad_00977[0x1];
    int fadeOnTop;                                   // 0x978: 1 = the fade is drawn below the cinema bars
    int m_0097c;
    int m_00980[1000];
    int m_01920[1000];
    int m_028c0[1000];
    int m_03860;
    int m_03864;
    int m_03868;
    void *data;                                      // 0x386c: passed to the callback
    int m_03870;
    char pad_03874[0x118];
    char m_0398c;
    char pad_0398d[0xf];
    int m_0399c;
    float fadeAmount;                                // 0x39a0: fade strength in percent (0 = off)
    char pad_039a4[0x4];
    int m_039a8;
    char m_039ac[0x20][0x10][0x10];
    char pad_059ac[0x200];
    char m_05bac[0x20];
    char pad_05bcc[0xe0];
    int m_05cac[0x20];
    char pad_05d2c[0x80];
    int m_05dac[0x20];
    char pad_05e2c[0x4];
    Vec4 m_05e30;
    KeyBank m_05e40[4];
    char pad_08850[0x10];
    void *models;                                    // 0x8860: the map's model records
    char pad_08864[0x4];
    WorldNode nodes[0x20];                           // 0x8868: map model nodes
    float m_39c68[0x20];
    float m_39ce8[0x20];
    float m_39d68[0x20];
    float m_39de8[0x20];
    float m_39e68[0x20];
    float m_39ee8[0x20];
    float m_39f68[0x20];
    float m_39fe8[0x20];
    float m_3a068[0x20];
    float m_3a0e8[0x20];
    float m_3a168[0x20];
    float m_3a1e8[0x20];
    float m_3a268[0x20];
    float m_3a2e8[0x20];
    float m_3a368[0x20];
    float m_3a3e8[0x20];
    float m_3a468[0x20];
    float m_3a4e8[0x20];
    KeyBank m_3a568[0x20];
    char pad_4f5e8[0x80];
    char m_4f668[0x20][0x20];
    char pad_4fa68[0x4];
    SceneCallback callback;                          // 0x4fa6c: host notification callback
    char pad_4fa70[0x972c4];
    int m_e6d34[0x14];
    int m_e6d84[0x14];
    char pad_e6dd4[0x664];
    Vec4 m_e7438[0x80];
    char m_e7c38[0x20][0x20];
    char pad_e8038[0xc00];
    char m_e8c38[0x80][0x20];
    char m_e9c38[0x20];
    char m_e9c58[0x60][0x20];
    char pad_ea858[0x400];
    int m_eac58[0x80];
    int m_eae58[0x80];
    short m_eb058[0x20];
    char pad_eb098[0x2c0];
    KeyBank m_eb358[0x80];
    Vec4 targets[0x80];                              // 0x13f558: target of each model's state
    char active[0x80];                               // 0x13fd58: 1 while the model's state runs
    char barsFull;                                   // 0x13fdd8: non-zero = show the cinema bars fully at once
    char pad_13fdd9[0x3];
    int textures[0x20];                              // 0x13fddc: texture slots used by the map (-1 = none)
    char pad_13fe5c[0x180];
    Vec4 m_13ffdc[0x20];
    Vec4 m_1401dc[0x20];
    char pad_1403dc[0x380];
    Vec4 m_14075c[0x20];
    Vec4 m_14095c[0x20];
    char pad_140b5c[0x80];
    Vec4 m_140bdc[0x20];
    Vec4 m_140ddc[0x20];
    char pad_140fdc[0xc0];
    char m_14109c[0x20];
    char pad_1410bc[0xc];
    int m_1410c8;
    int notifyPending;                               // 0x1410cc: a 0x5e0 notification must be sent on release
    WorldRenderer();
    int Bind(void *models, void *mapRoot, int *state, SceneCallback callback, void *data);
    int M_10167390(int a, int b, int c);           // 0x10167390
};

// 0x101670e0 (WorldRenderer::WorldRenderer). Constructs the members, clears the two 0x14-entry tables, the names and
// flags of the 0x20 map slots and sets their texture slots to -1.
WorldRenderer::WorldRenderer()
{
    for (int i = 0; i < 0x14; i++)
    {
        m_e6d34[i] = 0;
        m_e6d84[i] = 0;
    }
    for (int i = 0; i < 0x20; i++)
    {
        m_e7c38[i][0] = 0;
        m_14109c[i] = 0;
        textures[i] = -1;
        m_05bac[i] = 0;
        m_4f668[i][0] = 0;
    }
}

extern "C" int DAT_1101de34;
extern "C" float DAT_10203420;

// 0x1017c9c0 (WorldRenderer::Bind). Resets the scene for a new map: the character slots get their default names
// ("キャラ0".. as Shift-JIS), the three 1000-entry tables -9999, the map slots, textures, node tables and fade state
// are cleared, M_10167390(0, 0, 0) runs, and the model records, callback and callback data are stored. mapRoot and
// state are not used. Returns 1.
int WorldRenderer::Bind(void *models, void *mapRoot, int *state, SceneCallback callback, void *data)
{
    m_00040 = -1;
    m_00976 = 0;
    m_00975 = 0;
    for (int i = 0; i < 0x20; i++)
    {
        m_eb058[i] = -1;
        sprintf(characterNames[i], "\x83\x4c\x83\x83\x83\x89%d", i);
    }
    m_0097c = 0;
    DAT_1101de34 = 0;
    m_00974 = 0;
    fadeOnTop = 1;
    barsFull = 0;
    DAT_10203420 = 1.0f;
    for (int i = 0; i < 1000; i++)
    {
        m_00980[i] = -9999;
        m_01920[i] = -9999;
        m_028c0[i] = -9999;
    }
    m_03860 = -9999;
    m_0398c = 0;
    for (int i = 0; i < 0x20; i++)
    {
        for (int j = 0; j < 0x10; j++)
            m_039ac[i][j][0] = 0;
        m_e7c38[i][0] = 0;
        m_14109c[i] = 0;
        textures[i] = -1;
        m_05bac[i] = 0;
        m_4f668[i][0] = 0;
    }
    for (int i = 0; i < 0x20; i++)
    {
        m_05bac[i] = 0;
        m_3a0e8[i] = 0;
        m_3a268[i] = 0;
        m_3a3e8[i] = 0;
        m_3a1e8[i] = 0;
        m_3a368[i] = 0;
        m_3a4e8[i] = 0;
        m_39c68[i] = 0;
        m_39de8[i] = 1.0f;
        m_39f68[i] = 1.0f;
        m_39d68[i] = 0;
        m_39ee8[i] = 0;
        m_3a068[i] = 0;
        textures[i] = -1;
        m_4f668[i][0] = 0;
    }
    m_1410c8 = 0;
    notifyPending = 0;
    m_039a8 = -1;
    m_03868 = 900;
    this->data = data;
    m_03870 = 0;
    m_0399c = 0;
    fadeAmount = 0;
    M_10167390(0, 0, 0);
    this->callback = callback;
    this->models = models;
    m_03864 = 0;
    for (int i = 0; i < 0x60; i++)
    {
        m_eae58[i] = -1;
        m_eac58[i] = -1;
        m_e8c38[i][0] = 0;
        m_e9c58[i][0] = 0;
    }
    m_e9c38[0] = 0;
    for (int i = 0; i < 0x20; i++)
    {
        m_05dac[i] = 0;
        m_05cac[i] = 0;
    }
    return 1;
}
