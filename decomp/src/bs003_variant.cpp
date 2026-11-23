// The BS_003 model (a boss of the game, loaded on a character slot) in one of five texture variants: four textures
// are loaded for the variant and put on the model's parts (BS_003.lwo, BS_003w.lwo, ...).

#include "texture_load.h"

// Loads ITI texture `name` from the data folder (the ITI branch of LoadTexture, as in face_anim.cpp).
inline int LoadIti(const char *name)
{
    return FUN_10174ce0(FUN_1013f8a0(name, 0), 0, 0);
}

// The material table of a node's mesh (engine_10143.cpp SlotTable), only the two lists used here.
struct MeshData
{
    char pad_0000[0x14];
    int *materials;                                  // 0x14: current material of each entry
    int *secondary;                                  // 0x18: the entries' second texture
};

#include "node.h"

// A character record (world.h Character), only its model root.
struct Character
{
    char pad_0000[0x22fc];
    Node *root;                                      // 0x22fc
    char pad_2300[0x19c];
};

extern "C" Character DAT_10db6ed8[0x80];

// 0x10034920 (Bs003_SetVariant). Loads the four textures of variant `variant` (0-4; others load nothing) and, when
// character `index` has a model, puts them on the BS_003 parts: the body and its second texture on BS_003.lwo, the
// wing (?) textures on BS_003w.lwo and BS_003b.lwo, the body texture on BS_003c, BS_003f and BS_003ball. Returns 0.
extern "C" int FUN_10034920(int index, int variant)
{
    int body;
    int body2;
    int wing;
    int wing2;
    switch (variant)
    {
    case 0:
        body = LoadIti("BS_003.iti");
        body2 = LoadIti("BS_003l.iti");
        wing = LoadIti("BS_003b.iti");
        wing2 = LoadIti("BS_003bl.iti");
        break;
    case 1:
        body = LoadIti("BS_003t.iti");
        body2 = LoadIti("BS_003l.iti");
        wing = LoadIti("BS_003tb.iti");
        wing2 = LoadIti("BS_003bl.iti");
        break;
    case 2:
        body = LoadIti("BS_003i.iti");
        body2 = LoadIti("BS_003l2.iti");
        wing = LoadIti("BS_003ib.iti");
        wing2 = LoadIti("BS_003l3.iti");
        break;
    case 3:
        body = LoadIti("BS_003f.iti");
        body2 = LoadIti("BS_003l.iti");
        wing = LoadIti("BS_003fb.iti");
        wing2 = LoadIti("BS_003bl.iti");
        break;
    case 4:
        body = LoadIti("BS_003i2.iti");
        body2 = LoadIti("BS_003l4.iti");
        wing = LoadIti("BS_003b2.iti");
        wing2 = LoadIti("BS_003l5.iti");
        break;
    }
    if (DAT_10db6ed8[index].root != 0)
    {
        Node *node = DAT_10db6ed8[index].root->Find("BS_003.lwo", 0);
        if (node != 0)
        {
            node->ReplaceMaterial(node->mesh->materials[0], body);
            *node->mesh->secondary = body2;
        }
        node = DAT_10db6ed8[index].root->Find("BS_003w.lwo", 0);
        if (node != 0)
        {
            node->ReplaceMaterial(node->mesh->materials[0], wing);
            *node->mesh->secondary = wing2;
        }
        node = DAT_10db6ed8[index].root->Find("BS_003b.lwo", 0);
        if (node != 0)
        {
            node->ReplaceMaterial(node->mesh->materials[0], wing);
            *node->mesh->secondary = wing2;
        }
        node = DAT_10db6ed8[index].root->Find("BS_003c.lwo", 0);
        if (node != 0)
            node->ReplaceMaterial(node->mesh->materials[0], body);
        node = DAT_10db6ed8[index].root->Find("BS_003f.lwo", 0);
        if (node != 0)
            node->ReplaceMaterial(node->mesh->materials[0], body);
        node = DAT_10db6ed8[index].root->Find("BS_003ball.lwo", 0);
        if (node != 0)
            node->ReplaceMaterial(node->mesh->materials[0], body);
    }
    return 0;
}
