// Colour transform of the Packs image encoder (EN build, EN-only: no JP twin found).
#include "packs_image_encode.h"

// Weights of the forward transform (the arithmetic of Packs_TransformPixel, which EN inlines here).
const int PACKS_COMPONENT_WEIGHT_THREE = 3;
const int PACKS_COMPONENT_SCALE_AB = 8;
const int PACKS_COMPONENT_SCALE_C = 20;

// Packs_ComponentsToPlanes: turns the packed 3-byte pixels (`width` x `height`, three bytes each) into the sample planes
// `first`, `second` and `third` with the same transform as Packs_TransformPixel; `scratch` is not touched. Called by the
// image encoder (Packs_EncodeImage); returns 0.
extern "C" int Packs_ComponentsToPlanes(const unsigned char *pixels, short *scratch, short *first, short *second,
                                        short *third, int width, int height)
{
    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            int red = pixels[0];
            int green = pixels[1];
            int blue = pixels[2];
            pixels += 3;
            first[column] = (red + (blue + green * 2) * PACKS_COMPONENT_WEIGHT_THREE) * PACKS_COMPONENT_SCALE_AB;
            second[column] = ((green * 4 + blue * -PACKS_COMPONENT_WEIGHT_THREE) - red) * PACKS_COMPONENT_SCALE_AB;
            third[column] = (blue - red) * PACKS_COMPONENT_SCALE_C;
        }
        first += width;
        second += width;
        third += width;
    }
    return 0;
}
