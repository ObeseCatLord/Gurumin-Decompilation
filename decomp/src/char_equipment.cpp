// Dropping a character's equipment: the pieces a character wears break off and fall as shaga models. A character's
// script code holds four two-digit equipment fields, weapon (0x32..0x3d), mount, clothes and head, which pick the
// model files EEQ_01..EEQ_25 of the pieces.

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"

#include "char_vars.h"
#include "map_vars.h"
#include "game_vars.h"

#include "effect_manager.h"
#include "texture_manager.h"

#include "shaga.h"
#include "char_equipment.h"

extern "C" int FUN_1013f880(void);                                         // the game's random number
extern "C" void FUN_1000ec70(int frames, float r, float g, float b, float a);   // ColourFade_Start
extern "C" int FUN_1003a7d0(const char *path, float x, float y, float z, int start, float ox, float oy, float oz);  // EffectModel_Place
int FUN_100404e0(int index, int mount, int clothes, int head, int weapon, int skipKindSelect, int alternative);  // Char_PutOnEquipment (char_equip_models.h; an #include here changes FUN_100615f0's code)
extern "C" int FUN_1001b0a0(int index, int alternative, int quiet);        // Char_SelectKind

#define EQUIP_FIELD_UNIT 100
#define BASE_KIND_PACKED_GROUP 0x10
#define BASE_KIND_PAIRED_WEAPONS 0x31
#define MOUNT_PIECE_RADIUS 0x19
#define DROP_FORCE_SOFT 0.3f
#define DROP_FORCE_HARD 1.0f
#define MODEL_REACH_SCALE 0.578f
#define HEADING_DEGREES 0x168
#define HEADING_PI 3.1415f
#define HEADING_HALF_TURN 180.0f
#define FADE_FRAMES 15
#define WEAPON_MARK_NONE -2
#define EFFECT_OWNER_BASE 400

// 0x100615f0 (Char_DropEquipment; C++ linkage, the original has an EH frame). Character `index` loses what it wears. Its script code AABBCCDD holds the weapon
// AA, the mount BB, the clothes CC and the head piece DD. With `scatter` clear, the mount (node "@norisou") is
// dropped first, else the clothes (node "@hukusou"), else the head piece (node "@atamasou"), else the weapon (node
// "@hidaritesou" or "@migitesou"): the node is cut from the model and a piece model of the same kind falls from its
// place (Shaga_BreakOff, plus an effect model for the mount and clothes); one piece is dropped per call and its
// field is cleared. With `scatter` set the mount and clothes are kept and the heading is randomised while the head
// and weapon pieces are made. `bothHands` clears the weapon field after the left hand's weapon too. The packed
// code is stored back and the character's kind selected again unless it is an enemy. Returns 0.
int FUN_100615f0(int index, int scatter, int bothHands)
{
    Vec4 position;
    Matrix4 matrix;
    int code = DAT_102548c0[index];
    int hand = 0;
    float savedHeading = DAT_10db6ee8[index].targetHeading;
    int weapon = code / 1000000 % EQUIP_FIELD_UNIT;
    int mount = code / 10000 % EQUIP_FIELD_UNIT;
    int clothes = code / EQUIP_FIELD_UNIT % EQUIP_FIELD_UNIT;
    int head = code % EQUIP_FIELD_UNIT;
    DAT_10468730[index] = 1;
    int scaled = 0;
    if (DAT_10daaae8[index].kind == CHAR_KIND_ENEMY &&
        (DAT_1020b430[index].behaviour == 0xb || DAT_1020b430[index].behaviour == 0xc))
        FUN_1000ec70(FADE_FRAMES, 1.0f, 1.0f, 1.0f, 1.0f);
    if (DAT_10daaae8[index].kind == BASE_KIND_PACKED_GROUP || DAT_1030b668[index] == BASE_KIND_PACKED_GROUP ||
        DAT_10daaae8[index].kind == BASE_KIND_PAIRED_WEAPONS || DAT_1030b668[index] == BASE_KIND_PAIRED_WEAPONS)
        scaled = 1;
    if (scatter != 0)
        DAT_10daaa80.RemoveOwnedBy(index);
    DAT_10daaa80.RemoveOwnedBy(index + EFFECT_OWNER_BASE);
    int slot;
    Node *piece;
    if (mount != 0 && scatter == 0)
    {
        DAT_10daaae8[index].held = 0;
        savedHeading = 0;
        position.Fill(savedHeading);
        DAT_10db6ee8[index].root->matrix.SetTranslation(position);
        DAT_10db6ee8[index].colliderReach = DAT_10db6ee8[index].collider->bounds.w * MODEL_REACH_SCALE;
        piece = DAT_10db6ee8[index].root->Find("@norisou", 0);
        if (piece != 0)
        {
            position.Copy(piece->worldMatrix.GetTranslation());
            matrix = piece->parent->worldMatrix;
            piece->parent->RemoveChild(piece, 0);
        }
        slot = 0;
        switch (mount)
        {
        case 1:
            FUN_1003a7d0("EEQ_15br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_15d.it3", MOUNT_PIECE_RADIUS, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 2:
            FUN_1003a7d0("EEQ_25br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_25d.it3", MOUNT_PIECE_RADIUS, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 3:
            FUN_1003a7d0("EEQ_25bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_25bd.it3", MOUNT_PIECE_RADIUS, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        }
        DAT_10836c70.ResetFrameTimer();
        if (slot > -1)
        {
            DAT_1042db08[slot] = -1;
            DAT_1021d018[slot] = -index;
            DAT_10446e00[slot]->RestartColourPlugs("stop", DAT_104bf8a4);
        }
        mount = 0;
    }
    else if (clothes != 0 && scatter == 0)
    {
        DAT_10daaae8[index].twoHanded = 0;
        piece = DAT_10db6ee8[index].root->Find("@hukusou", 0);
        if (piece != 0)
        {
            position.Copy(piece->worldMatrix.GetTranslation());
            matrix = piece->parent->worldMatrix;
            piece->parent->RemoveChild(piece, 0);
        }
        slot = 0;
        switch (clothes)
        {
        case 1:
            FUN_1003a7d0("EEQ_01br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_01d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 2:
            FUN_1003a7d0("EEQ_02br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_01d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 3:
            FUN_1003a7d0("EEQ_03br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_01d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 4:
            FUN_1003a7d0("EEQ_04br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_04d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 5:
            FUN_1003a7d0("EEQ_05br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_05d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 6:
            FUN_1003a7d0("EEQ_06br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_06d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 7:
            FUN_1003a7d0("EEQ_12br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_12d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 8:
            FUN_1003a7d0("EEQ_13br.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_13d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 9:
            FUN_1003a7d0("EEQ_12bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_12d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 10:
            FUN_1003a7d0("EEQ_13bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_13bd.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 11:
            FUN_1003a7d0("EEQ_01bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_01d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 12:
            FUN_1003a7d0("EEQ_02bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_01d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 13:
            FUN_1003a7d0("EEQ_03bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_01d.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 14:
            FUN_1003a7d0("EEQ_04bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_04bd.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 15:
            FUN_1003a7d0("EEQ_05bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_05bd.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        case 16:
            FUN_1003a7d0("EEQ_06bb.it3", position.x, position.y, position.z, 0, 0, 0, 0);
            slot = FUN_10042140(index, "EEQ_06bd.it3", 0x19, &position, &matrix, scaled, DROP_FORCE_SOFT, 1);
            break;
        }
        DAT_10836c70.ResetFrameTimer();
        if (slot > -1)
        {
            DAT_1042db08[slot] = -1;
            DAT_1021d018[slot] = -index;
            DAT_10446e00[slot]->RestartColourPlugs("stop", DAT_104bf8a4);
        }
        clothes = 0;
    }
    else if (head != 0)
    {
        piece = DAT_10db6ee8[index].root->Find("@atamasou", 0);
        if (piece != 0)
        {
            position.Copy(piece->worldMatrix.GetTranslation());
            matrix = piece->parent->worldMatrix;
            piece->parent->RemoveChild(piece, 0);
        }
        if (scatter != 0)
        {
            float facing = (float)(FUN_1013f880() % HEADING_DEGREES) * HEADING_PI / HEADING_HALF_TURN;
            DAT_10db6ee8[index].targetHeading = facing;
            DAT_10db6ee8[index].heading = facing;
        }
        slot = 0;
        switch (head)
        {
            case 1:
                slot = FUN_10042140(index, "EEQ_07.it3", 45, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 2:
                slot = FUN_10042140(index, "EEQ_08d.it3", 20, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 3:
                slot = FUN_10042140(index, "EEQ_09d.it3", 25, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 4:
                slot = FUN_10042140(index, "EEQ_10d.it3", 25, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 5:
                slot = FUN_10042140(index, "EEQ_11d.it3", 30, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 6:
                slot = FUN_10042140(index, "EEQ_16d.it3", 21, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 7:
                slot = FUN_10042140(index, "EEQ_17d.it3", 20, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 8:
                slot = FUN_10042140(index, "EEQ_18d.it3", 30, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 9:
                slot = FUN_10042140(index, "EEQ_14d.it3", 35, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 10:
                slot = FUN_10042140(index, "EEQ_07b.it3", 45, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 11:
                slot = FUN_10042140(index, "EEQ_08bd.it3", 20, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 12:
                slot = FUN_10042140(index, "EEQ_09bd.it3", 25, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 13:
                slot = FUN_10042140(index, "EEQ_10bd.it3", 25, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 14:
                slot = FUN_10042140(index, "EEQ_11bd.it3", 30, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 15:
                slot = FUN_10042140(index, "EEQ_14bd.it3", 35, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 16:
                slot = FUN_10042140(index, "EEQ_16bd.it3", 21, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
            case 17:
                slot = FUN_10042140(index, "EEQ_18bd.it3", 30, &position, &matrix, scaled, DROP_FORCE_SOFT, 0);
                break;
        }
        if (slot > -1)
        {
            DAT_1021d018[slot] = -index;
            DAT_1042db08[slot] = -1;
            DAT_10446e00[slot]->RestartColourPlugs("stop", DAT_104bf8a4);
        }
        if (scatter != 0)
        {
            DAT_10db6ee8[index].targetHeading = savedHeading;
            DAT_10db6ee8[index].heading = savedHeading;
        }
        head = 0;
    }
    else if (weapon != 0)
    {
        DAT_10daaae8[index].twoHanded = 0;
        Node *leftWeapon = DAT_10db6ee8[index].root->Find("@hidaritesou", 0);
        if (leftWeapon != 0)
        {
            position.Copy(leftWeapon->worldMatrix.GetTranslation());
            matrix = leftWeapon->parent->worldMatrix;
            leftWeapon->parent->RemoveChild(leftWeapon, 0);
            hand = 1;
        }
        else
        {
            Node *rightWeapon = DAT_10db6ee8[index].root->Find("@migitesou", 0);
            if (rightWeapon != 0)
            {
                position.Copy(rightWeapon->worldMatrix.GetTranslation());
                matrix = rightWeapon->parent->worldMatrix;
                rightWeapon->parent->RemoveChild(rightWeapon, 0);
                bothHands = 1;
            }
        }
        if (scatter != 0)
        {
            float facing = (float)(FUN_1013f880() % HEADING_DEGREES) * HEADING_PI / HEADING_HALF_TURN;
            DAT_10db6ee8[index].targetHeading = facing;
            DAT_10db6ee8[index].heading = facing;
        }
        switch (weapon)
        {
        case 50:
            slot = FUN_10042140(index, "EEQ_19d.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 51:
            slot = FUN_10042140(index, "EEQ_19bd.it3", 25, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 52:
            slot = FUN_10042140(index, "EEQ_20d.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 53:
            slot = FUN_10042140(index, "EEQ_20bd.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 54:
            if (hand != 0)
                slot = FUN_10042140(index, "EEQ_21td.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            else
                slot = FUN_10042140(index, "EEQ_21d.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 55:
            if (hand != 0)
                slot = FUN_10042140(index, "EEQ21btd.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            else
                slot = FUN_10042140(index, "EEQ_21bd.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 56:
            slot = FUN_10042140(index, "EEQ_22d.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 57:
            slot = FUN_10042140(index, "EEQ_22bd.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 58:
            slot = FUN_10042140(index, "EEQ_23d.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            DAT_10db6ee8[index].mode = 1;
            DAT_10db6ee8[index].m_0267 = 0;
            break;
        case 59:
            slot = FUN_10042140(index, "EEQ_23bd.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            DAT_10db6ee8[index].mode = 1;
            DAT_10db6ee8[index].m_0267 = 0;
            break;
        case 60:
            if (hand != 0)
                slot = FUN_10042140(index, "EEQ_24ld.it3", 35, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            else
                slot = FUN_10042140(index, "EEQ_24rd.it3", 35, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        case 61:
            slot = FUN_10042140(index, "EEQ_24bd.it3", 20, &position, &matrix, 0, DROP_FORCE_HARD, 0);
            break;
        }
        if (slot > -1)
        {
            DAT_1021d018[slot] = -index;
            if (DAT_10446e00[slot] != 0)
                DAT_10446e00[slot]->RestartColourPlugs("stop", DAT_104bf8a4);
            if (DAT_1030b668[index] == BASE_KIND_PAIRED_WEAPONS)
            {
                if ((DAT_1021bdf8[index][2 + hand] = DAT_10446e00[slot]) != 0)
                    DAT_1020b430[index].values[1 + hand] = (hand * 100 + weapon) * 1000 + slot;
                else
                    DAT_1020b430[index].values[1 + hand] = WEAPON_MARK_NONE;
            }
            if (DAT_10446e00[slot] != 0)
                DAT_1042db08[slot] = hand * 1000 + weapon;
            else
                DAT_10446e00[slot] = 0;
        }
        else if (DAT_1030b668[index] == BASE_KIND_PAIRED_WEAPONS)
            DAT_1020b430[index].values[1 + hand] = WEAPON_MARK_NONE;
        if (scatter != 0)
        {
            DAT_10db6ee8[index].targetHeading = savedHeading;
            DAT_10db6ee8[index].heading = savedHeading;
        }
        if (hand == 0 || bothHands != 0)
            weapon = 0;
    }
    DAT_10219e40[index] = 0xf;
    int shownWeapon;
    if (DAT_1030b668[index] == BASE_KIND_PAIRED_WEAPONS)
    {
        if (scatter != 0)
            DAT_10daaa80.RemoveOwnedBy(index + EFFECT_OWNER_BASE);
        Node *left = DAT_10db6ee8[index].root->Find("@hidaritesou", 0);
        Node *right = DAT_10db6ee8[index].root->Find("@migitesou", 0);
        if (left == 0 && right == 0)
            shownWeapon = 0;
        else
            shownWeapon = weapon;
    }
    else
        shownWeapon = weapon;
    DAT_102548c0[index] = ((shownWeapon * EQUIP_FIELD_UNIT + mount) * EQUIP_FIELD_UNIT + clothes) * EQUIP_FIELD_UNIT + head;
    FUN_100404e0(index, mount, clothes, head, shownWeapon, 1, hand);
    if (DAT_10daaae8[index].kind != CHAR_KIND_ENEMY)
        FUN_1001b0a0(index, hand, 0);
    return 0;
}
