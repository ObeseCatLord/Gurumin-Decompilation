// Forward sample transform of the Packs image encoder (EN build, EN-only: no JP twin found).
#include "packs_codec.h"

// Weights of the three transform outputs (the inverse is Packs_PlanesToComponents).
const int PACKS_TRANSFORM_SCALE_A = 8;
const int PACKS_TRANSFORM_SCALE_C = 20;
const int PACKS_TRANSFORM_THREE = 3;

// Writes the three transformed samples of one pixel: planeA = 8 (first + 3 (third + 2 second)), planeB = 8 (4 second - 3
// third - first), planeC = 20 (third - first), each stored at `index`. Called by the image encoder.
extern "C" void Packs_TransformPixel(short *planeA, short *planeB, short *planeC, int index, int first, int second,
                                     int third)
{
    planeA[index] = (first + (third + second * 2) * PACKS_TRANSFORM_THREE) * PACKS_TRANSFORM_SCALE_A;
    planeB[index] = ((second * 4 + third * -PACKS_TRANSFORM_THREE) - first) * PACKS_TRANSFORM_SCALE_A;
    planeC[index] = (third - first) * PACKS_TRANSFORM_SCALE_C;
}
