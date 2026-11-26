// Switch gimmicks (EN build). Ported from the matched JP source decomp/src/switches.cpp.
#include "char_script.h"
#include "game.h"
#include "switches.h"

// Switch_TurnOn: turns the switch in slot `slot` on: kind 0x13 shows frame 1 and plays sound 0x38a, kind 0x14 shows
// frame 0x1f and plays 0x389. Both set their script flag and raise the door events (on for 0x13, off for 0x14).
// Returns 0. Called by the switch behaviours.
extern "C" int Switch_TurnOn(int slot)
{
    if (g_charKind[slot] == SWITCH_KIND_LEVER_DOOR)
    {
        g_switchFrame[slot] = SWITCH_FRAME_LEVER_ON;
        int flag = g_switchFlag[slot];
        if (flag <= SCRIPT_FLAG_LAST && flag != 0)
            g_game.scriptFlags[flag] = 1;
        g_game.RaiseDoorEvents(g_switchDoorId[slot], 1);
        if (g_soundsMuted == 0)
            Sound_PlayEffect(SOUND_LEVER_ON, &g_characterModels[slot].matrix.GetTranslation(), SWITCH_SOUND_VOLUME);
    }
    if (g_charKind[slot] == SWITCH_KIND_PLATE_DOOR)
    {
        g_switchFrame[slot] = SWITCH_FRAME_PLATE_ON;
        int flag = g_switchFlag[slot];
        if (flag <= SCRIPT_FLAG_LAST && flag != 0)
            g_game.scriptFlags[flag] = 1;
        g_game.RaiseDoorEvents(g_switchDoorId[slot], 0);
        if (g_soundsMuted == 0)
            Sound_PlayEffect(SOUND_PLATE_ON, &g_characterModels[slot].matrix.GetTranslation(), SWITCH_SOUND_VOLUME);
    }
    return 0;
}

// Switch_TurnOff: turns the switch back: kind 0x13 shows frame 0x3d (sound 0x38c), kind 0x14 frame 0x5b (sound
// 0x38b); the script flag is cleared. Returns 0.
extern "C" int Switch_TurnOff(int slot)
{
    if (g_charKind[slot] == SWITCH_KIND_LEVER_DOOR)
    {
        g_switchFrame[slot] = SWITCH_FRAME_LEVER_OFF;
        int flag = g_switchFlag[slot];
        if (flag <= SCRIPT_FLAG_LAST && flag != 0)
            g_game.scriptFlags[flag] = 0;
        g_game.RaiseDoorEvents(g_switchDoorId[slot], 1);
        if (g_soundsMuted == 0)
            Sound_PlayEffect(SOUND_LEVER_OFF, &g_characterModels[slot].matrix.GetTranslation(), SWITCH_SOUND_VOLUME);
    }
    if (g_charKind[slot] == SWITCH_KIND_PLATE_DOOR)
    {
        g_switchFrame[slot] = SWITCH_FRAME_PLATE_OFF;
        int flag = g_switchFlag[slot];
        if (flag <= SCRIPT_FLAG_LAST && flag != 0)
            g_game.scriptFlags[flag] = 0;
        g_game.RaiseDoorEvents(g_switchDoorId[slot], 0);
        if (g_soundsMuted == 0)
            Sound_PlayEffect(SOUND_PLATE_OFF, &g_characterModels[slot].matrix.GetTranslation(), SWITCH_SOUND_VOLUME);
    }
    return 0;
}

// Switch_Release: lets go of the object held by `holder`: a character holder gives back one held switch and raises
// the lever events of its switch node off; an object holder loses its held bonus of 1000. Returns 1, or 0 when
// `holder` holds nothing. Called by the pick-up and throw behaviours.
extern "C" int Switch_Release(int holder)
{
    for (int i = 0; i < SWITCH_HOLD_SLOTS; i++)
    {
        if (g_switchHeldObject[i] != 0 && g_switchHolder[i] == holder)
        {
            int who = g_switchHolder[i];
            if (who < SWITCH_HOLDER_OBJECT_BASE)
            {
                g_charRecords[who].switchesHeld--;
                g_game.RaiseLeverEvents(g_characterModels[who].switchNode, 0);
            }
            else if (g_objectSwitchState[who % SWITCH_HOLD_SLOTS] >= SWITCH_HELD_BONUS)
                g_objectSwitchState[who % SWITCH_HOLD_SLOTS] -= SWITCH_HELD_BONUS;
            g_switchHeldObject[i] = 0;
            return 1;
        }
    }
    return 0;
}

// g_charKind of a character whose switch node is turned on while it holds something.
const int SWITCH_KIND_HOLDER = 0x60;
// CharRecord::switchesHeld bits: set for effect kinds 0 and 1, and for kind 2.
const int SWITCH_HELD_BIT_PRIMARY = 1;
const int SWITCH_HELD_BIT_SECONDARY = 2;
// Effect kinds of a hold.
const int SWITCH_EFFECT_KIND_0 = 0;
const int SWITCH_EFFECT_KIND_1 = 1;
const int SWITCH_EFFECT_KIND_2 = 2;
// Slot number the search for a free slot reports as "none free" when the holder's own model is held (a leftover of a
// smaller table: the original compares with 0x40 instead of SWITCH_HOLD_SLOTS there).
const int SWITCH_OWN_MODEL_NO_SLOT = 0x40;

// Switch_Hold: see the declaration in switches.h. Called when a character picks up a switch object and by the map
// scripts.
extern "C" int Switch_Hold(int holder, void *object, int kind)
{
    int slot;
    if (object != 0)
    {
        for (slot = 0; slot < SWITCH_HOLD_SLOTS; slot++)
        {
            if (g_switchHeldObject[slot] == object)
                return 0;
        }
        for (slot = 0; slot < SWITCH_HOLD_SLOTS; slot++)
        {
            if (g_switchHeldObject[slot] == 0)
                break;
        }
        if (slot == SWITCH_HOLD_SLOTS)
            return 0;
        g_switchHeldObject[slot] = object;
    }
    else
    {
        object = g_characterModels[holder].root;
        for (slot = 0; slot < SWITCH_HOLD_SLOTS; slot++)
        {
            if (g_switchHeldObject[slot] == object)
                return 0;
        }
        for (slot = 0; slot < SWITCH_HOLD_SLOTS; slot++)
        {
            if (g_switchHeldObject[slot] == 0)
                break;
        }
        if (slot == SWITCH_OWN_MODEL_NO_SLOT)
            return 0;
        g_switchHeldObject[slot] = object;
    }
    g_switchHolder[slot] = holder;
    if (kind == SWITCH_EFFECT_KIND_0)
    {
        if (holder < SWITCH_HOLDER_OBJECT_BASE)
        {
            g_charRecords[holder].switchesHeld |= SWITCH_HELD_BIT_PRIMARY;
            if (g_charKind[holder] == SWITCH_KIND_HOLDER)
                g_game.RaiseLeverEvents(g_characterModels[holder].switchNode, 1);
        }
        else
            g_objectSwitchState[holder % SWITCH_HOLD_SLOTS] += SWITCH_HELD_BONUS;
    }
    else if (kind == SWITCH_EFFECT_KIND_1)
    {
        if (holder < SWITCH_HOLDER_OBJECT_BASE)
        {
            g_charRecords[holder].switchesHeld |= SWITCH_HELD_BIT_PRIMARY;
            if (g_charKind[holder] == SWITCH_KIND_HOLDER)
                g_game.RaiseLeverEvents(g_characterModels[holder].switchNode, 1);
        }
        else
            g_objectSwitchState[holder % SWITCH_HOLD_SLOTS] += SWITCH_HELD_BONUS;
    }
    else if (kind == SWITCH_EFFECT_KIND_2)
    {
        if (holder < SWITCH_HOLDER_OBJECT_BASE)
        {
            g_charRecords[holder].switchesHeld |= SWITCH_HELD_BIT_SECONDARY;
            if (g_charKind[holder] == SWITCH_KIND_HOLDER)
                g_game.RaiseLeverEvents(g_characterModels[holder].switchNode, 1);
        }
        else
            g_objectSwitchState[holder % SWITCH_HOLD_SLOTS] += SWITCH_HELD_BONUS;
    }
    g_switchEffectKind[slot] = kind;
    g_switchEffectStarted[slot] = 0;
    g_switchSlotSetUp[slot] = 1;
    return 1;
}

// Switch_ResetByNode: for every object slot whose switch node is `switchNode`, clears state values 0 and 1 and stores
// `value` in state value SWITCH_VALUE_SETTING; then marks a voice event as asked for. Returns 0.
extern "C" int Switch_ResetByNode(int switchNode, int value)
{
    for (int slot = OBJECT_SLOT_FIRST; slot < ENEMY_SLOT_END; slot++)
    {
        if (g_characterModels[slot].switchNode == switchNode)
        {
            g_charState[slot].values[SWITCH_VALUE_SETTING] = value;
            g_charState[slot].values[0] = 0;
            g_charState[slot].values[1] = 0;
        }
    }
    g_scriptPlayback = 2;
    return 0;
}
