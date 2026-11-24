// Models attached to a character's hands (EN build). Ported from the matched JP source decomp/src/char_attach.cpp.
#include <string.h>
#include "char_attach.h"
#include "item_info.h"
#include "anim_model.h"
#include "anti_tamper.h"
#include "char.h"
#include "engine.h"
#include "node.h"

// Char_AttachHandModel: with `attach` = 1 loads model `name` as a child "scop" of the right hand of character
// `index` (once). Otherwise removes the "scop" child again. Returns 0 when the character has no model, else 1.
// Called by the weapon and item scripts.
extern "C" int Char_AttachHandModel(int index, int attach, const char *name)
{
    if (g_characterModels[index].root == 0)
        return 0;
    if (attach == 1)
    {
        if (g_characterModels[index].root->Find("scop", 0) == 0)
        {
            Node *hand = g_characterModels[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, ATTACH_NODE_POSITION);
                child->Load(Path_Data(name, 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
                return 1;
            }
        }
    }
    else
    {
        Node *child = g_characterModels[index].root->Find("scop", 0);
        if (child != 0)
            child->parent->RemoveChild(child, 0);
    }
    return 1;
}

// Char_AttachLeftHand: left-hand counterpart of Char_AttachHandModel: with `attach` = 1 loads model `name` as a child
// "scop_l" of character `index`'s "Bip01 L Hand", first removing a "scop_l" that is already there (by calling
// itself). Otherwise removes the "scop_l" child. Returns 0.
extern "C" int Char_AttachLeftHand(int index, int attach, const char *name)
{
    if (g_characterModels[index].root != 0)
    {
        if (attach == 1)
        {
            if (g_characterModels[index].root->Find("scop_l", 0) != 0)
                Char_AttachLeftHand(index, 0, 0);
            Node *hand = g_characterModels[index].root->Find("Bip01 L Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, ATTACH_NODE_POSITION);
                child->Load(Path_Data(name, 0), 0, 0, 0);
                strcpy(child->name, "scop_l");
                child->parent = hand;
                child->BuildMesh(1, 0);
                return 0;
            }
        }
        else
        {
            Node *child = g_characterModels[index].root->Find("scop_l", 0);
            if (child != 0)
                child->parent->RemoveChild(child, 0);
        }
    }
    return 0;
}

// Char_SetHandTool: changes the tool in character `index`'s right hand ("scop" child): mode 1 attaches the player's
// drill (the costume-6 scope, drill 5 or the drill of the current level) and loads the drill motions, mode 2 attaches
// the brush, mode -1 toggles the drill and any other mode removes the tool. Then the player's pointer nodes
// ("scop-pt", "charbase0" and for costume 6 "scop-2pt"/"scop-3pt") are looked up again. Returns 0 when the character
// has no model, else 1. Called by the item and cut-scene scripts.
extern "C" int Char_SetHandTool(int mode, int index)
{
    if (g_characterModels[index].root == 0)
        return 0;
    if (mode == HAND_TOOL_TOGGLE)
    {
        if (g_characterModels[index].root->Find("scop", 0) != 0)
            mode = HAND_TOOL_NONE;
        else
            mode = HAND_TOOL_DRILL;
    }
    if (mode == HAND_TOOL_DRILL)
    {
        if (g_characterModels[index].root->Find("scop", 0) == 0)
        {
            Node *hand = g_characterModels[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, ATTACH_NODE_POSITION);
                int level = g_guardedValue6;
                if (g_guardedValue9 == COSTUME_SCOPE)
                    child->Load(Path_Data("spscop.it3", 0), 0, 0, 0);
                else if (g_drill5 == DRILL5_ON)
                    child->Load(Path_Data("drill5.it3", 0), 0, 0, 0);
                else if (level == 0)
                    child->Load(Path_Data("char02-d.it3", 0), 0, 0, 0);
                else if (level == DRILL_LEVEL_1)
                    child->Load(Path_Data("drill2.it3", 0), 0, 0, 0);
                else if (level == DRILL_LEVEL_2)
                    child->Load(Path_Data("drill3.it3", 0), 0, 0, 0);
                else
                    child->Load(Path_Data("drill4.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
                child->PreScanFile(Path_Data("motionzD.it3", 0), 0);
                FrameTimer_ResetNext2();
            }
        }
    }
    else if (mode == HAND_TOOL_BRUSH)
    {
        if (g_characterModels[index].root->Find("scop", 0) == 0)
        {
            Node *hand = g_characterModels[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, ATTACH_NODE_POSITION);
                child->Load(Path_Data("O_brush.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
            }
        }
    }
    else
    {
        Node *child = g_characterModels[index].root->Find("scop", 0);
        if (child != 0)
            child->parent->RemoveChild(child, 0);
    }
    if (g_characterModels[0].root != 0)
    {
        g_scopePointNode = g_characterModels[0].root->Find("scop-pt", 0);
        if (g_characterModels[0].root != 0)
            g_charBaseNode = g_characterModels[0].root->Find("charbase0", 0);
        else
            g_charBaseNode = 0;
        g_scopePoint2Node = 0;
        g_scopePoint3Node = 0;
        if (g_guardedValue9 == COSTUME_SCOPE)
        {
            g_scopePoint2Node = g_characterModels[0].root->Find("scop-2pt", 0);
            g_scopePoint3Node = g_characterModels[0].root->Find("scop-3pt", 0);
        }
    }
    else
    {
        g_scopePointNode = 0;
        g_charBaseNode = 0;
        g_scopePoint2Node = 0;
        g_scopePoint3Node = 0;
    }
    return 1;
}

// Char_SetHandToolPlain: Char_SetHandTool without drill 5 and without loading the drill motions. Called by the
// scripts that give a plain tool.
extern "C" int Char_SetHandToolPlain(int mode, int index)
{
    if (g_characterModels[index].root == 0)
        return 0;
    if (mode == HAND_TOOL_TOGGLE)
    {
        if (g_characterModels[index].root->Find("scop", 0) != 0)
            mode = HAND_TOOL_NONE;
        else
            mode = HAND_TOOL_DRILL;
    }
    if (mode == HAND_TOOL_DRILL)
    {
        if (g_characterModels[index].root->Find("scop", 0) == 0)
        {
            Node *hand = g_characterModels[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, ATTACH_NODE_POSITION);
                int level = g_guardedValue6;
                if (g_guardedValue9 == COSTUME_SCOPE)
                    child->Load(Path_Data("spscop.it3", 0), 0, 0, 0);
                else if (level == 0)
                    child->Load(Path_Data("char02-d.it3", 0), 0, 0, 0);
                else if (level == DRILL_LEVEL_1)
                    child->Load(Path_Data("drill2.it3", 0), 0, 0, 0);
                else if (level == DRILL_LEVEL_2)
                    child->Load(Path_Data("drill3.it3", 0), 0, 0, 0);
                else
                    child->Load(Path_Data("drill4.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
            }
        }
    }
    else if (mode == HAND_TOOL_BRUSH)
    {
        if (g_characterModels[index].root->Find("scop", 0) == 0)
        {
            Node *hand = g_characterModels[index].root->Find("Bip01 R Hand", 0);
            if (hand != 0)
            {
                Node *child = hand->AddChild(0, ATTACH_NODE_POSITION);
                child->Load(Path_Data("O_brush.it3", 0), 0, 0, 0);
                strcpy(child->name, "scop");
                child->parent = hand;
                child->BuildMesh(1, 0);
            }
        }
    }
    else
    {
        Node *child = g_characterModels[index].root->Find("scop", 0);
        if (child != 0)
            child->parent->RemoveChild(child, 0);
    }
    if (g_characterModels[0].root != 0)
    {
        g_scopePointNode = g_characterModels[0].root->Find("scop-pt", 0);
        if (g_characterModels[0].root != 0)
            g_charBaseNode = g_characterModels[0].root->Find("charbase0", 0);
        else
            g_charBaseNode = 0;
        g_scopePoint2Node = 0;
        g_scopePoint3Node = 0;
        if (g_guardedValue9 == COSTUME_SCOPE)
        {
            g_scopePoint2Node = g_characterModels[0].root->Find("scop-2pt", 0);
            g_scopePoint3Node = g_characterModels[0].root->Find("scop-3pt", 0);
        }
    }
    else
    {
        g_scopePointNode = 0;
        g_charBaseNode = 0;
        g_scopePoint2Node = 0;
        g_scopePoint3Node = 0;
    }
    return 1;
}

// Char_WearHelmet: puts helmet item `item` on the player (0 or less = take the helmet off; items of other types are
// ignored): the old helmet goes, the two face meshes are hidden for item 0x2a (else shown), the face variant follows
// the costume (and the matching helmet: costume 1 with item 0x38, 5 with 0x28, 2 with 0x39, 4 with 0x25, 3 with 0x3d;
// costume 6 always) and the helmet model is attached to the head. Returns 1, or 0 when the player has no model or the
// helmet was only taken off. Called by the equipment menu.
extern "C" int Char_WearHelmet(int item)
{
    if (item > 0 && g_itemTable[item].type != ITEM_TYPE_EQUIPMENT)
        return 1;
    Node *root = g_characterModels[0].root;
    if (root == 0)
        return 0;
    g_helmetFaceSettings = Helmet_FaceSettings(item);
    Node *helmet = root->Find("helmet", 0);
    if (helmet != 0)
        helmet->parent->RemoveChild(helmet, 0);
    Node *face = g_characterModels[0].root->Find("face_char02bf.lwo", 0);
    if (item == ITEM_HELMET_NO_FACE)
    {
        if (face != 0)
            face->flags |= NODE_FLAG_HIDDEN;
        face = g_characterModels[0].root->Find("face_char02bm.lwo", 0);
        if (face != 0)
            face->flags |= NODE_FLAG_HIDDEN;
    }
    else
    {
        if (face != 0)
            face->flags &= ~NODE_FLAG_HIDDEN;
        face = g_characterModels[0].root->Find("face_char02bm.lwo", 0);
        if (face != 0)
            face->flags &= ~NODE_FLAG_HIDDEN;
    }
    g_helmetKind = 0;
    if (g_guardedValue9 == COSTUME_SCOPE)
        g_helmetKind = COSTUME_SCOPE;
    if (item <= 0)
        return 0;
    if (g_guardedValue9 == 1)
    {
        if (item == ITEM_HELMET_COSTUME_1)
            g_helmetKind = 1;
    }
    else if (g_guardedValue9 == 5)
    {
        if (item == ITEM_HELMET_COSTUME_5)
            g_helmetKind = 2;
    }
    else if (g_guardedValue9 == 2)
    {
        if (item == ITEM_HELMET_COSTUME_2)
            g_helmetKind = 3;
    }
    else if (g_guardedValue9 == 4)
    {
        if (item == ITEM_HELMET_COSTUME_4)
            g_helmetKind = 4;
    }
    else if (g_guardedValue9 == 3)
    {
        if (item == ITEM_HELMET_COSTUME_3)
            g_helmetKind = 5;
    }
    else if (g_guardedValue9 == COSTUME_SCOPE)
        g_helmetKind = COSTUME_SCOPE;
    if (g_characterModels[0].root->Find("helmet", 0) == 0)
    {
        Node *head = g_characterModels[0].root->Find("Bip01 Head", 0);
        if (head != 0)
        {
            helmet = head->AddChild(0, HELMET_NODE_POSITION);
            g_helmetOn = 1;
            helmet->Load(Path_Data(g_itemModelPaths[item], 0), 0, 0, 0);
            strcpy(helmet->name, "helmet");
            helmet->parent = head;
            helmet->Rebuild();
            helmet->BuildMesh(1, 0);
            Char_ReloadModel(0);
            FrameTimer_ResetNext2();
            g_characterModels[0].walkingRoute = 0;
            g_characterModels[0].playback->headerFlag = 0;
            g_characterModels[0].PlayMotion(2, 0, 5, 1.0f);
        }
    }
    return 1;
}
