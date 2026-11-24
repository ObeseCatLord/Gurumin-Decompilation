// The model bank (EN build): up to 16 extra models (map objects, effects) with their own model records, the scene's
// light nodes and the shadow casters. Only the members EN code has used are named.
// This is the one declaration of ModelBank in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_MODEL_BANK_H
#define GURUMIN_EN_MODEL_BANK_H

#include "anim_model.h"

struct DrawList;   // the list the node drawing queues into (node.h)

// Number of model records in the bank.
const int MODEL_BANK_SIZE = 16;
// Shadow render targets, lens flare textures and crossfade render targets of the bank.
const int MODEL_BANK_SHADOW_TARGETS = 10;
const int MODEL_BANK_LENS_TEXTURES = 4;
const int MODEL_BANK_CROSSFADE_TARGETS = 3;
// Light nodes of the scene.
const int MODEL_BANK_LIGHT_COUNT = 0x20;
// Models that take part in the scene lighting.
const int MODEL_BANK_LIT_COUNT = 0x80;
// Light vectors of each lit model.
const int MODEL_BANK_LIGHT_VECTORS = 22;
// AnimModel::collisionLimit of a bank model that never times out.
const int MODEL_BANK_NO_TIMEOUT = 99999;
// Lens flare points: entries of the brightness table, and of the screen position table .
const int MODEL_BANK_FLARE_COUNT = 0x20;
const int MODEL_BANK_SCREEN_COUNT = 0x20;
// Bytes of a ModelBank.
const int MODEL_BANK_BYTES = 0x31724;   // (was 0x31624: the constructor at EN 0x70a120 constructs 0x20 flare vectors from 0x31524)

struct ModelBank
{
    Node *lights[MODEL_BANK_LIGHT_COUNT];        // 0x0000: light nodes of the scene
    AnimModel *litModels[MODEL_BANK_LIT_COUNT];  // 0x0080: models that take part in the scene lighting
    int shadows[MODEL_BANK_SHADOW_TARGETS];      // 0x0280: the shadow render targets "shadow0".."shadow9"
    char m_02a8[0x480 - 0x2a8];                  // 0x02a8: not recovered
    int shadowTexture;                           // 0x0480: texture slot of the shadow blob (kage.bmp)
    Vec4 casterPosition[MODEL_BANK_LIT_COUNT];   // 0x0484: per shadow caster (passed to the shadow drawing)
    Vec4 casterParams[MODEL_BANK_LIT_COUNT];     // 0x0c84: per shadow caster
    float casterSize[MODEL_BANK_LIT_COUNT];      // 0x1484: per shadow caster
    int casters[MODEL_BANK_LIT_COUNT];           // 0x1684: shadow casters (indexes into litModels)
    int shadowCasterCount;                       // 0x1884: number of shadow casters
    Vec4 lighting[MODEL_BANK_LIT_COUNT * MODEL_BANK_LIGHT_VECTORS];  // 0x1888: 22 light vectors for each entry of litModels
    Vec4 spareLighting[MODEL_BANK_LIGHT_VECTORS];  // 0xc888: lighting vectors of the spare entry
    AnimModel models[MODEL_BANK_SIZE];           // 0xc9e8: the bank's model records (root = 0: free)
    const Vec4 *params[MODEL_BANK_SIZE];         // 0x313a8: per bank model: the position it is placed at
    int m_313e8;                                 // 0x313e8: not recovered
    int enabled;                                 // 0x313ec: 1 when the current map uses the bank
    int lens[MODEL_BANK_LENS_TEXTURES];          // 0x313f0: lens flare textures (lens01.bmp .. lens04.bmp)
    char m_31400[0x31470 - 0x31400];             // 0x31400: not recovered
    float flareAlpha[MODEL_BANK_FLARE_COUNT];    // 0x31470: brightness of each flare point (0..1)
    int crossfade[MODEL_BANK_CROSSFADE_TARGETS]; // 0x314f0: "crossfade0".."crossfade2" render targets (screen size)
    int mirror;                                  // 0x314fc: 256x256 render target
    int saveBuffer;                              // 0x31500: render target
    int minimap;                                 // 0x31504: render target
    int mirror2;                                 // 0x31508: render target
    int saveBuffer2;                             // 0x3150c: render target
    int minimap2;                                // 0x31510: render target
    char m_31514[0x31520 - 0x31514];             // 0x31514: not recovered
    int flareCount;                              // 0x31520: flare points collected this frame
    Vec4 flareScreen[MODEL_BANK_SCREEN_COUNT];   // 0x31524: screen position of each flare point (x, y, depth, w)

    ModelBank();                                 // EN 0x70a120 (JP 0x1017c8e0): builds the vector tables and model records, bank off
    int Reset();                                 // EN 0x740e20 (JP model_bank.cpp)
    int CreateTextures();                        // EN 0x7343a0 (JP 0x1017c160): loads the shadow and lens textures, creates the render targets
    int DrawLensFlares();                        // EN 0x729610 (JP 0x10166520)
    int DrawShadows(Node *scene, DrawList *list, Matrix4 *view, int a3);       // EN 0x72a420 (JP 0x10165e00)
    int DrawShadowsInto(Node *scene, DrawList *list, Matrix4 *view, int a3);   // EN 0x72a710 (JP 0x10166100)
    int Add(Node *root, const Vec4 *position);   // EN 0x70a440 (JP 0x1017c570): puts the model with node tree `root` in the first free record; 0 when all are in use
    int AddLitModel(AnimModel *model);           // EN 0x70a2b0 (JP 0x10164af0): adds `model` to the lit models with default light vectors
    int CollectLights(Node *root);              // EN 0x745b60 (JP 0x1017c610): rebuilds the light list from the "Omn" and "Spot" nodes below `root`
    const void *LightDirection(AnimModel *model, const Vec4 *eye, const Vec4 *target);   // EN 0x7311a0 (JP 0x10164c10): the lights that light `model` seen from `eye` towards `target`
    int AddFlarePoint(Vec4 position, const Matrix4 *viewProjection);   // EN 0x70ae10 (JP 0x10165140)
};

// Render and texture state the bank reads or sets (JP: decomp/include/render_vars.h).
extern "C" int g_shadowsEnabled;           // EN 0x00908a10 (JP DAT_10203434): 1 when shadows are drawn
extern "C" int g_shadowsSwitchedOn;        // EN 0x00908a14 (JP DAT_10203438): shadows switched on in the options
extern "C" int g_blueSquareSeen;           // EN 0x021e4808 (JP DAT_10f654d4): 1 = the first read-back saw the blue square (shadow drawing works)
extern "C" int g_shadowTargetsCopy[MODEL_BANK_SHADOW_TARGETS];   // EN 0x01664470 (JP DAT_10f8d9b8): copy of the shadow render targets
extern "C" int g_bankFlag;                 // EN 0x01664464 (JP DAT_1101d864): cleared when the bank's textures are created
extern "C" int g_sharedTextureLoading;     // EN 0x0166d004 (JP DAT_1101d878): non-zero while the shared textures load (images without generated alpha are not kept)
extern "C" int g_celTexture;               // EN 0x021a352c (JP DAT_10f655ac): cel.bmp (cel shading ramp)
extern "C" int g_celTexture2;              // EN 0x021a3530 (JP DAT_10f655b0): cel2.bmp
extern "C" int g_renderToTextureWorks;     // EN 0x021e4804 (JP DAT_10f654d0): 1 = rendering to textures works
extern "C" int g_crossfadeTargets;         // EN 0x021e47fc (JP DAT_10f654e4): 1 = screen-size crossfade targets
extern "C" int g_mirrorTargetA;            // EN 0x021a34d4 (JP DAT_10f6559c): the mirror target
extern "C" int g_mirrorTargetB;            // EN 0x021a34d8 (JP DAT_10f655a0): the mirror target
extern "C" int g_lensFlaresEnabled;        // EN 0x00908a18 (JP DAT_1020343c): lens flares switched on

// The model bank of the scene. EN 0x00d75d58 (JP DAT_10804bf8).
extern "C" ModelBank g_modelBank;

#endif
