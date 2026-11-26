// Playback slot and follow-up setters called by the scene script commands (EN build, EN-only: no JP twin found).
#include "anim_model.h"

// Playback::SetSlotSource: sets the motion source and the layer of slot `slot`. Returns 0 (and changes nothing) when the
// layer is above PLAYBACK_LAYER_COUNT - 1, the slot is above PLAYBACK_SLOT_COUNT - 1 or the source is empty, else 1.
// Called by the scene script command that binds a motion to a slot (0x631890).
int Playback::SetSlotSource(void *source, int slot, int layer)
{
    if (layer > PLAYBACK_LAYER_COUNT - 1)
        return 0;
    if (slot > PLAYBACK_SLOT_COUNT - 1)
        return 0;
    if (source == 0)
        return 0;
    slotSource[slot] = source;
    slotLayer[slot] = layer;
    return 1;
}

// Playback::SetFollowUp: sets follow-up entry `index` of layer `layer` (the layer that plays after it). Returns 0 (and
// changes nothing) when the layer is above PLAYBACK_LAYER_COUNT - 1 or the index is above PLAYBACK_FOLLOWUP_COUNT - 1, else 1.
// Called by the scene script command DIS (0x631890).
int Playback::SetFollowUp(int layer, int index, int value)
{
    if (layer > PLAYBACK_LAYER_COUNT - 1)
        return 0;
    if (index > PLAYBACK_FOLLOWUP_COUNT - 1)
        return 0;
    layerParams[layer][index] = value;
    return 1;
}
