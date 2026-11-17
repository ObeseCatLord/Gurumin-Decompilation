// Visibility cells. A map is split into up to 64 boxes; each cell lists the cells visible from it, and only
// the visible cells are drawn. DAT_11afffe8[i] = cell i visible this frame.

#include <string.h>

struct VisBox;

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
    int InBox(const VisBox *box);
};

struct VisBox
{
    Vec4 min;
    Vec4 max;
};

struct VisCell
{
    unsigned char count;
    unsigned char neighbours[0x3f];
};

struct VisData
{
    VisCell cells[0x40];
    VisBox boxes[0x40];
};

struct VisNode
{
    char pad_0000[0x1864];
    VisData *vis;
    int UpdateVisibleCells();
};

extern "C" int DAT_10f65520;
extern "C" Vec4 DAT_10f66908;
extern "C" Vec4 DAT_10f668e8;
extern "C" char DAT_11afffe8[0x40];

// Finds the cell boxes that contain the eye position and marks them and all cells listed as visible from them.
// Not byte-identical yet (see open.tsv).
int VisNode::UpdateVisibleCells()
{
    if (vis != 0)
    {
        Vec4 eye;
        VisBox *box = vis->boxes;
        if (DAT_10f65520 != 0)
            eye.Copy(DAT_10f66908);
        else
            eye.Copy(DAT_10f668e8);
        memset(DAT_11afffe8, 0, sizeof(DAT_11afffe8));
        for (int i = 0; i < 0x40; i++, box++)
        {
            if (vis->cells[i].count != 0 && eye.InBox(box))
            {
                DAT_11afffe8[i] = 1;
                for (int j = 0; j < vis->cells[i].count; j++)
                    DAT_11afffe8[vis->cells[i].neighbours[j]] = 1;
            }
        }
        return 1;
    }
    return 0;
}
extern "C" VisNode *DAT_10255f08;

// 1 when `pos` lies in a currently visible cell (or the map has no visibility data), else 0. Used to skip
// updating or drawing objects that cannot be seen.
extern "C" int FUN_10015860(Vec4 pos)
{
    VisData *vis = DAT_10255f08->vis;
    if (vis == 0)
        return 1;
    VisBox *box = vis->boxes;
    for (int i = 0; i < 0x40; i++, box++)
    {
        if (DAT_11afffe8[i] != 0 && pos.InBox(box))
            return 1;
    }
    return 0;
}
