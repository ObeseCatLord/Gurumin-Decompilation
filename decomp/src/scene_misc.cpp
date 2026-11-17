// Model owner (SceneOwner = CharModel) playback forwarders, the glyph cache and effect nodes.
// Every SceneOwner method below first creates the 0x7c84-byte playback object on first use (allocate, Init
// 0x10169f90, bind to this model 0x1016a020) and then forwards the call to it.

#include <string.h>

struct Object
{
    char data[0x40];
    ~Object();
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Fill(const float &value);
};

struct Scene
{
    void M_10169f90();
    void M_1016a020(void *owner);
    void M_1016a100(int a, int b, int c, int d, int e, int f);
    int M_1016a260(int layer, int slot, int x, int y);
    int M_1016a220(int a, int slot, int value);
    int M_1016a2a0(int layer, int slot, int value);
    char pad_0000[0xbec];
    int m_0bec;
};

struct SceneOwner
{
    char pad_0000[0x298];
    Scene *scene;
    int M_1017af40(int a, int b, int c, int d, int e, int f);
    int M_1017afb0(int layer, int slot, int x, int y);
    int M_1017b020(int a, int slot, int value);
    int M_1017b080(int layer, int slot, int value);
    int M_1017b160();
};

struct GlyphCache
{
    char pad_0000[0x80];
    int m_0080;
    char m_0084;
    char pad_0085[0x7f];
    int m_0104;
    int m_0108[0x400];
    int m_1108[0x40];
    int m_1208[0x44];
    int m_1318[0x440];
    char pad_2418[0x3200];
    int m_5618;
    void Clear();
};

struct ColorTables
{
    char pad_0000[0x14];
    Vec4 a[0x100];
    Vec4 b[0x100];
    Vec4 c[0x100];
    Vec4 d[0x100];
    ColorTables();
};

struct Widget
{
    char pad_0000[0xc4];
    Object m_00c4;
    Object m_0104;
    Object m_0144;
    ~Widget();
};

struct Node
{
    char data[0x18a0];
    int M_101abe30(const char *path, int a, int b);
    void BuildMesh(int recurse, int flags);
};

struct Placement
{
    char pad_0000[0x200];
    int kind;
    char pad_0204[0x37];
    char name[0x40];
    char pad_027b[0x15d];
};

extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" Placement DAT_1101dde0[];
extern "C" Node DAT_111be9c8[];
extern "C" int DAT_10ff1df8[];
extern "C" const char *FUN_1013f8a0(const char *format, ...);

// Empties the glyph cache used by the Windows-font text path (counters and four tables).
void GlyphCache::Clear()
{
    m_5618 = 0;
    m_0084 = 0;
    m_0104 = 0;
    m_0080 = 0;
    memset(m_0108, 0, sizeof(m_0108));
    memset(m_1318, 0, sizeof(m_1318));
    memset(m_1108, 0, sizeof(m_1108));
    memset(m_1208, 0, sizeof(m_1208));
}

// Constructor: the member constructors do the work.
ColorTables::ColorTables()
{
}

// Destructor: the member destructors do the work.
Widget::~Widget()
{
}

// Forwards a six-value playback setting to the playback object (Playback::M_1016a100).
int SceneOwner::M_1017af40(int a, int b, int c, int d, int e, int f)
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
    }
    scene->M_1016a100(a, b, c, d, e, f);
    return 1;
}

// Sets the (x, y) offset of slot `slot` in layer `layer` (LayerTable::M_1016a260).
int SceneOwner::M_1017afb0(int layer, int slot, int x, int y)
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
    }
    scene->M_1016a260(layer, slot, x, y);
    return 1;
}

// Loads the model of effect `index` when its kind is 4, 10 or 20 (from the effect name table), starts it and
// marks the effect node as used.
extern "C" int __stdcall FUN_1016fa40(int index)
{
    int kind = DAT_1101dde0[index].kind;
    if (kind == 4 || kind == 10 || kind == 20)
    {
        DAT_111be9c8[index].M_101abe30(FUN_1013f8a0(DAT_1101dde0[index].name, 0), 0, 0);
        DAT_111be9c8[index].BuildMesh(1, 0);
        DAT_10ff1df8[index] = 1;
    }
    return 0;
}

// Sets slot `slot`'s layer source and value (LayerTable::M_1016a220).
int SceneOwner::M_1017b020(int a, int slot, int value)
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
    }
    scene->M_1016a220(a, slot, value);
    return 1;
}

// Sets parameter `slot` of layer `layer` (LayerTable::M_1016a2a0).
int SceneOwner::M_1017b080(int layer, int slot, int value)
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
    }
    scene->M_1016a2a0(layer, slot, value);
    return 1;
}

// 1 when the playback has reached its end state (m_0bec == 1).
int SceneOwner::M_1017b160()
{
    if (scene == 0)
    {
        scene = (Scene *)DAT_11b0b938(0x7c84, "");
        scene->M_10169f90();
        scene->M_1016a020(this);
    }
    return scene->m_0bec == 1;
}