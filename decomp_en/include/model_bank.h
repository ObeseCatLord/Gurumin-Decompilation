// The model bank (EN build): up to 16 extra models (map objects, effects) with their own model records, the scene's
// light nodes and the shadow casters. Only the members EN code has used are named.
// This is the one declaration of ModelBank in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_MODEL_BANK_H
#define GURUMIN_EN_MODEL_BANK_H

#include "anim_model.h"

// Number of model records in the bank.
const int MODEL_BANK_SIZE = 16;
// Light nodes of the scene.
const int MODEL_BANK_LIGHT_COUNT = 0x20;
// Models that take part in the scene lighting.
const int MODEL_BANK_LIT_COUNT = 0x80;
// Light vectors of each lit model.
const int MODEL_BANK_LIGHT_VECTORS = 22;
// AnimModel::collisionLimit of a bank model that never times out.
const int MODEL_BANK_NO_TIMEOUT = 99999;
// Lens flare points: entries of the brightness table, and of the screen position table (up to the end of the bank).
const int MODEL_BANK_FLARE_COUNT = 0x20;
const int MODEL_BANK_SCREEN_COUNT = 0x10;
// Bytes of a ModelBank.
const int MODEL_BANK_BYTES = 0x31624;

struct ModelBank
{
    Node *lights[MODEL_BANK_LIGHT_COUNT];        // 0x0000: light nodes of the scene
    AnimModel *litModels[MODEL_BANK_LIT_COUNT];  // 0x0080: models that take part in the scene lighting
    char m_0280[0x1884 - 0x280];                 // 0x0280: shadow render targets and casters
    int shadowCasterCount;                       // 0x1884: number of shadow casters
    Vec4 lighting[MODEL_BANK_LIT_COUNT * MODEL_BANK_LIGHT_VECTORS];  // 0x1888: 22 light vectors for each entry of litModels
    char m_c888[0xc9e8 - 0xc888];                // 0xc888: lighting vectors of the spare entry
    AnimModel models[MODEL_BANK_SIZE];           // 0xc9e8: the bank's model records (root = 0: free)
    const Vec4 *params[MODEL_BANK_SIZE];         // 0x313a8: per bank model: the position it is placed at
    int m_313e8;                                 // 0x313e8: not recovered
    int enabled;                                 // 0x313ec: 1 when the current map uses the bank
    char m_313f0[0x31470 - 0x313f0];             // 0x313f0: lens flare textures
    float flareAlpha[MODEL_BANK_FLARE_COUNT];    // 0x31470: brightness of each flare point (0..1)
    char m_314f0[0x31520 - 0x314f0];             // 0x314f0: render targets
    int flareCount;                              // 0x31520: flare points collected this frame
    Vec4 flareScreen[MODEL_BANK_SCREEN_COUNT];   // 0x31524: screen position of each flare point (x, y, depth, w)

    int Reset();                                 // EN 0x740e20 (JP model_bank.cpp)
    int Add(Node *root, const Vec4 *position);   // EN 0x70a440 (JP 0x1017c570): puts the model with node tree `root` in the first free record; 0 when all are in use
    int AddLitModel(AnimModel *model);           // EN 0x70a2b0 (JP 0x10164af0): adds `model` to the lit models with default light vectors
    int CollectLights(Node *root);              // EN 0x745b60 (JP 0x1017c610): rebuilds the light list from the "Omn" and "Spot" nodes below `root`
    int AddFlarePoint(Vec4 position, const Matrix4 *viewProjection);   // EN 0x70ae10 (JP 0x10165140)
};

#endif
