// Model records and banks (EN build): saved-state limits, character slots, model bank reset. Ported from the matched JP
// source decomp/src/model_misc.cpp and model_bank.cpp.
#include "anim_model.h"
#include "character_bank.h"
#include "model_bank.h"
#include "node.h"
#include "node_data.h"

// Rotation limit in radians (+-45 degrees as the game rounds it).
const float ROTATION_LIMIT = 0.785375f;

// AnimModel::SetDropCollider: sets the collision node the model drops onto. Returns 1.
int AnimModel::SetDropCollider(Node *node)
{
    dropCollider = node;
    return 1;
}

// NodeExtra::Clear: frees the arrays of all five key tracks and drops the three track overrides. Called by
// Node::Release before the block itself is freed.
void NodeExtra::Clear()
{
    tracks[NODE_TRACK_TRANSLATION].Free();
    tracks[NODE_TRACK_ROTATION].Free();
    tracks[NODE_TRACK_SCALE].Free();
    tracks[NODE_TRACK_COLOUR].Free();
    tracks[NODE_TRACK_EXTRA].Free();
    translationOverride = 0;
    rotationOverride = 0;
    scaleOverride = 0;
}

// NodeExtra::Init: empties the five tracks and gives each its default key at frame 0 (translation 0, 0, 0; rotation
// 0, 0, 0, 1; scale 1, 1, 1; colour 1, 1, 1, 1; extra 0, 0, 0, 0). Called when a node's extra block is allocated.
void NodeExtra::Init()
{
    m_0070[0] = 0;
    m_0070[1] = 0;
    m_0070[2] = 0;
    m_0070[3] = 0;
    tracks[NODE_TRACK_TRANSLATION].Init();
    tracks[NODE_TRACK_ROTATION].Init();
    tracks[NODE_TRACK_SCALE].Init();
    tracks[NODE_TRACK_COLOUR].Init();
    tracks[NODE_TRACK_EXTRA].Init();
    translationOverride = 0;
    rotationOverride = 0;
    scaleOverride = 0;
    tracks[NODE_TRACK_TRANSLATION].Insert(0, 0, 0, 0, 0);
    tracks[NODE_TRACK_ROTATION].Insert(0, 0, 0, 0, 1.0f);
    tracks[NODE_TRACK_SCALE].Insert(0, 1.0f, 1.0f, 1.0f, 0);
    tracks[NODE_TRACK_COLOUR].Insert(0, 1.0f, 1.0f, 1.0f, 1.0f);
    tracks[NODE_TRACK_EXTRA].Insert(0, 0, 0, 0, 0);
}

// NodeSaved::Reset: resets the rotation limits to +-45 degrees on all three axes. Called when a saved-state block is
// allocated (Node::AllocSaved).
void NodeSaved::Reset()
{
    m_00b8 = 0;
    limitMin[0] = -ROTATION_LIMIT;
    limitMin[1] = -ROTATION_LIMIT;
    limitMin[2] = -ROTATION_LIMIT;
    limitMax[0] = ROTATION_LIMIT;
    limitMax[1] = ROTATION_LIMIT;
    limitMax[2] = ROTATION_LIMIT;
    state = 0;
}

// CharacterBank::Assign: assigns character `character` to model slot `slot` with `value` and flags its model record as
// important.
int CharacterBank::Assign(int slot, int character, int value)
{
    modelAlias[slot] = character;
    characters[character].important = 1;
    modelPartner[slot] = value;
    return 1;
}

// ModelBank::Reset: empties the bank: clears the light list, the lit model table, every model record's root and the
// counters.
int ModelBank::Reset()
{
    for (int j = 0; j < MODEL_BANK_LIT_COUNT; j++)
        litModels[j] = 0;
    shadowCasterCount = 0;
    for (int i = 0; i < MODEL_BANK_SIZE; i++)
        models[i].root = 0;
    m_313e8 = 0;
    for (int k = 0; k < MODEL_BANK_LIGHT_COUNT; k++)
        lights[k] = 0;
    return 1;
}

// Lights looked up by name prefix ('@' makes Node::Find a prefix search): up to this many omni lights, then spot lights.
const int LIGHTS_PER_KIND = 0x20;

// ModelBank::CollectLights: rebuilds the light list from the scene below `root`: up to 32 nodes whose names start with
// "Omn" (omni lights) followed by up to 32 starting with "Spot" (spot lights). Missing lights are skipped, so the list
// stays packed. Returns 0 when the bank is not in use, else 1. (The list has 32 entries; with more than 32 lights in
// total it overruns into litModels.) Called when a map's scene is loaded.
int ModelBank::CollectLights(Node *root)
{
    if (enabled == 0)
        return 0;
    for (int k = 0; k < MODEL_BANK_LIGHT_COUNT; k++)
        lights[k] = 0;
    int count = 0;
    int i;
    for (i = 0; i < LIGHTS_PER_KIND; i++)
    {
        lights[count] = root->Find("@Omn", i);
        if (lights[count] != 0)
            count++;
    }
    Node **slot;
    for (i = 0, slot = &lights[count]; i < LIGHTS_PER_KIND; i++)
    {
        *slot = root->Find("@Spot", i);
        if (*slot != 0)
            slot++;
    }
    return 1;
}

// Margin the flare point is allowed to lie outside the view volume, and the depth range over which its brightness
// fades in (1/70 of the range).
const float FLARE_VIEW_MARGIN = 24.0f;
const float FLARE_FADE_SCALE = 70.0f;

// ModelBank::AddFlarePoint: adds a lens-flare point for the world position `position` (the scene code passes the
// light/sun position): when it is on screen (inside the view volume widened by 24) and in front of the far plane, its
// screen position is recorded with a brightness that fades in over the last 1/70 of the depth range. Returns 1.
int ModelBank::AddFlarePoint(Vec4 position, const Matrix4 *viewProjection)
{
    if (View_ContainsPoint(&position, viewProjection, FLARE_VIEW_MARGIN) != 0)
    {
        flareScreen[flareCount].Copy(Project(position, *viewProjection));
        if (flareScreen[flareCount].z > 0.0f && flareScreen[flareCount].z < 1.0f)
        {
            float alpha = (1.0f - flareScreen[flareCount].z) * FLARE_FADE_SCALE;
            flareAlpha[flareCount] = 1.0f > alpha ? alpha : 1.0f;
            flareCount++;
        }
    }
    return 1;
}

// ModelBank::AddLitModel: adds `model` to the lit models (once; only models with a loaded model, and only while the
// bank is in use) and gives it default light parameters: three lights, each a direction and a colour (the key light
// pointing +z, a fill light pointing -y and a weak one pointing +y at half strength, all white). Returns 1 when it was
// added, else 0.
int ModelBank::AddLitModel(AnimModel *model)
{
    if (enabled == 0)
        return 0;
    if (model == 0)
        return 0;
    if (model->loadedRoot == 0)
        return 0;
    int i;
    for (i = 0; i < MODEL_BANK_LIT_COUNT; i++)
    {
        if (litModels[i] == model)
            return 0;
    }
    for (i = 0; i < MODEL_BANK_LIT_COUNT; i++)
    {
        if (litModels[i] == 0)
        {
            litModels[i] = model;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 0].x = 0;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 0].y = 0;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 0].z = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 0].w = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 1].x = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 1].y = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 1].z = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 1].w = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 2].x = 0;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 2].y = -1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 2].z = 0;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 2].w = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 3].x = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 3].y = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 3].z = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 3].w = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 4].x = 0;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 4].y = 0.5f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 4].z = 0;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 4].w = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 5].x = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 5].y = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 5].z = 1.0f;
            lighting[i * MODEL_BANK_LIGHT_VECTORS + 5].w = 1.0f;
            return 1;
        }
    }
    return 0;
}

// ModelBank::Add: adds the model with node tree `root` to the first free record (scale 1, no timeout), remembers the
// position it is placed at and sets the record up (AddLitModel). Returns 0 when all 16 records are in use.
int ModelBank::Add(Node *root, const Vec4 *position)
{
    for (int i = 0; i < MODEL_BANK_SIZE; i++)
    {
        if (models[i].root == 0)
        {
            models[i].root = root;
            models[i].loadedRoot = root;
            models[i].uniformScale = 1.0f;
            models[i].collisionLimit = MODEL_BANK_NO_TIMEOUT;
            models[i].bankIndex = i;
            params[i] = position;
            AddLitModel(&models[i]);
            return 1;
        }
    }
    return 0;
}
