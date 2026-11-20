// "shaga" effect slots: models built from EEQR.it3 by FUN_100425f0 (node "shaga_000"), one per slot of
// DAT_10446e00. Loading takes long, so the frame timer is reset before and after (no frame skip afterwards).

#include <stdio.h>
#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    void Fill(const float &value);               // 0x101acc40: sets all four components
    void RotateZ(float angle);                   // 0x101ac300
};

struct Matrix4
{
    float m[16];
    Matrix4();                                   // 0x101ae770
    ~Matrix4();                                  // 0x1013eda0 (empty)
    void SetTranslation(Vec4 position);          // 0x101adef0
    Matrix4 operator*(const Matrix4 &other);     // 0x101b0670: matrix product
};

Matrix4 RotationX(float angle);                  // 0x101b1ad0
Matrix4 RotationZ(float angle);                  // 0x101b1b40

struct Node
{
    char name[0x1cc];                            // 0x00: node name
    Matrix4 local;                               // 0x1cc: matrix relative to the parent
    char pad_020c[0x540];
    Node *parent;                                // 0x74c
    int M_1019b300(const char *motion, int value);  // restarts the two colour plugs (scene_node.cpp)
    Node *Find(const char *name, int index);     // 0x1019ece0
    Node *AddChild(int a, int size);             // 0x101abc30
    int Load(const char *path, int a, int b, int c); // 0x101ac010
    void M_1019afa0(int a, int b);               // 0x1019afa0
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
extern "C" Node *DAT_10255f08;               // the map's root node
extern "C" char DAT_1020da24[];
extern "C" float DAT_1030cd98[];              // their radius
extern "C" Vec4 DAT_10da9160[];               // their velocity
extern "C" char DAT_102194c8[];
extern "C" const char *FUN_1013f8a0(const char *name, int folder);   // builds the data path of `name`
extern "C" int FUN_1013f880(void);            // the game's random number
extern "C" int FUN_1001af10(int index);       // Shaga_PushOut

// 0x100425f0 (Shaga_Create). Takes the first free shaga slot (of 0x28; -1 when all are used, 0 without a map) and
// builds its model under the map's "shaga_000" node (made when missing): a "soubihagareNN" node with EEQR.it3 that
// holds model `model`, turned at random about z, x and z again and placed at `position`. The slot gets the radius,
// counter 0x23 and a velocity: none when `force` is below 0.01, else 20..25 * force outwards and 35..45 up, turned
// to `heading` (+-5 degrees) or at random when `heading` is below -100. Restarts the "anime" colour plugs, pushes
// the model out of the world collision and returns the slot.
int FUN_100425f0(const Vec4 *position, const char *model, int radius, float heading, float force)
{
    Vec4 velocity;
    if (DAT_10255f08 == 0)
        return 0;
    for (int slot = 0; slot < 0x28; slot++)
    {
        if (DAT_10446e00[slot] == 0)
        {
            DAT_1020da24[slot] = 1;
            Node *shaga = DAT_10255f08->Find("@shaga_000", 0);
            if (shaga == 0)
            {
                shaga = DAT_10255f08->AddChild(0, 0x40);
                strcpy(shaga->name, "shaga_000");
                shaga->parent = DAT_10255f08;
            }
            Node *piece = shaga->AddChild(0, 0x40);
            piece->Load(FUN_1013f8a0("EEQR.it3", 0), 0, 0, 0);
            sprintf(piece->name, "soubihagare%02d", slot);
            piece->parent = shaga;
            Node *part = piece->AddChild(0, 0x40);
            part->Load(FUN_1013f8a0(model, 0), 0, 0, 0);
            part->parent = piece;
            piece->M_1019afa0(1, 0);
            piece->local = RotationX((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f) *
                           RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
            piece->local = piece->local * RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
            piece->local.SetTranslation(*position);
            DAT_10446e00[slot] = piece;
            DAT_1030cd98[slot] = (float)radius;
            DAT_1021d018[slot] = 0x23;
            DAT_10254798[slot] = 0;
            if (force < 0.01f)
                velocity.Fill(0.0f);
            else
            {
                velocity.Fill(0.0f);
                velocity.y = ((float)(FUN_1013f880() % 100) / 20.0f + 20.0f) * force;
                velocity.z = 45.0f - (float)(FUN_1013f880() % 100) / 10.0f;
            }
            if (heading < -100.0f)
                velocity.RotateZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
            else
                velocity.RotateZ((float)(FUN_1013f880() % 100 - 50) * 3.1415f / 1800.0f + heading);
            DAT_10da9160[slot].Copy(velocity);
            piece->M_1019b300("anime", DAT_104bf8a4);
            DAT_102194c8[slot] = 0;
            FUN_1001af10(slot);
            return slot;
        }
    }
    return -1;
}

// Creates a shaga effect (FUN_100425f0 with the same arguments), clears the slot's two targets (-1) and its
// counter, and restarts the model's colour plugs with "stop". Returns 0.
extern "C" int FUN_10062750(const Vec4 *a, const char *b, int c, float d, float e)
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
