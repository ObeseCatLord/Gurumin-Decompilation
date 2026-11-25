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
// Render_Command code that sets the texture addressing of stage 0 (value 0 = wrap).
const int RENDER_TEXTURE_ADDRESS = 2;
// Render_Command code that turns depth writes on (value 1) or off (0).
const int RENDER_DEPTH_WRITE = 9;
// Render_Command codes that clear the target to colour `value` (4) and end the scene to begin a new one (6).
const int RENDER_CLEAR = 4;
const int RENDER_FLUSH_SCENE = 6;
// Horizontal placement of a 640x480 rectangle on a wider screen (DrawTinted's last argument).
const int ASPECT_STRETCH = 0;   // stretched with the screen
const int ASPECT_CENTRE = 1;    // keeps its 4:3 width, centred
const int ASPECT_LEFT = 2;      // keeps its 4:3 width, left edge scaled
// Blend modes of RENDER_BLEND_MODE: source alpha / inverse source alpha (the default), and added to the target.
const int BLEND_ALPHA = 0;
const int BLEND_ADD = 1;
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

// A transformed screen vertex with four texture coordinate pairs (0x38 bytes, FVF position + diffuse + specular +
// four texture coordinate sets): what the multi-stage quad draws take.
struct MultiTexVertex
{
    float x;                    // 0x00: screen x
    float y;                    // 0x04: screen y
    float z;                    // 0x08: depth
    float rhw;                  // 0x0c: 1 (already transformed)
    unsigned int diffuse;       // 0x10: diffuse colour, ARGB
    unsigned int specular;      // 0x14: second colour
    struct
    {
        float u;                // u of the texture coordinate set
        float v;                // v of the texture coordinate set
    } uv[4];                    // 0x18: texture coordinate sets 0 to 3
};

// A texture object (0x38 bytes) owning a Direct3D texture; only the members EN code has used are named.
// TextureObject::Create alpha argument: no alpha image, or one of the two generated alpha kinds (any larger value is a
// pointer to an alpha image: one byte per pixel at every third byte of an RGB-sized buffer).
const int TEXTURE_ALPHA_NONE = 0;
const int TEXTURE_ALPHA_GENERATED_1 = 1;
const int TEXTURE_ALPHA_GENERATED_2 = 2;
// Largest width or height of a texture.
const int TEXTURE_MAX_SIDE = 0x800;
// Bytes per pixel of the RGB source images, and the share of speckled pixels (alpha kept when random & 7 < 2).
const int TEXTURE_SOURCE_BYTES = 3;
const int TEXTURE_SPECKLE_MASK = 7;
const int TEXTURE_SPECKLE_KEEP = 2;
struct TextureObject
{
    IDirect3DTexture9 *texture;   // 0x00: the Direct3D texture
    IDirect3DSurface9 *surface;   // 0x04: surface of a render-target texture, may be 0
    int width;                    // 0x08: texture width in pixels
    int height;                   // 0x0c: texture height in pixels
    int target;                   // 0x10: 0 = system memory texture, 1 = R5G6B5 render target, 2 = A8R8G8B8 render target
    int poolIndex;                // 0x14: buffer pool index of the frames
    int m_0018;                   // 0x18: not yet named
    int poolSlot;                 // 0x1c: non-zero when the frames live in the buffer pool
    int sourceAlpha;              // 0x20: alpha of the source image: a pixel buffer or an ALPHA_* mode
    const unsigned char *sourcePixels;   // 0x24: kept source pixels (0 = none kept)
    char hasOwnAlpha;             // 0x28: 1 when the source has an alpha image of its own (not generated)
    char m_0029[3];               // 0x29: padding
    int sourceWidth;              // 0x2c: width of the source image
    int sourceHeight;             // 0x30: height of the source image
    int generatedAlpha;      // 0x34: 1 when the alpha is generated: rebuilt after a device reset

    // Creates the texture from width x height pixels (RGB, or `alpha` as the alpha image or mode). Returns non-zero when it
    // was created. EN 0x71c410.
    int Create(int width, int height, const void *pixels, int alpha, int flags);
    // EN 0x77d950 (JP 0x1014f1f0): (re)creates this texture at width x height and forgets it as a bound texture. `alpha` is 0
    // (system memory R5G6B5 texture filled from the 24-bit `pixels`), UPLOAD_TARGET_565 or UPLOAD_TARGET_8888 (an empty
    // render target) or a second 24-bit image whose first byte per pixel is the alpha of an A4R4G4B4 texture. Returns 0
    // when Direct3D fails, else 1 (also without a device).
    int Upload(int width, int height, unsigned char *pixels, const unsigned char *alpha);
    // EN 0x7a5b00 (JP 0x10144ee0): halves a 24-bit image in place (each 2x2 block averaged). `this` is not used.
    int HalveRgb24(unsigned char *pixels, int width, int height);
    int BindStage0();                                       // EN 0x7a58b0 (JP 0x1014b670): binds to stage 0 unless already bound
    int BindStage1();                                       // EN 0x7a5840 (JP 0x1014b6e0): binds to stage 1 unless already bound
    int BeginRenderTo(int width, int height, int offset);   // EN 0x7a4bf0 (JP 0x10143870)
    int UpdateImage(int value);                             // EN 0x746550 (JP 0x10169f70): BeginRenderTo with the source size (out of line)
    int Release();                                          // EN 0x7a3e50 (JP 0x1014f520): frees the Direct3D texture and surface, forgets bindings
    void Unload();                                          // EN 0x740ea0 (JP 0x10169e40): frees the kept image and the texture
    int DrawTriangles(const void *vertices, int triangles); // EN 0x77ee90 (JP 0x1014bb70)
    int DrawStrip(ScreenVertex *vertices, int count);        // EN 0x77e360 (JP 0x1014ba20)
    int DrawScreenCopy(int left, int top, int right, int bottom, float z, float alpha);  // EN 0x78a950 (JP 0x1014b750)
    int DrawColoured(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                     float alpha, float angle, float red, float green, float blue);  // EN 0x78ed90 (JP 0x1014c960)
    int DrawCell(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                 float alpha, float angle);                 // EN 0x7918b0 (JP 0x1014bc60)
    // EN 0x78ae20 (JP 0x1014c430): draws the cell (u0, v0)-(u1, v1) into the rectangle, faded by `alpha`, turned by `angle`.
    // `aspect` is an ASPECT_* value: how the rectangle is placed on a screen wider than 4:3.
    int DrawTinted(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                   float alpha, float angle, float grey, int aspect);
    // The panel frame and fill draws (9-slice frame and background of a menu panel); the parameters are the rectangle,
    // depth, alpha, a textured flag and a border size.
    int DrawFrame(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);     // EN 0x797f30 (JP 0x1014d4e0 family)
    // EN 0x78ce10: binds this texture to stages 0 to 3 and draws a quad covering the largest viewport size through
    // the pixel shader (all four coordinate sets map the texture's whole square, half a texel in from each edge). Returns 0.
    int DrawThroughShader();
    int DrawFrameAlt(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);  // EN 0x7955c0
    int DrawFrameAltStretched(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);  // EN 0x794c70 (JP 0x1014e5d0)
    int DrawFill(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);      // EN 0x798980 (JP 0x1014ce40)
    int DrawFillAlt(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);   // EN 0x799230
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

    // Points the batch at `buffer` of `capacity` vertices (inline in EN code; JP 0x10172a80).
    void SetBuffer(ScreenVertex *buffer, int capacityVertices)
    {
        vertices = buffer;
        capacity = capacityVertices;
    }
    int Reset();                                                         // EN 0x741ae0 (JP 0x10172aa0)
    // EN 0x70afe0 (JP 0x10172cb0): appends a quad (two triangles) from (left, top) to (right, bottom) at depth z with
    // texture coordinates (u0, v0)-(u1, v1), the alpha and two colours; the corners are turned by `rotation` when not 0.
    int AppendQuad(float left, float top, float right, float bottom, float u0, float v0, float u1, float v1, float z,
                   float alpha, float r, float g, float b, float r2, float g2, float b2, float rotation);
    // EN 0x70bce0 (JP 0x10188f10): AppendQuad with `texture`, flushing first when the texture changes or the buffer is full.
    int AddQuad(TextureObject *texture, float left, float top, float right, float bottom, float u0, float v0,
                float u1, float v1, float z, float alpha, float r, float g, float b, float r2, float g2, float b2,
                float rotation);
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
    int OnLostDevice();            // EN 0x79f760 (JP 0x1014b2a0): releases every default-pool resource before the device is reset
    int OnResetDevice();           // EN 0x79cd80 (JP 0x1013fbb0): recreates them afterwards
    int PumpMessages();            // EN 0x7a2240 (JP 0x1013ec90): handles pending window messages
    int PumpAndRecover();          // EN 0x7a2050 (JP 0x1015c850): message pump that waits while the game is inactive or the device is lost
    int Present(const struct tagRECT *source);   // EN 0x7a7fa0 (JP 0x1013edc0): ends the scene and presents a rectangle of the back buffer
    int ResetFrameTimer();         // EN 0x7a40f0 (JP 0x1013eee0): sets the next-frame deadline to now plus one frame (FrameTimer_ResetNext)
};

// The renderer object (`this` of the TextureManager methods). EN 0x00da747c (JP DAT_10836c70).
extern "C" TextureManager g_textureManager;

// Releases the Direct3D data of every texture whose alpha is generated (before a device reset). EN 0x740bb0 (JP 0x101734d0).
extern "C" int Textures_ReleaseGenerated(void);
// Recreates those textures after a device reset (not ported yet). EN 0x73fb80 (JP 0x10173510).
extern "C" int Textures_RestoreGenerated(void);

// Creates a render-target texture called `name` of width x height from `pixels` (one source pixel for targets), with
// the given alpha mode, and returns its slot. EN 0x71c5b0 (JP 0x10175c90).
extern "C" int Texture_CreateFromPixels(const char *name, int width, int height, unsigned char *pixels, int alpha, int flags);

// Like Texture_CreateFromPixels with the size, pixels and alpha taken from `source` (all white when it kept no pixels).
// Returns the slot, or -1. EN 0x71c740 (JP 0x10175a90).
extern "C" int Texture_CreateCopy(const char *name, const TextureObject *source, int flags);

// Loads a .bmp texture (with an optional alpha image) and returns its slot. EN 0x737ac0 (JP 0x10173550).
extern "C" int Texture_LoadBmp(const char *path, const char *alphaPath, int reload);

// Loads the texture called `name` from the data folder: an .iti file when the name ends in "iti", else a .bmp with the
// optional alpha image `alphaName` (0 = none). Returns its slot. EN 0x609bf0 (JP 0x1001e1a0).
extern "C" int Texture_Load(const char *name, const char *alphaName);

// Loads the texture file `path` (an .iti file) and returns its slot; `a` and `b` are the load options. EN 0x736f60 (JP 0x10174ce0).
extern "C" int Texture_LoadIti(const char *path, int a, int b);

// Draws texture slot `slot` into a screen rectangle from a source rectangle in texture pixels; 0 for a negative slot.
extern "C" int Texture_DrawSpritePixels(int slot, int left, int top, int right, int bottom, float u0, float v0, float u1,
                                        float v1, float z, float alpha, float angle);   // EN 0x71d180 (JP 0x101616e0)

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

// Inline copy of Texture_Release: the original inlines it into callers that release a texture slot.
__forceinline int TextureSlot_Release(int slot, int force)
{
    if (slot == -1 || g_textureNames[slot][0] == 0)
        return -1;
    g_textureRefCount[slot]--;
    char refs = g_textureRefCount[slot];
    if (force != 0 || refs == 0)
    {
        g_textureRefCount[slot] = 0;
        g_textures[slot].Unload();
        g_textureNames[slot][0] = 0;
    }
    return g_textureRefCount[slot];
}

// Switches the batch to blend mode `mode`, flushing the queued triangles when the mode changes.
inline int TriangleBatch::SetBlend(int mode, int unused)
{
    if (blendMode != BLEND_MODE_UNSET)
    {
        if (blendMode != mode)
            Flush();
        blendMode = mode;
        return 0;
    }
    blendMode = mode;
    return 0;
}

// Queues a textured quad (two triangles, six vertices) with `newTexture`, flushing first when the texture changes or
// the buffer is full. Returns 0. Called by the effect record drawing.
inline int TriangleBatch::AddQuad(TextureObject *newTexture, float left, float top, float right, float bottom, float u0,
                                  float v0, float u1, float v1, float z, float alpha, float r, float g, float b,
                                  float r2, float g2, float b2, float rotation)
{
    g_pendingBatches++;
    if (texture != 0 && texture != newTexture)
        Flush();
    texture = newTexture;
    if (count + TRIANGLE_VERTICES * 2 >= capacity)
        Flush();
    AppendQuad(left, top, right, bottom, u0, v0, u1, v1, z, alpha, r, g, b, r2, g2, b2, rotation);
    return 0;
}

extern "C" int Texture_BindStage1(int index);   // EN 0x7469d0 (JP 0x101610b0): binds slot `index` to stage 1 (negative = unbind)
extern "C" int Texture_BindStage0(int index);   // EN 0x746a10 (JP 0x10161070): binds slot `index` to stage 0 (negative = unbind)

#endif
