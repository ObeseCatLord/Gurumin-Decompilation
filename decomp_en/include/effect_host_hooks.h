// Host hooks of the effect system (EN build, EN-only: no JP twin found). The init code of the effect manager (EN 0x5f2cd0)
// stores these functions in the hook slots EN 0x01674650..0x01674660 and the effect update (EN 0x717xxx) calls them when
// an effect event asks the game for something.
#ifndef GURUMIN_EN_EFFECT_HOST_HOOKS_H
#define GURUMIN_EN_EFFECT_HOST_HOOKS_H

#include "effect_options.h"
#include "vec4.h"

// Codes of EffectHook_HostCommand (the first argument, a float).
const float HOST_COMMAND_SOUND = 0.0f;       // plays sound effect (int)c at `position`
const float HOST_COMMAND_CAPTION = 1.0f;     // shows a fixed title caption
const float HOST_COMMAND_FADE = 2.0f;        // fades the screen to the colour (a, b, c) over HOST_COMMAND_FADE_FRAMES frames
const float HOST_COMMAND_KNOCK_BACK = 4.0f;  // runs the player knock-back
// Frames of the fade.
const int HOST_COMMAND_FADE_FRAMES = 15;
// Text of the caption.
const char HOST_COMMAND_CAPTION_TEXT[] = "4537";

// The player knock-back step (not matched yet; JP twin Player_KnockBack). EN 0x5e75f0.
extern "C" int Player_KnockBack(void);

// Runs the host command `command`: sound, caption, colour fade or knock-back; any other code does nothing. Returns 0. Called
// through the hook slot EN 0x01674654 (effect event types 5, 6 and 7). EN 0x5c3000.
extern "C" int EffectHook_HostCommand(float command, float a, float b, float c, const Vec4 *position);

// Entries of the candidate lists of EffectHook_PickTarget.
const int HOOK_TARGET_MAX = 24;
// Height above a target's position that a homing effect aims at.
const float HOOK_TARGET_HEIGHT = 60.0f;
// Targets farther than this (squared distance) from the effect are ignored.
const float HOOK_TARGET_RANGE_SQUARED = 4000000.0f;
// Margin of the view test, and the amount added to the key of targets outside the view (they sort after all others).
const float HOOK_TARGET_VIEW_MARGIN = 24.0f;
const float HOOK_TARGET_OFFSCREEN_PENALTY = 1000000.0f;
// g_charKind of the marker characters a homing effect may also aim at when the option byte of its options is 1.
const char HOOK_TARGET_KIND_MARKER = 0x60;
// EffectOptions::m_000e value that allows the marker characters as targets.
const char HOOK_TARGET_ALLOW_MARKERS = 1;

// Chooses the target of a homing effect started at (x, y, z). A target kept in options->m_0014 stays while it is alive and in
// the world; otherwise the enemies of slots ENEMY_SLOT_FIRST..ENEMY_SLOT_END that are in range (and, with
// HOOK_TARGET_ALLOW_MARKERS, the markers) are sorted nearest first (those outside the view last) and the one at position
// (int)`cycle` modulo their count becomes the target. Writes the aim point (the target's position raised by
// HOOK_TARGET_HEIGHT) to *options->steering. Returns the kept target slot, 1 for a new target, or -1 when there is none. Called
// through the hook slot EN 0x0167465c (effect event type 5). EN 0x5c3100.
extern "C" int EffectHook_PickTarget(float x, float y, float z, float cycle, EffectOptions *options);

#endif
