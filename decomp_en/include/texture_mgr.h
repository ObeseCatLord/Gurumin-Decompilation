// Textures, the 2D triangle batch and the renderer object of the EN build (JP: decomp/include/texture_object.h,
// tri_batch.h, texture_manager.h, render_vars.h). This is the one declaration of these types and globals in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_TEXTURE_MGR_H
#define GURUMIN_EN_TEXTURE_MGR_H

struct IDirect3DTexture9;
struct IDirect3DBaseTexture9;
struct IDirect3DSurface9;

// Number of texture slots.
const int TEXTURE_SLOT_COUNT = 1600;
// Bytes of one texture slot name (empty name = free slot).
const int TEXTURE_NAME_SIZE = 0x40;
// TriangleBatch::blendMode before any SetBlend.
const int BLEND_MODE_UNSET = -1;
// Render_Command code that sets the blend mode.
const int RENDER_BLEND_MODE = 0;
// Vertices per triangle.
const int TRIANGLE_VERTICES = 3;

// A transformed screen vertex (0x20 bytes, FVF position + diffuse + specular + one texture coordinate): what
// TextureObject::DrawStrip and TriangleBatch take.
struct ScreenVertex
{
    float x;                    // 0x00: screen x in the 640x480 space
    float y;                    // 0x04: screen y
    float z;                    // 0x08: depth (0.9 to 1.0 is visible)
    float rhw;                  // 0x0c: 1 (already transformed)
    union                       // 0x10: diffuse colour, ARGB
    {
        unsigned int diffuse;
        struct
        {
            unsigned char blue;
            unsigned char green;
            unsigned char red;
            unsigned char alpha;    // set to 0 to hide a vertex that is off screen
        };
    };
    unsigned int specular;      // 0x14: second colour (0x00RRGGBB)
    float u;                    // 0x18: texture coordinate u
    float v;                    // 0x1c: texture coordinate v
};

// A texture object (0x38 bytes) owning a Direct3D texture; only the members EN code has used are named.
struct TextureObject
{
    IDirect3DTexture9 *texture;   // 0x00: the Direct3D texture
    IDirect3DSurface9 *surface;   // 0x04: surface of a render-target texture, may be 0
    int width;                    // 0x08: texture width in pixels
    int height;                   // 0x0c: texture height in pixels
    int m_0010;                   // 0x10: not yet named
    int poolIndex;                // 0x14: buffer pool index of the frames
    int m_0018;                   // 0x18: not yet named
    int poolSlot;                 // 0x1c: non-zero when the frames live in the buffer pool
    char m_0020[0x2c - 0x20];     // 0x20: source pixels and alpha (not split up yet)
    int sourceWidth;              // 0x2c: width of the source image
    int sourceHeight;             // 0x30: height of the source image
    int generatedAlpha;      // 0x34: 1 when the alpha is generated: rebuilt after a device reset

    int BindStage0();                                       // EN 0x7a58b0 (JP 0x1014b670): binds to stage 0 unless already bound
    int BindStage1();                                       // EN 0x7a5840 (JP 0x1014b6e0): binds to stage 1 unless already bound
    int BeginRenderTo(int width, int height, int offset);   // EN 0x7a4bf0 (JP 0x10143870)
    int UpdateImage(int value);                             // EN 0x746550 (JP 0x10169f70): BeginRenderTo with the source size
    void Unload();                                          // EN 0x740ea0 (JP 0x10169e40): frees the kept image and the texture
    int DrawTriangles(const void *vertices, int triangles); // EN 0x77ee90 (JP 0x1014bb70)
    int DrawStrip(ScreenVertex *vertices, int count);        // EN 0x77e360 (JP 0x1014ba20)
    int DrawScreenCopy(int left, int top, int right, int bottom, float z, float alpha);  // EN 0x78a950 (JP 0x1014b750)
    int DrawColoured(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                     float alpha, float angle, float red, float green, float blue);  // EN 0x78ed90 (JP 0x1014c960)
    int DrawCell(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                 float alpha, float angle);                 // EN 0x7918b0 (JP 0x1014bc60)
    int Write16(int x, int y, int rowPixels, int rows, int pitch, const void *bits);  // EN 0x77d2e0 (JP 0x1014f060)
    int WriteRgb32(int left, int top, int right, int bottom, int pitch, const unsigned char *bits);  // EN 0x77d030 (JP 0x1014ef20)
    int WritePaletted(int left, int top, int right, int bottom, int pitch, const unsigned char *bits,
                      const unsigned short *palette);       // EN 0x77d1b0 (JP 0x1014f120)
};

// Vertices (three per triangle) collected during a frame and drawn with one texture and blend mode.
struct TriangleBatch
{
    ScreenVertex *vertices;  // 0x00: vertex buffer (screen vertices, 0x20 bytes each)
    int capacity;            // 0x04: number of vertices the buffer holds
    int count;               // 0x08: number of vertices queued (three per triangle)
    int m_000c;              // 0x0c: not recovered
    TextureObject *texture;  // 0x10: texture to draw with (0 = nothing to draw)
    int blendMode;           // 0x14: render mode passed to Render_Command while drawing (BLEND_MODE_UNSET = none yet)

    int Reset();                                                         // EN 0x741ae0 (JP 0x10172aa0)
    int AppendVertices(const ScreenVertex *source, int count);           // EN 0x70aec0 (JP 0x10172ac0)
    int Flush();                                                         // EN 0x70be40 (JP 0x101733f0)
    int SetBlend(int mode, int unused);                                  // EN 0x70bff0 (JP 0x10173450)
    int AddVertices(TextureObject *texture, void *source, int count);    // EN 0x70bdf0 (JP 0x10188eb0)
    int DrawWith(TextureObject *texture);                                // EN 0x71dec0 (JP 0x10172a40)
};

// The renderer object that owns the textures; `this` is mostly unused.
struct TextureManager
{
    int UnloadAllTextures();       // EN 0x740fe0 (JP 0x10173480): unloads every texture slot, clears the names
    int Shutdown();                // EN 0x7a3ad0 (JP 0x1015c730): shuts the renderer down
    void UnloadAndShutdown();      // EN 0x740c30 (JP 0x10175e70)
};

extern "C" int Render_Command(int command, int value, int extra);   // EN 0x7a4f60 (JP 0x1013f070)

// Returns the texture to draw for a pooled texture (the buffer pool keeps its frames). EN 0x7a11e0 (JP 0x10148590).
extern "C" IDirect3DBaseTexture9 *TexturePool_Get(int poolIndex, IDirect3DTexture9 *texture, int width, int height, int slot);

extern "C" int g_textureBindCount;                                  // EN 0x021e4824 (JP DAT_10f141d4): texture binds this frame
extern "C" int g_pendingBatches;                                    // EN 0x0166f410: batches still pending this frame
extern "C" char g_textureNames[TEXTURE_SLOT_COUNT][TEXTURE_NAME_SIZE];  // EN 0x0168c7d0: slot names (empty = free)
extern "C" char g_textureRefCount[TEXTURE_SLOT_COUNT];              // EN 0x0162e2d0: references per slot
extern "C" char g_textureKept[TEXTURE_SLOT_COUNT];                  // EN 0x0162e910: 1 = keep across a map change
extern "C" TextureObject g_textures[TEXTURE_SLOT_COUNT];            // EN 0x01d8f6a0: every texture slot

// After a map change: unloads every texture that was not marked as kept and has no generated alpha (EN 0x747c70).
extern "C" int Texture_ReloadMissing(void);
// Releases one reference to texture slot `slot`; unloads it when the count reaches zero or `force` is set. Returns the
// remaining count, or -1 for an unused slot (EN 0x748470).
extern "C" int Texture_Release(int slot, int force);

extern "C" int Texture_BindStage1(int index);   // EN 0x7469d0 (JP 0x101610b0): binds slot `index` to stage 1 (negative = unbind)
extern "C" int Texture_BindStage0(int index);   // EN 0x746a10 (JP 0x10161070): binds slot `index` to stage 0 (negative = unbind)

#endif
