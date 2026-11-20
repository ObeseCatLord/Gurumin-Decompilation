// The overhead view of the map (0x1000f5a0, called by 0x1000f7f0): a matrix that looks straight down on the map
// node, and the map's bounds copied to the pit and visible-area globals.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    void Set(float x, float y, float z, float w);   // 0x101ac9c0
};

struct Matrix4
{
    float m[16];
    Matrix4();                                   // 0x101ae770
    ~Matrix4();                                  // 0x1013eda0 (empty)
    void SetIdentity();                          // 0x101ae7a0
    void SetTranslation(Vec4 position);          // 0x101adef0
    void SetPosition(float x, float y, float z);    // 0x101adec0
    Matrix4 operator*(const Matrix4 &other);     // 0x101b0670: matrix product
};

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space

// A scene node (scene_node.cpp Node), only what is used here.
struct Node
{
    char pad_0000[0x1cc];
    Matrix4 local;            // 0x1cc: matrix relative to the parent
    char pad_020c[0x4d8];
    Vec4 boundsMin;           // 0x6e4: lower corner of the node's box
    Vec4 boundsMax;           // 0x6f4: upper corner of the node's box
    char pad_0704[0x4c];
    int childCount;           // 0x750
    char pad_0754[0x1000];
    Node *children[0x40];     // 0x1754
};

extern "C" Node *DAT_104c007c;    // the map's model
extern "C" Node *DAT_10255f08;    // the map's root node
extern "C" float DAT_104bfe24;    // the pit's floor height
extern "C" Vec4 DAT_104d29e8;     // the pit area: minimum corner
extern "C" Vec4 DAT_104d29f8;     // the pit area: maximum corner
extern "C" Vec4 DAT_10f668a8;     // the visible area: minimum corner
extern "C" Vec4 DAT_10f668b8;     // the visible area: maximum corner
extern "C" float DAT_10f65594;    // height of the view
extern "C" Vec4 DAT_10f65688;     // camera position (projected here)
extern "C" Vec4 DAT_10f668e8;     // camera target (projected here)

// 0x1000f5a0 (Map_BuildTopView). Takes the map's model (its single child when it has 0x20 children, or the map's
// root node without one): `out` becomes an overhead projection (z mirrored, moved up twice the view height) times
// `matrix`, where the view height is the middle of the model's box. The box becomes the pit area and the visible
// area (both at the view height), and the camera position and target are projected with it. Returns the node.
Node *FUN_1000f5a0(Matrix4 *out, const Matrix4 &matrix)
{
    Matrix4 placement;
    Vec4 origin;
    origin.Set(0, 0, 0, 0);
    placement.SetTranslation(origin);
    Node *map = DAT_104c007c;
    if (map->childCount == 0x20)
        map = map->children[0];
    if (map == 0)
        map = DAT_10255f08;
    Matrix4 view;
    Matrix4 projection;
    view = map->local;
    DAT_104bfe24 = map->boundsMin.z;
    view.SetPosition(0, 0, (map->boundsMax.z + map->boundsMin.z) * 0.5f);
    projection.SetIdentity();
    projection.m[14] = view.m[14] + view.m[14];
    DAT_10f65594 = view.m[14];
    projection.m[10] = -1.0f;
    *out = projection * matrix;
    DAT_104d29e8.Copy(map->boundsMin);
    DAT_104d29f8.Copy(map->boundsMax);
    DAT_10f668a8.Copy(DAT_104d29e8);
    DAT_10f668b8.Copy(DAT_104d29f8);
    DAT_10f668a8.z = DAT_10f65594;
    DAT_10f668b8.z = DAT_10f65594;
    DAT_10f65688.Copy(Project(DAT_10f65688, projection));
    DAT_10f668e8.Copy(Project(DAT_10f668e8, projection));
    return map;
}
