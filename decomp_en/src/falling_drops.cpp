// Falling drops on the screen (EN build). Ported from decomp/src/falling_drops.cpp.
#include "camera.h"
#include "falling_drops.h"
#include "map_view.h"
#include "ui_batch.h"

// Per update every drop moves by this in x and y, and down by DROP_FALL_SPEED in z.
const float DROP_DRIFT = 0.02f;
const float DROP_FALL_SPEED = 3.5f;
// The 640x480 screen the projected drop is mapped to.
const float DROP_SCREEN_WIDTH = 640.0f;
const float DROP_SCREEN_HEIGHT = 480.0f;
// Only drops whose projected depth is in this range are drawn.
const double DROP_DEPTH_NEAR = 0.9;
const float DROP_DEPTH_FAR = 1.0f;
// Size of the drawn quad at depth 0 (scaled by 1 - depth).
const float DROP_SIZE_SCALE = 344.0f;
// Drop state values: first fading step, last fading step (then the drop is freed), steps over which it fades.
const int DROP_STATE_FALLING = 1;
const int DROP_STATE_FADE_START = 2;
const int DROP_STATE_FADE_END = 30;
const float DROP_FADE_STEPS = 30.0f;
// Landing height offsets: even drops land at -10, odd ones at 800 relative to the camera target.
const int DROP_LANDING_EVEN = -10;
const int DROP_LANDING_ODD = 800;
const int DROP_LANDING_KINDS = 2;

// Drops_Update: moves every drop (x and y by 0.02, down by 3.5), draws it when its depth is between 0.9 and 1.0 (larger
// when nearer, fading with its state) and advances its state: a falling drop starts fading once it is below the
// landing height (with an offset of -10 for even drops and 800 for odd ones); a fading drop counts up while below that
// height and down while above it, and is freed after 30. Returns 0. Called every frame while it rains.
extern "C" int Drops_Update(void)
{
    g_batch2D.Reset();
    for (int i = 0; i < DROP_COUNT; i++)
    {
        if (g_dropState[i] != 0)
        {
            g_dropPositions[i].x += DROP_DRIFT;
            g_dropPositions[i].y += DROP_DRIFT;
            g_dropPositions[i].z -= DROP_FALL_SPEED;
            Vec4 screen;
            screen.Copy(Project(g_dropPositions[i], g_cameraViewProjection));
            screen.x = screen.x * DROP_SCREEN_WIDTH / 2.0f + DROP_SCREEN_WIDTH / 2.0f;
            screen.y = DROP_SCREEN_HEIGHT / 2.0f - screen.y * DROP_SCREEN_HEIGHT / 2.0f;
            if (!(screen.z > DROP_DEPTH_FAR || screen.z < DROP_DEPTH_NEAR))
            {
                float size = (1.0f - screen.z) * DROP_SIZE_SCALE;
                float alpha = 1.0f - (float)g_dropState[i] / DROP_FADE_STEPS;
                g_batch2D.AppendQuad(screen.x - size, screen.y - size, size + screen.x, size + screen.y, 0, 0, 1.0f,
                                     1.0f, screen.z, alpha, 1.0f, 1.0f, 1.0f, 0, 0, 0, 0);
            }
            int offsets[DROP_LANDING_KINDS];
            offsets[0] = DROP_LANDING_EVEN;
            offsets[1] = DROP_LANDING_ODD;
            if (g_dropState[i] > DROP_STATE_FALLING)
            {
                if ((float)offsets[i % DROP_LANDING_KINDS] + g_dropPositions[i].z < g_cameraTargetNow.z)
                    g_dropState[i]++;
                else
                    g_dropState[i]--;
                if (g_dropState[i] > DROP_STATE_FADE_END)
                    g_dropState[i] = 0;
            }
            else if ((float)offsets[i % DROP_LANDING_KINDS] + g_dropPositions[i].z < g_cameraTargetNow.z)
                g_dropState[i] = DROP_STATE_FADE_START;
        }
    }
    return 0;
}
