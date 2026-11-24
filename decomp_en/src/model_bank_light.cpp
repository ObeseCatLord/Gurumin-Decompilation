// Lights of the bank models (EN build). JP twin: decomp/src/model_bank.cpp (ModelBank::LightForRoot).
#include "model_bank.h"
#include "vec4.h"

// The lit entry of the bank that holds the lights for models that are not bank models.
const int LIGHT_FALLBACK_ENTRY = MODEL_BANK_LIT_COUNT - 1;
// First vector of that entry in the lighting table.
const int FALLBACK_BASE = LIGHT_FALLBACK_ENTRY * MODEL_BANK_LIGHT_VECTORS;
// Vectors of a lit entry that LightForRoot fills in.
enum LightVector
{
    LIGHT_FILL_DIRECTION = 0,   // direction of the fill light
    LIGHT_FILL_COLOUR = 1,      // colour of the fill light
    LIGHT_KEY_DIRECTION = 6,    // direction of the key light, towards the eye
    LIGHT_KEY_COLOUR = 7,       // colour of the key light
    LIGHT_AMBIENT = 16          // ambient light colour
};
// (Constants passed by reference stay literals: a named float would be passed by its address.)
// Ambient brightness of the fallback lights.
const float FALLBACK_AMBIENT = 0.1f;
// Leftover light offsets of the original: a point built from the zero vector.
const float SCRATCH_OFFSET_X = 100.0f;
const float SCRATCH_OFFSET_Y = 200.0f;
const float SCRATCH_OFFSET_Z = 500.0f;

// ModelBank::LightDirection: returns the 22 light vectors of `model` (0 when the bank is not enabled or the model is not
// lit). When `eye` and `target` are given, the key light of the entry is aimed from `target` to `eye`, its colour
// grows with the distance and the ambient light is set. Without a model the last lit entry is rebuilt with the
// fallback lights instead. Called by the character and map drawing.
const void *ModelBank::LightDirection(AnimModel *model, const Vec4 *eye, const Vec4 *target)
{
    if (enabled == 0)
        return 0;
    if (model == 0)
    {
        if (eye != 0 && target != 0)
        {
            Vec4 zero;
            zero.Set(0, 0, 0, 0);
            Vec4 scratchColour;
            Vec4 scratchPosition;
            Vec4 white(1.0f, 1.0f, 1.0f, 0);
            scratchColour.Copy(zero);
            scratchPosition.Copy(zero);
            scratchPosition.z += SCRATCH_OFFSET_Z;
            scratchPosition.x += SCRATCH_OFFSET_X;
            scratchPosition.y -= SCRATCH_OFFSET_Y;
            lighting[FALLBACK_BASE + LIGHT_FILL_DIRECTION].Set(1.0f, -1.0f, 1.0f, 0);
            lighting[FALLBACK_BASE + LIGHT_FILL_COLOUR].Set(1.0f, 1.0f, 1.0f, 0);
            lighting[FALLBACK_BASE + 2].Set(0, 0, 0, 0);
            lighting[FALLBACK_BASE + 3].Set(0, 0, 0, 0);
            lighting[FALLBACK_BASE + 4].Set(0, 0, 0, 0);
            lighting[FALLBACK_BASE + 5].Set(0, 0, 0, 0);
            lighting[FALLBACK_BASE + LIGHT_AMBIENT].Set(FALLBACK_AMBIENT, FALLBACK_AMBIENT, FALLBACK_AMBIENT, 0);
            lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION].Copy(*eye - *target);
            lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION].Copy(Vec3_Normalized(&lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION]) * 2.0f);  // key direction length 2
            lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION].w = 1.0f;
            lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].w = 1.0f;
            float brightness = (*eye - *target).Length() / 1500.0f + 1.2f;  // distance fade 1500, base brightness 1.2
            lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].z = brightness;
            lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].y = brightness;
            lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].x = brightness;
            lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].Copy(lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR] * 0.8f);  // key colour scale 0.8
        }
        return &lighting[FALLBACK_BASE];
    }
    for (int i = 0; i < MODEL_BANK_LIT_COUNT; i++)
    {
        if (litModels[i] == model)
        {
            if (eye != 0 && target != 0)
            {
                Vec4 *light = &lighting[i * MODEL_BANK_LIGHT_VECTORS];
                light[LIGHT_KEY_DIRECTION].Copy(*eye - *target);
                light[LIGHT_KEY_DIRECTION].Copy(Vec3_Normalized(&light[LIGHT_KEY_DIRECTION]) * 2.0f);  // key direction length 2
                light[LIGHT_KEY_DIRECTION].w = 1.0f;
                light[LIGHT_KEY_COLOUR].w = 1.0f;
                float brightness = (*eye - *target).Length() / 1500.0f + 1.2f;  // distance fade 1500, base brightness 1.2
                light[LIGHT_KEY_COLOUR].z = brightness;
                light[LIGHT_KEY_COLOUR].y = brightness;
                light[LIGHT_KEY_COLOUR].x = brightness;
                light[LIGHT_KEY_COLOUR].Copy(light[LIGHT_KEY_COLOUR] * 0.8f);  // key colour scale 0.8
                light[LIGHT_AMBIENT].Set(FALLBACK_AMBIENT, FALLBACK_AMBIENT, FALLBACK_AMBIENT, 0);
            }
            return &lighting[i * MODEL_BANK_LIGHT_VECTORS];
        }
    }
    return 0;
}

// ModelBank::LightForRoot: returns the lights for the bank model whose node tree is `root` (see LightDirection). A
// root that is 0 or not in the bank gets the lights of the last lit entry, which are rebuilt from `eye` and `target`
// when both are given. Returns 0 when the bank is not enabled. Called by the map view drawing.
const void *ModelBank::LightForRoot(Node *root, const Vec4 *eye, const Vec4 *target)
{
    if (enabled == 0)
        return 0;
    if (root != 0)
    {
        int i;
        for (i = 0; i < MODEL_BANK_SIZE; i++)
        {
            if (root == models[i].root)
                break;
        }
        if (i != MODEL_BANK_SIZE)
            return LightDirection(&models[i], eye, target);
    }
    if (eye != 0 && target != 0)
    {
        Vec4 zero;
        zero.Set(0, 0, 0, 0);
        Vec4 scratchColour;
        Vec4 scratchPosition;
        Vec4 white(1.0f, 1.0f, 1.0f, 0);
        scratchColour.Copy(zero);
        scratchPosition.Copy(zero);
        scratchPosition.z += SCRATCH_OFFSET_Z;
        scratchPosition.x += SCRATCH_OFFSET_X;
        scratchPosition.y -= SCRATCH_OFFSET_Y;
        lighting[FALLBACK_BASE + LIGHT_FILL_DIRECTION].Set(1.0f, -1.0f, 1.0f, 0);
        lighting[FALLBACK_BASE + LIGHT_FILL_COLOUR].Set(1.0f, 1.0f, 1.0f, 0);
        lighting[FALLBACK_BASE + 2].Set(0, 0, 0, 0);
        lighting[FALLBACK_BASE + 3].Set(0, 0, 0, 0);
        lighting[FALLBACK_BASE + 4].Set(0, 0, 0, 0);
        lighting[FALLBACK_BASE + 5].Set(0, 0, 0, 0);
        lighting[FALLBACK_BASE + LIGHT_AMBIENT].Set(FALLBACK_AMBIENT, FALLBACK_AMBIENT, FALLBACK_AMBIENT, 0);
        lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION].Copy(*eye - *target);
        lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION].Copy(Vec3_Normalized(&lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION]) * 2.0f);  // key direction length 2
        lighting[FALLBACK_BASE + LIGHT_KEY_DIRECTION].w = 1.0f;
        lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].w = 1.0f;
        float brightness = (*eye - *target).Length() / 1500.0f + 1.2f;  // distance fade 1500, base brightness 1.2
        lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].z = brightness;
        lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].y = brightness;
        lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].x = brightness;
        lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR].Copy(lighting[FALLBACK_BASE + LIGHT_KEY_COLOUR] * 0.8f);  // key colour scale 0.8
    }
    return &lighting[FALLBACK_BASE];
}
