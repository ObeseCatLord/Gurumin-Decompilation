// Models attached to a character's bones (for example a held item on "Bip01 R Hand").

#include <string.h>

struct Node
{
    char name[0x74c];         // name at 0x00 (0x40 bytes), the rest of the node (scene_node.cpp)
    Node *parent;             // 0x74c
    Node *Find(const char *name, int flags);
    Node *AddChild(const char *name, int a);                // 0x101abc30
    int Load(const char *path, int a, int b, int c);        // 0x101ac010
    int BuildMesh(int recurse, int flags);
    int RemoveChild(Node *child, int keep);                 // 0x101a2f30
};

// The part of a Character record (world.h) used here.
struct CharacterRoot
{
    char pad_0000[0x22fc];
    Node *root;               // 0x22fc: root node of the model
    char pad_2300[0x19c];
};

extern "C" CharacterRoot DAT_10db6ed8[0x80];
extern "C" const char *FUN_1013f8a0(const char *name, int folder);

// With `attach` = 1: loads model `name` as a child "scop" of the right hand of character `index` (once).
// Otherwise removes the "scop" child again. Returns 0 when the character has no model, else 1.
extern "C" int FUN_10012450(int index, int attach, const char *name)
{
    if (DAT_10db6ed8[index].root == 0)
        return 0;
    if (attach == 1)
    {
        if (DAT_10db6ed8[index].root->Find("scop", 0) == 0)
        {
            Node *hand = DAT_10db6ed8[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, 0x40);
                child->Load(FUN_1013f8a0(name, 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
                return 1;
            }
        }
    }
    else
    {
        Node *child = DAT_10db6ed8[index].root->Find("scop", 0);
        if (child != 0)
            child->parent->RemoveChild(child, 0);
    }
    return 1;
}
