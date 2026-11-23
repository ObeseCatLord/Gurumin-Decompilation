// A three-digit counter shown on a model (run by the object behaviour 0x100bcb30): the digit nodes "@numone",
// "@numten" and "@numhundred" of the counter model play a motion named after the digit ("Zaro" .. "Nine", the
// game's spelling) or, while a digit turns over, "to0ne" .. "toZaro". The counter follows script flag 0x9c2.

#include "node.h"

#include "game.h"

#include "char_state.h"

extern "C" Node *DAT_10255f08;                       // the counter model
extern "C" int DAT_104bf8a4;                         // frames of a digit motion
extern "C" int DAT_10c62edc[0x1000];                 // script flags

// 0x1001d2d0 (DigitCounter_Update). One frame of the counter of character slot `index`. Idle: when flag 0x9c2 is
// one more than the count the digits start turning (phase 1); any other change shows the new value at once. While
// turning, phase 1 plays the "to" motion of each digit that changes; after ten frames the count goes up by one
// (at most 1000) and the digits show it. Reaching 999 raises event 0x340 once (flag 0x80b). Returns 0.
extern "C" int FUN_1001d2d0(int index)
{
    int digits[6];                                   // [0..2] the shown value, [3..5] the new one (ones first)
    Node *node;
    if (DAT_1020b430[index].counterPhase != 0)
    {
        if (DAT_1020b430[index].counterPhase == 1)
        {
            int count = DAT_1020b430[index].counterValue;
            int next = count + 1;
            if (next > 1000)
                next = 1000;
            digits[0] = count % 10;
            digits[1] = count / 10 % 10;
            digits[2] = count / 100 % 10;
            digits[3] = next % 10;
            digits[4] = next / 10 % 10;
            digits[5] = next / 100 % 10;
            for (int i = 0; i < 3; i++)
            {
                if (digits[i] != digits[i + 3])
                {
                    switch (i)
                    {
                    case 0:
                        node = DAT_10255f08->Find("@numone", 0);
                        break;
                    case 1:
                        node = DAT_10255f08->Find("@numten", 0);
                        break;
                    case 2:
                        node = DAT_10255f08->Find("@numhundred", 0);
                        break;
                    }
                    if (node != 0)
                    {
                        switch (digits[i])
                        {
                        case 0:
                            node->RestartColourPlugs("to0ne", DAT_104bf8a4);
                            break;
                        case 1:
                            node->RestartColourPlugs("toTwo", DAT_104bf8a4);
                            break;
                        case 2:
                            node->RestartColourPlugs("toThree", DAT_104bf8a4);
                            break;
                        case 3:
                            node->RestartColourPlugs("toFour", DAT_104bf8a4);
                            break;
                        case 4:
                            node->RestartColourPlugs("toFive", DAT_104bf8a4);
                            break;
                        case 5:
                            node->RestartColourPlugs("toSix", DAT_104bf8a4);
                            break;
                        case 6:
                            node->RestartColourPlugs("toSeven", DAT_104bf8a4);
                            break;
                        case 7:
                            node->RestartColourPlugs("toEight", DAT_104bf8a4);
                            break;
                        case 8:
                            node->RestartColourPlugs("toNine", DAT_104bf8a4);
                            break;
                        case 9:
                            node->RestartColourPlugs("toZaro", DAT_104bf8a4);
                            break;
                        }
                    }
                }
            }
        }
        if (++DAT_1020b430[index].counterPhase == 11)
        {
            DAT_1020b430[index].counterPhase = 0;
            DAT_1020b430[index].counterValue++;
            if (DAT_1020b430[index].counterValue > 1000)
                DAT_1020b430[index].counterValue = 1000;
            int count = DAT_1020b430[index].counterValue;
            digits[3] = count % 10;
            digits[4] = count / 10 % 10;
            digits[5] = count / 100 % 10;
            for (int i = 0; i < 3; i++)
            {
                switch (i)
                {
                case 0:
                    node = DAT_10255f08->Find("@numone", 0);
                    break;
                case 1:
                    node = DAT_10255f08->Find("@numten", 0);
                    break;
                case 2:
                    node = DAT_10255f08->Find("@numhundred", 0);
                    break;
                }
                if (node != 0)
                {
                    switch (digits[i + 3])
                    {
                    case 0:
                        node->RestartColourPlugs("Zaro", DAT_104bf8a4);
                        break;
                    case 1:
                        node->RestartColourPlugs("0ne", DAT_104bf8a4);
                        break;
                    case 2:
                        node->RestartColourPlugs("Two", DAT_104bf8a4);
                        break;
                    case 3:
                        node->RestartColourPlugs("Three", DAT_104bf8a4);
                        break;
                    case 4:
                        node->RestartColourPlugs("Four", DAT_104bf8a4);
                        break;
                    case 5:
                        node->RestartColourPlugs("Five", DAT_104bf8a4);
                        break;
                    case 6:
                        node->RestartColourPlugs("Six", DAT_104bf8a4);
                        break;
                    case 7:
                        node->RestartColourPlugs("Seven", DAT_104bf8a4);
                        break;
                    case 8:
                        node->RestartColourPlugs("Eight", DAT_104bf8a4);
                        break;
                    case 9:
                        node->RestartColourPlugs("Nine", DAT_104bf8a4);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        int count = DAT_1020b430[index].counterValue;
        if (count == 999 && DAT_10c62edc[0x80b] == 0)
        {
            DAT_10874848.RegisterEvent(0x340);
            DAT_10c62edc[0x80b] = 1;
        }
        int target = DAT_10c62edc[0x9c2];
        if (count == target)
            return 0;
        if (count == target - 1)
        {
            DAT_1020b430[index].counterPhase = 1;
            return 0;
        }
        DAT_1020b430[index].counterValue = target;
        digits[3] = target % 10;
        digits[4] = target / 10 % 10;
        digits[5] = target / 100 % 10;
        for (int i = 0; i < 3; i++)
        {
            switch (i)
            {
            case 0:
                node = DAT_10255f08->Find("@numone", 0);
                break;
            case 1:
                node = DAT_10255f08->Find("@numten", 0);
                break;
            case 2:
                node = DAT_10255f08->Find("@numhundred", 0);
                break;
            }
            if (node != 0)
            {
                switch (digits[i + 3])
                {
                case 0:
                    node->RestartColourPlugs("Zaro", DAT_104bf8a4);
                    break;
                case 1:
                    node->RestartColourPlugs("0ne", DAT_104bf8a4);
                    break;
                case 2:
                    node->RestartColourPlugs("Two", DAT_104bf8a4);
                    break;
                case 3:
                    node->RestartColourPlugs("Three", DAT_104bf8a4);
                    break;
                case 4:
                    node->RestartColourPlugs("Four", DAT_104bf8a4);
                    break;
                case 5:
                    node->RestartColourPlugs("Five", DAT_104bf8a4);
                    break;
                case 6:
                    node->RestartColourPlugs("Six", DAT_104bf8a4);
                    break;
                case 7:
                    node->RestartColourPlugs("Seven", DAT_104bf8a4);
                    break;
                case 8:
                    node->RestartColourPlugs("Eight", DAT_104bf8a4);
                    break;
                case 9:
                    node->RestartColourPlugs("Nine", DAT_104bf8a4);
                    break;
                }
            }
        }
    }
    return 0;
}
