// EN-only constructors and destructors without a JP twin (batch 11, section 4).
#include "effect_model.h"
#include "slot_table.h"

// Constructor of a mesh table: starts out reset (no texture slot, no entries). Runs for every node's mesh object.
SlotTable::SlotTable()
{
    Reset();
}

// Destructor of one effect slot: the members destroy themselves. Runs for the effect model table at exit.
EffectModel::~EffectModel()
{
}
