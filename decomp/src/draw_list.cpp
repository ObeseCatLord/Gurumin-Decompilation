// The deferred draw list: nodes queued during the scene walk (with their pass, colour and extra parameters) and drawn
// later pass by pass.

#include "matrix4.h"

#include "vec4.h"

struct MeshData
{
    int SetRange(int first, int last);           // 0x10146d90 (SlotTable::M_10146d90)
    void UploadBones(Matrix4 *bones, int count, Matrix4 *world, int view);   // 0x10145a50
    // 0x10151ba0: draws the mesh with the world matrix and the given lighting parameters.
    int DrawLit(Matrix4 *world, const Matrix4 *view, int mode, const void *lights, Vec4 *a, Vec4 *b);
};

// Skinning data of a node (node_draw.cpp NodeSkin).
struct NodeSkin
{
    char pad_0000[0x4618];
    Matrix4 bones[0x40];      // 0x4618
    int boneCount;            // 0x5618
};

#include "node.h"

extern "C" int DAT_11b044d0;                  // number of queued nodes
extern "C" unsigned char DAT_11b00030[0x40];  // pass of each queued node
extern "C" Node *DAT_11b00070[0x40];          // the queued nodes
extern "C" int DAT_11b00170[0x40];            // the value DAT_10f655c4 takes while the node draws
extern "C" char DAT_11b00270[0x40];           // the value DAT_10f65520 takes for drawing mode 2
extern "C" char DAT_11af3f68[0x40];           // light matrix set of each node (drawing mode 2)
extern "C" char DAT_11afffa8[0x40];           // light set of each node (drawing mode 2)
extern "C" Vec4 DAT_11b07128[0x40];           // colour of each queued node
extern "C" Vec4 DAT_11b04518[0x40];
extern "C" Vec4 DAT_11b04918[0x40];
extern "C" Matrix4 DAT_11b06518[];            // light matrix sets
extern "C" char DAT_11b04d18[][0x80];         // light sets
extern "C" Vec4 DAT_10f66948;                 // the current draw colour
extern "C" int DAT_10f655c4;
extern "C" int DAT_10f65520;
extern "C" void FUN_1013f070(int mode, int a, int b);   // sets a 2D/3D render state

// 0x101999a0 (DrawList_DrawPass). Draws every queued node of pass `pass` with the view `view`: uploads its bones when
// it is skinned, limits the mesh to the frame shown, sets its colour and parameters and draws it (drawing mode 2
// with its own light set and DAT_10f65520 changed for the call). The draw colour and DAT_10f655c4 are restored
// afterwards. Returns 0.
extern "C" int FUN_101999a0(int unused, const Matrix4 *view, int lights, unsigned int pass)
{
    Vec4 savedColour = DAT_10f66948;
    int saved = DAT_10f655c4;
    for (int i = 0; i < DAT_11b044d0; i++)
    {
        if (DAT_11b00030[i] != pass)
            continue;
        NodeSkin *skin = DAT_11b00070[i]->skin;
        if (skin != 0 && skin->boneCount != 0)
            DAT_11b00070[i]->mesh->UploadBones(skin->bones, skin->boneCount, &DAT_11b00070[i]->worldMatrix, (int)view);
        Node *node = DAT_11b00070[i];
        if (node->animated != 0)
        {
            int frame = node->frame;
            if (frame >= node->frameCount)
                frame = node->frameCount - 1;
            node->mesh->SetRange(node->frameSize * frame, (frame + 1) * node->frameSize);
        }
        else
            node->mesh->SetRange(0, 0x400);
        if (DAT_11b00070[i]->flags & 0x800000)
            FUN_1013f070(9, 0, 0);
        DAT_10f66948.Copy(DAT_11b07128[i]);
        DAT_10f655c4 = DAT_11b00170[i];
        int mode = DAT_11b00070[i]->drawMode;
        if (mode == 2)
        {
            int previous = DAT_10f65520;
            DAT_10f65520 = DAT_11b00270[i];
            FUN_1013f070(10, 2, 0);
            FUN_1013f070(0, 0, 0);
            DAT_11b00070[i]->mesh->DrawLit(&DAT_11b00070[i]->worldMatrix, &DAT_11b06518[DAT_11af3f68[i]], 4,
                                           DAT_11b04d18[DAT_11afffa8[i]], &DAT_11b04518[i], &DAT_11b04918[i]);
            DAT_10f65520 = previous;
            FUN_1013f070(10, 0, 0);
        }
        else
            DAT_11b00070[i]->mesh->DrawLit(&DAT_11b00070[i]->worldMatrix, view, mode, (const void *)lights,
                                           &DAT_11b04518[i], &DAT_11b04918[i]);
        if (DAT_11b00070[i]->flags & 0x800000)
            FUN_1013f070(9, 1, 0);
    }
    DAT_10f66948.Copy(savedColour);
    DAT_10f655c4 = saved;
    return 0;
}
