// Weapon models in a character's hands (EN build). Ported from the matched JP source decomp/src/weapon_attach.cpp.
#include <string.h>
#include "weapon_attach.h"
#include "char_attach.h"
#include "char.h"
#include "engine.h"
#include "node.h"
#include "shaga.h"

// Char_AttachWeapon: gives character `index` weapon `weapon` (0x32..0x3d; 0 keeps the current ones). Removes the old
// right and/or left hand models (as asked by `right` and `left`), then loads the weapon's model file(s) under
// "Bip01 R Hand" (and "Bip01 L Hand" for the two-model weapons). An unknown weapon marks both hands with
// WEAPON_MARK_NONE. Then restarts the "anime2" motion of shaga node `slot` (or clears its timer) and refreshes the
// render mode. Returns 0. Called by the equipment scripts.
extern "C" int Char_AttachWeapon(int index, int weapon, int right, int left, int slot)
{
    char rightFile[0x100];
    char leftFile[0x100];
    if (g_characterModels[index].root == 0)
        return 0;
    if (weapon != 0)
    {
        if (right != 0)
        {
            Node *old = g_characterModels[index].root->Find("@migitesou", 0);
            if (old != 0)
                old->parent->RemoveChild(old, 0);
        }
        if (left != 0)
        {
            Node *old = g_characterModels[index].root->Find("@hidaritesou", 0);
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
        case WEAPON_PAIR_A:
            strcpy(rightFile, "EEQ_21.it3");
            strcpy(leftFile, "EEQ_21t.it3");
            models = 2;
            break;
        case WEAPON_PAIR_B:
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
        case WEAPON_LAST:
            strcpy(rightFile, "EEQ_24b.it3");
            strcpy(leftFile, "EEQ_24b.it3");
            models = 2;
            break;
        default:
            g_charState[index].values[CHARSTATE_WEAPON_MARK_RIGHT] = WEAPON_MARK_NONE;
            g_charState[index].values[CHARSTATE_WEAPON_MARK_LEFT] = WEAPON_MARK_NONE;
            goto attached;
        }
        switch (models)
        {
        case 1:
            if (right != 0)
            {
                Node *hand = g_characterModels[index].root->Find("@Bip01 R Hand", 0);
                if (hand != 0)
                {
                    Node *model = hand->AddChild(0, ATTACH_NODE_POSITION);
                    model->Load(Path_Data(rightFile, 0), 0, 0, 0);
                    strcpy(model->name, "migitesou");
                    model->parent = hand;
                    model->BuildMesh(1, 0);
                }
            }
            break;
        case 2:
            if (right != 0)
            {
                Node *hand = g_characterModels[index].root->Find("@Bip01 R Hand", 0);
                if (hand != 0)
                {
                    Node *model = hand->AddChild(0, ATTACH_NODE_POSITION);
                    model->Load(Path_Data(rightFile, 0), 0, 0, 0);
                    strcpy(model->name, "migitesou");
                    model->parent = hand;
                    model->BuildMesh(1, 0);
                }
            }
            if (left != 0)
            {
                if (weapon == WEAPON_PAIR_A || weapon == WEAPON_PAIR_B)
                    g_charRecords[index].twoHanded = 1;
                Node *hand = g_characterModels[index].root->Find("@Bip01 L Hand", 0);
                if (hand != 0)
                {
                    Node *model = hand->AddChild(0, ATTACH_NODE_POSITION);
                    model->Load(Path_Data(leftFile, 0), 0, 0, 0);
                    strcpy(model->name, "hidaritesou");
                    model->parent = hand;
                    model->BuildMesh(1, 0);
                }
            }
            break;
        }
    }
attached:
    if (g_shagaNode[slot] != 0)
    {
        g_shagaTargetB[slot] = WEAPON_MOTION_TIMER;
        g_shagaNode[slot]->RestartColourPlugs("anime2", g_colourPlugFrames);
    }
    else
        g_shagaTargetB[slot] = 0;
    if (g_refreshRenderMode != 0)
        g_characterModels[index].root->SetRenderModeExcept(0, 0, WEAPON_RENDER_MODE);
    return 0;
}
