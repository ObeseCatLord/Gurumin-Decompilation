// Textures of the BS_003 boss (EN build). Ported from the matched JP source decomp/src/bs003_variant.cpp.
#include "anim_model.h"
#include "char.h"
#include "engine.h"
#include "node.h"
#include "slot_table.h"
#include "texture_mgr.h"

// Loads the .iti texture `name` of the data folder and returns its slot.
static inline int LoadIti(const char *name)
{
    return Texture_LoadIti(Path_Data(name, 0), 0, 0);
}

// Bs003_SetVariant: loads the four textures of variant `variant` (0-4; others load nothing) and, when character `index` has
// a model, puts them on the BS_003 parts: the body and its second texture on BS_003.lwo, the wing textures on BS_003w.lwo
// and BS_003b.lwo, the body texture on BS_003c, BS_003f and BS_003ball. Returns 0. Called when the boss is set up.
extern "C" int Bs003_SetVariant(int index, int variant)
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
    if (g_characterModels[index].root != 0)
    {
        Node *node = g_characterModels[index].root->Find("BS_003.lwo", 0);
        if (node != 0)
        {
            node->ReplaceMaterial(node->mesh->material[0], body);
            *node->mesh->secondaryTextures = body2;
        }
        node = g_characterModels[index].root->Find("BS_003w.lwo", 0);
        if (node != 0)
        {
            node->ReplaceMaterial(node->mesh->material[0], wing);
            *node->mesh->secondaryTextures = wing2;
        }
        node = g_characterModels[index].root->Find("BS_003b.lwo", 0);
        if (node != 0)
        {
            node->ReplaceMaterial(node->mesh->material[0], wing);
            *node->mesh->secondaryTextures = wing2;
        }
        node = g_characterModels[index].root->Find("BS_003c.lwo", 0);
        if (node != 0)
            node->ReplaceMaterial(node->mesh->material[0], body);
        node = g_characterModels[index].root->Find("BS_003f.lwo", 0);
        if (node != 0)
            node->ReplaceMaterial(node->mesh->material[0], body);
        node = g_characterModels[index].root->Find("BS_003ball.lwo", 0);
        if (node != 0)
            node->ReplaceMaterial(node->mesh->material[0], body);
    }
    return 0;
}
