// Model records and banks (EN build): saved-state limits, character slots, model bank reset. Ported from the matched JP
// source decomp/src/model_misc.cpp and model_bank.cpp.
#include "anim_model.h"
#include "character_bank.h"
#include "model_bank.h"
#include "node_data.h"

// Rotation limit in radians (+-45 degrees as the game rounds it).
const float ROTATION_LIMIT = 0.785375f;

// AnimModel::SetDropCollider: sets the collision node the model drops onto. Returns 1.
int AnimModel::SetDropCollider(Node *node)
{
    dropCollider = node;
    return 1;
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
