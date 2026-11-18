// Initialising a scene node (Node, 0x18a0 bytes; scene_node.cpp has the fields that are understood). The fields are
// set in the original's order; most of their meanings are still open. Matrix4 has the engine's constructor and empty
// destructor because IdentityMatrix() returns a temporary.

#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Set(float x, float y, float z, float w);   // 0x101ac9c0
    void Fill(const float &value);                  // 0x101acc40
};

struct Matrix4
{
    float m[16];
    Matrix4();                                      // 0x101ae770
    ~Matrix4();                                     // 0x1013eda0 (empty)
};

Matrix4 IdentityMatrix();                           // 0x101b0ba0

extern "C" char DAT_11b07118[];                     // default target object of saved-matrix state 0x400000

struct Node
{
    char name[0x40];
    char pad_0040[0x20];
    int m_0060;
    int m_0064;
    int m_0068;
    int m_006c;
    int m_0070;
    int m_0074;
    int m_0078;
    int m_007c;
    int m_0080;
    int m_0084;
    char m_0088;
    char m_0089;
    short m_008a;
    int m_008c;
    int m_0090;
    int m_0094;
    char * m_0098;
    int mesh;
    char pad_00a0[0x8];
    int m_00a8;
    int flags;
    float cullMargin;
    int m_00b4;
    int m_00b8;
    int m_00bc;
    int m_00c0[16];
    int m_0100[16];
    int m_0140[16];
    int m_0180[16];
    int m_01c0;
    int m_01c4;
    int m_01c8;
    Matrix4 localMatrix;
    char pad_020c[0x40];
    Matrix4 world;
    int m_028c;
    int matrixOverride;
    int m_0294;
    Vec4 m_0298;
    int extra;
    Vec4 m_02ac;
    char pad_02bc[0x10];
    int m_02cc;
    unsigned char kind;
    char pad_02d1[0x403];
    Vec4 boundsCentre;
    char pad_06e4[0x20];
    char skinned;
    char pad_0705[0x3];
    int skin;
    char m_070c;
    char pad_070d[0x3f];
    int parent;
    int childCount;
    char childNames[0x40][0x40];
    char pad_1754[0x100];
    int saved;
    char m_1858;
    char pad_1859[0x7];
    int m_1860;
    int m_1864;
    float m_1868;
    float m_186c;
    float m_1870;
    float m_1874;
    unsigned char m_1878;
    char m_1879;
    char m_187a;
    unsigned char m_187b;
    char m_187c;
    char m_187d;
    unsigned char m_187e;
    char pad_187f[0x1];
    float m_1880;
    int m_1884;
    float m_1888;
    int m_188c;
    int m_1890;
    int m_1894;
    char m_1898;
    unsigned char m_1899;
    unsigned char m_189a;
    unsigned char m_189b;
    char pad_189c[0x4];
    void Init();
};

// 0x10191500 (Node::Init). Resets every field of a new node: called "root", no mesh, children or skin, identity world
// and local matrices, cull margin 3.84, unit scale (m_02ac and m_1868..m_1874), and the remaining defaults.
void Node::Init()
{
    m_1860 = 0;
    m_0089 = 0;
    m_008a = 0;
    m_008c = 0;
    m_0088 = 0;
    kind = 0xff;
    m_0298.Fill(0);
    m_1899 = 30;
    m_0074 = 0;
    m_0078 = 0;
    m_187a = 0;
    m_187b = 45;
    m_1879 = 0;
    m_187c = 0;
    m_187d = 0;
    m_187e = 0xff;
    m_0098 = DAT_11b07118;
    m_1858 = 0;
    m_1884 = 0;
    m_1888 = 45.0f;
    m_188c = 0;
    m_1890 = 0;
    m_028c = 0;
    m_189a = 0xff;
    m_189b = 0xff;
    m_1898 = 0;
    m_1880 = 1.0f;
    m_02ac.Fill(1.0f);
    m_1878 = 0xe0;
    m_0080 = 0;
    m_0084 = 0;
    m_007c = 0;
    mesh = 0;
    boundsCentre.Set(0, 0, 0, 0);
    m_00bc = 0;
    m_02cc = 0;
    strcpy(name, "root");
    m_01c0 = 0;
    m_01c8 = 0;
    m_01c4 = 0;
    extra = 0;
    saved = 0;
    skin = 0;
    m_1864 = 0;
    for (int i = 0; i < 16; i++)
    {
        m_00c0[i] = 0;
        m_0100[i] = 0;
        m_0140[i] = 0;
        m_0180[i] = 0;
    }
    m_00a8 = 0;
    m_00b8 = 0;
    flags = 0;
    cullMargin = 3.84f;
    m_00b4 = 0;
    m_0070 = 0;
    m_1894 = 0;
    m_0294 = 0;
    parent = 0;
    skinned = 0;
    m_0090 = 0;
    m_0094 = 0;
    childCount = 0;
    matrixOverride = 0;
    m_0060 = 0;
    m_0068 = 0;
    m_006c = 0;
    m_0064 = 0;
    world = IdentityMatrix();
    world.m[12] = 10000.0f;
    localMatrix = IdentityMatrix();
    m_070c = 0;
    memset(childNames, 0, sizeof(childNames));
    m_1868 = 1.0f;
    m_186c = 1.0f;
    m_1870 = 1.0f;
    m_1874 = 1.0f;
}
