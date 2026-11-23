// Globals of the renderer, textures, meshes, scene nodes and effects.
// Generated from the per-file extern declarations by tools/globals_driver.py; this is the one
// declaration of each of them: include this header, never redeclare one. Comments say what is known.
#ifndef GURUMIN_RENDER_VARS_H
#define GURUMIN_RENDER_VARS_H

#include "d3d9_min.h"
#include "matrix4.h"
#include "model_def.h"
#include "vec4.h"

struct FadeNode;   // node_fade.h
struct FadeOwner;   // node_fade.h
struct IUnknownLike;   // com_min.h
struct MeshData;   // mesh.h
struct Node;   // node.h

struct TextureRef;   // texture_ref.h

extern "C" int DAT_10202b08;                              // debug keys enabled
extern "C" int DAT_10202b0c;                              // number of frame buffers
extern "C" float DAT_10202b18;                            // largest viewport size (both width and height); size of the square crossfade targets
extern "C" int DAT_10202b24;                              // back-buffer width set at device creation
extern "C" int DAT_10202b28;                              // back-buffer height
extern "C" int DAT_10202b34;                              // back buffer width
extern "C" int DAT_10202b38;                              // back-buffer height
extern "C" int DAT_10203344;
extern "C" int DAT_1020340c;                              // texture memory budget of the pool
extern "C" float DAT_10203420;
extern "C" int DAT_10203424;                              // counts requests whose x is 530 or more (never read here)
extern "C" int DAT_10203434;                              // 1 when shadows are drawn
extern "C" int DAT_10203438;                              // shadows switched on
extern "C" int DAT_1020343c;                              // lens flares switched on
extern "C" int DAT_10203474;
extern "C" int DAT_10203478;
extern "C" int DAT_1020347c;
extern "C" FadeNode *DAT_1021a1e0[];                      // the nodes
extern "C" FadeOwner *DAT_10254ac0[];                     // owner of each node (0 = none)
extern "C" int DAT_104b7224;                              // texture slot of the iris wipe
extern "C" int DAT_104bfe68;
extern "C" int DAT_104bfeb0;
extern "C" int DAT_104bfeb4;
extern "C" int DAT_104c0198;
extern "C" int DAT_104c0228;
extern "C" int DAT_104c02f0;                              // "DAT" source of the player for kind 4
extern "C" char DAT_104c0309;
extern "C" int DAT_104c0314;                              // "DAT" source of the other characters
extern "C" int DAT_104c03b0;                              // number of fading nodes
extern "C" float DAT_104c03ec;                            // opening progress of the overlay mode
extern "C" Vec4 DAT_10868818;                             // saved player velocity
extern "C" ModelDef DAT_10b314c4[];
extern "C" int DAT_10c636c4;
extern "C" int DAT_10c652cc;                              // "DAT" source of the player for the other kinds
extern "C" Vec4 DAT_10c66edc;                             // overlay camera position
extern "C" Vec4 DAT_10da9150;                             // saved player position
extern "C" Vec4 DAT_10edbdf8;                             // overlay camera target
extern "C" int DAT_10f08d30[2];                           // frames left for each of the two colour blends (0 = idle)
extern "C" IUnknownLike *DAT_10f08d38;                    // depth buffer for off-screen targets
extern "C" unsigned long DAT_10f08d40[8][0x100];
extern "C" int DAT_10f0cd40[0x200];
extern "C" void *DAT_10f0d540;                            // saved depth buffer
extern "C" int DAT_10f0d5c8;                              // draw calls this frame (statistics); number of draw calls this frame
extern "C" IUnknownLike *DAT_10f0d5e0[0x5a0];
extern "C" D3DVIEWPORT9_ DAT_10f0ece0;                    // saved viewport
extern "C" int DAT_10f0edf8;
extern "C" IUnknownLike *DAT_10f0ee00[16][0x46];          // vertex/index buffers of the 16 draw lists (FUN_1014ddf0)
extern "C" int DAT_10f0ff80;
extern "C" char DAT_10f0ff84[];                           // the D3DPRESENT_PARAMETERS used for Reset
extern "C" int DAT_10f140c0;
extern "C" int DAT_10f141d4;                              // number of texture switches this frame (statistics)
extern "C" IUnknownLike *DAT_10f141d8;                    // the IDirect3D9 object
extern "C" unsigned long DAT_10f141e0[8][0x100];
extern "C" int DAT_10f181e0[0x5a0];
extern "C" IUnknownLike *DAT_10f1a864;
extern "C" int DAT_10f1a86c;
extern "C" IUnknownLike *DAT_10f1a870[0x5a0];
extern "C" IUnknownLike *DAT_10f1bff0[0x200];
extern "C" IUnknownLike *DAT_10f1c7f0[0x5a0];
extern "C" int DAT_10f1de70[0x200];
extern "C" int DAT_10f1e670[0x5a0];
extern "C" IDirect3DDevice9 *DAT_10f1fcf0;                // the Direct3D device
extern "C" float DAT_10f1fcf8;                            // step per update; step per tick
extern "C" IUnknownLike *DAT_10f1fcfc[2];                 // the two swap targets, created with the device (Render_CreateDevice)
extern "C" IUnknownLike *DAT_10f1fd00;
extern "C" IUnknownLike *DAT_10f1fd04[8];                 // render-target surfaces
extern "C" void *DAT_10f1fd0c;                            // the crossfade pixel shader
extern "C" IUnknownLike *DAT_10f1fd2c;
extern "C" D3DVIEWPORT9_ DAT_10f1fd30;                    // the full-screen viewport
extern "C" D3DVIEWPORT9_ DAT_10f1fd48;                    // the off-screen viewport
extern "C" int DAT_10f1fd90[0x200];
extern "C" char DAT_10f60690[0x5a0];
extern "C" void *DAT_10f60d30;                            // saved render target
extern "C" IUnknownLike *DAT_10f60d38[0x5a0];
extern "C" unsigned long DAT_10f623d0[0x100];
extern "C" float DAT_10f628d0;                            // limit
extern "C" int DAT_10f628e8[0x5a0];
extern "C" char DAT_10f64368[4][0x100];
extern "C" int DAT_10f65470;                              // saved width
extern "C" int DAT_10f65474;                              // saved height
extern "C" int DAT_10f65478;                              // non-zero while an off-screen target is set up (then the saved state is kept); 1 while the indicator draws (render target reset in progress)
extern "C" int DAT_10f6547c;
extern "C" int DAT_10f65480;                              // split screen: 0 = whole screen, 1 = left half, 2 = right half
extern "C" int DAT_10f65484;                              // non-zero = render to the back buffer (no swap target)
extern "C" int DAT_10f6548c;
extern "C" int DAT_10f65490;                              // swap target in use (0 or 1)
extern "C" int DAT_10f65494;
extern "C" int DAT_10f65498;                              // 1 while the frame blend is active
extern "C" float DAT_10f6549c;                            // current strength
extern "C" IUnknownLike *DAT_10f654ac;                    // dynamic vertex buffer of the 2D drawing code
extern "C" int DAT_10f654c0;                              // counts down by one per tick
extern "C" int DAT_10f654c4;
extern "C" int DAT_10f654cc;                              // non-zero when off-screen render targets are supported; render-to-texture available (RenderTarget_Probe); 1 while a texture is the render target
extern "C" int DAT_10f654d0;                              // 1 = rendering to textures works
extern "C" int DAT_10f654d4;                              // 1 = the first read-back saw the blue square
extern "C" int DAT_10f654dc;                              // frames since the rate was last measured
extern "C" int DAT_10f654e4;                              // 1 = screen-size crossfade targets
extern "C" float DAT_10f654e8;                            // measured frames per second
extern "C" int DAT_10f654f0;                              // 1 = wait with Sleep and run one step per frame (smooth mode)
extern "C" int DAT_10f654f4;                              // frame blending allowed by the settings
extern "C" char DAT_10f65500;
extern "C" int DAT_10f65504;                              // engine ticks so far
extern "C" int DAT_10f65514;
extern "C" int DAT_10f65520;                              // 1 while the overhead pass draws
extern "C" int DAT_10f6552c;                              // set when Windows asked the game to quit (WM_QUIT); 1 once WM_QUIT was received
extern "C" int DAT_10f65530;                              // 1 while the game window is active
extern "C" int DAT_10f6553c;
extern "C" int DAT_10f65540;                              // with 32-bit colour: non-zero = R8G8B8, else X8R8G8B8
extern "C" int DAT_10f6559c;
extern "C" int DAT_10f655a0;
extern "C" int DAT_10f655ac;                              // cel.bmp (cel shading ramp)
extern "C" int DAT_10f655b0;                              // cel2.bmp
extern "C" int DAT_10f655c4;
extern "C" int DAT_10f656a0;                              // 1 = keep running while inactive
extern "C" char DAT_10f65754[];                           // its window class name
extern "C" Vec4 DAT_10f657c4[2];                          // per-frame colour steps
extern "C" Vec4 DAT_10f65828[2];                          // target colours (set exactly on the last frame to avoid rounding drift)
extern "C" Vec4 DAT_10f66948;                             // the current draw colour
extern "C" Vec4 DAT_10f66978[2];                          // current colours
extern "C" unsigned char DAT_10f68138[0x25800];           // buffer for one streamed frame record
extern "C" int DAT_10f8d9b8[10];                          // copy of the shadow targets
extern "C" char DAT_10f8dbb8[1600][0x40];
extern "C" char DAT_10fa6db8[1600];
extern "C" int DAT_10fa73f8[32];                          // their definitions
extern "C" MeshData *DAT_10fa7478[];                      // per mesh buffer slot: the mesh
extern "C" char DAT_10fada90[0xc80][0x20];
extern "C" char DAT_10fdeb90[1600];                       // reference count per texture slot
extern "C" TextureRef *DAT_10fdf1d0[];                   // per motion cache slot: the shared reference (scene_node.cpp)
extern "C" int DAT_10fe01d8[];
extern "C" int DAT_10feb3e0[0xc80];                       // per mesh buffer slot: 1 = owned by a single node; reference count of each model name
extern "C" int DAT_10fee5e0[];
extern "C" int DAT_10ff19e8[0x100];                       // collision data of each model table entry; per model: its collision data
extern "C" int DAT_10ff1dec;                              // batches still pending this frame
extern "C" int DAT_10ff1df8[0x47e];                       // 1 once a definition's resources are loaded
extern "C" int DAT_10ff2ff8[];
extern "C" char DAT_10ffe5f8[1600][0x40];                 // their names (empty = free)
extern "C" int DAT_110175f8;                              // number of motion blends started
extern "C" int DAT_11017600[];                            // the number of frames
extern "C" int DAT_110186a8[32];                          // recently started script ids
extern "C" Matrix4 *DAT_1101c820[];                       // the frame matrices
extern "C" int DAT_1101d864;
extern "C" int DAT_1101d874;                              // texture memory in use (bytes)
extern "C" int DAT_1101d878;                              // non-zero = images without generated alpha are not kept; set while loading the shared textures; set while creating shared textures
extern "C" int DAT_1101de34;
extern "C" int DAT_1101de60;                              // effects started so far
extern "C" char DAT_1101de64;                             // next entry of that cache
extern "C" Matrix4 DAT_118e69a8;
extern "C" char DAT_11af3f68[0x40];                       // light matrix set of each node (drawing mode 2)
extern "C" char DAT_11afffa8[0x40];                       // light set of each node (drawing mode 2)
extern "C" Node *DAT_11b00070[0x40];                      // the queued nodes
extern "C" int DAT_11b00170[0x40];                        // the value DAT_10f655c4 takes while the node draws
extern "C" char DAT_11b00270[0x40];                       // the value DAT_10f65520 takes for drawing mode 2
extern "C" int DAT_11b002b8;
extern "C" char DAT_11b002c8;                             // 1 while a model-against-world test runs
extern "C" int DAT_11b044d0;                              // number of queued nodes
extern "C" int DAT_11b044dc;
extern "C" int DAT_11b044e0;
extern "C" int DAT_11b044e4;
extern "C" int DAT_11b044e8;
extern "C" int DAT_11b044ec;                              // non-zero = the plugs do not give the frame
extern "C" int DAT_11b044f4;
extern "C" float DAT_11b044f8;                            // radius of the current proximity pass (used by M_10194f40); the value the frame-giving plugs are sampled at
extern "C" int DAT_11b04500;
extern "C" int DAT_11b04504;
extern "C" char DAT_11b04508;                             // set while an important model changes motion (read by the playback code)
extern "C" int DAT_11b0450c;
extern "C" int DAT_11b04510;                              // non-zero = also test the nodes marked in m_0704
extern "C" char DAT_11b04514;                             // non-zero = only the enabled collision regions are drawn
extern "C" Vec4 DAT_11b04518[0x40];
extern "C" Vec4 DAT_11b04918[0x40];
extern "C" Matrix4 DAT_11b06518[0x30];                    // light matrix sets
extern "C" char DAT_11b07118[];                           // default target object of saved-matrix state 0x400000
extern "C" Vec4 DAT_11b07128[0x40];                       // colour of each queued node

#endif
