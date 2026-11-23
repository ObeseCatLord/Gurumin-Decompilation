// Frozen water: maps with a water surface can freeze it (the map setup at 0x100fc4a0 finds the nodes and creates the
// "ICERIPA" child). Node flag 0x20 makes a node solid (see wall_flags.cpp).

#include "node.h"

extern "C" Node *DAT_104c007c;   // the water surface node
extern "C" Node *DAT_104c0084;   // "ICERIPA": the ice surface added below the water node
extern "C" Node *DAT_104c0080;   // second water node (found next to the surface)
extern "C" char DAT_104c02dc;    // 1 while the water is frozen
extern "C" float DAT_104bfe24;   // height of the ice surface
extern "C" float DAT_10db9248;   // the player's height (character 0, position z)

// 0x10011fb0 (Water_SetFrozen). Freezes (`frozen` != 0) or thaws the water of the current map, when the map has
// all three water nodes. Freezing lifts the player onto the ice when below it and switches the node flags so the
// ice is solid; thawing switches them back. Returns 0.
extern "C" int FUN_10011fb0(int frozen)
{
    if (DAT_104c007c != 0 && DAT_104c0084 != 0 && DAT_104c0080 != 0)
    {
        if (frozen != 0)
        {
            if (DAT_10db9248 < DAT_104bfe24)
                DAT_10db9248 = DAT_104bfe24;
            DAT_104c02dc = 1;
            DAT_104c007c->SetFlags(0x20);
            DAT_104c0080->ClearFlags(0x20);
            DAT_104c0080->SetFlags(0x20);
            DAT_104c0080->ClearFlags(0x20);
            DAT_104c0080->children[0]->ClearFlags(0x20);
            DAT_104c0084->SetFlags(0x4000004);
            DAT_104c0080->SetFlags(2);
            return 0;
        }
        DAT_104c02dc = 0;
        DAT_104c0080->SetFlags(0x20);
        DAT_104c0084->ClearFlags(4);
        DAT_104c007c->ClearFlags(0x20);
    }
    return 0;
}
