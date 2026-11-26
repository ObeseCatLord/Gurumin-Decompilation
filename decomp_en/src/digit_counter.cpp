// A three-digit counter shown on a model (EN build, JP twin digit_counter.cpp; run by the object behaviour): the digit
// nodes "@numone", "@numten" and "@numhundred" of the map play a motion named after the digit ("Zaro" .. "Nine", the
// game's spelling) or, while a digit turns over, "to0ne" .. "toZaro". The counter follows script flag 0x9c2.
#include "char.h"
#include "char_dispatch.h"
#include "game.h"
#include "listbank.h"
#include "node.h"
#include "shaga.h"
#include "visibility.h"

// Script flags: the number the counter shows, and the flag that records the event for reaching 999 was raised.
const int FLAG_COUNTER_TARGET = 0x9c2;
const int FLAG_COUNTER_999_RAISED = 0x80b;
// Largest count, the count that raises the event, and the event.
const int COUNTER_MAX = 1000;
const int COUNTER_EVENT_COUNT = 999;
const int EVENT_COUNTER_999 = 0x340;
// Counter phases: 0 idle, 1 first frame of a turn-over, COUNTER_PHASE_END when the turn-over is done.
const int COUNTER_PHASE_TURN = 1;
const int COUNTER_PHASE_END = 11;
// Digits of a three-digit number.
const int DIGIT_COUNT = 3;
const int DIGIT_BASE = 10;
const int DIGIT_HUNDRED = 100;

// EN 0x63bc80 (JP DigitCounter_Update). One frame of the counter of character slot `index`. Idle: when flag 0x9c2 is
// one more than the count the digits start turning (phase 1); any other change shows the new value at once. While
// turning, phase 1 plays the "to" motion of each digit that changes; after ten frames the count goes up by one
// (at most 1000) and the digits show it. Reaching 999 raises event 0x340 once (flag 0x80b). Returns 0.
extern "C" int DigitCounter_Update(int index)
{
    int digits[6];                                   // [0..2] the new value, [3..5] the shown one (ones first)
    Node *node;
    if (g_charState[index].values[0] != 0)           // values[0] is the counter phase
    {
        if (g_charState[index].values[0] == COUNTER_PHASE_TURN)
        {
            int count = g_charState[index].counterValue;
            int next = count + 1;
            if (next > COUNTER_MAX)
                next = COUNTER_MAX;
            digits[3] = count % DIGIT_BASE;
            digits[4] = count / DIGIT_BASE % DIGIT_BASE;
            digits[5] = count / DIGIT_HUNDRED % DIGIT_BASE;
            digits[0] = next % DIGIT_BASE;
            digits[1] = next / DIGIT_BASE % DIGIT_BASE;
            digits[2] = next / DIGIT_HUNDRED % DIGIT_BASE;
            for (int i = 0; i < DIGIT_COUNT; i++)
            {
                if (digits[i + DIGIT_COUNT] != digits[i])
                {
                    switch (i)
                    {
                    case 0:
                        node = g_mapRootNode->Find("@numone", 0);
                        break;
                    case 1:
                        node = g_mapRootNode->Find("@numten", 0);
                        break;
                    case 2:
                        node = g_mapRootNode->Find("@numhundred", 0);
                        break;
                    }
                    if (node != 0)
                    {
                        switch (digits[i + DIGIT_COUNT])
                        {
                        case 0:
                            node->RestartColourPlugs("to0ne", g_colourPlugFrames);
                            break;
                        case 1:
                            node->RestartColourPlugs("toTwo", g_colourPlugFrames);
                            break;
                        case 2:
                            node->RestartColourPlugs("toThree", g_colourPlugFrames);
                            break;
                        case 3:
                            node->RestartColourPlugs("toFour", g_colourPlugFrames);
                            break;
                        case 4:
                            node->RestartColourPlugs("toFive", g_colourPlugFrames);
                            break;
                        case 5:
                            node->RestartColourPlugs("toSix", g_colourPlugFrames);
                            break;
                        case 6:
                            node->RestartColourPlugs("toSeven", g_colourPlugFrames);
                            break;
                        case 7:
                            node->RestartColourPlugs("toEight", g_colourPlugFrames);
                            break;
                        case 8:
                            node->RestartColourPlugs("toNine", g_colourPlugFrames);
                            break;
                        case 9:
                            node->RestartColourPlugs("toZaro", g_colourPlugFrames);
                            break;
                        }
                    }
                }
            }
        }
        if (++g_charState[index].values[0] == COUNTER_PHASE_END)
        {
            g_charState[index].values[0] = 0;
            g_charState[index].counterValue++;
            if (g_charState[index].counterValue > COUNTER_MAX)
                g_charState[index].counterValue = COUNTER_MAX;
            int count = g_charState[index].counterValue;
            digits[0] = count % DIGIT_BASE;
            digits[1] = count / DIGIT_BASE % DIGIT_BASE;
            digits[2] = count / DIGIT_HUNDRED % DIGIT_BASE;
            for (int i = 0; i < DIGIT_COUNT; i++)
            {
                switch (i)
                {
                case 0:
                    node = g_mapRootNode->Find("@numone", 0);
                    break;
                case 1:
                    node = g_mapRootNode->Find("@numten", 0);
                    break;
                case 2:
                    node = g_mapRootNode->Find("@numhundred", 0);
                    break;
                }
                if (node != 0)
                {
                    switch (digits[i])
                    {
                    case 0:
                        node->RestartColourPlugs("Zaro", g_colourPlugFrames);
                        break;
                    case 1:
                        node->RestartColourPlugs("0ne", g_colourPlugFrames);
                        break;
                    case 2:
                        node->RestartColourPlugs("Two", g_colourPlugFrames);
                        break;
                    case 3:
                        node->RestartColourPlugs("Three", g_colourPlugFrames);
                        break;
                    case 4:
                        node->RestartColourPlugs("Four", g_colourPlugFrames);
                        break;
                    case 5:
                        node->RestartColourPlugs("Five", g_colourPlugFrames);
                        break;
                    case 6:
                        node->RestartColourPlugs("Six", g_colourPlugFrames);
                        break;
                    case 7:
                        node->RestartColourPlugs("Seven", g_colourPlugFrames);
                        break;
                    case 8:
                        node->RestartColourPlugs("Eight", g_colourPlugFrames);
                        break;
                    case 9:
                        node->RestartColourPlugs("Nine", g_colourPlugFrames);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        int count = g_charState[index].counterValue;
        if (count == COUNTER_EVENT_COUNT && g_scriptFlags[FLAG_COUNTER_999_RAISED] == 0)
        {
            g_game.RegisterEvent(EVENT_COUNTER_999);
            g_scriptFlags[FLAG_COUNTER_999_RAISED] = 1;
        }
        int target = g_scriptFlags[FLAG_COUNTER_TARGET];
        if (count == target)
            return 0;
        if (count == target - 1)
        {
            g_charState[index].values[0] = COUNTER_PHASE_TURN;
            return 0;
        }
        g_charState[index].counterValue = target;
        digits[0] = target % DIGIT_BASE;
        digits[1] = target / DIGIT_BASE % DIGIT_BASE;
        digits[2] = target / DIGIT_HUNDRED % DIGIT_BASE;
        for (int i = 0; i < DIGIT_COUNT; i++)
        {
            switch (i)
            {
            case 0:
                node = g_mapRootNode->Find("@numone", 0);
                break;
            case 1:
                node = g_mapRootNode->Find("@numten", 0);
                break;
            case 2:
                node = g_mapRootNode->Find("@numhundred", 0);
                break;
            }
            if (node != 0)
            {
                switch (digits[i])
                {
                case 0:
                    node->RestartColourPlugs("Zaro", g_colourPlugFrames);
                    break;
                case 1:
                    node->RestartColourPlugs("0ne", g_colourPlugFrames);
                    break;
                case 2:
                    node->RestartColourPlugs("Two", g_colourPlugFrames);
                    break;
                case 3:
                    node->RestartColourPlugs("Three", g_colourPlugFrames);
                    break;
                case 4:
                    node->RestartColourPlugs("Four", g_colourPlugFrames);
                    break;
                case 5:
                    node->RestartColourPlugs("Five", g_colourPlugFrames);
                    break;
                case 6:
                    node->RestartColourPlugs("Six", g_colourPlugFrames);
                    break;
                case 7:
                    node->RestartColourPlugs("Seven", g_colourPlugFrames);
                    break;
                case 8:
                    node->RestartColourPlugs("Eight", g_colourPlugFrames);
                    break;
                case 9:
                    node->RestartColourPlugs("Nine", g_colourPlugFrames);
                    break;
                }
            }
        }
    }
    return 0;
}
