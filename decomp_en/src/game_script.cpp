// Map event and script text functions of the Game object (EN build). Ported from the matched JP source
// decomp/src/app_0003.cpp.
#include <string.h>

#include "game.h"

// Event name the KSKON events are registered under.
static const char EVENT_KSKON[] = "KSKON";
// Event name the TALK events are registered under (talking to a character).
static const char EVENT_TALK[] = "TALK";

// Game::RaiseKskonEvents: registers every map event named "KSKON" whose argument is `argument` and that applies now.
// Returns 0. Called by the script commands.
int Game::RaiseKskonEvents(int argument)
{
    for (int i = 0; i < mapEventCount; i++)
    {
        if (strcmp(mapEvents[i].name, EVENT_KSKON) == 0 && mapEvents[i].argument == argument)
        {
            if (TriggerApplies(&mapEvents[i], 1))
                RegisterEvent(mapEvents[i].value);
        }
    }
    return 0;
}

// Game::RaiseTalkEvents: like RaiseKskonEvents for the "TALK" events of `argument` (talking to a character).
int Game::RaiseTalkEvents(int argument)
{
    for (int i = 0; i < mapEventCount; i++)
    {
        if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, EVENT_TALK) == 0)
        {
            if (TriggerApplies(&mapEvents[i], 1))
                RegisterEvent(mapEvents[i].value);
        }
    }
    return 0;
}

// Like ReadWord, but stays on the current line: reaching a line break before the word starts returns 0 (no
// further word on this line). Used for commands whose arguments must be on the same line.
int Game::ReadWordInLine()
{
    token[0] = 0;
    int quotes = 0;
    int length = 0;
    for (;;)
    {
        // Skip tabs and spaces before the word; a line break ends the search.
        for (;;)
        {
            int at = pos;
            if (at >= end)
                return 0;
            char c = text[at];
            if (c == '\r')
                return 0;
            if (c == '\n')
                return 0;
            if (c == '\t')
                pos = at + 1;
            else if (c == ' ')
                pos = at + 1;
            else
                break;
        }
        // Collect the word.
        for (;;)
        {
            if (pos >= end)
                return 0;
            char c = text[pos];
            if (c == '"')
            {
                quotes++;
                pos++;
                continue;
            }
            // Outside quotes (an even number seen so far) a separator ends the word.
            if (((quotes & 1) == 0 && c == '\r') || ((quotes & 1) == 0 && c == '\n') ||
                ((quotes & 1) == 0 && c == '\t') || ((quotes & 1) == 0 && c == ' '))
                return 1;
            if (length == 0 && c == '/')
                break;
            token[length] = c;
            pos++;
            length++;
            token[length] = 0;
        }
        // Comment: skip to the end of the line, then look for a word again.
        for (;;)
        {
            if (pos >= end)
                return 0;
            if (text[pos] == '\r')
                break;
            pos++;
        }
    }
}

// Reads the next word of the script text into `token`. Words are separated by line breaks, tabs and spaces;
// text between double quotes may contain them (the quotes themselves are dropped). A word starting with '/' is
// a comment: the rest of its line is skipped. Returns 1 when a word was read, 0 at the end of the text (with any
// partial word left in `token`).
int Game::ReadWord()
{
    int length = 0;
    int quotes = 0;
    token[0] = 0;
    for (;;)
    {
        // Skip separators before the word.
        for (;;)
        {
            int at = pos;
            if (at >= end)
                return 0;
            char c = text[at];
            if (c == '\r')
                pos = at + 1;
            else if (c == '\n')
                pos = at + 1;
            else if (c == '\t')
                pos = at + 1;
            else if (c == ' ')
                pos = at + 1;
            else
                break;
        }
        // Collect the word.
        for (;;)
        {
            if (pos >= end)
                return 0;
            char c = text[pos];
            if (c == '"')
            {
                quotes++;
                pos++;
                continue;
            }
            // Outside quotes (an even number seen so far) a separator ends the word.
            if (((quotes & 1) == 0 && c == '\r') || ((quotes & 1) == 0 && c == '\n') ||
                ((quotes & 1) == 0 && c == '\t') || ((quotes & 1) == 0 && c == ' '))
                return 1;
            if (length == 0 && c == '/')
                break;
            token[length] = c;
            pos++;
            length++;
            token[length] = 0;
        }
        // Comment: skip to the end of the line, then look for a word again.
        for (;;)
        {
            if (pos >= end)
                return 0;
            if (text[pos] == '\r')
                break;
            pos++;
        }
    }
}

// Calls `callback` for every event trigger whose map is the current one (or 9999 = any) and whose four script
// flags all pass. Returns 0.
int Game::ForEachActiveTrigger(int (*callback)(GameTrigger *trigger))
{
    for (int i = 0; i < triggerCount; i++)
    {
        GameTrigger *trigger = &triggers[i];
        if ((trigger->map == 9999 || trigger->map == currentMap) && TestScriptFlag(trigger->flags[0]) &&
            TestScriptFlag(trigger->flags[1]) && TestScriptFlag(trigger->flags[2]) &&
            TestScriptFlag(trigger->flags[3]))
            callback(trigger);
    }
    return 0;
}

// 0x10055410 (Game::RunQueuedEvents). Takes the queued events not yet handled (id > 0, handled flag 0), moves all
// queued ids into runningEvents (so events queued while these run are told apart, see RegisterEvent) and clears
// the queue, then calls `callback` for every queued trigger of one of the taken events on the current map (or
// 9999 = any) whose four script flags pass. Returns 1 when a trigger ran, else 0.
int Game::RunQueuedEvents(int (*callback)(GameTrigger *trigger))
{
    int ids[0x100];
    int count = 0;
    int i;
    for (i = 0; i < 0x100; i++)
    {
        if (queuedEvents[i][0] > 0 && queuedEvents[i][1] == 0)
            ids[count++] = queuedEvents[i][0];
    }
    for (i = 0; i < 0x100; i++)
        runningEvents[i] = queuedEvents[i][0];
    for (i = 0; i < 0x100; i++)
        queuedEvents[i][0] = 0;
    int ran = 0;
    int map = currentMap;
    for (int t = 0; t < queuedTriggerCount; t++)
    {
        for (int j = 0; j < count; j++)
        {
            if (ids[j] == queuedTriggers[t].value)
            {
                GameTrigger *trigger = &queuedTriggers[t];
                if ((trigger->map == 9999 || map == trigger->map) &&
                    TestScriptFlag(trigger->flags[0]) && TestScriptFlag(trigger->flags[1]) &&
                    TestScriptFlag(trigger->flags[2]) && TestScriptFlag(trigger->flags[3]))
                {
                    ran = 1;
                    callback(trigger);
                }
            }
        }
    }
    return ran;
}

// 0x10055690 (Game::IsQueuedEventReady). Collects the queued events not yet handled (id > 0, handled flag 0) and
// returns 1 when one of them has a queued trigger on the current map (or 9999 = any) whose four script flags pass;
// otherwise 0.
int Game::IsQueuedEventReady()
{
    int ids[0x100];
    int count = 0;
    for (int i = 0; i < 0x100; i++)
    {
        if (queuedEvents[i][0] > 0 && queuedEvents[i][1] == 0)
            ids[count++] = queuedEvents[i][0];
    }
    for (int t = 0; t < queuedTriggerCount; t++)
    {
        GameTrigger *trigger = &queuedTriggers[t];
        for (int j = 0; j < count; j++)
        {
            if (ids[j] == trigger->value)
            {
                if ((trigger->map == 9999 || currentMap == trigger->map) && TestScriptFlag(trigger->flags[0]) &&
                    TestScriptFlag(trigger->flags[1]) && TestScriptFlag(trigger->flags[2]) &&
                    TestScriptFlag(trigger->flags[3]))
                    return 1;
            }
        }
    }
    return 0;
}

// Game::RaiseDoorEvents: registers the map events of `argument` named "DROPEN" when `on` is 1, else those named "DRCLOSE" (a door gimmick opening or closing),
// when they apply now. Returns 0. Called by the gimmick code.
int Game::RaiseDoorEvents(int argument, int on)
{
    if (on == 1)
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, "DROPEN") == 0)
            {
                if (TriggerApplies(&mapEvents[i], 1))
                    RegisterEvent(mapEvents[i].value);
            }
        }
    }
    else
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, "DRCLOSE") == 0)
            {
                if (TriggerApplies(&mapEvents[i], 1))
                    RegisterEvent(mapEvents[i].value);
            }
        }
    }
    return 0;
}

// Game::RaiseLeverEvents: registers the map events of `argument` named "LEVERON" when `on` is 1, else those named "LEVEROFF" (a lever node switched),
// when they apply now. Returns 0. Called by the gimmick code.
int Game::RaiseLeverEvents(int argument, int on)
{
    if (on == 1)
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, "LEVERON") == 0)
            {
                if (TriggerApplies(&mapEvents[i], 1))
                    RegisterEvent(mapEvents[i].value);
            }
        }
    }
    else
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, "LEVEROFF") == 0)
            {
                if (TriggerApplies(&mapEvents[i], 1))
                    RegisterEvent(mapEvents[i].value);
            }
        }
    }
    return 0;
}

// Game::RaiseFireEvents: registers the map events of `argument` named "MFIREON" when `on` is 1, else those named "MFIREOFF" (a fire lit or put out),
// when they apply now. Returns 0. Called by the gimmick code.
int Game::RaiseFireEvents(int argument, int on)
{
    if (on == 1)
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, "MFIREON") == 0)
            {
                if (TriggerApplies(&mapEvents[i], 1))
                    RegisterEvent(mapEvents[i].value);
            }
        }
    }
    else
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, "MFIREOFF") == 0)
            {
                if (TriggerApplies(&mapEvents[i], 1))
                    RegisterEvent(mapEvents[i].value);
            }
        }
    }
    return 0;
}

// Game::RaiseOpenEvents: like RaiseTalkEvents for the "OPEN" events of `argument` (opening a door or chest).
int Game::RaiseOpenEvents(int argument)
{
    for (int i = 0; i < mapEventCount; i++)
    {
        if (mapEvents[i].argument == argument && strcmp(mapEvents[i].name, "OPEN") == 0)
        {
            if (TriggerApplies(&mapEvents[i], 1))
                RegisterEvent(mapEvents[i].value);
        }
    }
    return 0;
}

// Game::RaiseGameOverEvents: registers every map event named "GAMEOVER" that applies now, so the game-over scripts
// run. Returns 0.
int Game::RaiseGameOverEvents()
{
    for (int i = 0; i < mapEventCount; i++)
    {
        if (strcmp(mapEvents[i].name, "GAMEOVER") == 0)
        {
            if (TriggerApplies(&mapEvents[i], 1))
                RegisterEvent(mapEvents[i].value);
        }
    }
    return 0;
}
