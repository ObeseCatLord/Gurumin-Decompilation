// A texture object (0x38 bytes; the table of 1600 at DAT_118d0ba8 holds every texture slot): owns a Direct3D
// texture. Animated or streamed textures keep their frames in the buffer pool (buffer_pool.cpp); for those
// `poolSlot` != 0 and the texture to bind is fetched from the pool when the object is bound. Methods:
// texture_bind.cpp (binding, drawing, writing, capturing), texture_create.cpp (Create), texture_mgr.cpp.
// This is the one declaration of this type: include this header, never redeclare it.
#ifndef GURUMIN_TEXTURE_OBJECT_H
#define GURUMIN_TEXTURE_OBJECT_H

#include "d3d9_min.h"

struct Vec4;
struct ScreenVertex;

// Number of texture slots in DAT_118d0ba8.
#define TEXTURE_SLOT_COUNT 1600

struct TextureObject
{
    IDirect3DTexture9Like *texture;     // 0x00: Direct3D texture
    IDirect3DSurface9Like *surface;     // 0x04: surface of a render-target texture (or a second part), may be 0
    int width;               // 0x08: texture width in pixels
    int height;              // 0x0c: texture height in pixels
    int target;              // 0x10: 1 = R5G6B5 render target, 2 = A8R8G8B8 render target, else 0
    int poolIndex;           // 0x14: buffer pool index of the frames
    int pad_0018;
    int poolSlot;            // 0x1c: non-zero when the frames live in the buffer pool (1 = R5G6B5, 2 = A4R4G4B4)
    int sourceAlpha;         // 0x20: the alpha pixels it was created with (or 0, 1, 2), see Create
    int sourcePixels;        // 0x24: copy of the RGB pixels it was created from (0 when none was kept)
    int pad_0028;
    int sourceWidth;         // 0x2c: size of the source pixels
    int sourceHeight;        // 0x30
    void *loaded;            // 0x34: non-zero while the slot's texture data is loaded (texture_mgr.cpp)
    int BindStage0();
    int BindStage1();
    int Write16(int x, int y, int rowPixels, int rows, int pitch, const void *bits);
    int Release();
    int CaptureRgb24(unsigned char *out);
    int CaptureRgbRows(unsigned char *out);
    int CaptureRgbFirst(unsigned char *out);
    int BeginRenderTo(int width, int height, int offset);
    int HalveRgb24(unsigned char *pixels, int width, int height);
    int Upload(int width, int height, unsigned char *pixels, const unsigned char *alpha);
    int DrawTriangles(const void *vertices, int triangles);
    int DrawStrip(struct ScreenVertex *vertices, int count);
    int DrawScreenCopy(int left, int top, int right, int bottom, float z, float alpha);
    int DrawQuad(const struct Vec4 *positions, const struct Vec4 *uvs, const struct Vec4 *colours,
                 const struct Vec4 *speculars, float alpha, int skip);
    int DrawThroughShader(TextureObject *second, TextureObject *third);
    int DrawCell(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                 float alpha, float angle);
    int DrawTinted(int left, int top, int right, int bottom, float u0, float v0, float u1, float v1, float z,
                   float alpha, float angle, float grey);
    int DrawFrame(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);
    int DrawFillAlt(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);
    int DrawFrameAlt(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);
    int DrawFrameAltStretched(int left, int top, int right, int bottom, float z, float alpha, int textured,
                              int border);
    int DrawFrameAltTinted(int left, int top, int right, int bottom, float z, float alpha, int textured, int border,
                           unsigned int rgb);
    int DrawFrameAltPlain(int left, int top, int right, int bottom, float z, float alpha, int vertexAlpha,
                          int border, unsigned int rgb);
    int WriteRgb32(int left, int top, int right, int bottom, int pitch, const unsigned char *bits);
    int WritePaletted(int left, int top, int right, int bottom, int pitch, const unsigned char *bits,
                      const unsigned short *palette);
    // 0x10169c60 (texture_create.cpp): creates the texture from `width` x `height` RGB pixels; `alpha` is 0 (none),
    // 1 or 2 (generated) or the alpha pixels. Returns 0 on failure.
    int Create(int width, int height, const void *pixels, int alpha, int flags);
    void Recreate();                                 // 0x10169df0: re-creates the texture after a device reset
    void Restore();                                  // 0x10169e10: restores the texture data
    void Unload();                                   // 0x10169e40: releases the slot's texture
    int UpdateImage(int flags);                      // 0x10169f70: makes the texture the render target (0 = failed)
    // 0x1014ce40 (named from game_panels.cpp, not yet defined in a source file): fills the panel background; the
    // parameters follow DrawFrame's.
    int DrawFill(int left, int top, int right, int bottom, float z, float alpha, int textured, int border);
};

// Every texture slot.
extern "C" TextureObject DAT_118d0ba8[TEXTURE_SLOT_COUNT];

#endif
