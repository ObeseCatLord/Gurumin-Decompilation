// The clear-marker nodes of the map: for every map record whose frame lies on a walkable stretch of the camera
// path, a node under "ClearORoot" that shows the state of the map (cleared, new or master) at that point.

#include <stdio.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"

#include "key_track.h"

#include "node.h"
#include "node_data.h"

#include "map_records.h"
#include "map_vars.h"
#include "render_vars.h"
#include "game_vars.h"
#include "ui_vars.h"

extern "C" const char *FUN_1013f8a0(const char *name, int folder);
extern "C" int FUN_10021380(int map);            // Map_SetEntryFlags
extern "C" Node *FUN_10018100(const Vec4 *point);  // Map_FindRegionAt

Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360

// Script flag `index` (as Game::TestScriptFlag does it): a negative index tests that the flag is clear, flag 0 is
// always set and indexes past 3999 read as clear (or as set for the negated test).
inline int ScriptFlag(int index)
{
    if (index < 0)
    {
        index = -index;
        if (index > 3999)
            return 1;
        return DAT_10c62edc[index] == 0;
    }
    if (index > 3999)
        return 0;
    if (index == 0)
        return 1;
    return DAT_10c62edc[index];
}

#define MAP_ENTRY_ICON_FLAG 0x453   // script flag: base of the two flags that say how far the map is cleared
#define LAST_FADING_NODE 0x5f      // index of the last of the 0x60 fading node slots
#define MARKER_REACH 100.0f         // longest camera path segment a record can lie on

// 0x1004cf20 (ClearMarkers_Create). Replaces the "ClearORoot" node of the map with an empty one and, for every map
// record of an entered map whose frame lies between two camera path keys that both have ground, adds a child
// "o_cnode%02d" (at most 0x60 fading nodes) holding the model of its state at the record's position on the path:
// "O_master.it3" when the clear flag (the entry flag's value) is above 1, "O_clear.it3" for another non-zero value
// and "O_new.it3" when it is 0 and the second flag (500 further on) is clear. The models join the fading list
// (render_vars.h) with the map region they lie in. Returns 0.
int ClearMarkers_Create()
{
    Vec4 position;
    if (DAT_10255f08 != 0)
    {
        Node *old = DAT_10255f08->Find("ClearORoot", 0);
        if (old != 0)
            old->parent->RemoveChild(old, 0);
        Node *root = DAT_10255f08->AddChild(0, 0x40);
        strcpy(root->name, "ClearORoot");
        Node *marker = root;
        for (int index = 0; index < DAT_10216dc4; index++)
        {
            if (FUN_10021380(DAT_10210fc0[index].entry.id) == 0)
                continue;
            char *ground = DAT_104c0394;
            int cleared = ScriptFlag(DAT_10c62edc[MAP_ENTRY_ICON_FLAG]);
            int second = ScriptFlag(DAT_10c62edc[MAP_ENTRY_ICON_FLAG] + 500);
            for (int key = 0; key < DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION].count - 1; key++)
            {
                if (ground == 0)
                    continue;
                if (DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION].frames[key] > DAT_10210fc0[index].camera.frame ||
                    DAT_104c03ac->extra->tracks[NODE_TRACK_TRANSLATION].frames[key + 1] < DAT_10210fc0[index].camera.frame ||
                    ground[key] != 0 || ground[key + 1] != 0)
                    continue;
                Vec4 from;
                Vec4 to;
                NodeExtra *path = DAT_104c03ac->extra;
                from.Copy(path->AnimatedMatrix(path->tracks[NODE_TRACK_TRANSLATION].frames[key]).GetTranslation());
                path = DAT_104c03ac->extra;
                to.Copy(path->AnimatedMatrix(path->tracks[NODE_TRACK_TRANSLATION].frames[key] + 1).GetTranslation());
                if ((from - to).Length() > MARKER_REACH)
                    continue;
                position.Copy(DAT_104c03ac->extra->AnimatedMatrix(DAT_10210fc0[index].camera.frame).GetTranslation());
                if (cleared > 0 || second == 0)
                {
                    if (DAT_104c03b0 > LAST_FADING_NODE)
                        break;
                    marker = root->AddChild(0, 0x40);
                    sprintf(marker->name, "o_cnode%02d", DAT_104c03b0);
                    marker->parent = root;
                    marker->matrix.SetIdentity();
                }
                Node *model;
                if (cleared > 1)
                {
                    model = marker->AddChild(0, 0x40);
                    model->Load(FUN_1013f8a0("O_master.it3", 0), 0, 0, 0);
                    model->parent = marker;
                    model->matrix = TranslationMatrix(position.x, position.y, position.z);
                }
                else if (cleared != 0)
                {
                    model = marker->AddChild(0, 0x40);
                    model->Load(FUN_1013f8a0("O_clear.it3", 0), 0, 0, 0);
                    model->parent = marker;
                    model->matrix = TranslationMatrix(position.x, position.y, position.z);
                }
                else if (second == 0)
                {
                    model = marker->AddChild(0, 0x40);
                    model->Load(FUN_1013f8a0("O_new.it3", 0), 0, 0, 0);
                    model->parent = marker;
                    model->matrix = TranslationMatrix(position.x, position.y, position.z);
                }
                else
                    break;
                if (cleared > 0 || second == 0)
                {
                    if (DAT_104c03b0 <= LAST_FADING_NODE)
                    {
                        DAT_1021a1e0[DAT_104c03b0] = model;
                        DAT_10254ac0[DAT_104c03b0] = 0;
                        if (DAT_10255f08->visibilityCells != 0)
                            DAT_10254ac0[DAT_104c03b0] = FUN_10018100(&position);
                        DAT_104c03b0++;
                    }
                }
                break;
            }
        }
        root->BuildMesh(1, 0);
        root->Rebuild();
    }
    return 0;
}
