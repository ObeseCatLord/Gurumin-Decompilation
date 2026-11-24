// The world object (global instance at 0x10c68000): the current map's models, textures and draw lists.
// RunScriptFrames (world_script.cpp) runs the scene script and animates the world for a frame.

#include "world_renderer.h"

extern "C" void FUN_10143e00(int left, int top, int right, int bottom, int colour, int mode, float alpha);  // fills a rectangle

#include "camera_vars.h"

// Once per frame while enabled: rebuilds the world draw lists (RunScriptFrames) when the frame changed since the last
// call, publishes the frame number to four globals read by the renderers and increments it.
int WorldRenderer::AdvanceFrame()
{
    if (enabled != 0)
    {
        if (frame != lastFrame)
            RunScriptFrames(0);
        lastFrame = frame;
        DAT_1101de08 = frame;
        DAT_1101de0c = frame;
        DAT_1101de10 = frame;
        DAT_1101de14 = frame;
        frame++;
    }
    return 0;
}

extern "C" int FUN_10174c70(int slot, int force);  // releases one reference to a texture slot (texture_mgr.cpp)

// Unloads the current map: sends the pending 0x5e0 notification, releases the map's texture slots and the 32
// model nodes, and marks the world as not loaded. Returns 0.
int WorldRenderer::Release()
{
    movieActive = 0;
    if (notifyPending != 0)
    {
        notifyPending = 0;
        callback(0x5e0, 0);
    }
    for (int i = 0; i < 0x20; i++)
    {
        if (textures[i] != -1)
        {
            FUN_10174c70(textures[i], 0);
            textures[i] = -1;
        }
    }
    for (int j = 0; j < 0x20; j++)
    {
        nodes[j].Release();
        loaded = 0;
    }
    return 0;
}

// Jumps the world to frame `target`, rebuilding every frame on the way so animated parts end in the right state:
// forwards from the current frame when the target is ahead, otherwise from frame 0. Enables frame updates.
int WorldRenderer::Seek(int target)
{
    int f = frame;
    if (f < target)
    {
        do
        {
            frame = f;
            lastFrame = f - 1;
            RunScriptFrames(1);
            f++;
        } while (f <= target);
        enabled = 1;
        return 0;
    }
    frame = target;
    for (f = 0; f <= target; f++)
    {
        lastFrame = f - 1;
        frame = f;
        RunScriptFrames(1);
    }
    enabled = 1;
    return 0;
}

// 0x101692c0. Draws the screen overlays of the world: the cinema bars at the top (34 pixels) and bottom (34 pixels)
// of the 640x480 screen, fading in over the first 30 frames (at once when barsFull is set), and the full-screen
// fade in fadeColour at fadeAmount percent, before or after the bars depending on fadeOnTop.
void WorldRenderer::DrawOverlay()
{
    if (fadeOnTop == 1 && m_00975 == 0 && fadeAmount != 0.0f)
        FUN_10143e00(0, 0, 640, 480, fadeColour, 0, fadeAmount / 100.0f);
    int bars = 30;
    if (frame <= 30)
        bars = frame;
    if (barsFull != 0)
        bars = 30;
    float barAlpha = (float)bars / 30.0f;
    FUN_10143e00(0, 0, 640, 34, 0, 0, barAlpha);
    FUN_10143e00(0, 446, 640, 480, 0, 0, barAlpha);
    if (fadeOnTop == 0 && m_00975 == 0 && fadeAmount != 0.0f)
        FUN_10143e00(0, 0, 640, 480, fadeColour, 0, fadeAmount / 100.0f);
}
