// "shaga" effect slots: models built from EEQR.it3 by FUN_100425f0 (node "shaga_000"), one per slot of
// DAT_10446e00. Loading takes long, so the frame timer is reset before and after (no frame skip afterwards).

#include <stdio.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"

Matrix4 RotationX(float angle);                  // 0x101b1ad0
Matrix4 RotationZ(float angle);                  // 0x101b1b40
Matrix4 Scaling(float x, float y, float z);      // 0x101b13c0

#include "node.h"

// Engine object at 0x10836c70; M_1013eee0 is FrameTimer_ResetNext (engine_1013e.cpp).
struct Engine
{
    int M_1013eee0();
};

// A character record seen from its model part (char_move.cpp CharRecord), only the fields used here.
struct CharRecord
{
    char pad_0000[0x2328];
    Matrix4 matrix;           // 0x2328: world matrix
    char pad_2368[0x114];
    float heading;            // 0x247c: facing angle in radians
    char pad_2480[0x1c];
};

extern "C" Engine DAT_10836c70;
extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" int DAT_1030b668[0x80];            // base kind of each character
extern "C" int DAT_10c64ad8;                  // the player's stamina gauge
extern "C" void FUN_1000e750(void);           // AntiTamper_Store
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

// 0x10042140 (Shaga_BreakOff). Like Shaga_Create for a piece breaking off character `index` (with `tired`, the
// stamina gauge first rises by 2): the piece starts from `matrix` turned at random about z and is launched away from
// the character's facing (+-5 degrees). With `scaled`, base kinds 0x10 and 0x31 make it twice or 1.2 times as big.
// It is raised so that it starts at least its radius + 1 above the character. A piece without room for its model is
// removed again and the next slot tried. Returns the slot, -1 when all are used, 0 without a map.
int FUN_10042140(int index, const char *model, int radius, const Vec4 *position, const Matrix4 *matrix, int scaled,
                 float force, int tired)
{
    Vec4 velocity;
    float scale = 1.0f;
    if (tired != 0)
    {
        DAT_10c64ad8 += 2;
        FUN_1000e750();
    }
    if (DAT_10255f08 == 0)
        return 0;
    for (int slot = 0; slot < 0x28; slot++)
    {
        if (DAT_10446e00[slot] != 0)
            continue;
        Node *shaga = DAT_10255f08->Find("@shaga_000", 0);
        if (shaga == 0)
        {
            shaga = DAT_10255f08->AddChild(0, 0x40);
            strcpy(shaga->name, "shaga_000");
            shaga->parent = DAT_10255f08;
        }
        Node *piece = shaga->AddChild(0, 0x40);
        if (piece == 0)
            continue;
        piece->Load(FUN_1013f8a0("EEQR.it3", 0), 0, 0, 0);
        sprintf(piece->name, "soubihagare%02d", slot);
        piece->parent = shaga;
        Node *part = piece->AddChild(0, 0x40);
        if (part == 0)
        {
            piece->parent->RemoveChild(piece, 0);
            continue;
        }
        part->Load(FUN_1013f8a0(model, 0), 0, 0, 0);
        part->parent = piece;
        piece->BuildMesh(1, 0);
        piece->matrix = *matrix;
        piece->matrix = piece->matrix * RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
        piece->matrix.SetTranslation(*position);
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
            velocity.RotateZ((float)(FUN_1013f880() % 100 - 50) * 3.1415f / 1800.0f - DAT_10db6ee8[index].heading);
        }
        DAT_10da9160[slot].Copy(velocity);
        piece->RestartColourPlugs("anime", DAT_104bf8a4);
        DAT_1020da24[slot] = 1;
        if (scaled != 0)
        {
            switch (DAT_1030b668[index])
            {
            case 0x10:
                scale = 2.0f;
                DAT_1020da24[slot] = 4;
                break;
            case 0x31:
                scale = 1.2f;
                DAT_1020da24[slot] = 2;
                break;
            }
            DAT_1030cd98[slot] = scale * DAT_1030cd98[slot];
            part->matrix = part->matrix * Scaling(scale, scale, scale);
        }
        float above = position->z - DAT_10db6ee8[index].matrix.m[14];
        float lowest = DAT_1030cd98[slot] + 1.0f;
        if (above < lowest)
            piece->matrix.m[14] += lowest - above;
        DAT_102194c8[slot] = 0;
        FUN_1001af10(slot);
        DAT_10836c70.M_1013eee0();
        return slot;
    }
    return -1;
}

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
            piece->BuildMesh(1, 0);
            piece->matrix = RotationX((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f) *
                           RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
            piece->matrix = piece->matrix * RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
            piece->matrix.SetTranslation(*position);
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
            piece->RestartColourPlugs("anime", DAT_104bf8a4);
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
        DAT_10446e00[slot]->RestartColourPlugs("stop", DAT_104bf8a4);
    }
    DAT_10836c70.M_1013eee0();
    return 0;
}
