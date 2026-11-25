// Turning the model bank on (EN build; JP twin: 0x101649c0).
#include "model_bank.h"

// Empties the bank (no lit models, no light nodes, no shadow casters), gives the spare lighting entry the default
// lights of a new lit model (three lights, each a direction and a white colour: the key light pointing +z, a fill light
// pointing -y and a weak one pointing +y at half strength) and switches the bank on. Called when a map that uses the
// bank is started. Returns 1. EN 0x734b50.
int ModelBank::Enable()
{
    m_313e8 = 0;
    for (int i = 0; i < MODEL_BANK_LIT_COUNT; i++)
        litModels[i] = 0;
    for (int i = 0; i < MODEL_BANK_LIGHT_COUNT; i++)
        lights[i] = 0;
    shadowCasterCount = 0;
    spareLighting[0].x = 0;
    spareLighting[0].y = 0;
    spareLighting[0].z = 1.0f;
    spareLighting[0].w = 1.0f;
    spareLighting[1].x = 1.0f;
    spareLighting[1].y = 1.0f;
    spareLighting[1].z = 1.0f;
    spareLighting[1].w = 1.0f;
    spareLighting[2].x = 0;
    spareLighting[2].y = -1.0f;
    spareLighting[2].z = 0;
    spareLighting[2].w = 1.0f;
    spareLighting[3].x = 1.0f;
    spareLighting[3].y = 1.0f;
    spareLighting[3].z = 1.0f;
    spareLighting[3].w = 1.0f;
    spareLighting[4].x = 0;
    spareLighting[4].y = 0.5f;
    spareLighting[4].z = 0;
    spareLighting[4].w = 1.0f;
    spareLighting[5].x = 1.0f;
    spareLighting[5].y = 1.0f;
    spareLighting[5].z = 1.0f;
    spareLighting[5].w = 1.0f;
    enabled = 1;
    return 1;
}
