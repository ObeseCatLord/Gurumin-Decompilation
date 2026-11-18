// Texture stage helpers and the material slot table of a mesh.

#include "d3d9_min.h"

struct SlotTable
{
    int m_0000;
    int pad_0004[2];
    int m_000c;
    int *m_0010;
    int *m_0014;
    int m_0018;
    int m_001c;
    int m_0020;
    int m_0024;
    int m_0028;
    int m_002c;
    int m_0030;
    int m_0034;
    int m_0038;
    int m_003c;
    int m_0040;
    int m_0044;
    int m_0048;

    void M_10145a10();
    int M_10146d90(int first, int last);
    int M_10147690(int key, int value);
    int M_101476e0(int index, int value);
};

struct IntPair
{
    int first;
    int second;
    IntPair();
};

extern "C" IDirect3DDevice9 *DAT_10f1fcf0;
extern "C" int DAT_10f6550c;
extern "C" int DAT_10f65510;
extern "C" int DAT_10f181e0[0x5a0];

// Unbinds texture stage 0 (only when something is bound).
extern "C" void FUN_10143b50(void)
{
    if (DAT_10f6550c != 1)
    {
        DAT_10f1fcf0->SetTexture(0, 0);
        DAT_10f6550c = 1;
    }
}

// Unbinds texture stage 1 (only when something is bound).
extern "C" void FUN_10143b80(void)
{
    if (DAT_10f65510 != 1)
    {
        DAT_10f1fcf0->SetTexture(1, 0);
        DAT_10f65510 = 1;
    }
}

// Constructor: both values 0.
IntPair::IntPair()
{
    first = 0;
    second = 0;
}

// Unbinds texture stages 0..3. Returns 0.
extern "C" int FUN_101446b0(void)
{
    FUN_10143b50();
    FUN_10143b80();
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    return 0;
}

// Resets the table: no owner (-1), no entries, all pointers cleared.
void SlotTable::M_10145a10()
{
    m_0000 = -1;
    m_0044 = 0;
    m_000c = 0;
    m_0014 = 0;
    m_0010 = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_0024 = 0;
    m_0028 = 0;
    m_002c = 0;
    m_0030 = 0;
    m_0034 = 0;
    m_0038 = 0;
    m_0048 = 0;
}

// Limits later operations to entries first..last-1.
int SlotTable::M_10146d90(int first, int last)
{
    m_003c = first;
    m_0040 = last;
    return 1;
}

// Replaces the material of every entry in range whose current or original material is `key` (or of all
// entries when key is -1) with `value`. Node::M_1019e830 runs it over a whole model (texture swaps,
// e.g. costume colours).
int SlotTable::M_10147690(int key, int value)
{
    for (int i = m_003c; i < m_0044 && i < m_0040; i++)
    {
        if (key == -1 || m_0014[i] == key || m_0010[i] == key)
            m_0014[i] = value;
    }
    return 0;
}

// Sets the material of entry `index` (when in range).
int SlotTable::M_101476e0(int index, int value)
{
    if (index < m_0044 && index < m_0040)
        m_0014[index] = value;
    return 0;
}

// Ages every texture slot by one frame (DAT_10f181e0, used to evict unused textures).
extern "C" int FUN_10147a00(void)
{
    for (int i = 0; i < 0x5a0; i++)
        DAT_10f181e0[i]++;
    return 1;
}

// Pre-transformed vertex with colours and one texture coordinate (FVF 0x1c4), 0x20 bytes.
struct ColourVertex
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned int diffuse;
    unsigned int specular;
    float u;
    float v;
};

extern "C" void FUN_1013ef30(int state, int value);              // Render_SetRenderState (cached)
extern "C" void FUN_1013ef90(int stage, int type, int value);    // Render_SetTextureStageState (cached)
extern "C" void FUN_1013f070(int mode, int a, int b);            // sets the 2D blend mode

// 0x10144040. Draws a 34x34 blue square (0xff0000ff) in the top-left corner of the screen (from -1 to 33 in x and y), untextured
// (stage 0 passes the vertex colour through), then sets 2D blend mode 6. Returns 1.
extern "C" int FUN_10144040(void)
{
    ColourVertex quad[4];
    FUN_10143b50();
    // The corners (-1, -1), (33, -1), (-1, 33), (33, 33) with texture coordinates 0.001..1; the shared values are
    // set with chained assignments.
    quad[2].x = quad[1].y = quad[0].y = quad[0].x = -1.0f;
    quad[2].u = quad[1].v = quad[0].v = quad[0].u = 0.001f;
    quad[3].y = quad[3].x = quad[2].y = quad[1].x = 33.0f;
    quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = 0xff0000ff;
    quad[3].v = quad[3].u = quad[2].v = quad[1].u = 1.0f;
    quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = 0xff000000;
    quad[0].z = quad[1].z = quad[2].z = quad[3].z = 0;
    quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = 1.0f;
    FUN_1013ef30(0x16, 3);                  // D3DRS_CULLMODE = D3DCULL_CCW
    FUN_1013ef30(0x8d, 1);                  // D3DRS_COLORVERTEX
    DAT_10f1fcf0->SetFVF(0x1c4);
    DAT_10f1fcf0->SetVertexShader(0);
    FUN_1013ef30(0x91, 1);                  // D3DRS_DIFFUSEMATERIALSOURCE = D3DMCS_COLOR1
    FUN_1013ef90(0, 4, 3);                  // alpha: select argument 2
    FUN_1013ef90(0, 5, 2);                  //   texture
    FUN_1013ef90(0, 6, 0);                  //   diffuse
    FUN_1013ef90(0, 1, 3);                  // colour: select argument 2
    FUN_1013ef90(0, 2, 2);                  //   texture
    FUN_1013ef90(0, 3, 0);                  //   diffuse
    DAT_10f1fcf0->DrawPrimitiveUP(5 /* D3DPT_TRIANGLESTRIP */, 2, quad, 0x20);
    FUN_1013f070(6, 0, 0);
    return 1;
}

// 0x101441b0. Same as FUN_10144040 with a 257x257 black square (from -1 to 256): the background of the 256x256
// save thumbnail (both are called by FUN_10017d30). Returns 1.
extern "C" int FUN_101441b0(void)
{
    ColourVertex quad[4];
    FUN_10143b50();
    // The corners (-1, -1), (256, -1), (-1, 256), (256, 256) with texture coordinates 0.001..1; the shared values are
    // set with chained assignments.
    quad[2].x = quad[1].y = quad[0].y = quad[0].x = -1.0f;
    quad[2].u = quad[1].v = quad[0].v = quad[0].u = 0.001f;
    quad[3].y = quad[3].x = quad[2].y = quad[1].x = 256.0f;
    quad[0].diffuse = quad[1].diffuse = quad[2].diffuse = quad[3].diffuse = 0xff000000;
    quad[3].v = quad[3].u = quad[2].v = quad[1].u = 1.0f;
    quad[0].specular = quad[1].specular = quad[2].specular = quad[3].specular = 0xff0000ff;
    quad[0].z = quad[1].z = quad[2].z = quad[3].z = 0;
    quad[0].rhw = quad[1].rhw = quad[2].rhw = quad[3].rhw = 1.0f;
    FUN_1013ef30(0x16, 3);                  // D3DRS_CULLMODE = D3DCULL_CCW
    FUN_1013ef30(0x8d, 1);                  // D3DRS_COLORVERTEX
    DAT_10f1fcf0->SetFVF(0x1c4);
    DAT_10f1fcf0->SetVertexShader(0);
    FUN_1013ef30(0x91, 1);                  // D3DRS_DIFFUSEMATERIALSOURCE = D3DMCS_COLOR1
    FUN_1013ef90(0, 4, 3);                  // alpha: select argument 2
    FUN_1013ef90(0, 5, 2);                  //   texture
    FUN_1013ef90(0, 6, 0);                  //   diffuse
    FUN_1013ef90(0, 1, 3);                  // colour: select argument 2
    FUN_1013ef90(0, 2, 2);                  //   texture
    FUN_1013ef90(0, 3, 0);                  //   diffuse
    DAT_10f1fcf0->DrawPrimitiveUP(5 /* D3DPT_TRIANGLESTRIP */, 2, quad, 0x20);
    FUN_1013f070(6, 0, 0);
    return 1;
}
