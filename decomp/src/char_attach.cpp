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
    int Rebuild();                                          // 0x101ac1a0
    int RemoveChild(Node *child, int keep);                 // 0x101a2f30
    int LoadMotion(const char *path, int a);                // 0x101abd60 (Model_LoadIt3Alt)
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
extern "C" int FUN_1013faf0(void);       // FrameTimer_ResetNext2
extern "C" int DAT_10c655f0;             // current costume (6 = the one with three pointer nodes)
extern "C" int DAT_10c6566c;             // 1 = drill 5
extern "C" int DAT_10c64ad4;             // drill level 0..3
extern "C" Node *DAT_104c00f8;           // the player's "scop-pt" node (0 = none)
extern "C" Node *DAT_104c00fc;           // the player's "charbase0" node
extern "C" Node *DAT_104c0100;           // "scop-2pt" (costume 6 only)
extern "C" Node *DAT_104c0104;           // "scop-3pt" (costume 6 only)

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

// 0x10012510 (Char_SetHandTool). Changes the tool in character `index`'s right hand ("scop" child): mode 1 attaches
// the player's drill (the costume-6 scope, drill 5 or the drill of the current level) and loads the drill motions,
// mode 2 attaches the brush, mode -1 toggles the drill and any other mode removes the tool. Then the player's pointer
// nodes ("scop-pt", "charbase0" and for costume 6 "scop-2pt"/"scop-3pt") are looked up again. Returns 0 when the
// character has no model, else 1.
extern "C" int FUN_10012510(int mode, int index)
{
    if (DAT_10db6ed8[index].root == 0)
        return 0;
    if (mode == -1)   // toggle: remove the tool when there is one, else attach the drill
    {
        if (DAT_10db6ed8[index].root->Find("scop", 0) != 0)
            mode = 0;
        else
            mode = 1;
    }
    if (mode == 1)
    {
        if (DAT_10db6ed8[index].root->Find("scop", 0) == 0)
        {
            Node *hand = DAT_10db6ed8[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, 0x40);
                // The costume-6 scope, drill 5, or the drill of the current level.
                int level = DAT_10c64ad4;
                if (DAT_10c655f0 == 6)
                    child->Load(FUN_1013f8a0("spscop.it3", 0), 0, 0, 0);
                else if (DAT_10c6566c == 1)
                    child->Load(FUN_1013f8a0("drill5.it3", 0), 0, 0, 0);
                else if (level == 0)
                    child->Load(FUN_1013f8a0("char02-d.it3", 0), 0, 0, 0);
                else if (level == 1)
                    child->Load(FUN_1013f8a0("drill2.it3", 0), 0, 0, 0);
                else if (level == 2)
                    child->Load(FUN_1013f8a0("drill3.it3", 0), 0, 0, 0);
                else
                    child->Load(FUN_1013f8a0("drill4.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
                child->LoadMotion(FUN_1013f8a0("motionzD.it3", 0), 0);
                FUN_1013faf0();
            }
        }
    }
    else if (mode == 2)
    {
        if (DAT_10db6ed8[index].root->Find("scop", 0) == 0)
        {
            Node *hand = DAT_10db6ed8[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, 0x40);
                child->Load(FUN_1013f8a0("O_brush.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
            }
        }
    }
    else
    {
        Node *child = DAT_10db6ed8[index].root->Find("scop", 0);
        if (child != 0)
            child->parent->RemoveChild(child, 0);
    }
    if (DAT_10db6ed8[0].root != 0)
    {
        DAT_104c00f8 = DAT_10db6ed8[0].root->Find("scop-pt", 0);
        if (DAT_10db6ed8[0].root != 0)
            DAT_104c00fc = DAT_10db6ed8[0].root->Find("charbase0", 0);
        else
            DAT_104c00fc = 0;
        DAT_104c0100 = 0;
        DAT_104c0104 = 0;
        if (DAT_10c655f0 == 6)
        {
            DAT_104c0100 = DAT_10db6ed8[0].root->Find("scop-2pt", 0);
            DAT_104c0104 = DAT_10db6ed8[0].root->Find("scop-3pt", 0);
        }
    }
    else
    {
        DAT_104c00f8 = 0;
        DAT_104c00fc = 0;
        DAT_104c0100 = 0;
        DAT_104c0104 = 0;
    }
    return 1;
}

// 0x10012780 (Char_SetHandToolPlain). Char_SetHandTool without drill 5 and without loading the drill motions.
extern "C" int FUN_10012780(int mode, int index)
{
    if (DAT_10db6ed8[index].root == 0)
        return 0;
    if (mode == -1)   // toggle: remove the tool when there is one, else attach the drill
    {
        if (DAT_10db6ed8[index].root->Find("scop", 0) != 0)
            mode = 0;
        else
            mode = 1;
    }
    if (mode == 1)
    {
        if (DAT_10db6ed8[index].root->Find("scop", 0) == 0)
        {
            Node *hand = DAT_10db6ed8[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, 0x40);
                // The costume-6 scope or the drill of the current level.
                int level = DAT_10c64ad4;
                if (DAT_10c655f0 == 6)
                    child->Load(FUN_1013f8a0("spscop.it3", 0), 0, 0, 0);
                else if (level == 0)
                    child->Load(FUN_1013f8a0("char02-d.it3", 0), 0, 0, 0);
                else if (level == 1)
                    child->Load(FUN_1013f8a0("drill2.it3", 0), 0, 0, 0);
                else if (level == 2)
                    child->Load(FUN_1013f8a0("drill3.it3", 0), 0, 0, 0);
                else
                    child->Load(FUN_1013f8a0("drill4.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
            }
        }
    }
    else if (mode == 2)
    {
        if (DAT_10db6ed8[index].root->Find("scop", 0) == 0)
        {
            Node *hand = DAT_10db6ed8[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, 0x40);
                child->Load(FUN_1013f8a0("O_brush.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
            }
        }
    }
    else
    {
        Node *child = DAT_10db6ed8[index].root->Find("scop", 0);
        if (child != 0)
            child->parent->RemoveChild(child, 0);
    }
    if (DAT_10db6ed8[0].root != 0)
    {
        DAT_104c00f8 = DAT_10db6ed8[0].root->Find("scop-pt", 0);
        if (DAT_10db6ed8[0].root != 0)
            DAT_104c00fc = DAT_10db6ed8[0].root->Find("charbase0", 0);
        else
            DAT_104c00fc = 0;
        DAT_104c0100 = 0;
        DAT_104c0104 = 0;
        if (DAT_10c655f0 == 6)
        {
            DAT_104c0100 = DAT_10db6ed8[0].root->Find("scop-2pt", 0);
            DAT_104c0104 = DAT_10db6ed8[0].root->Find("scop-3pt", 0);
        }
    }
    else
    {
        DAT_104c00f8 = 0;
        DAT_104c00fc = 0;
        DAT_104c0100 = 0;
        DAT_104c0104 = 0;
    }
    return 1;
}

// Replaces the "helmet" child of character `index`'s "Bip01 Head" with model `name` (0 = just remove it). Returns 0
// when only removing or when the head is missing, else the result of building the new helmet (or the old helmet
// node when one is still attached). Not byte-identical yet: the original returns the null `head` already in eax
// through the shared epilogue (see open.tsv).
extern "C" int FUN_100143f0(int index, const char *name)
{
    Node *old = DAT_10db6ed8[index].root->Find("helmet", 0);
    if (old != 0)
        old->parent->RemoveChild(old, 0);
    if (name == 0)
        return 0;
    Node *helmet = DAT_10db6ed8[index].root->Find("helmet", 0);
    if (helmet != 0)
        return (int)helmet;
    Node *head = DAT_10db6ed8[index].root->Find("Bip01 Head", 0);
    if (head != 0)
    {
        helmet = head->AddChild(0, 2);
        helmet->Load(FUN_1013f8a0(name, 0), 0, 0, 0);
        strcpy(helmet->name, "helmet");
        helmet->parent = head;
        helmet->Rebuild();
        return helmet->BuildMesh(1, 0);
    }
    return 0;
}

// Left-hand counterpart of FUN_10012450: with `attach` = 1 loads model `name` as a child "scop_l" of character
// `index`'s "Bip01 L Hand", first removing a "scop_l" that is already there (by calling itself). Otherwise removes
// the "scop_l" child. Returns 0.
extern "C" int FUN_1001a760(int index, int attach, const char *name)
{
    if (DAT_10db6ed8[index].root != 0)
    {
        if (attach == 1)
        {
            if (DAT_10db6ed8[index].root->Find("scop_l", 0) != 0)
                FUN_1001a760(index, 0, 0);
            Node *hand = DAT_10db6ed8[index].root->Find("Bip01 L Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, 0x40);
                child->Load(FUN_1013f8a0(name, 0), 0, 0, 0);
                strcpy(child->name, "scop_l");
                child->parent = hand;
                child->BuildMesh(1, 0);
                return 0;
            }
        }
        else
        {
            Node *child = DAT_10db6ed8[index].root->Find("scop_l", 0);
            if (child != 0)
                child->parent->RemoveChild(child, 0);
        }
    }
    return 0;
}
