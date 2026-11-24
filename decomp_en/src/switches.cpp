// Switch gimmicks (EN build). Ported from the matched JP source decomp/src/switches.cpp.
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
