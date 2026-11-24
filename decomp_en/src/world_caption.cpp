// Per-frame screen overlays of the world renderer (EN build). Ported from decomp/src/world_caption.cpp.
#include "world_renderer.h"

// WorldRenderer::DrawScreenOverlays: draws the caption, then the cinema bars and fades (DrawOverlay). Called once per
// frame at the end of the world draw.
void WorldRenderer::DrawScreenOverlays()
{
    DrawCaption();
    DrawOverlay();
}
