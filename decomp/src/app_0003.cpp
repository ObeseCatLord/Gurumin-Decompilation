// Script flags, a rectangle test and the script tokenizers (Game::TestScriptFlag is inline in game.h).

#include <string.h>
#include "game.h"

// Sets script flag `index` to `value` (flag 0 and out-of-range indexes are ignored).
int Game::SetScriptFlag(int index, int value)
{
    if (index > 3999)
        return 0;
    if (index != 0)
        m_3ee694[index] = value;
    return 1;
}

// 1 when the point (x, y) lies inside the rectangle (edges included), else 0.
extern "C" int __stdcall FUN_10008430(int x, int y, int left, int top, int right, int bottom)
{
    if (x < left || y < top || x > right || y > bottom)
        return 0;
    return 1;
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

// Like ReadWord, but stays on the current line: reaching a line break before the word starts returns 0 (no
// further word on this line). Used for commands whose arguments must be on the same line.
int Game::ReadWordInLine()
{
    int quotes = 0;
    token[0] = 0;
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

// Calls `callback` for every event trigger whose map is the current one (or 9999 = any) and whose four script
// flags all pass. Returns 0.
int Game::ForEachActiveTrigger(void (*callback)(GameTrigger *trigger))
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

// Registers every map event named "GAMEOVER" whose map is the current one (or 9999 = any) and whose four script
// flags pass, so the game-over scripts run. Returns 0. Not byte-identical yet: the original walks the list with a
// pointer to flags[1], ours to flags[0] (see open.tsv).
int Game::RaiseGameOverEvents()
{
    for (int i = 0; i < mapEventCount; i++)
    {
        GameTrigger *event = &mapEvents[i];
        if (strcmp(event->name, "GAMEOVER") == 0 && (event->map == 9999 || event->map == currentMap) &&
            TestScriptFlag(event->flags[0]) && TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) &&
            TestScriptFlag(event->flags[3]))
            RegisterEvent(event->value);
    }
    return 0;
}

// Finds the map entry for `map` that applies now (its map is 9999 or the current map, and all four script flags
// pass) and copies its name, file and title strings and its value out (the three unused arguments are ignored).
// Returns 1, or 0 when there is none.
int Game::FindMapEntry(int map, char *name, char *file, char *title, int unused1, int unused2, int unused3, int *value)
{
    for (int i = 0; i < m_a4614; i++)
    {
        GameNamedEntry *entry = &m_95e14[i];
        if (entry->map == map && (entry->map == 9999 || entry->map == currentMap) &&
            TestScriptFlag(entry->flags[0]) && TestScriptFlag(entry->flags[1]) && TestScriptFlag(entry->flags[2]) &&
            TestScriptFlag(entry->flags[3]))
        {
            strcpy(name, m_95e14[i].name);
            strcpy(file, m_95e14[i].file);
            strcpy(title, m_95e14[i].title);
            *value = m_95e14[i].value;
            return 1;
        }
    }
    return 0;
}

// 0x10054970. Registers every map event named "KSKON" whose argument is `argument` and that applies now (its map is
// the current one or 9999, and its four script flags pass). Returns 0.
int Game::RaiseKskonEvents(int argument)
{
    for (int i = 0; i < mapEventCount; i++)
    {
        GameTrigger *event = &mapEvents[i];
        if (strcmp(event->name, "KSKON") == 0 && event->argument == argument &&
            (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
            TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
            RegisterEvent(event->value);
    }
    return 0;
}

// 0x10054b70. Same for the "TALK" events of `argument` (talking to a character).
int Game::RaiseTalkEvents(int argument)
{
    for (int i = 0; i < mapEventCount; i++)
    {
        GameTrigger *event = &mapEvents[i];
        if (event->argument == argument && strcmp(event->name, "TALK") == 0 &&
            (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
            TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
            RegisterEvent(event->value);
    }
    return 0;
}

// 0x10054c60. Same for the "OPEN" events of `argument` (opening a door or chest).
int Game::RaiseOpenEvents(int argument)
{
    for (int i = 0; i < mapEventCount; i++)
    {
        GameTrigger *event = &mapEvents[i];
        if (event->argument == argument && strcmp(event->name, "OPEN") == 0 &&
            (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
            TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
            RegisterEvent(event->value);
    }
    return 0;
}

// 0x10054d50. Same for the "MFIREON" events of `argument` when `on` is 1, else for its "MFIREOFF" events (a fire
// being lit or put out).
int Game::RaiseFireEvents(int argument, int on)
{
    if (on == 1)
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            GameTrigger *event = &mapEvents[i];
            if (event->argument == argument && strcmp(event->name, "MFIREON") == 0 &&
                (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
                TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
                RegisterEvent(event->value);
        }
    }
    else
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            GameTrigger *event = &mapEvents[i];
            if (event->argument == argument && strcmp(event->name, "MFIREOFF") == 0 &&
                (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
                TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
                RegisterEvent(event->value);
        }
    }
    return 0;
}

// 0x10054f30. Same for the "LEVERON" or "LEVEROFF" events of `argument` (a lever node), by `on`.
int Game::RaiseLeverEvents(int argument, int on)
{
    if (on == 1)
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            GameTrigger *event = &mapEvents[i];
            if (event->argument == argument && strcmp(event->name, "LEVERON") == 0 &&
                (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
                TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
                RegisterEvent(event->value);
        }
    }
    else
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            GameTrigger *event = &mapEvents[i];
            if (event->argument == argument && strcmp(event->name, "LEVEROFF") == 0 &&
                (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
                TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
                RegisterEvent(event->value);
        }
    }
    return 0;
}

// 0x10055110. Same for the "DROPEN" or "DRCLOSE" events of `argument` (a door gimmick), by `on`.
int Game::RaiseDoorEvents(int argument, int on)
{
    if (on == 1)
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            GameTrigger *event = &mapEvents[i];
            if (event->argument == argument && strcmp(event->name, "DROPEN") == 0 &&
                (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
                TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
                RegisterEvent(event->value);
        }
    }
    else
    {
        for (int i = 0; i < mapEventCount; i++)
        {
            GameTrigger *event = &mapEvents[i];
            if (event->argument == argument && strcmp(event->name, "DRCLOSE") == 0 &&
                (event->map == 9999 || event->map == currentMap) && TestScriptFlag(event->flags[0]) &&
                TestScriptFlag(event->flags[1]) && TestScriptFlag(event->flags[2]) && TestScriptFlag(event->flags[3]))
                RegisterEvent(event->value);
        }
    }
    return 0;
}
