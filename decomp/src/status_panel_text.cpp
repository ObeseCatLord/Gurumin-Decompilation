// Fills the text lines of the status panel (drawn by 0x1001e340) from the record of a finished map.

#include <stdio.h>
#include <string.h>

#include "font_renderer.h"
#include "game.h"
#include "status_panel.h"
#include "ui_vars.h"

#define PANEL_OPENING_START 1           // states 1..14 open the panel, 15 = open, 16..29 close it
#define PANEL_OPEN 15
#define PANEL_CLOSING_START 16
#define PANEL_STATES 30
#define NO_RECORD_SHOWN -1
#define RECORD_FIRST_FLAG 0x4b0         // the map records are the flags 0x4b0..0x4ef (two flags each)
#define RECORD_LAST_FLAG 0x4ef
#define FLAG_PANEL_BLOCKED 297          // while set the panel stays closed
#define FLAG_ENEMIES_TOTAL 1024
#define FLAG_ITEMS_TOTAL_A 1048
#define FLAG_ITEMS_TOTAL_B 1084
#define TEXT_SCALE 0.7f
#define VALUE_RIGHT_X 135               // x where the number in front of the slash ends
#define SLASH_RIGHT_X 145               // x of the total behind it
#define COLUMN_CENTRE_X 140
#define NO_CLEAR_COLOUR_VALUE 2         // DAT_1020dc80 for an uncleared map

// The original expands no inline function in this unit (Game::TestScriptFlag stays a call), and copies its lines with
// an open-coded loop.
#pragma inline_depth(0)
#define CopyText(destination, source) \
    { \
        int index = 0; \
        char character; \
        do \
        { \
            character = (source)[index]; \
            (destination)[index] = character; \
            index++; \
        } while (character != 0); \
    }

// 0x1001e4b0 (StatusPanel_UpdateText). Steps the panel's counter DAT_1020c330 (1..14 opening, 15 open, 16..29 closing, then
// 0 and no record shown) and keeps it open while `recordFlag` names a map record (flags 0x4b0..0x4ef) and flag 297 is clear;
// otherwise it closes it. When a different record comes up, builds the text lines from the record's decimal digits:
// enemies defeated / total, containers opened / total, game-overs, the time as minutes : seconds and the grade letter
// (S+++ down to C), each number placed by the width of its text. `notCleared` puts dashes and question marks there instead.
// Returns 0.
extern "C" int FUN_1001e4b0(int recordFlag, int notCleared)
{
    if (((DAT_1020c330 > 0 && DAT_1020c330 < PANEL_OPEN) || (DAT_1020c330 > PANEL_OPEN && DAT_1020c330 < PANEL_STATES)) &&
        (DAT_1020c330 = DAT_1020c330 + 1, DAT_1020c330 == PANEL_STATES))
    {
        DAT_1020c330 = 0;
        DAT_1020c334 = NO_RECORD_SHOWN;
    }
    if (recordFlag < RECORD_FIRST_FLAG || recordFlag > RECORD_LAST_FLAG || DAT_10874848.m_3ee694[FLAG_PANEL_BLOCKED] != 0)
    {
        DAT_1020c334 = NO_RECORD_SHOWN;
        if (DAT_1020c330 == PANEL_OPEN)
            DAT_1020c330 = PANEL_CLOSING_START;
        else if (DAT_1020c330 < PANEL_OPEN && DAT_1020c330 > 0)
            DAT_1020c330 = PANEL_STATES - DAT_1020c330;
    }
    else
    {
        if (DAT_1020c330 == 0)
            DAT_1020c330 = PANEL_OPENING_START;
        else if (DAT_1020c330 > PANEL_OPEN)
            DAT_1020c330 = PANEL_STATES - DAT_1020c330;
        if (DAT_1020c334 != recordFlag)
        {
            char numberText[64];
            char enemiesLine[64];
            char containersLine[64];
            char unusedLine[64];
            char gameOverLine[64];
            char timeLine[64];
            char gradeLine[64];
            char scratch[256];

            DAT_1020c334 = recordFlag;
            int enemiesTotal = DAT_10874848.m_3ee694[FLAG_ENEMIES_TOTAL];
            int itemsTotalA = DAT_10874848.m_3ee694[FLAG_ITEMS_TOTAL_A];
            int itemsTotalB = DAT_10874848.m_3ee694[FLAG_ITEMS_TOTAL_B];
            int enemies = DAT_10874848.TestScriptFlag(recordFlag) / 100000;
            int vasesOpened = (DAT_10874848.TestScriptFlag(recordFlag) / 1000) % 100;
            int chestsOpened = DAT_10874848.TestScriptFlag(recordFlag) % 1000;
            recordFlag = recordFlag + 1;
            int minutes = DAT_10874848.TestScriptFlag(recordFlag) / 1000000;
            int seconds = (DAT_10874848.TestScriptFlag(recordFlag) / 10000) % 100;
            int gameOvers = (DAT_10874848.TestScriptFlag(recordFlag) / 100) % 100;
            int gradeDigits = DAT_10874848.TestScriptFlag(recordFlag) % 100;
            if (notCleared != 0)
            {
                strcpy(scratch, "\x93|\x82\xb5\x82\xbd\x93G\x82\xcc\x90\x94");
                CopyText(enemiesLine, scratch);
                strcpy(scratch, "\x95\xf3\x94\xa0\x82\xe2\x92\xd9\x82\xcc\x90\x94");
                CopyText(containersLine, scratch);
                sprintf(gameOverLine, "\x82" "f\x82\x81\x82\x8d\x82\x85\x82n\x82\x96\x82\x85\x82\x92");
                sprintf(timeLine, "\x8e\x9e\x8a\xd4");
                sprintf(gradeLine, "\x91\x8d\x8d\x87\x95]\x89\xbf");
                sprintf(enemiesLine, "%ss119\x81H\x81H\x81^\x81H\x81H", enemiesLine);
                sprintf(containersLine, "%ss119\x81H\x81H\x81^\x81H\x81H", containersLine);
                sprintf(unusedLine, "%ss119\x81H\x81H\x81^\x81H\x81H", unusedLine);
                sprintf(gameOverLine, "%ss135\x81|\x81|", gameOverLine);
                sprintf(timeLine, "%ss122\x81|\x81|\x81" "F\x81|\x81|", timeLine);
                sprintf(gradeLine, "%ss142\x81|", gradeLine);
                DAT_1020dc80 = NO_CLEAR_COLOUR_VALUE;
            }
            else
            {
                strcpy(scratch, "\x93|\x82\xb5\x82\xbd\x93G\x82\xcc\x90\x94");
                CopyText(enemiesLine, scratch);
                strcpy(scratch, "\x95\xf3\x94\xa0\x82\xe2\x92\xd9\x82\xcc\x90\x94");
                CopyText(containersLine, scratch);
                sprintf(gameOverLine, "\x82" "f\x82\x81\x82\x8d\x82\x85\x82n\x82\x96\x82\x85\x82\x92");
                sprintf(timeLine, "\x8e\x9e\x8a\xd4");
                sprintf(gradeLine, "\x91\x8d\x8d\x87\x95]\x89\xbf");
                sprintf(numberText, "%d", enemies);
                int width = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                sprintf(enemiesLine, "%ss%03d%ds140\x81^", enemiesLine, VALUE_RIGHT_X - width, enemies);
                sprintf(numberText, "\x81^");
                width = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                sprintf(enemiesLine, "%ss%03d%d", enemiesLine, width + SLASH_RIGHT_X, enemiesTotal);
                int containers = chestsOpened + vasesOpened;
                sprintf(numberText, "%d", containers);
                width = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                sprintf(containersLine, "%ss%03d%ds140\x81^", containersLine, VALUE_RIGHT_X - width, containers);
                sprintf(numberText, "\x81^");
                width = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                sprintf(containersLine, "%ss%03d%d", containersLine, width + SLASH_RIGHT_X, itemsTotalB + itemsTotalA);
                sprintf(numberText, "%d", gameOvers);
                int overWidth = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                int slashWidth = DAT_1020e538.TextWidth("\x81^", TEXT_SCALE);
                int centre = (slashWidth >> 1) + COLUMN_CENTRE_X;
                sprintf(gameOverLine, "%ss%03d%s", gameOverLine, centre - (overWidth >> 1), numberText);
                sprintf(numberText, "%02d", minutes);
                width = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                int colonWidth = DAT_1020e538.TextWidth("\x81" "F", TEXT_SCALE);
                sprintf(timeLine, "%ss%03d%ss%03d\x81" "F", timeLine, VALUE_RIGHT_X - width, numberText, (centre - (colonWidth >> 1)) + 1);
                sprintf(numberText, "\x81^");
                int textWidth = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                sprintf(timeLine, "%ss%03d%02d", timeLine, textWidth + SLASH_RIGHT_X, seconds);
                switch (gradeDigits)
                {
                case 1:
                    strcpy(numberText, "\x82" "a");
                    break;
                case 2:
                    strcpy(numberText, "\x82`");
                    break;
                case 3:
                    strcpy(numberText, "\x82`\x81{");
                    break;
                case 4:
                    strcpy(numberText, "\x82r");
                    break;
                case 5:
                    strcpy(numberText, "\x82r\x81{");
                    break;
                case 6:
                    strcpy(numberText, "\x82r\x81{\x81{");
                    break;
                case 7:
                    strcpy(numberText, "\x82r\x81{\x81{\x81{");
                    break;
                default:
                    strcpy(numberText, "\x82" "b");
                }
                int gradeHalf = textWidth >> 1;
                width = DAT_1020e538.TextWidth(numberText, TEXT_SCALE);
                textWidth = (int)(float)width;
                sprintf(gradeLine, "%ss%03d%s", gradeLine, gradeHalf - (textWidth >> 1) + COLUMN_CENTRE_X, numberText);
                DAT_1020dc80 = (char)gradeDigits;
            }
            CopyText(DAT_1020da50[0], enemiesLine);
            CopyText(DAT_1020da50[1], containersLine);
            CopyText(DAT_1020da50[2], unusedLine);
            CopyText(DAT_1020da50[3], gameOverLine);
            CopyText(DAT_1020da50[4], timeLine);
            CopyText(DAT_1020da50[5], gradeLine);
        }
    }
    return 0;
}
