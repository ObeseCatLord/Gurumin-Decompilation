// Script flags of the Game object (EN build). Ported from the matched JP source decomp/src/app_0003.cpp.
#include "game.h"

// Sets script flag `index` to `value` (flag 0 and out-of-range indexes are ignored). Returns 1 when in range.
int Game::SetScriptFlag(int index, int value)
{
    if (index > SCRIPT_FLAG_LAST)
        return 0;
    if (index != 0)
        scriptFlags[index] = value;
    return 1;
}

// Game::TriggerApplies: 1 when `trigger` applies now: its map is the current one or any map (only tested when
// `checkMap` is set) and all four of its script flags pass. Called by the Raise*Events functions.
int Game::TriggerApplies(GameTrigger *trigger, int checkMap)
{
    if (checkMap != 0 && trigger->map != TRIGGER_ANY_MAP && trigger->map != currentMap)
        return 0;
    if (!TestScriptFlag(trigger->flags[0]) || !TestScriptFlag(trigger->flags[1]) ||
        !TestScriptFlag(trigger->flags[2]))
        return 0;
    return TestScriptFlag(trigger->flags[3]) != 0;
}
