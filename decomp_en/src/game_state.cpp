// Inventory, message window, looping sounds and the colour fade (EN build). Ported from the matched JP sources
// decomp/src/app_0009.cpp and app_0013.cpp.
#include <string.h>

#include "game_vars.h"
#include "anim_model.h"
#include "char.h"
#include "engine.h"
#include "node.h"
#include "scene_start.h"

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

// Scene_ResetBattleState: resets the battle and HUD state for a new scene and, when the attack overload counter
// reached COMBO_FINISH_OVERLOAD or more, starts the white fade of the combo finish. (JP twin: app_0013.cpp; it lives
// beside ColourFade_Start here because the compiler inlined the fade into it.) Called by Scene_Start.
extern "C" int Scene_ResetBattleState(void)
{
    g_battleStateBc8860 = 0;
    g_battleStateA207c7 = 0;
    g_battleStateA150b4 = 0;
    g_battleStateA14f7c = 0;
    g_battleStateA14f80 = 0;
    if (g_greetingFrames != 0)
    {
        g_characterModels[0].mode = 1;
        g_characterModels[0].verticalSpeed = 0;
        g_characterModels[0].m_237c = 1.0f;
        g_characterModels[0].alpha = 1.0f;
        g_greetingFrames = 0;
        g_battleStateA1513c = 0;
    }
    g_battleStateA14f87 = 0;
    if (g_attackOverload >= COMBO_FINISH_OVERLOAD)
        ColourFade_Start(COMBO_FINISH_FADE_FRAMES, 1.0f, 1.0f, 1.0f, 1.0f);
    g_attackOverload = 0;
    return 0;
}

// Folder argument of Path_Data for the model files of the game's data folder.
const int SPAWN_PATH_FOLDER = 0;
// Frames the "too many objects" message stays up.
const int SPAWN_FULL_MESSAGE_FRAMES = 0x50;
// "Too many objects." with full-width spaces.
#define SPAWN_FULL_MESSAGE "\x54\x6f\x6f\x81\x40\x6d\x61\x6e\x79\x81\x40\x6f\x62\x6a\x65\x63\x74\x73\x2e"

// Object_SpawnModel (EN-only; it sits here because the compiler inlined Message_Show into it): loads model file `path` into slot g_nextObjectSlot, frees the unique mesh data of its node tree,
// puts it at (x, y, z), marks the slot as kind CHAR_KIND_SPAWNED_MODEL, gives the nodes their colours (slot number,
// SPAWNED_COLOUR_A, SPAWNED_COLOUR_B) and resets the scale to 1. Returns the slot. When the slots are used up it
// shows the message "Too many objects." and returns 0.
extern "C" int Object_SpawnModel(const char *path, int x, int y, int z)
{
    if (g_nextObjectSlot >= OBJECT_SPAWN_LIMIT)
    {
        Message_Show(SPAWN_FULL_MESSAGE, 0, SPAWN_FULL_MESSAGE_FRAMES);
        return 0;
    }
    int slot = g_nextObjectSlot++;
    g_characterModels[slot].LoadFile(Path_Data(path, SPAWN_PATH_FOLDER));
    g_characterModels[slot].root->ReleaseUniqueMeshData();
    g_characterModels[slot].Teleport((float)x, (float)y, (float)z);
    g_charKind[slot] = CHAR_KIND_SPAWNED_MODEL;
    g_characterModels[slot].m_0266 = 1;
    g_characterModels[slot].root->SetColourC((char)slot, SPAWNED_COLOUR_MASK);
    g_characterModels[slot].root->SetColourA(SPAWNED_COLOUR_A, SPAWNED_COLOUR_MASK);
    g_characterModels[slot].root->SetColourB(SPAWNED_COLOUR_B, SPAWNED_COLOUR_MASK);
    g_characterModels[slot].scale.Set(1.0f, 1.0f, 1.0f, 1.0f);
    return slot;
}
