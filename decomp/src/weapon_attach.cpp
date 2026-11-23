// Attaching weapon models to a character's hands (called from 0x100a8ac0): the weapon number selects the model
// files, which are loaded as nodes "migitesou" (right hand) and "hidaritesou" (left hand) under the hand bones.

#include <string.h>

#include "node.h"

#include "anim_model.h"
#include "char_state.h"

#include "char_slot.h"

extern "C" Node *DAT_10446e00[0x28];
extern "C" int DAT_1021d018[0x28];
extern "C" int DAT_104bf8a4;
extern "C" int DAT_10f654b0;
extern "C" const char *FUN_1013f8a0(const char *name, int folder);  // builds the data path of `name`

// 0x1001a990 (Char_AttachWeapon). Gives character `index` weapon `weapon` (0x32..0x3d; 0 keeps the current ones):
// removes the old right and/or left hand models (as asked by `right` and `left`), then loads the weapon's model
// file(s) EEQ_19..EEQ_24 under "Bip01 R Hand" (and "Bip01 L Hand" for the two-model weapons). An unknown weapon
// marks weaponMarkA and weaponMarkB with -2. Then restarts the "anime2" motion of display node `slot` (or clears its
// counter) and refreshes the model. Returns 0.
extern "C" int FUN_1001a990(int index, int weapon, int right, int left, int slot)
{
    char rightFile[0x100];
    char leftFile[0x100];
    if (DAT_10db6ee8[index].root == 0)
        return 0;
    if (weapon != 0)
    {
        if (right != 0)
        {
            Node *old = DAT_10db6ee8[index].root->Find("@migitesou", 0);
            if (old != 0)
                old->parent->RemoveChild(old, 0);
        }
        if (left != 0)
        {
            Node *old = DAT_10db6ee8[index].root->Find("@hidaritesou", 0);
            if (old != 0)
                old->parent->RemoveChild(old, 0);
        }
        int models;
        switch (weapon)
        {
        case 0x32:
            strcpy(rightFile, "EEQ_19.it3");
            models = 1;
            break;
        case 0x33:
            strcpy(rightFile, "EEQ_19b.it3");
            models = 1;
            break;
        case 0x34:
            strcpy(rightFile, "EEQ_20.it3");
            models = 1;
            break;
        case 0x35:
            strcpy(rightFile, "EEQ_20b.it3");
            models = 1;
            break;
        case 0x36:
            strcpy(rightFile, "EEQ_21.it3");
            strcpy(leftFile, "EEQ_21t.it3");
            models = 2;
            break;
        case 0x37:
            strcpy(rightFile, "EEQ_21b.it3");
            strcpy(leftFile, "EEQ_21bt.it3");
            models = 2;
            break;
        case 0x38:
            strcpy(rightFile, "EEQ_22.it3");
            models = 1;
            break;
        case 0x39:
            strcpy(rightFile, "EEQ_22b.it3");
            models = 1;
            break;
        case 0x3a:
            strcpy(rightFile, "EEQ_23.it3");
            models = 1;
            break;
        case 0x3b:
            strcpy(rightFile, "EEQ_23b.it3");
            models = 1;
            break;
        case 0x3c:
            strcpy(rightFile, "EEQ_24r.it3");
            strcpy(leftFile, "EEQ_24l.it3");
            models = 2;
            break;
        case 0x3d:
            strcpy(rightFile, "EEQ_24b.it3");
            strcpy(leftFile, "EEQ_24b.it3");
            models = 2;
            break;
        default:
            DAT_1020b430[index].weaponMarkA = -2;
            DAT_1020b430[index].weaponMarkB = -2;
            goto attached;
        }
        switch (models)
        {
        case 1:
            if (right != 0)
            {
                Node *hand = DAT_10db6ee8[index].root->Find("@Bip01 R Hand", 0);
                if (hand != 0)
                {
                    Node *model = hand->AddChild(0, 0x40);
                    model->Load(FUN_1013f8a0(rightFile, 0), 0, 0, 0);
                    strcpy(model->name, "migitesou");
                    model->parent = hand;
                    model->BuildMesh(1, 0);
                }
            }
            break;
        case 2:
            if (right != 0)
            {
                Node *hand = DAT_10db6ee8[index].root->Find("@Bip01 R Hand", 0);
                if (hand != 0)
                {
                    Node *model = hand->AddChild(0, 0x40);
                    model->Load(FUN_1013f8a0(rightFile, 0), 0, 0, 0);
                    strcpy(model->name, "migitesou");
                    model->parent = hand;
                    model->BuildMesh(1, 0);
                }
            }
            if (left != 0)
            {
                if (weapon == 0x36 || weapon == 0x37)
                    DAT_10daaae8[index].twoHanded = 1;
                Node *hand = DAT_10db6ee8[index].root->Find("@Bip01 L Hand", 0);
                if (hand != 0)
                {
                    Node *model = hand->AddChild(0, 0x40);
                    model->Load(FUN_1013f8a0(leftFile, 0), 0, 0, 0);
                    strcpy(model->name, "hidaritesou");
                    model->parent = hand;
                    model->BuildMesh(1, 0);
                }
            }
            break;
        }
    }
attached:
    if (DAT_10446e00[slot] != 0)
    {
        DAT_1021d018[slot] = 0xcc;
        DAT_10446e00[slot]->RestartColourPlugs("anime2", DAT_104bf8a4);
    }
    else
        DAT_1021d018[slot] = 0;
    if (DAT_10f654b0 != 0)
        DAT_10db6ee8[index].root->M_1019b0c0(0, 0, 0x1f);
    return 0;
}
