// Per-level loops of the Packs image codec (EN build, EN-only: no JP twin found).
#include "packs_codec.h"

// Packs_ReconstructLevels: rebuilds the levels of one plane, last level first. Every level is `widths[level]` by
// `heights[level]`; the first level's width is the full plane width. Called by Packs_ReconstructPlanes.
extern "C" void Packs_ReconstructLevels(short *plane, short *scratch, int *widths, int *heights, int levelCount)
{
    for (int level = levelCount - 1; level >= 0; level--)
        Packs_ReconstructLevel(plane, scratch, widths[level], heights[level], widths[0]);
}

// Packs_ReconstructPlanes: runs Packs_ReconstructLevels on each of the three planes of an image with the same level
// sizes. Called by the image decoder.
extern "C" void Packs_ReconstructPlanes(short *planeA, short *planeB, short *planeC, short *scratch, int *widths,
                                        int *heights, int levelCount)
{
    Packs_ReconstructLevels(planeA, scratch, widths, heights, levelCount);
    Packs_ReconstructLevels(planeB, scratch, widths, heights, levelCount);
    Packs_ReconstructLevels(planeC, scratch, widths, heights, levelCount);
}

// Packs_DecomposeLevels: splits the levels of one plane, first level first (the encoder's order). Called by the image
// encoder.
extern "C" void Packs_DecomposeLevels(short *plane, short *scratch, int *widths, int *heights, int levelCount)
{
    for (int level = 0; level < levelCount; level++)
        Packs_DecomposeLevel(plane, scratch, widths[level], heights[level], widths[0]);
}
