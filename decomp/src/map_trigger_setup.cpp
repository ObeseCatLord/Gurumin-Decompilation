// Setting up the objects a map's triggers name. The map setup (Map_SetUpFromEntry, Map_SetUpByNumber) hands every
// active trigger of the map to Map_SetUpTrigger, which reads the trigger's name ("FLAG", "BOBJ", "LIFT", "ADOOR",
// "LEVER", ...) and its numbers and sets up what it names: script flags, snow, camera jump points, nodes hung on the
// map's model, effects, and the placed objects (platforms, doors, levers, buttons, rocks, ...) that live in the
// character table.

#include <stdio.h>
#include <string.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "mesh.h"
#include "anim_model.h"
#include "game.h"
#include "gimmick_slot.h"
#include "effect_holds.h"
#include "visibility.h"
#include "map_records.h"

#include "char_slot.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_setup.h"
#include "map_vars.h"
#include "ground_height.h"
#include "ground_mesh.h"
#include "world.h"
#include "map_trigger_setup.h"

extern "C" int FUN_1000ed70(const char *text, const char *subtext, int frames);   // opens the message window
extern "C" const char *FUN_1013f8a0(const char *name, int folder);                // builds the data path of `name`
extern "C" int FUN_1013f880(void);                                                // the game's random number
extern "C" int FUN_10017b80(void);                                                // first free effect slot
extern "C" int FUN_10017c00(const char *name, int x, int y, int z);               // loads a model into an object slot
extern "C" int FUN_1003e650(int value, int x, int y, int z, const char *name, int id);   // starts a type-14 effect
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);       // plays a sound effect
extern "C" int FUN_100143c0(int index);
extern "C" void FUN_100169c0(int slot, int style);

Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360
Matrix4 RotationZ(float angle);                         // 0x101b1b40

#define DEGREES_TO_RADIANS 0.017452778f       // 3.1415 / 180
#define PARENT_TABLE_LIMIT 0x20               // nodes the map can carry (DAT_1021b548)
#define NODE_NAME_CAPACITY 0x40               // second argument of AddChild
#define ANY_NODE_MASK (-1)                    // mask of Node::M_10194630 that matches every node
#define COLLISION_BOX_MASK 0x200              // NODE_FLAG_SHAPE_BOX: nodes drawn as their box in the debug pass
#define COLLISION_BOX_NODES 3                 // collision boxes searched per door or platform
#define NAME_BUFFER_SIZE 0x40
#define TILE_NAME_SIZE 0x100
#define SCRIPT_FLAG_LAST 3999                 // last script flag
#define SCRIPT_FLAG_ARMED 1                   // low bit of an armed trigger flag
#define EFFECT_NOT_PLAYING (-11)              // DAT_10447338 value of a sound that is not playing
#define TRUCK_START_X (-50000.0f)             // where the "TRUCK" node waits
#define ENVIRONMENT_SOUND_VOLUME 1.3f
#define OBJECT_HIDDEN_HEIGHT (-1000.0f)       // AnimModel::m_0068 of the platforms and switches

#define MODEL_ICE_PLATFORM 0x33               // models whose default colour follows the map
#define MODEL_ICE_PLATFORM_B 0xc5

#define SLOT_KIND_FLOAT 0x42
#define SLOT_KIND_FLOAT2 0x45
#define SLOT_KIND_FIRE 0x60
#define SLOT_KIND_LIFT 0x4d
#define SLOT_KIND_LIFT2 0x4e
#define SLOT_KIND_LIFT3 0x4f
#define SLOT_KIND_LIFT4 0x46
#define SLOT_KIND_STUBO 0x7a
#define SLOT_KIND_TUBO 0x50
#define SLOT_KIND_HOUDAI 0x2c
#define SLOT_KIND_ROCK 0x51
#define SLOT_KIND_PCON 5
#define SLOT_KIND_NCON 0x5f
#define SLOT_KIND_TCON 6
#define SLOT_KIND_ADOOR 0x13
#define SLOT_KIND_ADOOR1 0x14
#define SLOT_KIND_ABOX 0xf
#define SLOT_KIND_LEVER 0x16
#define SLOT_KIND_BUTTON 0x5c

#define GIMMICK_TYPE_CHEST 10
#define GIMMICK_TYPE_LEVER 0xb
#define GIMMICK_TYPE_BUTTON 0xf

#define SNAP_STEP 75.0f                       // AnimModel::SnapPosition step of the connectors
#define RENDER_MODE_CONNECTOR 0x27

// Arms script flag `flag` (when it is not set yet) with the two numbers of a trigger.
static __forceinline void ArmScriptFlag(int flag, int high, int low)
{
    if (DAT_10c62edc[flag] == 0)
        DAT_10c62edc[flag] = ((high << 8) | low) << 5 | SCRIPT_FLAG_ARMED;
}

// Turns the box of collision node `node` with the object's heading and re-fits the box around its two corners.
static __forceinline void TurnCollisionBox(Node *node, float heading)
{
    Vec4 corner[2];
    corner[0].Copy(node->box[NODE_BOX_MIN]);
    corner[0].RotateZ(heading);
    corner[1].Copy(node->box[NODE_BOX_MAX]);
    corner[1].RotateZ(heading);
    node->box[NODE_BOX_MIN].x = corner[0].x > corner[1].x ? corner[1].x : corner[0].x;
    node->box[NODE_BOX_MIN].y = corner[0].y > corner[1].y ? corner[1].y : corner[0].y;
    node->box[NODE_BOX_MIN].z = corner[0].z > corner[1].z ? corner[1].z : corner[0].z;
    node->box[NODE_BOX_MAX].x = corner[0].x < corner[1].x ? corner[1].x : corner[0].x;
    node->box[NODE_BOX_MAX].y = corner[0].y < corner[1].y ? corner[1].y : corner[0].y;
    node->box[NODE_BOX_MAX].z = corner[0].z < corner[1].z ? corner[1].z : corner[0].z;
}

// Turns the collision boxes (the first three nodes marked as boxes) of object `slot` with its heading.
static __forceinline void TurnObjectBoxes(int slot)
{
    for (int i = 0; i < COLLISION_BOX_NODES; i++)
    {
        Node *box = DAT_10db6ee8[slot].root->FindByFlags(COLLISION_BOX_MASK, i, 0);
        if (box != 0)
            TurnCollisionBox(box, DAT_10db6ee8[slot].targetHeading);
    }
}

// Sets `tint` to the colour the current map gives the object of model `model`: white, dimmed or warmed in a few
// maps, more when the lighting variant DAT_10c655e0 is on.
static __forceinline void SetMapTint(Vec4 &tint, int model)
{
    tint.Fill(1.0f);
    if (DAT_10c62ed0 == 0x66 || DAT_10c62ed0 == 0x67 || DAT_10c62ed0 == 0x68)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(1.0f, 1.0f, 0.7f, 1.0f);
        else
            tint.Set(0.8f, 0.9f, 1.0f, 1.0f);
    }
    if (DAT_10c62ed0 == 0x6f || DAT_10c62ed0 == 0x70 || DAT_10c62ed0 == 0x71)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(1.0f, 1.0f, 0.85f, 1.0f);
        else
            tint.Set(0.85f, 0.9f, 1.0f, 1.0f);
    }
    if (DAT_10c62ed0 == 0x1f5 || DAT_10c62ed0 == 0x1f6)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(1.0f, 1.0f, 0.85f, 1.0f);
        else
            tint.Set(0.85f, 0.9f, 1.0f, 1.0f);
    }
    if (DAT_10c62ed0 == 0x1f7 || DAT_10c62ed0 == 0x1f8)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(1.0f, 1.0f, 0.8f, 1.0f);
        else
            tint.Set(0.7f, 0.8f, 1.0f, 1.0f);
    }
    if (DAT_10c62ed0 == 0xc9 || DAT_10c62ed0 == 0xca)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(0.7f, 0.8f, 0.7f, 1.0f);
        else
            tint.Set(model == MODEL_ICE_PLATFORM_B ? 0.5f : 0.4f, 0.75f, 0.7f, 1.0f);
    }
    if (DAT_10c62ed0 == 0xcb || DAT_10c62ed0 == 0xcc || DAT_10c62ed0 == 0xcd)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(0.7f, 0.8f, 0.7f, 1.0f);
        else
            tint.Set(model == MODEL_ICE_PLATFORM_B ? 0.5f : 0.3f, 0.6f, 0.7f, 1.0f);
    }
    if (DAT_10c62ed0 == 0x259 || DAT_10c62ed0 == 0x25a || DAT_10c62ed0 == 0x25b || DAT_10c62ed0 == 0x25c ||
        DAT_10c62ed0 == 0x25d)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(1.0f, 1.0f, 1.0f, 1.0f);
        else
            tint.Set(0.6f, 0.8f, 1.0f, 1.0f);
    }
    if (DAT_10c62ed0 == 0x12d || DAT_10c62ed0 == 0x12e || DAT_10c62ed0 == 0x12f || DAT_10c62ed0 == 0x130 ||
        DAT_10c62ed0 == 0x131)
    {
        if (DAT_10c655e0 == 0)
            tint.Set(0.8f, 0.8f, 1.0f, 1.0f);
        else
            tint.Set(0.5f, 0.6f, 0.8f, 1.0f);
    }
}

// Loads model file `file` into `preload` and frees it again, which leaves it in the file cache.
static __forceinline void CacheModel(Node &preload, const char *file)
{
    preload.Load(FUN_1013f8a0(file, 0), 0, 0, 0);
    preload.Release();
}

// Gives `node` the visibility region (the map root keeps the region boxes in `visibilityCells`) that contains the
// position of `matrix`.
static __forceinline void TagNodeRegion(Node *node, Matrix4 *matrix)
{
    VisData *visibility = DAT_10255f08->visibilityCells;
    if (visibility != 0)
        node->TagRegion(visibility->boxes, (const char (*)[NODE_NAME_SIZE])visibility, matrix);
}

// 0x10058790 (Map_SetUpTrigger). Sets up the object named by map trigger `trigger`. The name picks the kind; the
// numbers are in trigger->values (per kind: model, x, y, z, heading, script flag, ...), the strings in
// trigger->labels (node names, motion names). Kinds: FLAG (set a script flag), SNOW, OTI (a marker position), REG
// (a route point), KSKK (a lever node), BOBJ/BOBJ2/OBJ/OBJ2 (models hung on the map), DAO/DAO2 and EFFECT* (effects),
// BOX (a chest), EVENT, ICON, IWA, TRUCK, HORI (ground objects), FLOAT/LIFT*/FIRE/STUBO/TUBO/HOUDAI/ROCK (moving
// objects), ENVSE (looping sound), SETJUMP (camera jump points), PCON/NCON/TCON (connectors), ADOOR/ADOOR1 (doors),
// ABOX (chest), LEVER and BUTTON (switches); a leading "!" is a variant. Returns 0.
int Map_SetUpTrigger(GameTrigger *trigger)
{

    if (strcmp(trigger->name, "FLAG") == 0)
    {
        int flag = trigger->values[0];
        int value = trigger->values[1];
        if (flag <= SCRIPT_FLAG_LAST && flag != 0)
            DAT_10c62edc[flag] = value;
    }
    if (strcmp(trigger->name, "SNOW") == 0)
    {
        DAT_104c0224 = (char)trigger->values[0];
        DAT_1083631c.x = (float)trigger->values[1];
        DAT_1083631c.y = (float)trigger->values[2];
        DAT_1083631c.z = (float)trigger->values[3];
        DAT_1083631c.w = (float)trigger->values[4];
    }
    if (strcmp(trigger->name, "OTI") == 0)
    {
        DAT_1030c1f8[(unsigned char)DAT_104c01f4] = trigger->values[0];
        int index = (unsigned char)DAT_104c01f4;
        DAT_10daa400[index].x = (float)trigger->values[1];
        DAT_10daa400[index].y = (float)trigger->values[2];
        DAT_10daa400[index].z = (float)trigger->values[3];
        DAT_104c01f4++;
    }
    if (strcmp(trigger->name, "REG") == 0)
    {
        DAT_10210fc0[trigger->values[0]].points[trigger->values[1]].x = trigger->values[2];
        DAT_10210fc0[trigger->values[0]].points[trigger->values[1]].y = trigger->values[3];
        DAT_10210fc0[trigger->values[0]].points[trigger->values[1]].z = trigger->values[4];
        DAT_10210fc0[trigger->values[0]].points[trigger->values[1]].code = trigger->values[5];
    }
    if (strcmp(trigger->name, "KSKK") == 0)
    {
        DAT_1042da88[trigger->values[0]] = (short)trigger->values[1];
        DAT_104678a0[trigger->values[0]] = 0;
        if (trigger->values[2] > 0)
        {
            DAT_104678a0[trigger->values[0]] = (short)trigger->values[2];
            if (DAT_10874848.TestScriptFlag(trigger->values[2]) != 0 && DAT_1020af20[trigger->values[0]] != 0)
            {
                int lever = trigger->values[0];
                char name[NAME_BUFFER_SIZE];
                Vec4 position = DAT_1020af20[lever]->matrix.GetTranslation();
                sprintf(name, "@ksk%02d", lever);
                for (int i = 0; i < 6; i++)
                {
                    Node *part = (Node *)DAT_10255f08->M_1019ed10(name, i);
                    if (part == 0)
                        break;
                    part->SetFlags(NODE_FLAG_HIDDEN);
                    part->ClearFlags(4);
                }
                DAT_1020af20[lever] = 0;
                DAT_1021fa40[lever] = 0;
            }
        }
    }
    if (strcmp(trigger->name, "BOBJ") == 0)
    {
        if (DAT_104bfdc4 > PARENT_TABLE_LIMIT - 1)
        {
            DAT_104bfd4c = 0x50;
            DAT_104c01fc = 0;
            DAT_104bf8c8 = 1;
            DAT_104bf8cc = 0;
            DAT_104bf8d0 = 0;
            DAT_10467ea0[0] = 0;
            strcpy(DAT_10219bc0, DAT_101d76d0);
            return 0;
        }
        DAT_10467e20[DAT_104bfdc4] = DAT_10255f08;
        Node *node = DAT_10255f08->AddChild(0, NODE_NAME_CAPACITY);
        node->Load(FUN_1013f8a0(DAT_10894698[trigger->values[3]], 0), 0, 0, 0);
        node->Rebuild();
        node->BuildMesh(1, 0);
        node->matrix = RotationZ((float)trigger->values[4] * 3.1415f / 180.0f) *
                       TranslationMatrix((float)trigger->values[0], (float)trigger->values[1],
                                         (float)trigger->values[2]);
        if (_stricmp(node->name, "healmacha") == 0)
            DAT_104c00d8 = (int)node;
        TagNodeRegion(node, &node->matrix);
        DAT_1021b548[DAT_104bfdc4] = node;
        DAT_104bfdc4++;
    }
    if (strcmp(trigger->name, "BOBJ2") == 0)
    {
        if (DAT_104bfdc4 > PARENT_TABLE_LIMIT - 1)
            goto TooManyNodes;
        Node *parent = DAT_10255f08->Find(trigger->labels[0], 0);
        if (parent != 0)
        {
            DAT_10467e20[DAT_104bfdc4] = parent;
            Node *node = parent->AddChild(0, NODE_NAME_CAPACITY);
            node->Load(FUN_1013f8a0(DAT_10894698[trigger->values[1]], 0), 0, 0, 0);
            node->Rebuild();
            node->BuildMesh(1, 0);
            if (_stricmp(node->name, "healmacha") == 0)
                DAT_104c00d8 = (int)node;
            TagNodeRegion(node, &parent->matrix);
            DAT_1021b548[DAT_104bfdc4] = node;
            DAT_104bfdc4++;
        }
    }
    if (strcmp(trigger->name, "OBJ") == 0)
    {
        if (DAT_104bfdc4 > PARENT_TABLE_LIMIT - 1)
            goto TooManyNodes;
        DAT_10467e20[DAT_104bfdc4] = DAT_10255f08;
        Node *node = DAT_10255f08->AddChild(0, NODE_NAME_CAPACITY);
        node->Load(FUN_1013f8a0(DAT_10894698[trigger->values[3]], 0), 0, 0, 0);
        node->BuildMesh(1, 0);
        node->matrix = RotationZ((float)trigger->values[4] * 3.1415f / 180.0f) *
                       TranslationMatrix((float)trigger->values[0], (float)trigger->values[1],
                                         (float)trigger->values[2]);
        if (_stricmp(node->name, "healmacha") == 0)
            DAT_104c00d8 = (int)node;
        TagNodeRegion(node, &node->matrix);
        DAT_1021b548[DAT_104bfdc4] = node;
        DAT_104bfdc4++;
    }
    if (strcmp(trigger->name, "OBJ2") == 0)
    {
        if (DAT_104bfdc4 > PARENT_TABLE_LIMIT - 1)
            goto TooManyNodes;
        Node *parent = DAT_10255f08->Find(trigger->labels[0], 0);
        if (parent != 0)
        {
            DAT_10467e20[DAT_104bfdc4] = parent;
            Node *node = parent->AddChild(0, NODE_NAME_CAPACITY);
            node->Load(FUN_1013f8a0(DAT_10894698[trigger->values[1]], 0), 0, 0, 0);
            node->BuildMesh(1, 0);
            if (_stricmp(node->name, "healmacha") == 0)
                DAT_104c00d8 = (int)node;
            TagNodeRegion(node, &parent->matrix);
            DAT_1021b548[DAT_104bfdc4] = node;
            DAT_104bfdc4++;
        }
    }
    if (strcmp(trigger->name, "DAO") == 0)
        FUN_1003e650(0, trigger->values[0], trigger->values[1], trigger->values[2], trigger->labels[4],
                     trigger->values[5]);
    if (strcmp(trigger->name, "DAO2") == 0)
    {
        Node *marker = DAT_10255f08->Find(trigger->labels[0], 0);
        if (marker != 0)
            FUN_1003e650(0, (int)marker->matrix.translation.x, (int)marker->matrix.translation.y,
                         (int)marker->matrix.translation.z, trigger->labels[2], trigger->values[3]);
    }
    if (strcmp(trigger->name, "BOX") == 0)
    {
        Node *chest = DAT_10255f08->Find(trigger->labels[0], 0);
        if (chest != 0)
        {
            int slot = FUN_10017b80();
            GimmickSlot &gimmick = DAT_10860cec[slot + 1];
            gimmick.node = chest;
            gimmick.owner = 0;
            gimmick.m_0018 = 0;
            gimmick.m_000c = -1;
            gimmick.type = GIMMICK_TYPE_CHEST;
            gimmick.id = trigger->values[3];
            ArmScriptFlag(gimmick.id, trigger->values[1], trigger->values[2]);
            strcpy(gimmick.motionOn, trigger->labels[4]);
            strcpy(gimmick.motionOff, trigger->labels[5]);
        }
    }
    if (strcmp(trigger->name, "EFFECTD") == 0)
    {
        DAT_1020dcf8[DAT_1020b22c].effect = trigger->values[0];
        DAT_1020dcf8[DAT_1020b22c].position[0] = trigger->values[1];
        DAT_1020dcf8[DAT_1020b22c].position[1] = trigger->values[2];
        DAT_1020dcf8[DAT_1020b22c].position[2] = trigger->values[3];
        DAT_1020dcf8[DAT_1020b22c].m_0010 = trigger->values[4];
        DAT_1020dcf8[DAT_1020b22c].age = trigger->values[4] - 1;
        DAT_1020dcf8[DAT_1020b22c].value = trigger->values[5];
        DAT_1020dcf8[DAT_1020b22c].m_001c = trigger->values[6];
        DAT_1020b22c++;
    }
    if (strcmp(trigger->name, "EFFECT") == 0)
    {
        DAT_1020dcf8[DAT_1020b22c].effect = trigger->values[0];
        DAT_1020dcf8[DAT_1020b22c].position[0] = trigger->values[1];
        DAT_1020dcf8[DAT_1020b22c].position[1] = trigger->values[2];
        DAT_1020dcf8[DAT_1020b22c].position[2] = trigger->values[3];
        DAT_1020dcf8[DAT_1020b22c].m_0010 = trigger->values[4];
        DAT_1020dcf8[DAT_1020b22c].age = trigger->values[4] - 1;
        DAT_1020dcf8[DAT_1020b22c].value = 0;
        DAT_1020dcf8[DAT_1020b22c].m_001c = 0;
        DAT_1020b22c++;
    }
    if (strcmp(trigger->name, "EFFECT2") == 0)
    {
        Node *marker = DAT_10255f08->Find(trigger->labels[1], 0);
        int hold = DAT_1020b22c;
        if (marker != 0)
        {
            DAT_1020dcf8[DAT_1020b22c].effect = trigger->values[0];
            DAT_1020dcf8[hold].position[0] = (int)marker->matrix.translation.x;
            DAT_1020dcf8[hold].position[1] = (int)marker->matrix.translation.y;
            DAT_1020dcf8[hold].position[2] = (int)marker->matrix.translation.z;
            DAT_1020dcf8[hold].m_0010 = trigger->values[2];
            DAT_1020dcf8[hold].age = trigger->values[2] - 1;
            DAT_1020b22c = hold + 1;
            DAT_1020dcf8[hold].value = 0;
            DAT_1020dcf8[hold].m_001c = 0;
        }
    }
    if (strcmp(trigger->name, "EVENT") == 0)
        DAT_10874848.RegisterEvent(trigger->values[0]);
    if (strcmp(trigger->name, "ICON") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].mode = 0;
    }
    if (strcmp(trigger->name, "IWA") == 0)
    {
        if (DAT_104bfdc4 > PARENT_TABLE_LIMIT - 1)
            goto TooManyNodes;
        DAT_10467e20[DAT_104bfdc4] = DAT_10255f08;
        Node *node = DAT_10255f08->AddChild(0, NODE_NAME_CAPACITY);
        node->Load(FUN_1013f8a0(DAT_10894698[trigger->values[0]], 0), 0, 0, 0);
        node->BuildMesh(1, 0);
        node->matrix = TranslationMatrix((float)trigger->values[1], (float)trigger->values[2],
                                         (float)trigger->values[3]);
        TagNodeRegion(node, &node->matrix);
        DAT_1021b548[DAT_104bfdc4] = node;
        DAT_104bfdc4++;
        DAT_10467208[DAT_104c0218] = &node->matrix;
        DAT_104c0218++;
    }
    if (strcmp(trigger->name, "TRUCK") == 0)
    {
        if (DAT_104bfdc4 > PARENT_TABLE_LIMIT - 1)
            goto TooManyNodes;
        DAT_10467e20[DAT_104bfdc4] = DAT_10255f08;
        Node *node = DAT_10255f08->AddChild(0, NODE_NAME_CAPACITY);
        node->Load(FUN_1013f8a0(trigger->labels[0], 0), 0, 0, 0);
        node->BuildMesh(1, 0);
        node->matrix = TranslationMatrix(TRUCK_START_X, 0, 0);
        TagNodeRegion(node, &node->matrix);
        DAT_1021b548[DAT_104bfdc4] = node;
        DAT_104bfdc4++;
        DAT_104c021c = node;
    }
    if (strcmp(trigger->name, "HORI") == 0)
    {
        Node *floors = DAT_10255f08->Find("horiyuka", 0);
        if (floors == 0)
        {
            floors = DAT_10255f08->AddChild(0, NODE_NAME_CAPACITY);
            strcpy(floors->name, "horiyuka");
            floors->flags = NODE_FLAG_NEVER_CULLED;
            floors->AllocMaterials(1);
            floors->materials[0].m_017c[0] = 1;
        }
        char name[TILE_NAME_SIZE];
        int ground = trigger->values[3];
        sprintf(name, "horiyuka%02d", ground);
        if (floors->Find(name, 0) == 0)
            floors->RemoveChild(0, 0);
        Node *tile = floors->AddChild(0, NODE_NAME_CAPACITY);
        strcpy(tile->name, name);
        tile->flags = NODE_FLAG_NEVER_CULLED;
        tile->AllocMaterials(1);
        tile->materials[0].m_017c[0] = 1;
        strcpy(tile->materials[0].textures[0], trigger->labels[5]);
        tile->M_1019b050(0x1e);
        tile->matrix.translation.x = (float)trigger->values[0];
        tile->matrix.translation.y = (float)trigger->values[1];
        tile->matrix.translation.z = (float)trigger->values[2];
        TagNodeRegion(tile, &tile->matrix);
        DAT_1021b5c8[ground] = 0;
        DAT_10253f18[ground] = 0;
        DAT_104472b0[ground] = (unsigned char)(trigger->values[4] % 10);
        DAT_1032d7a8[ground] = (short)(trigger->values[6] / 10000);
        DAT_1020d860[ground] = (GroundOrigin *)tile;
        FUN_100169c0(ground, (trigger->values[4] % 100) / 10);
        FUN_10016ec0(0, ground, 0);
    }
    if (strcmp(trigger->name, "FLOAT") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].m_0068 = OBJECT_HIDDEN_HEIGHT;
        DAT_1021f3c0[slot] = SLOT_KIND_FLOAT;
        DAT_10db6ee8[slot].root->M_10194630(slot, ANY_NODE_MASK);
        FUN_100143c0(slot);
    }
    if (strcmp(trigger->name, "FLOAT2") == 0)
    {
        if (DAT_104bfdc4 > PARENT_TABLE_LIMIT - 1)
        {
        TooManyNodes:
            FUN_1000ed70(DAT_101d76d0, 0, 0x50);
            return 0;
        }
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].m_0068 = OBJECT_HIDDEN_HEIGHT;
        DAT_1021f3c0[slot] = SLOT_KIND_FLOAT2;
        DAT_10db6ee8[slot].root->M_10194630(slot, ANY_NODE_MASK);
        DAT_10db6ee8[slot].m_0274 = 0;
        DAT_10467e20[DAT_104bfdc4] = DAT_10255f08;
        Node *platform = DAT_10255f08->AddChild(0, NODE_NAME_CAPACITY);
        platform->Load(FUN_1013f8a0("lifta.it3", 0), 0, 0, 0);
        platform->BuildMesh(1, 0);
        platform->matrix = RotationZ((float)trigger->values[4] * 3.1415f / 180.0f) *
                           TranslationMatrix((float)trigger->values[0], (float)trigger->values[1],
                                             (float)trigger->values[2]);
        DAT_1021b548[DAT_104bfdc4] = platform;
        DAT_10db6ee8[slot].bankIndex = DAT_104bfdc4;
        DAT_104bfdc4++;
    }
    if (strcmp(trigger->name, "FIRE") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].root->M_10191e50();
        DAT_1021f3c0[slot] = SLOT_KIND_FIRE;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[4];
        DAT_10daaae8[slot].switchesHeld = 0;
    }
    if (strcmp(trigger->name, "LIFT") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_1021f3c0[slot] = SLOT_KIND_LIFT;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[4];
        DAT_10db6ee8[slot].m_0068 = OBJECT_HIDDEN_HEIGHT;
        DAT_10daaae8[slot].kind = trigger->values[5];
        memset(&DAT_1020b430[slot], 0, sizeof(CharState));
        Node *cached = DAT_10255f08->AddChild(0, NODE_NAME_CAPACITY);
        cached->Load(FUN_1013f8a0("O_015k.it3", 0), 0, 0, 0);
        cached->BuildMesh(1, 0);
        DAT_1020b430[slot].values[10] = trigger->values[6];
        DAT_1020b430[slot].values[11] = trigger->values[7];
        DAT_1020b430[slot].words[14] = 0;
        DAT_10db6ee8[slot].m_0274 = 0;
        DAT_10db6ee8[slot].m_0278 = 0;
        DAT_10db6ee8[slot].m_027c = 0;
    }
    if (strcmp(trigger->name, "LIFT2") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_1021f3c0[slot] = SLOT_KIND_LIFT2;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[4];
        DAT_10db6ee8[slot].m_0068 = OBJECT_HIDDEN_HEIGHT;
        DAT_10daaae8[slot].kind = trigger->values[5];
        memset(&DAT_1020b430[slot], 0, sizeof(CharState));
        DAT_1020b430[slot].values[10] = trigger->values[6];
        DAT_1020b430[slot].values[11] = trigger->values[7];
        DAT_1020b430[slot].values[0] = 0;
        DAT_1020b430[slot].values[1] = 0;
        DAT_1020b430[slot].words[14] = 0;
    }
    if (strcmp(trigger->name, "LIFT3") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_1021f3c0[slot] = SLOT_KIND_LIFT3;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[4];
        DAT_10db6ee8[slot].m_0068 = OBJECT_HIDDEN_HEIGHT;
        DAT_10daaae8[slot].kind = trigger->values[5];
        memset(&DAT_1020b430[slot], 0, sizeof(CharState));
        DAT_1020b430[slot].values[10] = trigger->values[6];
        DAT_1020b430[slot].values[11] = trigger->values[7];
        DAT_1020b430[slot].values[0] = 0;
        DAT_1020b430[slot].values[1] = 0;
    }
    if (strcmp(trigger->name, "LIFT4") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_1021f3c0[slot] = SLOT_KIND_LIFT4;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[4];
        DAT_10daaae8[slot].kind = trigger->values[5];
        DAT_10db6ee8[slot].m_0068 = OBJECT_HIDDEN_HEIGHT;
        memset(&DAT_1020b430[slot], 0, sizeof(CharState));
        DAT_1020b430[slot].values[10] = trigger->values[6];
        DAT_1020b430[slot].values[11] = trigger->values[7];
        DAT_1020b430[slot].values[0] = 0;
        DAT_1020b430[slot].values[1] = 0;
        DAT_1020b430[slot].words[14] = 0;
    }
    if (strcmp(trigger->name, "STUBO") == 0)
    {
        int model = trigger->values[0];
        unsigned int variant = model != MODEL_ICE_PLATFORM;
        if (model == 0xdd)
            variant = 2;
        int slot = FUN_10017c00(DAT_10894698[model], trigger->values[1], trigger->values[2], trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_1021f3c0[slot] = SLOT_KIND_STUBO;
        DAT_10db6ee8[slot].m_0267 = 0;
        DAT_10db6ee8[slot].root->M_10194630(slot, ANY_NODE_MASK);
        DAT_10db6ee8[slot].m_0278 = 0;
        DAT_10db6ee8[slot].m_027c = 0;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        DAT_10db6ee8[slot].m_0280 = variant;
        ArmScriptFlag(trigger->values[6], trigger->values[4], trigger->values[5]);
    }
    if (strcmp(trigger->name, "TUBO") == 0)
    {
        int model = trigger->values[0] % 1000;
        int variant = 0;
        if (model == 253)
        {
            variant = 2;
            Node preload;
            CacheModel(preload, "O_070br.it3");
        }
        else if (model == 254)
        {
            variant = 3;
            Node preload;
            CacheModel(preload, "O_071br.it3");
        }
        else if (model == 112)
        {
            Node preload;
            CacheModel(preload, "EEQ_01d.it3");
            CacheModel(preload, "EEQ_04d.it3");
            CacheModel(preload, "EEQ_05d.it3");
            CacheModel(preload, "EEQ_06d.it3");
            CacheModel(preload, "EEQ_12d.it3");
            CacheModel(preload, "EEQ_13d.it3");
            CacheModel(preload, "EEQ_13bd.it3");
            CacheModel(preload, "EEQ_04bd.it3");
            CacheModel(preload, "EEQ_05bd.it3");
            CacheModel(preload, "EEQ_06bd.it3");
            CacheModel(preload, "O_075br.it3");
            variant = 4;
        }
        else if (model == 113)
        {
            Node preload;
            CacheModel(preload, "EEQ_01d.it3");
            CacheModel(preload, "EEQ_04d.it3");
            CacheModel(preload, "EEQ_05d.it3");
            CacheModel(preload, "EEQ_06d.it3");
            CacheModel(preload, "EEQ_12d.it3");
            CacheModel(preload, "EEQ_13d.it3");
            CacheModel(preload, "EEQ_13bd.it3");
            CacheModel(preload, "EEQ_04bd.it3");
            CacheModel(preload, "EEQ_05bd.it3");
            CacheModel(preload, "EEQ_06bd.it3");
            CacheModel(preload, "O_075mbr.it3");
            variant = 5;
        }
        else if (model == MODEL_ICE_PLATFORM_B)
        {
            variant = 1;
            Node preload;
            CacheModel(preload, "O_014mb.it3");
        }
        else
        {
            Node preload;
            CacheModel(preload, "O_014b.it3");
        }
        int slot = FUN_10017c00(DAT_10894698[model], trigger->values[1], trigger->values[2], trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_1021f3c0[slot] = SLOT_KIND_TUBO;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].root->M_10194630(slot, ANY_NODE_MASK);
        DAT_10db6ee8[slot].m_0278 = 0;
        DAT_10db6ee8[slot].m_027c = 0;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        DAT_10db6ee8[slot].m_0280 = variant;
        float heading = (float)trigger->values[7] * 3.1415f / 180.0f;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        int red = trigger->values[0] / 10000000;
        int green = (trigger->values[0] % 10000000) / 1000000;
        int blue = (trigger->values[0] % 1000000) / 100000;
        bool unset = false;
        if (red == 0 && green == 0 && blue == 0)
        {
            red = 9;
            green = 9;
            blue = 9;
            unset = true;
        }
        if (unset && (model == MODEL_ICE_PLATFORM || model == MODEL_ICE_PLATFORM_B))
        {
            Vec4 tint;
            SetMapTint(tint, model);
            DAT_10db6ee8[slot].scale.Set(1.0f, 1.0f, 1.0f, 1.0f);
            DAT_10db6ee8[slot].scale.Copy(tint * DAT_10db6ee8[slot].scale);
        }
        else
            DAT_10db6ee8[slot].scale.Set((float)red / 9.0f, (float)green / 9.0f, (float)blue / 9.0f, 1.0f);
        ArmScriptFlag(trigger->values[6], trigger->values[4], trigger->values[5]);
    }
    if (strcmp(trigger->name, "HOUDAI") == 0 && DAT_10874848.TestScriptFlag(trigger->values[6]) != 0x29a)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[4]], trigger->values[0], trigger->values[1],
                                trigger->values[2]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_1021f3c0[slot] = SLOT_KIND_HOUDAI;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].root->M_10194630(slot, ANY_NODE_MASK);
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        DAT_10db6ee8[slot].m_027c = 0;
        DAT_10db6ee8[slot].m_0278 = 0;
        DAT_10db6ee8[slot].m_0284 = 0;
        DAT_10db6ee8[slot].m_028c = 4;
        DAT_10db6ee8[slot].m_0274 = trigger->values[5];
        float heading = (float)trigger->values[3] * DEGREES_TO_RADIANS;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        TurnObjectBoxes(slot);
    }
    if (strcmp(trigger->name, "HOUDAI2") == 0 && DAT_10874848.TestScriptFlag(trigger->values[6]) != 0x29a)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[4]], trigger->values[0], trigger->values[1],
                                trigger->values[2]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_1021f3c0[slot] = SLOT_KIND_HOUDAI;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].root->M_10194630(slot, ANY_NODE_MASK);
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        DAT_10db6ee8[slot].m_027c = 0;
        DAT_10db6ee8[slot].m_0278 = 0;
        DAT_10db6ee8[slot].m_0284 = 1;
        DAT_10db6ee8[slot].m_028c = 4;
        DAT_10db6ee8[slot].m_0274 = trigger->values[5];
        float heading = (float)trigger->values[3] * DEGREES_TO_RADIANS;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        TurnObjectBoxes(slot);
    }
    if (strcmp(trigger->name, "ROCK") == 0 && DAT_10874848.TestScriptFlag(trigger->values[6]) != 0x29a)
    {
        int model = trigger->values[0] % 1000;
        int slot = FUN_10017c00(DAT_10894698[model], trigger->values[1], trigger->values[2], trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_1021f3c0[slot] = SLOT_KIND_ROCK;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].root->M_10194630(slot, ANY_NODE_MASK);
        DAT_10db6ee8[slot].m_027c = 0;
        DAT_10db6ee8[slot].m_0278 = 0;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        DAT_10db6ee8[slot].m_028c = 4;
        DAT_10db6ee8[slot].m_0284 = trigger->values[7];
        DAT_10db6ee8[slot].m_0274 = trigger->values[4];
        int red = trigger->values[0] / 10000000;
        int green = (trigger->values[0] % 10000000) / 1000000;
        int blue = (trigger->values[0] % 1000000) / 100000;
        bool unset = false;
        if (red == 0 && green == 0 && blue == 0)
        {
            red = 9;
            green = 9;
            blue = 9;
            unset = true;
        }
        float heading = (float)(trigger->values[5] % 1000) * DEGREES_TO_RADIANS;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        if (unset && (model == 15 || model == 163 || model == 164))
        {
            Vec4 tint;
            SetMapTint(tint, model);
            DAT_10db6ee8[slot].scale.Set((float)red / 9.0f, (float)green / 9.0f, (float)blue / 9.0f, 1.0f);
            DAT_10db6ee8[slot].scale.Copy(tint * DAT_10db6ee8[slot].scale);
        }
        else
            DAT_10db6ee8[slot].scale.Set((float)red / 9.0f, (float)green / 9.0f, (float)blue / 9.0f, 1.0f);
        TurnObjectBoxes(slot);
    }
    if (strcmp(trigger->name, "ENVSE") == 0)
    {
        Vec4 position;
        position.x = (float)trigger->values[1];
        position.y = (float)trigger->values[2];
        position.z = (float)trigger->values[3];
        DAT_10447338[trigger->values[0]] = EFFECT_NOT_PLAYING;
        FUN_1003ad20(trigger->values[0], &position, ENVIRONMENT_SOUND_VOLUME);
    }
    if (strcmp(trigger->name, "ENVSE2") == 0)
    {
        Vec4 position;
        Node *marker = DAT_10255f08->Find(trigger->labels[1], 0);
        if (marker != 0)
        {
            position.x = marker->matrix.translation.x;
            position.y = marker->matrix.translation.y;
            position.z = marker->matrix.translation.z;
            DAT_10447338[trigger->values[0]] = EFFECT_NOT_PLAYING;
            FUN_1003ad20(trigger->values[0], &position, ENVIRONMENT_SOUND_VOLUME);
        }
    }
    if (strcmp(trigger->name, "SETJUMP") == 0)
    {
        DAT_1032d5b8[DAT_1049f0f0] = (Vec3 *)&DAT_10838fc8[DAT_1049f0f0];
        DAT_10838fc8[DAT_1049f0f0].Set((float)trigger->values[0], (float)trigger->values[1],
                                       (float)trigger->values[2], 0);
        DAT_1021bbc0[DAT_1049f0f0] = (char)trigger->values[3];
        DAT_1049f0f0++;
    }
    if (strcmp(trigger->name, "SETJUMP2") == 0)
    {
        if (DAT_10db6ee8[trigger->values[0]].root == 0)
        {
            DAT_1032d5b8[DAT_1049f0f0] = &DAT_10db6ee8[trigger->values[0]].matrix.translation;
            DAT_1021bbc0[DAT_1049f0f0] = (char)trigger->values[2];
            DAT_1049f0f0++;
        }
        else
        {
            Node *jump = DAT_10db6ee8[trigger->values[0]].root->Find(trigger->labels[1], 0);
            Vec3 *target;
            if (jump == 0)
                target = &DAT_10db6ee8[trigger->values[0]].matrix.translation;
            else
                target = &jump->worldMatrix.translation;
            DAT_1032d5b8[DAT_1049f0f0] = target;
            DAT_1021bbc0[DAT_1049f0f0] = (char)trigger->values[2];
            DAT_1049f0f0++;
        }
    }
    if (strcmp(trigger->name, "PCON") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_1021f3c0[slot] = SLOT_KIND_PCON;
        DAT_10db6ee8[slot].m_0267 = 0;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        DAT_10db6ee8[slot].m_028c = 4;
        DAT_10db6ee8[slot].m_0288 = 0;
        DAT_10db6ee8[slot].m_0284 = FUN_1013f880() % 1000;
        ArmScriptFlag(trigger->values[6], trigger->values[4], trigger->values[5]);
        DAT_10db6ee8[slot].SnapPosition(SNAP_STEP, 0);
        DAT_10db6ee8[slot].root->M_1019b050(RENDER_MODE_CONNECTOR);
    }
    if (strcmp(trigger->name, "NCON") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_1021f3c0[slot] = SLOT_KIND_NCON;
        DAT_10db6ee8[slot].m_0267 = 0;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        ArmScriptFlag(trigger->values[6], trigger->values[4], trigger->values[5]);
        DAT_10db6ee8[slot].SnapPosition(SNAP_STEP, 0);
        DAT_10db6ee8[slot].root->M_1019b050(RENDER_MODE_CONNECTOR);
    }
    if (strcmp(trigger->name, "TCON") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_10db6ee8[slot].m_0267 = 0;
        DAT_10db6ee8[slot].m_02a2 = 1;
        DAT_1021f3c0[slot] = SLOT_KIND_TCON;
        DAT_10db6ee8[slot].scriptFlag = trigger->values[6];
        DAT_10db6ee8[slot].SnapPosition(SNAP_STEP, 0);
        DAT_10db6ee8[slot].root->M_1019b050(RENDER_MODE_CONNECTOR);
    }
    if (strcmp(trigger->name, "ADOOR") == 0 || strcmp(trigger->name, "!ADOOR") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_1021f3c0[slot] = SLOT_KIND_ADOOR;
        float heading = (float)trigger->values[5] * DEGREES_TO_RADIANS;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        DAT_10db6ee8[slot].m_0266 = 1;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].m_0284 = trigger->values[0];
        DAT_10db6ee8[slot].root->RestartColourPlugs(strcmp(trigger->name, "ADOOR") == 0 ? "CLOSE" : "OPEN",
                                                    DAT_104bf8a4);
        for (int i = 0; i < COLLISION_BOX_NODES; i++)
        {
            Node *box = DAT_10db6ee8[slot].root->FindByFlags(COLLISION_BOX_MASK, i, 0);
            if (box != 0)
            {
                TurnCollisionBox(box, DAT_10db6ee8[slot].targetHeading);
                box->M_10194630(i == 0 ? slot : 0, ANY_NODE_MASK);
                box->M_101946b0(0xff, ANY_NODE_MASK);
                box->M_10194730(0x4f, ANY_NODE_MASK);
            }
        }
        DAT_10219ec0[slot] = (short)trigger->values[4];
        DAT_1020b128[slot] = 0;
        DAT_1021b3c8[slot] = 0;
        DAT_102191c0[slot] = (short)trigger->values[6];
        DAT_1032d4b8[slot] = (short)trigger->values[7];
    }
    if (strcmp(trigger->name, "ADOOR1") == 0 || strcmp(trigger->name, "!ADOOR1") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_1021f3c0[slot] = SLOT_KIND_ADOOR1;
        float heading = (float)trigger->values[5] * DEGREES_TO_RADIANS;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        DAT_10db6ee8[slot].m_0266 = 1;
        DAT_10db6ee8[slot].m_17e8 = 1;
        DAT_10db6ee8[slot].m_0267 = 1;
        DAT_10db6ee8[slot].m_0284 = trigger->values[0];
        DAT_10db6ee8[slot].root->RestartColourPlugs(strcmp(trigger->name, "ADOOR1") == 0 ? "CLOSE" : "OPEN",
                                                    DAT_104bf8a4);
        for (int i = 0; i < COLLISION_BOX_NODES; i++)
        {
            Node *box = DAT_10db6ee8[slot].root->FindByFlags(COLLISION_BOX_MASK, i, 0);
            if (box != 0)
            {
                TurnCollisionBox(box, DAT_10db6ee8[slot].targetHeading);
                box->M_10194630(i == 0 ? slot : 0, ANY_NODE_MASK);
                box->M_101946b0(0xff, ANY_NODE_MASK);
                box->M_101946b0(0x4f, ANY_NODE_MASK);
            }
        }
        DAT_10219ec0[slot] = (short)trigger->values[4];
        DAT_1021b3c8[slot] = 0;
        DAT_1020b128[slot] = 0;
        DAT_102191c0[slot] = (short)trigger->values[6];
        DAT_1032d4b8[slot] = (short)trigger->values[7];
    }
    if (strcmp(trigger->name, "ABOX") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_10db6ee8[slot].m_0267 = 1;
        Node *scene = DAT_10db6ee8[slot].root->Find("\xbc\xb0\xdd \xd9\xb0\xc4", 0);
        if (scene != 0 || (scene = DAT_10db6ee8[slot].root->Find("root", 0)) != 0)
        {
            int gimmickIndex = FUN_10017b80();
            GimmickSlot &gimmick = DAT_10860cec[gimmickIndex + 1];
            gimmick.node = scene;
            DAT_1021f3c0[slot] = SLOT_KIND_ABOX;
            gimmick.owner = &DAT_10db6ee8[slot];
            gimmick.m_0018 = 0;
            gimmick.m_000c = slot;
            gimmick.type = GIMMICK_TYPE_CHEST;
            gimmick.id = trigger->values[6];
            ArmScriptFlag(trigger->values[6], trigger->values[4], trigger->values[5]);
            strcpy(gimmick.motionOn, "OPEN");
            strcpy(gimmick.motionOff, "CLOSE");
        }
        float heading = (float)trigger->values[7] * DEGREES_TO_RADIANS;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        Node *box = DAT_10db6ee8[slot].root->FindByFlags(COLLISION_BOX_MASK, 0, 0);
        if (box != 0)
            TurnCollisionBox(box, DAT_10db6ee8[slot].targetHeading);
        DAT_10db6ee8[slot].m_0266 = 1;
    }
    if (strcmp(trigger->name, "LEVER") == 0 || strcmp(trigger->name, "!LEVER") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot == 0)
            return 0;
        Node *scene = DAT_10db6ee8[slot].root->Find("\xbc\xb0\xdd \xd9\xb0\xc4", 0);
        DAT_10db6ee8[slot].m_17e8 = 2;
        DAT_10db6ee8[slot].m_0267 = 1;
        if (scene != 0 || (scene = DAT_10db6ee8[slot].root->Find("root", 0)) != 0)
        {
            int gimmickIndex = FUN_10017b80();
            GimmickSlot &gimmick = DAT_10860cec[gimmickIndex + 1];
            DAT_1021f3c0[slot] = SLOT_KIND_LEVER;
            gimmick.node = scene;
            gimmick.owner = &DAT_10db6ee8[slot];
            gimmick.m_000c = slot;
            gimmick.m_0018 = 0;
            gimmick.type = GIMMICK_TYPE_LEVER;
            gimmick.id = trigger->values[4];
            gimmick.flag = trigger->values[6];
            if (strcmp(trigger->name, "LEVER") == 0)
            {
                gimmick.targetFrame = 0;
                gimmick.frame = 0;
                scene->RestartColourPlugs("CLOSE", DAT_104bf8a4);
            }
            else
            {
                gimmick.targetFrame = 1;
                gimmick.frame = 1;
                scene->RestartColourPlugs("OPEN", DAT_104bf8a4);
            }
            strcpy(gimmick.motionOn, "OPEN0");
            strcpy(gimmick.motionOff, "CLOSE0");
        }
        float heading = (float)trigger->values[5] * DEGREES_TO_RADIANS;
        DAT_10db6ee8[slot].heading = heading;
        DAT_10db6ee8[slot].targetHeading = heading;
        Node *box = DAT_10db6ee8[slot].root->FindByFlags(COLLISION_BOX_MASK, 0, 0);
        if (box != 0)
            TurnCollisionBox(box, DAT_10db6ee8[slot].targetHeading);
        DAT_10db6ee8[slot].m_0266 = 1;
    }
    if (strcmp(trigger->name, "BUTTON") == 0 || strcmp(trigger->name, "!BUTTON") == 0)
    {
        int slot = FUN_10017c00(DAT_10894698[trigger->values[0]], trigger->values[1], trigger->values[2],
                                trigger->values[3]);
        if (slot != 0)
        {
            Node *scene = DAT_10db6ee8[slot].root->Find("\xbc\xb0\xdd \xd9\xb0\xc4", 0);
            DAT_10db6ee8[slot].m_17e8 = 1;
            DAT_10db6ee8[slot].m_0267 = 1;
            if (scene != 0 || (scene = DAT_10db6ee8[slot].root->Find("root", 0)) != 0)
            {
                int gimmickIndex = FUN_10017b80();
                GimmickSlot &gimmick = DAT_10860cec[gimmickIndex + 1];
                DAT_10db6ee8[slot].switchNode = gimmickIndex;
                DAT_1021f3c0[slot] = SLOT_KIND_BUTTON;
                gimmick.owner = &DAT_10db6ee8[slot];
                gimmick.m_000c = slot;
                gimmick.node = scene;
                gimmick.m_0018 = 0;
                gimmick.type = GIMMICK_TYPE_BUTTON;
                gimmick.id = trigger->values[4];
                gimmick.flag = trigger->values[6];
                bool pressed = strcmp(trigger->name, "BUTTON") != 0;
                gimmick.frame = pressed;
                gimmick.targetFrame = pressed;
                scene->RestartColourPlugs(pressed ? "keep-on" : "keep-off", DAT_104bf8a4);
                strcpy(gimmick.motionOn, "on");
                strcpy(gimmick.motionOff, "off");
            }
            float heading = (float)trigger->values[5] * DEGREES_TO_RADIANS;
            DAT_10db6ee8[slot].heading = heading;
            DAT_10db6ee8[slot].targetHeading = heading;
            Node *box = DAT_10db6ee8[slot].root->FindByFlags(COLLISION_BOX_MASK, 0, 0);
            if (box != 0)
                TurnCollisionBox(box, DAT_10db6ee8[slot].targetHeading);
            DAT_10db6ee8[slot].m_0266 = 1;
        }
    }
    return 0;
}
