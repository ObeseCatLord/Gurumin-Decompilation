// The world object (the map scene object): per-frame advance, seek, release and screen overlays (EN build). Ported
// from the matched JP source decomp/src/world_frame.cpp. RunScriptFrames runs the scene script and animates the world.
#include "render.h"
#include "texture_mgr.h"
#include "world_renderer.h"

// Number of frames over which the cinema bars fade in.
const int CINEMA_BAR_FADE_FRAMES = 30;
// Height in pixels of one cinema bar on the 640x480 screen.
const int CINEMA_BAR_HEIGHT = 34;
// Screen size the overlays are drawn for.
const int OVERLAY_SCREEN_WIDTH = 640;
const int OVERLAY_SCREEN_HEIGHT = 480;
// Host notification code sent when the world is released.
const int WORLD_NOTIFY_RELEASE = 0x5e0;

// Once per frame while enabled: rebuilds the world draw lists (RunScriptFrames) when the frame changed since the last
// call, publishes the frame number to four globals read by the renderers and increments it. Returns 0.
int WorldRenderer::AdvanceFrame()
{
    if (enabled != 0)
    {
        if (frame != lastFrame)
            RunScriptFrames(0);
        lastFrame = frame;
        g_worldFrameA = frame;
        g_worldFrameB = frame;
        g_worldFrameC = frame;
        g_worldFrameD = frame;
        frame++;
    }
    return 0;
}

// Unloads the current map: sends the pending release notification, releases the map's texture slots and the 32
// model nodes, and marks the world as not loaded. Returns 0. Called when the scene ends.
int WorldRenderer::Release()
{
    movieActive = 0;
    if (notifyPending != 0)
    {
        notifyPending = 0;
        callback(WORLD_NOTIFY_RELEASE, 0);
    }
    for (int i = 0; i < WORLD_NODE_COUNT; i++)
    {
        if (textures[i] != -1)
        {
            TextureSlot_Release(textures[i], 0);
            textures[i] = -1;
        }
    }
    for (int j = 0; j < WORLD_NODE_COUNT; j++)
    {
        nodes[j].Release();
        loaded = 0;
    }
    return 0;
}

// Jumps the world to frame `target`, rebuilding every frame on the way so animated parts end in the right state:
// forwards from the current frame when the target is ahead, otherwise from frame 0. Enables frame updates. Returns 0.
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

// Draws the screen overlays of the world: the cinema bars at the top and bottom of the 640x480 screen, fading in
// over the first CINEMA_BAR_FADE_FRAMES frames (at once when barsFull is set), and the full-screen fade in
// fadeColour at fadeAmount percent, before or after the bars depending on fadeOnTop. Called every frame.
void WorldRenderer::DrawOverlay()
{
    if (fadeOnTop == 1 && m_00975 == 0 && fadeAmount != 0.0f)
        Render_FillRect(0, 0, OVERLAY_SCREEN_WIDTH, OVERLAY_SCREEN_HEIGHT, fadeColour, 0.0f, fadeAmount / 100.0f);
    int bars = frame;
    if (bars > CINEMA_BAR_FADE_FRAMES)
        bars = CINEMA_BAR_FADE_FRAMES;
    if (barsFull != 0)
        bars = CINEMA_BAR_FADE_FRAMES;
    float barAlpha = (float)bars / (float)CINEMA_BAR_FADE_FRAMES;
    Render_FillRect(0, 0, OVERLAY_SCREEN_WIDTH, CINEMA_BAR_HEIGHT, 0, 0.0f, barAlpha);
    Render_FillRect(0, OVERLAY_SCREEN_HEIGHT - CINEMA_BAR_HEIGHT, OVERLAY_SCREEN_WIDTH, OVERLAY_SCREEN_HEIGHT, 0, 0.0f, barAlpha);
    if (fadeOnTop == 0 && m_00975 == 0 && fadeAmount != 0.0f)
        Render_FillRect(0, 0, OVERLAY_SCREEN_WIDTH, OVERLAY_SCREEN_HEIGHT, fadeColour, 0.0f, fadeAmount / 100.0f);
}
