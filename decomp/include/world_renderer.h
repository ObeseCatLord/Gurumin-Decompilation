// WorldRenderer: declared here once. The world object (global instance DAT_10c68000): the current map's models,
// textures, draw lists, fades and character messages (world_ctor.cpp, world_frame.cpp, world_model_state.cpp,
// scene_start.cpp).
#ifndef GURUMIN_WORLD_RENDERER_H
#define GURUMIN_WORLD_RENDERER_H

#include "vec4.h"
#include "matrix4.h"
#include "key_track.h"
#include "node.h"
#include "font_renderer.h"

struct AnimModel;

// Host notification callback of the world (codes such as 0x5e0 on release, 9999 when a character is loaded).
typedef int (*WorldCallback)(int code, int value);

// Number of picture slots (WorldRenderer::picture*).
#define PICTURE_COUNT 0x20

// What a picture shows (WorldRenderer::pictureKind).
enum PictureKind
{
    PICTURE_TEXTURE = 0,         // its texture slot with its own texture rectangle
    PICTURE_SCREEN = 1,          // the screen copy, a 2-texel border cut off
    PICTURE_SCREEN_FLIPPED = 2,  // the screen copy upside down, a 2-texel border cut off
    PICTURE_SCREEN_EXACT = 3     // the screen copy with half-texel edges
};

// How a picture is blended (WorldRenderer::pictureBlend).
enum PictureBlend
{
    PICTURE_BLEND_ALPHA = 0,     // BLEND_ALPHA
    PICTURE_BLEND_ADD = 1,       // BLEND_ADD
    PICTURE_BLEND_MULTIPLY = 2,  // BLEND_MULTIPLY
    PICTURE_BLEND_SHADOWED = 3,  // drawn twice: darkened 2 pixels down and right, then added on top
    PICTURE_BLEND_DARKEN = 4     // BLEND_DARKEN
};

struct WorldRenderer
{
    Matrix4 m_00000;
    int m_00040;
    char characterNames[0x20][0x20];                 // 0x44: "chara%d" (in Japanese) until a character is loaded
    int m_00444[0x80];
    int m_00644[0x80];
    char m_00844[0x80];                              // 0x844: `important` of each model record
    char m_008c4[0x80];                              // 0x8c4: m_0267 of each model record
    int m_00944[3][4];
    char m_00974;                                    // 0x974: set for costume 6
    char m_00975;                                    // 0x975: non-zero = no screen fade
    char m_00976;
    char pad_00977[0x1];
    int fadeOnTop;                                   // 0x978: 1 = the fade is drawn below the cinema bars
    int m_0097c;
    int m_00980[1000];
    int m_01920[1000];
    int m_028c0[1000];
    int m_03860;
    int m_03864;
    int m_03868;
    FontRenderer *font;                              // 0x386c: the text renderer (captions)
    int captionVisible;                              // 0x3870: 1 = the caption is drawn
    int captionSize;                                 // 0x3874: font size in percent, less 25
    int captionX;                                    // 0x3878: left of the caption, -1 = centred
    int captionY;                                    // 0x387c: baseline of the caption
    char caption[0x100];                             // 0x3880: caption text; a literal backslash-n splits two lines, an opening bracket starts the speech
    int m_03980;
    int m_03984;
    int m_03988;
    char m_0398c;
    char pad_0398d[0x3];
    int fadeColour;                                  // 0x3990: colour of the full-screen fade
    float fadeStep;                                  // 0x3994: added to fadeAmount each frame
    int fadeFrames;                                  // 0x3998: frames left of the fade
    float fadeTarget;
    float fadeAmount;                                // 0x39a0: fade strength in percent (0 = off)
    int fadeMode;
    int m_039a8;
    char m_039ac[0x20][0x10][0x10];
    char pad_059ac[0x200];
    char m_05bac[0x20];
    char pad_05bcc[0xe0];
    int m_05cac[0x20];
    char pad_05d2c[0x80];
    int m_05dac[0x20];
    char pad_05e2c[0x4];
    Vec4 m_05e30;
    KeyBank m_05e40[4];
    int m_08850;
    int m_08854;
    int m_08858;
    int m_0885c;
    AnimModel *models;                               // 0x8860: the map's model records (the character table)
    char pad_08864[0x4];
    Node nodes[0x20];                                // 0x8868: map model nodes
    float m_39c68[0x20];
    float m_39ce8[0x20];
    float m_39d68[0x20];
    float m_39de8[0x20];
    float m_39e68[0x20];
    float m_39ee8[0x20];
    float m_39f68[0x20];
    float m_39fe8[0x20];
    float m_3a068[0x20];
    float m_3a0e8[0x20];
    float m_3a168[0x20];
    float m_3a1e8[0x20];
    float m_3a268[0x20];
    float m_3a2e8[0x20];
    float m_3a368[0x20];
    float m_3a3e8[0x20];
    float m_3a468[0x20];
    float m_3a4e8[0x20];
    KeyBank m_3a568[0x20];
    char pad_4f5e8[0x80];
    char m_4f668[0x20][0x20];
    char loaded;                                     // 0x4fa68: 1 while a map is loaded
    char pad_4fa69[3];
    WorldCallback callback;                          // 0x4fa6c: host notification callback
    short messageIds[2400];                          // 0x4fa70: id of each line of the character's .itm file
    char messages[2400][0x100];                      // 0x50d30: the lines ("##nnnnn text")
    int messageCount;
    int m_e6d34[0x14];
    int m_e6d84[0x14];
    char pad_e6dd4[0x58];
    int enabled;                                     // 0xe6e2c: frame updates on
    int frame;                                       // 0xe6e30: current world frame
    int lastFrame;                                   // 0xe6e34: frame the draw lists were last built for
    int m_e6e38[0x80];
    int m_e7038[0x80];
    char pad_e7238[0x200];
    Vec4 m_e7438[0x80];
    char m_e7c38[0x20][0x20];
    char pad_e8038[0xc00];
    char m_e8c38[0x80][0x20];
    char m_e9c38[0x20];
    char m_e9c58[0x60][0x20];
    char pad_ea858[0x400];
    int m_eac58[0x80];
    int m_eae58[0x80];
    short m_eb058[0x20];
    char pad_eb098[0x2c0];
    KeyBank m_eb358[0x80];
    Vec4 targets[0x80];                              // 0x13f558: target of each model's state
    char active[0x80];                               // 0x13fd58: 1 while the model's state runs
    char barsFull;                                   // 0x13fdd8: non-zero = show the cinema bars fully at once
    char pad_13fdd9[0x3];
    int textures[0x20];                              // 0x13fddc: texture slots used by the map (-1 = none)
    // The 0x20 pictures (2D images placed by the scripts, drawn by DrawPictures 0x10167440): a picture shows a
    // texture slot (`textures`) or, for a non-zero kind, the copy of the screen (`screenTexture`).
    float pictureAlpha[0x20];                        // 0x13fe5c: opacity of each picture
    char pad_13fedc[0x100];
    Vec4 pictureUVs[0x20];                           // 0x13ffdc: texture rectangle u0, v0, u1, v1 (kind 0)
    Vec4 m_1401dc[0x20];
    char pad_1403dc[0x80];
    float pictureAngle[0x20];                        // 0x14045c: rotation of each picture
    char pad_1404dc[0x100];
    float pictureGrey[0x20];                         // 0x1405dc: grey level of the specular colour (0..1)
    char pad_14065c[0x100];
    Vec4 picturePositions[0x20];                     // 0x14075c: centre (640x480 screen, or world when in-world)
    Vec4 m_14095c[0x20];
    char pad_140b5c[0x80];
    Vec4 pictureSizes[0x20];                         // 0x140bdc: width (x) and height (y) on the 640x480 screen
    Vec4 m_140ddc[0x20];
    char pad_140fdc[0x80];
    char pictureInWorld[0x20];                       // 0x14105c: non-zero = placed in the world (not drawn here)
    char pictureBlend[0x20];                         // 0x14107c: PictureBlend
    char pictureKind[0x20];                          // 0x14109c: PictureKind
    int screenTexture;                               // 0x1410bc: texture slot holding the copy of the screen
    int screenWidth;                                 // 0x1410c0: width of the screen copy in texels
    int screenHeight;                                // 0x1410c4: height of the screen copy in texels
    int m_1410c8;
    int notifyPending;                               // 0x1410cc: a 0x5e0 notification must be sent on release
    WorldRenderer();
    int Bind(AnimModel *models, void *mapRoot, int *state, WorldCallback callback, void *data);  // 0x1017c9c0
    int LoadCharacter(const char *path);                                                       // 0x10166950
    int M_10166910(int slot, int character, int value);                                        // 0x10166910 (CharacterBank::M_10166910 in model_misc.cpp)
    int StartFade(int mode, int target, int frames);
    void TickFade();
    int SetModelState(int index, int mode);  // 0x10166f20 (world_model_state.cpp)
    int M_1017cc90(int flags);               // builds the world for a frame (world_frame.cpp)
    int AdvanceFrame();
    int Release();
    int Seek(int target);                    // 0x10189000
    void DrawCaption();                      // 0x101687e0 (world_caption.cpp)
    void DrawScreenOverlays();               // 0x10184160 (world_caption.cpp)
    void DrawOverlay();
    void DrawPictures(const Matrix4 &view);  // 0x10167440 (world_pictures.cpp)
    void DrawWorldPictures(const Matrix4 *view, const Matrix4 *camera);  // 0x10167d90 (world_in_pictures.cpp)
};

extern "C" WorldRenderer DAT_10c68000;  // the world

#endif
