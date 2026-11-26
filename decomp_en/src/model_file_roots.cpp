// Whole-table operations on the model file roots (EN build, EN-only: no JP twin found).
#include "anim_model.h"
#include "node.h"

// Clears the collision flag of the root node of every loaded model file (entries that are free are skipped). Returns 0.
// Called when a scene's collision is rebuilt (EN 0x605380, 0x603a90).
extern "C" int ModelFiles_ClearCollision(void)
{
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
    {
        if (g_modelFileRoots[i] != 0)
            g_modelFileRoots[i]->hasCollision = 0;
    }
    return 0;
}
