// Inventory, message window, looping sounds and the colour fade (EN build). Ported from the matched JP sources
// decomp/src/app_0009.cpp and app_0013.cpp.
#include <string.h>

#include "game_vars.h"

// Inventory_Remove: takes *amount of item *index. When there are not enough, takes all and returns 0 with
// *amount = -taken; otherwise returns 1 and clears *amount. Refreshes the anti-tamper copies. Called through the
// script command table.
extern "C" int Inventory_Remove(const int *index, int *amount)
{
    signed char stock = g_itemStock[*index];
    if (stock <= 0)
        return 0;
    if (*amount >= stock)
    {
        *amount = -stock;
        g_itemStock[*index] = 0;
        AntiTamper_StoreArrays();
        return 0;
    }
    g_itemStock[*index] = stock - *amount;
    *amount = 0;
    AntiTamper_StoreArrays();
    return 1;
}

// Message_Show: opens the message window with `text` (and an optional second line) for `frames` frames. Called
// through the script command table.
extern "C" int Message_Show(const char *text, const char *subtext, int frames)
{
    g_messageFrames = frames;
    g_messageAlternateStyle = 0;
    g_messageOpen = 1;
    g_messageProgress = 0;
    g_messageScroll = 0;
    g_messageSubtext[0] = 0;
    strcpy(g_messageText, text);
    if (subtext != 0)
        strcpy(g_messageSubtext, subtext);
    return 1;
}

// Message_ShowAlt: like Message_Show but in the alternative style.
extern "C" int Message_ShowAlt(const char *text, const char *subtext, int frames)
{
    g_messageFrames = frames;
    g_messageAlternateStyle = 1;
    g_messageOpen = 1;
    g_messageProgress = 0;
    g_messageScroll = 0;
    g_messageSubtext[0] = 0;
    strcpy(g_messageText, text);
    if (subtext != 0)
        strcpy(g_messageSubtext, subtext);
    return 1;
}

// SoundLoop_Stop: stops the looping sound of slot `index` (soundarc command 0x224) unless sound is muted; a free slot
// holds SOUND_LOOP_FREE. Called through the script command table.
extern "C" void SoundLoop_Stop(int index)
{
    if (g_soundMuted == 0)
    {
        short sound = g_soundLoopHandle[index];
        if (sound != SOUND_LOOP_FREE)
        {
            g_soundArcStopLoop(sound);
            g_soundLoopHandle[index] = SOUND_LOOP_FREE;
        }
    }
}

// ColourBlend_Step: advances the fade by one step, snapping to the target on the last frame. Called every frame.
extern "C" void ColourBlend_Step(void)
{
    int left = g_fadeFramesLeft;
    if (left != 0)
    {
        g_fadeFramesLeft = left - 1;
        g_fadeColour.Copy(g_fadeColour + g_fadeStep);
        if (g_fadeFramesLeft == 0)
            g_fadeColour.Copy(g_fadeTarget);
    }
}

// ColourFade_Start: starts a colour fade to (r, g, b, a) over `frames` frames; one frame or less sets the colour at
// once. Called through the script command table.
extern "C" void ColourFade_Start(int frames, float r, float g, float b, float a)
{
    if (frames <= 1)
    {
        g_fadeColour.Set(r, g, b, a);
        g_fadeFramesLeft = 0;
        return;
    }
    g_fadeTarget.Set(r, g, b, a);
    g_fadeStep.Copy((g_fadeTarget - g_fadeColour) * (1.0f / (float)frames));
    g_fadeFramesLeft = frames;
}
