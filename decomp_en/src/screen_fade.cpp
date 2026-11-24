// Screen fade drawing (EN build). Ported from the matched JP source decomp/src/screen_fade.cpp.
#include <math.h>

#include "render.h"
#include "volume_fade.h"

// The fade state is kept in the music-fade globals: g_bgmFadeTrack holds the fade kind (hundreds) and g_bgmFadeVolume
// the amount in percent.
const int FADE_KIND_STEP = 100;               // each kind adds this to the state value
const int FADE_KIND_WHITE = 1;                // fades to white
const int FADE_KIND_RED = 2;                  // fades to red
const int FADE_KIND_CUSTOM = 3;               // fades to g_fadeColourKind3
const int FADE_COLOUR_WHITE = 0xffffff;
const int FADE_COLOUR_RED = 0xff0000;
const int FADE_SCREEN_WIDTH = 640;            // the 2D screen the fade covers
const int FADE_SCREEN_HEIGHT = 480;
const float FADE_PERCENT = 100.0f;            // amounts are percent
const float FADE_BLACK_ABOVE = 95.0f;         // iris wipes turn into a plain black fade above this amount
const float IRIS_WOBBLE_DIVISOR = 15.0f;      // amount / 15 is the wobble angle in radians
const float IRIS_RADIUS_START = 100.0f;       // wobble radius at amount 0
const float IRIS_RADIUS_SHRINK = 0.5f;        // radius lost per percent
const float IRIS_GROW_FACTOR = 0.7f;          // iris growth = amount squared times this
const int IRIS_LEFT = -220;                   // iris rectangle around the wobble centre
const int IRIS_TOP = -300;
const int IRIS_RIGHT = 860;
const int IRIS_BOTTOM = 780;

// Draws the screen fade: a full-screen rectangle in the kind's colour with opacity amount / 100, or for an iris wipe
// (and amounts up to 95) the iris texture, which wobbles around the centre on a circle that shrinks as the amount
// grows and is drawn larger the higher the amount; above 95 the wipe is black. Called every frame by the scene loop.
extern "C" void Fade_Draw(void)
{
    int rest = g_bgmFadeTrack;
    int kind = 0;
    if (rest >= 3 * FADE_KIND_STEP)
    {
        kind = FADE_KIND_CUSTOM;
        rest -= 3 * FADE_KIND_STEP;
    }
    else if (rest >= 2 * FADE_KIND_STEP)
    {
        kind = FADE_KIND_RED;
        rest -= 2 * FADE_KIND_STEP;
    }
    else if (rest >= FADE_KIND_STEP)
    {
        kind = FADE_KIND_WHITE;
        rest -= FADE_KIND_STEP;
    }
    if (rest == 0)
    {
        if (kind == FADE_KIND_CUSTOM)
            Render_FillRect(0, 0, FADE_SCREEN_WIDTH, FADE_SCREEN_HEIGHT, g_fadeColourKind3, 0, g_bgmFadeVolume / FADE_PERCENT);
        else if (kind == FADE_KIND_RED)
            Render_FillRect(0, 0, FADE_SCREEN_WIDTH, FADE_SCREEN_HEIGHT, FADE_COLOUR_RED, 0, g_bgmFadeVolume / FADE_PERCENT);
        else if (kind != 0)
            Render_FillRect(0, 0, FADE_SCREEN_WIDTH, FADE_SCREEN_HEIGHT, FADE_COLOUR_WHITE, 0, g_bgmFadeVolume / FADE_PERCENT);
        else
            Render_FillRect(0, 0, FADE_SCREEN_WIDTH, FADE_SCREEN_HEIGHT, 0, 0, g_bgmFadeVolume / FADE_PERCENT);
    }
    else if (g_bgmFadeVolume > FADE_BLACK_ABOVE)
        Render_FillRect(0, 0, FADE_SCREEN_WIDTH, FADE_SCREEN_HEIGHT, 0, 0, g_bgmFadeVolume / FADE_PERCENT);
    else
    {
        int x = (int)(sin(g_bgmFadeVolume / IRIS_WOBBLE_DIVISOR) * (IRIS_RADIUS_START - g_bgmFadeVolume * IRIS_RADIUS_SHRINK));
        int y = (int)(cos(g_bgmFadeVolume / IRIS_WOBBLE_DIVISOR) * (IRIS_RADIUS_START - g_bgmFadeVolume * IRIS_RADIUS_SHRINK));
        int grow = (int)(g_bgmFadeVolume * g_bgmFadeVolume * IRIS_GROW_FACTOR);
        Render_Command(RENDER_TEXTURE_ADDRESS, 1, 0);
        Render_Command(RENDER_BLEND_MODE, 2, 0);
        float edge = (float)grow / (float)FADE_SCREEN_HEIGHT + 1.0f;
        float start = (float)-grow / (float)FADE_SCREEN_HEIGHT;
        g_textures[g_irisTextureSlot].DrawTinted(x + IRIS_LEFT, y + IRIS_TOP, x + IRIS_RIGHT, y + IRIS_BOTTOM, start, start,
                                                 edge, edge, 0, 1.0f, 0, 0, ASPECT_STRETCH);
        Render_Command(RENDER_BLEND_MODE, 0, 0);
        Render_Command(RENDER_TEXTURE_ADDRESS, 0, 0);
    }
}
