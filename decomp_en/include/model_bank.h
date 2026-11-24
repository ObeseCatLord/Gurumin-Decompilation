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
// Bytes of a ModelBank.
const int MODEL_BANK_BYTES = 0x31624;

struct ModelBank
{
    Node *lights[MODEL_BANK_LIGHT_COUNT];        // 0x0000: light nodes of the scene
    AnimModel *litModels[MODEL_BANK_LIT_COUNT];  // 0x0080: models that take part in the scene lighting
    char m_0280[0x1884 - 0x280];                 // 0x0280: shadow render targets and casters
    int shadowCasterCount;                       // 0x1884: number of shadow casters
    char m_1888[0xc9e8 - 0x1888];                // 0x1888: lighting vectors
    AnimModel models[MODEL_BANK_SIZE];           // 0xc9e8: the bank's model records (root = 0: free)
    char m_313a8[0x313e8 - 0x313a8];             // 0x313a8: per model placement
    int m_313e8;                                 // 0x313e8: not recovered
    char m_313ec[MODEL_BANK_BYTES - 0x313ec];    // 0x313ec: render targets and lens flare

    int Reset();                                 // EN 0x740e20 (JP model_bank.cpp)
};

#endif
