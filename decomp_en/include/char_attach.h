// Models attached to a character's hands (EN build; JP: decomp/src/char_attach.cpp).
#ifndef GURUMIN_EN_CHAR_ATTACH_H
#define GURUMIN_EN_CHAR_ATTACH_H

// Node mode that BuildMesh and AddChild use for a hand-held model's node.
const int ATTACH_NODE_POSITION = 0x40;
// With `attach` = 1 loads model `name` as a child "scop" of the right hand of character `index` (once); otherwise
// removes that child. Returns 0 when the character has no model, else 1. EN 0x640460 (JP 0x10012450).
extern "C" int Char_AttachHandModel(int index, int attach, const char *name);
// Left-hand counterpart with the child "scop_l": an existing one is removed first. Returns 0. EN 0x6cb9f0 (JP 0x1001a760).
extern "C" int Char_AttachLeftHand(int index, int attach, const char *name);

struct Node;

// Tool modes of Char_SetHandTool: -1 toggles the drill, 1 attaches the drill, 2 the brush, anything else removes the tool.
const int HAND_TOOL_TOGGLE = -1;
const int HAND_TOOL_NONE = 0;
const int HAND_TOOL_DRILL = 1;
const int HAND_TOOL_BRUSH = 2;
// Costume number that carries the scope instead of a drill.
const int COSTUME_SCOPE = 6;
// Value of the drill-5 flag that equips drill 5.
const int DRILL5_ON = 1;
// Drill levels with their own model; every higher level uses the last one.
const int DRILL_LEVEL_1 = 1;
const int DRILL_LEVEL_2 = 2;

// (The drill level is g_guardedValue6 (EN 0x0133487c) and the costume g_guardedValue9 (EN 0x01335398), declared in anti_tamper.h.)
extern "C" int g_drill5;               // EN 0x01335414 (JP DAT_10c6566c): 1 when drill 5 is equipped
extern "C" Node *g_scopePointNode;     // EN 0x009e33c4 (JP DAT_104c00f8): node "scop-pt" of the player
extern "C" Node *g_charBaseNode;       // EN 0x009e33c8 (JP DAT_104c00fc): node "charbase0" of the player
extern "C" Node *g_scopePoint2Node;    // EN 0x009e33cc (JP DAT_104c0100): node "scop-2pt" (costume 6)
extern "C" Node *g_scopePoint3Node;    // EN 0x009e33d0 (JP DAT_104c0104): node "scop-3pt" (costume 6)

// Changes the tool in character `index`'s right hand: attaches the drill (with the drill-5 and scope variants and the
// drill motions), the brush, toggles the drill or removes the tool, then looks up the player's pointer nodes again.
// Returns 0 when the character has no model, else 1. EN 0x640520 (JP 0x10012510).
extern "C" int Char_SetHandTool(int mode, int index);
// Char_SetHandTool without drill 5 and without loading the drill motions. EN 0x6407c0 (JP 0x10012780).
extern "C" int Char_SetHandToolPlain(int mode, int index);

// Item numbers of the helmets that change the player's face and model, with the helmet kind each costume uses.
const int ITEM_HELMET_NO_FACE = 0x2a;       // hides both face meshes
const int ITEM_HELMET_COSTUME_1 = 0x38;
const int ITEM_HELMET_COSTUME_5 = 0x28;
const int ITEM_HELMET_COSTUME_2 = 0x39;
const int ITEM_HELMET_COSTUME_4 = 0x25;
const int ITEM_HELMET_COSTUME_3 = 0x3d;
// Node position of the helmet's node under the head.
const int HELMET_NODE_POSITION = 2;

extern "C" int g_helmetFaceSettings;     // EN 0x00b493f8 (JP DAT_104c02c8): face settings of the player's helmet
extern "C" unsigned char g_helmetKind;         // EN 0x00bc8872 (JP DAT_104c033a): which helmet model variant the player wears (0 = none, 6 = costume 6)
extern "C" char g_helmetOn;              // EN 0x00b91eab (JP DAT_104c0319): set to 1 when a helmet node was created

// Face settings of helmet item `item`. EN 0x646df0 (JP 0x10014d30).
extern "C" int Helmet_FaceSettings(int item);
// Puts helmet item `item` on the player (0 or less takes the helmet off; items of other types are ignored). Returns
// 1, or 0 when the player has no model or the helmet was only taken off. EN 0x646bb0 (JP 0x100154c0).
extern "C" int Char_WearHelmet(int item);
// Puts the helmet model file `model` on the head of character `index` (0 removes the helmet). EN 0x646ae0 (JP 0x100143f0).
extern "C" int Char_SetHelmetModel(int index, const char *model);

#endif
