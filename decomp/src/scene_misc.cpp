// Model owner (SceneOwner = CharModel) playback forwarders, the glyph cache and effect nodes.
// Every SceneOwner method below first creates the 0x7c84-byte playback object on first use (allocate, Init
// 0x10169f90, bind to this model 0x1016a020) and then forwards the call to it.

#include <string.h>

struct Object
{
    char data[0x40];
    ~Object();
};

#include "vec4.h"

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

#include "node_data.h"

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

#include "node.h"

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

// 0x10166810. Empties a node's skin data: no bone, no bone name, not a link, no links or bones, all names cleared.
void NodeSkin::Clear()
{
    boneCount = 0;
    boneName[0] = 0;
    isLink = 0;
    bone = 0;
    memset(linkNames, 0, sizeof(linkNames));
    memset(boneNames, 0, sizeof(boneNames));
    memset(links, 0, sizeof(links));
    memset(bones, 0, sizeof(bones));
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
        DAT_111be9c8[index].LoadIt3(FUN_1013f8a0(DAT_1101dde0[index].name, 0), 0, 0);
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