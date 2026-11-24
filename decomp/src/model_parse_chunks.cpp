// Model files (.it3): the chunk parser that builds a node tree from a model file.

#include <stdio.h>
#include <string.h>

#include "game_file.h"

#include "node.h"
#include "mesh.h"
#include "node_data.h"
#include "node_channel.h"
#include "key_track.h"
#include "texture_ref.h"
#include "engine_memory.h"
#include "render_vars.h"
#include "texture_load_iti.h"
#include "game_load_data.h"
#include "model_load_vars.h"
#include "game_vars.h"

extern "C" int FUN_10169ab0(const char *folder, const char *name, int *freeSlot);  // motion name slot, -1 = none
extern "C" int FUN_101699d0(void);                   // a free motion name slot
extern "C" int FUN_101699f0(int slot, const char *folder, const char *name);       // names a motion slot
extern "C" int FUN_10169be0(TextureRef *texture);    // adds a reference
extern "C" int FUN_10169840(const char *folder, const char *name, int *freeSlot);  // ModelNames_Find
extern "C" int FUN_10169970(MeshData *mesh);         // MeshCache_AddRef
extern "C" int FUN_10174ce0(const char *path, int a, int b);                       // Texture_LoadIti

// Chunk tags of a model file.
#define TAG_INFO "INFO"   // a node: name, matrix, flags
#define TAG_IKNG "IKNG"   // rotation limits (saved-state block)
#define TAG_INFS "INFS"   // the file's name key check
#define TAG_RTYP "RTYP"   // render type
#define TAG_RTY2 "RTY2"   // render type, with the extra byte
#define TAG_CAME "CAME"   // camera path data
#define TAG_CDLO "CDLO"   // visibility cells
#define TAG_LIGT "LIGT"   // fog / light, first version
#define TAG_LIG2 "LIG2"   // fog / light, with density
#define TAG_LIG3 "LIG3"   // fog / light, with cone and range
#define TAG_PLU2 "PLU2"   // animation plugs, packed
#define TAG_PLUG "PLUG"   // animation plugs
#define TAG_KAN3 "KAN3"   // key tracks, five lists
#define TAG_KAN2 "KAN2"   // key tracks, four lists
#define TAG_KANI "KANI"   // key tracks, three lists
#define TAG_MANI "MANI"   // frame matrices
#define TAG_MAN2 "MAN2"   // frame matrices, packed
#define TAG_VPAC "VPAC"   // vertex packs
#define TAG_VPA2 "VPA2"   // vertex packs, packed
#define TAG_MAT3 "MAT3"   // materials
#define TAG_MAT4 "MAT4"   // materials, packed
#define TAG_MATE "MATE"   // materials, edge bytes cleared
#define TAG_MAT2 "MAT2"   // materials, packed, edge bytes cleared
#define TAG_CHID "CHID"   // parent and child names
#define TAG_BONE "BONE"   // skin data
#define TAG_BON2 "BON2"   // skin data, packed

#define POSES_PER_STEP 8
#define PARSE_MAX_NODES 1024
#define PARSE_NAME_SIZE 256
#define PARSE_USED_SIZE 4985
#define PARSE_BONE_COUNT 0x44
#define PARSE_TEXTURE_PATH_MAX 0x20
#define BLANK_TEXTURE "auto-white"
#define BLANK_TEXTURE_SIZE 0x80
#define SKIPPED_TEXTURE "deletefile"
#define FUR_PREFIX "Fur-%s"
#define SAVED_BLOCK_SIZE 0xdc
#define SKIN_BLOCK_SIZE 0x561c
#define VISIBILITY_BLOCK_SIZE 0x1800
#define FOG_CONE_DEFAULT 45.0f
#define MATERIAL_DETAIL_TEXTURE 3

// Gives material `index` of `node` its textures: the detail texture, then the main one. A missing or white main texture
// becomes the blank "auto-white" one when DAT_11b044e4 is set; with DAT_11b044e4 set a texture that cannot be loaded
// falls back to a copy of the fur texture. Used by the material chunks MAT3, MAT4 and MATE.
static __forceinline void LoadMaterialTextures(Node *node, int index)
{
    Material *material = &node->materials[index];
    if (material->textures[MATERIAL_DETAIL_TEXTURE][0] != 0)
    {
        if (strlen(material->textures[MATERIAL_DETAIL_TEXTURE]) < PARSE_TEXTURE_PATH_MAX)
        {
            char detailPath[PARSE_NAME_SIZE];
            sprintf(detailPath, material->textures[MATERIAL_DETAIL_TEXTURE]);
            FUN_10174ce0(detailPath, 0, 0);
        }
    }
    int quality = DAT_1101d840;
    if (material->textures[0][0] == 0 || memcmp(material->textures[0], BLANK_TEXTURE, sizeof(BLANK_TEXTURE)) == 0 ||
        memcmp(material->textures[0], "white", sizeof("white")) == 0)
    {
        if (DAT_11b044e4 != 0)
        {
            strcpy(material->textures[0], BLANK_TEXTURE);
            strcpy(material->textures[1], "0");
            int saved = DAT_1101d840;
            DAT_1101d840 = 0;
            FUN_10175c90(BLANK_TEXTURE, BLANK_TEXTURE_SIZE, BLANK_TEXTURE_SIZE, DAT_11af3fa8, 0, 1);
            DAT_1101d840 = saved;
            node->ProjectPlanarUVs(index);
        }
    }
    else
    {
        char texturePath[PARSE_NAME_SIZE];
        strcpy(texturePath, material->textures[0]);
        if (DAT_11b044e4 == 0)
        {
            DAT_1101d840 = quality;
            if (memcmp(texturePath, SKIPPED_TEXTURE, sizeof(SKIPPED_TEXTURE)) != 0)
                FUN_10174ce0(texturePath, 0, 0);
        }
        else
        {
            DAT_1101d840 = 0;
            if (FUN_10174ce0(texturePath, 1, 0) == -1)
            {
                char furName[PARSE_NAME_SIZE];
                sprintf(furName, FUR_PREFIX, DAT_10ffe5b8);
                FUN_10175a90(furName, &DAT_118d0b70, 1);
            }
            DAT_1101d840 = quality;
        }
    }
}

// 0x101a58a0 (It3_ParseChunks), called by Node::LoadIt3. Reads the chunks of the model file into a tree of nodes: INFO
// starts a node (the first one is this node, later ones are new nodes), the other chunks fill the latest node: render
// type, fog and light values, animation plugs, key tracks, frame matrices, vertex packs, materials (loading their
// textures), child names and skin data; INFS checks the file against the key sum and xor of its name and sets
// DAT_11b044f0 when they differ. `a` non-zero makes the frame matrices use a new cache slot instead of sharing. At the
// end of the file the nodes are linked to their parents and children by name and the ones left without a parent are
// freed. Returns 1.
int Node::ParseChunks(GameFile *file, int a, int b, int keySum, unsigned int keyCheck)
{
    Node *nodes[PARSE_MAX_NODES];
    char name[PARSE_NAME_SIZE];
    char tag[5];
    int chunkSize;
    int nameSum;
    int nameXor;
    int nameBase;
    int packedPlugs;
    int packedPlugSize;
    int plainPlugs;
    int plainPlugSize;
    int vpacSlot;
    int vpa2Slot;
    int nodeIndex = 0;
    int seenInfo = 0;
    Node *root = this;
    nodes[0] = root;
    file->Read(tag, 1, 4);
    tag[4] = 0;
    while (file->AtEnd() == 0)
    {
        if (strncmp(tag, TAG_INFO, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            file->Read(name, 1, NODE_NAME_SIZE);
            if (DAT_10f65538 != 0)
                DAT_10f65538(0, 0);
            if (seenInfo == 0)
                nodes[nodeIndex] = root;
            else
            {
                nodeIndex++;
                Node *created = (Node *)DAT_11b0b938(sizeof(Node), name);
                nodes[nodeIndex] = created;
                created->Init();
            }
            Node *node = nodes[nodeIndex];
            seenInfo = 1;
            strcpy(node->name, name);
            strcpy(node->folder, file->name);
            strcpy(node->m_0050, file->name);
            file->Read(&node->matrix, 1, sizeof(Matrix4));
            for (int pose = 0; pose < NODE_POSE_COUNT; pose += POSES_PER_STEP)
            {
                node->poses[pose] = node->matrix;
                node->poses[pose + 1] = node->matrix;
                node->poses[pose + 2] = node->matrix;
                node->poses[pose + 3] = node->matrix;
                node->poses[pose + 4] = node->matrix;
                node->poses[pose + 5] = node->matrix;
                node->poses[pose + 6] = node->matrix;
                node->poses[pose + 7] = node->matrix;
            }
            file->Read(&node->flags, 1, 4);
            file->Read(&node->cullMargin, 1, 4);
            file->Read(&node->group, 1, 4);
        }
        else if (strncmp(tag, TAG_IKNG, 4) == 0)
        {
            Node *node = nodes[nodeIndex];
            if (node->saved == 0)
            {
                node->saved = (NodeSaved *)DAT_11b0b938(SAVED_BLOCK_SIZE, "ik");
                node->saved->M_10166870();
            }
            file->Read(&chunkSize, 1, 4);
            file->Read(&node->saved->limitMin, 1, 0x20);
        }
        else if (strncmp(tag, TAG_INFS, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            file->Read(&nameSum, 1, 4);
            file->Read(&nameXor, 1, 4);
            file->Read(&nameBase, 1, 4);
            if (nameSum - nameBase != keySum)
                DAT_11b044f0 = 1;
            if (nameXor - nameBase != (int)keyCheck)
                DAT_11b044f0 = 1;
        }
        else if (strncmp(tag, TAG_RTYP, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->drawMode, 1, 4);
            file->Read(&node->animated, 1, 4);
            file->Read(&node->frameCount, 1, 4);
            file->Read(&node->frameSize, 1, 4);
        }
        else if (strncmp(tag, TAG_RTY2, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->drawMode, 1, 4);
            file->Read(&node->m_187e, 1, 1);
            file->Read(&node->animated, 1, 4);
            file->Read(&node->frameCount, 1, 4);
            file->Read(&node->frameSize, 1, 4);
        }
        else if (strncmp(tag, TAG_CAME, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->pathKind, 1, 1);
            file->Read(&node->m_187a, 1, 1);
            file->Read(&node->m_187b, 1, 1);
            file->Read(&node->m_187c, 1, 2);
        }
        else if (strncmp(tag, TAG_CDLO, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            void *cells = DAT_11b0b938(VISIBILITY_BLOCK_SIZE, "CDL");
            nodes[nodeIndex]->visibilityCells = (VisData *)cells;
            file->Read(cells, 1, VISIBILITY_BLOCK_SIZE);
        }
        else if (strncmp(tag, TAG_LIGT, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->fogColour, 1, sizeof(Vec4));
            file->Read(&node->m_1878, 1, 1);
            node->fogDensity = 1.0f;
            node->lightConeInner = 0;
            node->m_1888 = FOG_CONE_DEFAULT;
            node->fogNear = 0;
            node->fogFar = node->fogColour.w;
        }
        else if (strncmp(tag, TAG_LIG2, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->fogColour, 1, sizeof(Vec4));
            file->Read(&node->m_1878, 1, 1);
            file->Read(&node->fogDensity, 1, 4);
            node->lightConeInner = 0;
            node->m_1888 = FOG_CONE_DEFAULT;
            node->fogNear = 0;
            node->fogFar = node->fogColour.w;
        }
        else if (strncmp(tag, TAG_LIG3, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->fogColour, 1, sizeof(Vec4));
            file->Read(&node->m_1878, 1, 1);
            file->Read(&node->fogDensity, 1, 4);
            file->Read(&node->lightConeInner, 1, sizeof(Vec4));
        }
        else if (strncmp(tag, TAG_PLU2, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            for (int plug = 0; plug < NODE_PLUG_COUNT; plug++)
            {
                file->Read(&packedPlugs, 1, 4);
                if (packedPlugs != 0)
                {
                    file->Read(&packedPlugSize, 1, 4);
                    Channel *data = (Channel *)DAT_11b0b938(packedPlugSize, "->m_plugdata[g]");
                    Node *node = nodes[nodeIndex];
                    node->plugs[plug] = data;
                    file->ReadPacked(data, 1, packedPlugSize);
                    file->Read(&node->plugType[plug], 1, 4);
                }
            }
        }
        else if (strncmp(tag, TAG_PLUG, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            for (int plug = 0; plug < NODE_PLUG_COUNT; plug++)
            {
                file->Read(&plainPlugs, 1, 4);
                if (plainPlugs != 0)
                {
                    file->Read(&plainPlugSize, 1, 4);
                    Channel *data = (Channel *)DAT_11b0b938(plainPlugSize, "->m_plugdata[g]");
                    Node *node = nodes[nodeIndex];
                    node->plugs[plug] = data;
                    file->Read(data, 1, plainPlugSize);
                    file->Read(&node->plugType[plug], 1, 4);
                }
            }
        }
        else if (strncmp(tag, TAG_KAN3, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            node->M_101949d0();
            Vec4 key;
            int keyCount;
            int frame;
            char keyFlag;
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_TRANSLATION].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_TRANSLATION].flags[at] = keyFlag;
            }
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_ROTATION].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_ROTATION].flags[at] = keyFlag;
            }
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_SCALE].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_SCALE].flags[at] = keyFlag;
            }
            file->Read(&node->extra->m_0070[0], 1, 4);
            file->Read(&node->extra->m_0070[1], 1, 4);
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_COLOUR].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_COLOUR].flags[at] = keyFlag;
            }
            file->Read(&node->extra->m_0070[2], 1, 4);
            file->Read(&node->extra->m_0070[3], 1, 4);
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_EXTRA].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_EXTRA].flags[at] = keyFlag;
            }
        }
        else if (strncmp(tag, TAG_KAN2, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            node->M_101949d0();
            Vec4 key;
            int keyCount;
            int frame;
            char keyFlag;
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_TRANSLATION].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_TRANSLATION].flags[at] = keyFlag;
            }
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_ROTATION].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_ROTATION].flags[at] = keyFlag;
            }
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_SCALE].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_SCALE].flags[at] = keyFlag;
            }
            file->Read(&node->extra->m_0070[0], 1, 4);
            file->Read(&node->extra->m_0070[1], 1, 4);
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_COLOUR].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_COLOUR].flags[at] = keyFlag;
            }
            node->extra->m_0070[2] = 0;
            node->extra->m_0070[3] = 0;
        }
        else if (strncmp(tag, TAG_KANI, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            node->M_101949d0();
            Vec4 key;
            int keyCount;
            int frame;
            char keyFlag;
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_TRANSLATION].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_TRANSLATION].flags[at] = keyFlag;
            }
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_ROTATION].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_ROTATION].flags[at] = keyFlag;
            }
            file->Read(&keyCount, 1, 4);
            for (int i = 0; i < keyCount; i++)
            {
                file->Read(&frame, 1, 4);
                file->Read(&key, 1, sizeof(Vec4));
                file->Read(&keyFlag, 1, 1);
                int at = node->extra->tracks[NODE_TRACK_SCALE].Set(frame, key.x, key.y, key.z, key.w);
                node->extra->tracks[NODE_TRACK_SCALE].flags[at] = keyFlag;
            }
            node->extra->m_0070[0] = 0;
            node->extra->m_0070[1] = 0;
            node->extra->m_0070[2] = 0;
            node->extra->m_0070[3] = 0;
        }
        else if (strncmp(tag, TAG_MANI, 4) == 0)
        {
            int slot = -1;
            int found = -1;
            Node *node;
            if (a == 0)
            {
                node = nodes[nodeIndex];
                found = FUN_10169ab0(node->m_0050, node->name, &slot);
                if (found >= 0)
                {
                    node->m_028c = DAT_10fdf1d0[found];
                    node->frameMatrices = DAT_1101c820[found];
                    node->frameMatrixCount = DAT_11017600[found];
                    FUN_10169be0(node->m_028c);
                    file->Read(&chunkSize, 1, 4);
                    file->Seek(chunkSize, 1);
                }
            }
            else
            {
                slot = FUN_101699d0();
                node = nodes[nodeIndex];
                FUN_101699f0(slot, node->m_0050, node->name);
            }
            if (found < 0)
            {
                file->Read(&chunkSize, 1, 4);
                node->m_028c = (TextureRef *)DAT_11b0b938(4, "motion");
                file->Read(&node->frameMatrixCount, 1, 4);
                node->frameMatrices = (Matrix4 *)DAT_11b0b938(node->frameMatrixCount << 6, "tmp[i]->m_animat");
                for (int i = 0; i < nodes[nodeIndex]->frameMatrixCount; i++)
                    file->Read(&node->frameMatrices[i], 1, sizeof(Matrix4));
                node->m_028c->slot = slot;
                if (slot >= 0)
                {
                    DAT_10fdf1d0[slot] = node->m_028c;
                    DAT_1101c820[slot] = node->frameMatrices;
                    DAT_11017600[slot] = node->frameMatrixCount;
                    FUN_10169be0(node->m_028c);
                }
            }
        }
        else if (strncmp(tag, TAG_MAN2, 4) == 0)
        {
            int slot = -1;
            int found = -1;
            Node *node;
            if (a == 0)
            {
                node = nodes[nodeIndex];
                found = FUN_10169ab0(node->m_0050, node->name, &slot);
                if (found >= 0)
                {
                    node->m_028c = DAT_10fdf1d0[found];
                    node->frameMatrices = DAT_1101c820[found];
                    node->frameMatrixCount = DAT_11017600[found];
                    FUN_10169be0(node->m_028c);
                    file->Read(&chunkSize, 1, 4);
                    file->Seek(chunkSize, 1);
                }
            }
            else
            {
                slot = FUN_101699d0();
                node = nodes[nodeIndex];
                FUN_101699f0(slot, node->m_0050, node->name);
            }
            if (found < 0)
            {
                file->Read(&chunkSize, 1, 4);
                node->m_028c = (TextureRef *)DAT_11b0b938(4, "motion");
                file->Read(&node->frameMatrixCount, 1, 4);
                node->frameMatrices = (Matrix4 *)DAT_11b0b938(node->frameMatrixCount << 6, "tmp[i]->m_animat");
                file->ReadPacked(node->frameMatrices, 1, node->frameMatrixCount << 6);
                node->m_028c->slot = slot;
                if (slot >= 0)
                {
                    DAT_10fdf1d0[slot] = node->m_028c;
                    DAT_1101c820[slot] = node->frameMatrices;
                    DAT_11017600[slot] = node->frameMatrixCount;
                    FUN_10169be0(node->m_028c);
                }
            }
        }
        else if (strncmp(tag, TAG_VPAC, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            if (_strnicmp(node->name, "Bip", 3) == 0 || _strnicmp(node->name, "Bone", 4) == 0)
                file->Seek(chunkSize, 1);
            else
            {
                int found = FUN_10169840(node->folder, node->name, &vpacSlot);
                if (found < 0)
                {
                    file->Read(&node->packCount, 1, 4);
                    node->packs = (VertexPack *)DAT_11b0b938(node->packCount * sizeof(VertexPack), "tmp[i]->m_vpack");
                    node->indexLists = (IndexList *)DAT_11b0b938(node->packCount * sizeof(IndexList),
                                                                "tmp[i]->m_vpackindex");
                    for (int i = 0; i < nodes[nodeIndex]->packCount; i++)
                    {
                        node->packs[i].Init();
                        node->indexLists[i].Init();
                        file->Read(&node->packs[i].vertexCount, 1, 4);
                        file->Read(&node->packs[i], 1, sizeof(Vertex));
                        if (node->packs[i].vertexCount != 0)
                        {
                            node->packs[i].vertices = (Vertex *)DAT_11b0b938(
                                node->packs[i].vertexCount * sizeof(Vertex), "->m_vpack[x].m_vertex");
                            file->Read(node->packs[i].vertices, 1, node->packs[i].vertexCount * sizeof(Vertex));
                        }
                        file->Read(&node->indexLists[i].count, 1, 4);
                        file->Read(&node->indexLists[i].count, 1, 4);
                        if (node->indexLists[i].count != 0)
                        {
                            if (node->indexLists[i].count < 1)
                                node->indexLists[i].count = 0;
                            else
                            {
                                node->indexLists[i].indices = (int *)DAT_11b0b938(node->indexLists[i].count << 2,
                                                                                   "->m_vpackindex[x].m_index");
                                file->Read(node->indexLists[i].indices, 1, node->indexLists[i].count << 2);
                            }
                        }
                    }
                    if (node->mesh != 0)
                        fread(0, 1, 1, 0);
                    node->mesh = (MeshData *)DAT_11b0b938(sizeof(MeshData), "load2CObj");
                    node->mesh->Init();
                    node->mesh->bufferSlot = vpacSlot;
                    if (vpacSlot >= 0)
                    {
                        DAT_10fa7478[vpacSlot] = node->mesh;
                        DAT_10ff2ff8[vpacSlot] = (int)node->packs;
                        DAT_10fe01d8[vpacSlot] = (int)node->indexLists;
                        DAT_10fee5e0[vpacSlot] = node->packCount;
                        FUN_10169970(node->mesh);
                    }
                    node->ComputeBounds();
                }
                else
                {
                    node->mesh = DAT_10fa7478[found];
                    node->packs = (VertexPack *)DAT_10ff2ff8[found];
                    node->indexLists = (IndexList *)DAT_10fe01d8[found];
                    node->packCount = DAT_10fee5e0[found];
                    FUN_10169970(node->mesh);
                    file->Seek(chunkSize, 1);
                    node->ComputeBounds();
                }
            }
        }
        else if (strncmp(tag, TAG_VPA2, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            if (_strnicmp(node->name, "Bip", 3) == 0 || _strnicmp(node->name, "Bone", 4) == 0)
                file->Seek(chunkSize, 1);
            else
            {
                int found = FUN_10169840(node->folder, node->name, &vpa2Slot);
                if (found < 0)
                {
                    file->Read(&node->packCount, 1, 4);
                    node->packs = (VertexPack *)DAT_11b0b938(node->packCount * sizeof(VertexPack), "tmp[i]->m_vpack");
                    node->indexLists = (IndexList *)DAT_11b0b938(node->packCount * sizeof(IndexList),
                                                                "tmp[i]->m_vpackindex");
                    for (int i = 0; i < nodes[nodeIndex]->packCount; i++)
                    {
                        node->packs[i].Init();
                        node->indexLists[i].Init();
                        file->Read(&node->packs[i].vertexCount, 1, 4);
                        file->Read(&node->packs[i], 1, sizeof(Vertex));
                        if (node->packs[i].vertexCount != 0)
                        {
                            node->packs[i].vertices = (Vertex *)DAT_11b0b938(
                                node->packs[i].vertexCount * sizeof(Vertex), "->m_vpack[x].m_vertex");
                            file->ReadPacked(node->packs[i].vertices, 1, node->packs[i].vertexCount * sizeof(Vertex));
                        }
                        file->Read(&node->indexLists[i].count, 1, 4);
                        file->Read(&node->indexLists[i].count, 1, 4);
                        if (node->indexLists[i].count != 0)
                        {
                            if (node->indexLists[i].count < 1)
                                node->indexLists[i].count = 0;
                            else
                            {
                                node->indexLists[i].indices = (int *)DAT_11b0b938(node->indexLists[i].count << 2,
                                                                                   "->m_vpackindex[x].m_index");
                                file->ReadPacked(node->indexLists[i].indices, 1, node->indexLists[i].count << 2);
                            }
                        }
                    }
                    if (node->mesh != 0)
                        fread(0, 1, 1, 0);
                    node->mesh = (MeshData *)DAT_11b0b938(sizeof(MeshData), node->name);
                    node->mesh->Init();
                    node->mesh->bufferSlot = vpa2Slot;
                    if (vpa2Slot >= 0)
                    {
                        DAT_10fa7478[vpa2Slot] = node->mesh;
                        DAT_10ff2ff8[vpa2Slot] = (int)node->packs;
                        DAT_10fe01d8[vpa2Slot] = (int)node->indexLists;
                        DAT_10fee5e0[vpa2Slot] = node->packCount;
                        FUN_10169970(node->mesh);
                    }
                    node->ComputeBounds();
                }
                else
                {
                    node->mesh = DAT_10fa7478[found];
                    node->packs = (VertexPack *)DAT_10ff2ff8[found];
                    node->indexLists = (IndexList *)DAT_10fe01d8[found];
                    node->packCount = DAT_10fee5e0[found];
                    FUN_10169970(node->mesh);
                    file->Seek(chunkSize, 1);
                    node->ComputeBounds();
                }
            }
        }
        else if (strncmp(tag, TAG_MAT3, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->materialCount, 1, 4);
            node->materials = (Material *)DAT_11b0b938(node->materialCount * sizeof(Material), "tmp[i]->m_material");
            for (int i = 0; i < nodes[nodeIndex]->materialCount; i++)
            {
                file->Read(&node->materials[i], 1, sizeof(Material));
                LoadMaterialTextures(node, i);
            }
        }
        else if (strncmp(tag, TAG_MAT4, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->materialCount, 1, 4);
            node->materials = (Material *)DAT_11b0b938(node->materialCount * sizeof(Material), "tmp[i]->m_material");
            file->ReadPacked(node->materials, 1, node->materialCount * sizeof(Material));
            for (int i = 0; i < nodes[nodeIndex]->materialCount; i++)
                LoadMaterialTextures(node, i);
        }
        else if (strncmp(tag, TAG_MATE, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->materialCount, 1, 4);
            node->materials = (Material *)DAT_11b0b938(node->materialCount * sizeof(Material), "tmp[i]->m_material");
            for (int i = 0; i < nodes[nodeIndex]->materialCount; i++)
            {
                file->Read(&node->materials[i], 1, sizeof(Material));
                node->materials[i].m_017c[1] = 0;
                node->materials[i].m_017c[2] = 0;
                node->materials[i].m_017c[3] = 0;
                LoadMaterialTextures(node, i);
            }
        }
        else if (strncmp(tag, TAG_MAT2, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(&node->materialCount, 1, 4);
            node->materials = (Material *)DAT_11b0b938(node->materialCount * sizeof(Material), "tmp[i]->m_material");
            Material *packed = (Material *)DAT_11b0b938(node->materialCount * sizeof(Material), "tmp[i]->m_material");
            file->ReadPacked(packed, 1, node->materialCount * sizeof(Material));
            for (int i = 0; i < nodes[nodeIndex]->materialCount; i++)
            {
                node->materials[i] = packed[i];
                node->materials[i].m_017c[1] = 0;
                node->materials[i].m_017c[2] = 0;
                node->materials[i].m_017c[3] = 0;
                LoadMaterialTextures(node, i);
            }
            DAT_11b1ba40(packed);
        }
        else if (strncmp(tag, TAG_CHID, 4) == 0)
        {
            file->Read(&chunkSize, 1, 4);
            Node *node = nodes[nodeIndex];
            file->Read(node->m_070c, 1, NODE_NAME_SIZE);
            file->Read(&node->childCount, 1, 4);
            for (int i = 0; i < nodes[nodeIndex]->childCount; i++)
                file->Read(node->childNames[i], 1, NODE_NAME_SIZE);
        }
        else if (strncmp(tag, TAG_BONE, 4) == 0)
        {
            Node *node = nodes[nodeIndex];
            if (node->skin == 0)
            {
                node->skin = (NodeSkin *)DAT_11b0b938(SKIN_BLOCK_SIZE, "bone");
                node->skin->Clear();
            }
            file->Read(&chunkSize, 1, 4);
            file->Read(node->skin->boneName, 1, SKIN_NAME_SIZE);
            file->Read(&node->skin->boneCount, 1, 4);
            for (int i = 0; i < nodes[nodeIndex]->skin->boneCount; i++)
                file->Read(node->skin->linkNames[i], 1, SKIN_NAME_SIZE);
            for (int i = 0; i < PARSE_BONE_COUNT; i++)
            {
                file->Read(node->skin->boneNames[i], 1, SKIN_NAME_SIZE);
                file->Read(&node->skin->inverseBones[i], 1, sizeof(Matrix4));
                file->Read(&node->skin->bindPoses[i], 1, sizeof(Matrix4));
            }
        }
        else if (strncmp(tag, TAG_BON2, 4) == 0)
        {
            Node *node = nodes[nodeIndex];
            if (node->skin == 0)
            {
                node->skin = (NodeSkin *)DAT_11b0b938(SKIN_BLOCK_SIZE, "bone");
                node->skin->Clear();
            }
            file->Read(&chunkSize, 1, 4);
            file->Read(node->skin->boneName, 1, SKIN_NAME_SIZE);
            file->Read(&node->skin->boneCount, 1, 4);
            file->ReadPacked(node->skin->linkNames, 1, node->skin->boneCount << 6);
            file->ReadPacked(node->skin->boneNames, 1, PARSE_BONE_COUNT * SKIN_NAME_SIZE);
            file->ReadPacked(node->skin->inverseBones, 1, PARSE_BONE_COUNT * sizeof(Matrix4));
            file->ReadPacked(node->skin->bindPoses, 1, PARSE_BONE_COUNT * sizeof(Matrix4));
        }
        else
        {
            file->Read(&chunkSize, 1, 4);
            file->Seek(chunkSize, 1);
        }
        file->Read(tag, 1, 4);
        tag[4] = 0;
    }
    // From here nodeIndex is the number of nodes read.
    char used[PARSE_USED_SIZE];
    memset(used, 0, sizeof(used));
    nodeIndex++;
    for (int i = 0; i < nodeIndex; i++)
    {
        Node *node = nodes[i];
        for (int j = 0; j < nodeIndex; j++)
        {
            if (strcmp(node->m_070c, nodes[j]->name) == 0)
            {
                node->parent = nodes[j];
                break;
            }
        }
        for (int child = 0; child < node->childCount; child++)
        {
            for (int j = 0; j < nodeIndex; j++)
            {
                if (strcmp(node->childNames[child], nodes[j]->name) == 0 && used[j] == 0)
                {
                    node->children[child] = nodes[j];
                    used[j] = 1;
                    nodes[j]->parent = node;
                    break;
                }
            }
        }
    }
    for (int i = 1; i < nodeIndex; i++)
    {
        if (nodes[i]->parent == 0)
        {
            nodes[i]->Release();
            DAT_11b1ba40(nodes[i]);
        }
    }
    return 1;
}
