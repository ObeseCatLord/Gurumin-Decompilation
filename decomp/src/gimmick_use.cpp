// Using a lever or a chest: the interaction of the player with the gimmick in slot DAT_104bf888 (levers of type
// 0xb and 0xf, chests of type 10).

#include <stdio.h>
#include <string.h>
#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "game.h"
#include "game_vars.h"
#include "char_vars.h"
#include "item_info.h"
#include "gimmick_slot.h"

extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);
extern "C" int FUN_1000ed70(const char *text, const char *subtext, int frames);
extern "C" int FUN_10017b10(int index, int *high, int *low);
extern "C" int FUN_10012130(int *item, int *count);
extern "C" int FUN_1003b670(int value);
extern "C" int FUN_1013f880(void);

#define CURRENT_GIMMICK DAT_10860cec[DAT_104bf888]
#define GIMMICK_TYPE_CHEST 10
#define GIMMICK_TYPE_LEVER_SHORT 0xb
#define GIMMICK_TYPE_LEVER_LONG 0xf
#define GIMMICK_LEVER_SHORT_FRAMES 0xf
#define GIMMICK_LEVER_LONG_FRAMES 0x1e
#define GIMMICK_SOUND_LEVER_SHORT 0x37e
#define GIMMICK_SOUND_LEVER_LONG 0x37f
#define GIMMICK_SOUND_CHEST 0x380
#define GIMMICK_MESSAGE_FRAMES 0x28
#define GIMMICK_ITEM_FROM_FLAG_BASE 200
#define GIMMICK_ITEM_FLAG_OFFSET 0x856
#define GIMMICK_ITEM_REPLACEMENT 0x23
#define GIMMICK_FLAG_SEEN 0x80000000

// 0x1005ecb0 (Gimmick_Use). Toggles the lever in slot DAT_104bf888: plays its sound at the model, starts its colour
// plug motion, sets (or clears) its script flag and raises the LEVERON/LEVEROFF events. A chest opens: its script
// variable holds the packed item and count; the item is added to the inventory, a message tells what was found (or
// that the chest is empty, or that no more fit) and the player reacts with a voice line. Invalid slots show an error
// message. The caller passes 0, which is never read.
extern "C" void FUN_1005ecb0(int unused)
{
    char name[64];
    char message[128];
    int item;
    int count;

    if (CURRENT_GIMMICK.type == GIMMICK_TYPE_LEVER_SHORT)
    {
        if (CURRENT_GIMMICK.id == -1)
            goto invalid_lever;
        if (CURRENT_GIMMICK.frame == 0)
        {
            FUN_1003ad20(GIMMICK_SOUND_LEVER_SHORT, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
            CURRENT_GIMMICK.targetFrame = GIMMICK_LEVER_SHORT_FRAMES;
            CURRENT_GIMMICK.frame = 1;
            if (CURRENT_GIMMICK.node != 0)
                CURRENT_GIMMICK.node->RestartColourPlugs(CURRENT_GIMMICK.motionOn, DAT_104bf8a4);
            int flag = CURRENT_GIMMICK.flag;
            if (flag <= 3999 && flag != 0)
                DAT_10c62edc[flag] = 1;
            DAT_10874848.RaiseLeverEvents(CURRENT_GIMMICK.id, 1);
        }
        else if (CURRENT_GIMMICK.frame == GIMMICK_LEVER_SHORT_FRAMES)
        {
            FUN_1003ad20(GIMMICK_SOUND_LEVER_SHORT, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
            CURRENT_GIMMICK.targetFrame = 0;
            CURRENT_GIMMICK.frame = GIMMICK_LEVER_SHORT_FRAMES - 1;
            if (CURRENT_GIMMICK.node != 0)
                CURRENT_GIMMICK.node->RestartColourPlugs(CURRENT_GIMMICK.motionOff, DAT_104bf8a4);
            DAT_10874848.SetScriptFlag(CURRENT_GIMMICK.flag, 0);
            DAT_10874848.RaiseLeverEvents(CURRENT_GIMMICK.id, 0);
        }
    }
    if (CURRENT_GIMMICK.type == GIMMICK_TYPE_LEVER_LONG)
    {
        if (CURRENT_GIMMICK.id == -1)
        {
invalid_lever:
            FUN_1000ed70("\x95s\x90\xb3\x82\xc8\x83\x8c\x83o\x81[\x82\xc5\x82\xb7\x81" "B", "", GIMMICK_MESSAGE_FRAMES);
            return;
        }
        if (CURRENT_GIMMICK.frame == 0)
        {
            FUN_1003ad20(GIMMICK_SOUND_LEVER_LONG, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
            CURRENT_GIMMICK.targetFrame = GIMMICK_LEVER_LONG_FRAMES;
            CURRENT_GIMMICK.frame = 1;
            if (CURRENT_GIMMICK.node != 0)
                CURRENT_GIMMICK.node->RestartColourPlugs(CURRENT_GIMMICK.motionOn, DAT_104bf8a4);
            int flag = CURRENT_GIMMICK.flag;
            if (flag <= 3999 && flag != 0)
                DAT_10c62edc[flag] = 1;
            DAT_10874848.RaiseLeverEvents(CURRENT_GIMMICK.id, 1);
        }
        if (CURRENT_GIMMICK.frame == GIMMICK_LEVER_LONG_FRAMES)
        {
            FUN_1003ad20(GIMMICK_SOUND_LEVER_LONG, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
            CURRENT_GIMMICK.targetFrame = 0;
            CURRENT_GIMMICK.frame = GIMMICK_LEVER_LONG_FRAMES - 1;
            if (CURRENT_GIMMICK.node != 0)
                CURRENT_GIMMICK.node->RestartColourPlugs(CURRENT_GIMMICK.motionOff, DAT_104bf8a4);
            int flag = CURRENT_GIMMICK.flag;
            if (flag <= 3999 && flag != 0)
                DAT_10c62edc[flag] = 0;
            DAT_10874848.RaiseLeverEvents(CURRENT_GIMMICK.id, 0);
        }
    }
    if (CURRENT_GIMMICK.type != GIMMICK_TYPE_CHEST)
        return;
    if (CURRENT_GIMMICK.id == -1)
    {
        FUN_1000ed70("\x95s\x90\xb3\x82\xc8\x95\xf3\x94\xa0\x82\xc5\x82\xb7\x81" "B", "", GIMMICK_MESSAGE_FRAMES);
        return;
    }
    if (CURRENT_GIMMICK.frame == 0)
    {
        FUN_1003ad20(GIMMICK_SOUND_CHEST, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
        if (CURRENT_GIMMICK.node != 0)
            CURRENT_GIMMICK.node->RestartColourPlugs(CURRENT_GIMMICK.motionOn, DAT_104bf8a4);
    }
    if (CURRENT_GIMMICK.frame == GIMMICK_LEVER_LONG_FRAMES)
    {
        FUN_1003ad20(GIMMICK_SOUND_CHEST, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
        if (CURRENT_GIMMICK.node != 0)
            CURRENT_GIMMICK.node->RestartColourPlugs(CURRENT_GIMMICK.motionOff, DAT_104bf8a4);
    }
    if (CURRENT_GIMMICK.frame != 0)
        goto done;
    {
        int variable = CURRENT_GIMMICK.id;
        DAT_10874848.RaiseOpenEvents(variable);
        FUN_10017b10(variable, &item, &count);
        int found = count;
        if (item == 0 || count == 0)
            FUN_1000ed70("\x82\xa9\x82\xe7\x82\xc1\x82\xdb\x82\xbe\x81" "B", 0, GIMMICK_MESSAGE_FRAMES);
        else
        {
            if (item >= GIMMICK_ITEM_FROM_FLAG_BASE)
            {
                DAT_10874848.SetScriptFlag(item + GIMMICK_ITEM_FLAG_OFFSET, 1);
                item = GIMMICK_ITEM_REPLACEMENT;
            }
            int shown = item;
            if (FUN_10012130(&item, &count) != 0)
            {
                strcpy(name, DAT_1088e650[shown].name);
                sprintf(message, "\x81w%s\x81x\x82\xf0\x8e\xe8\x82\xc9\x93\xfc\x82\xea\x82\xbd\x81" "B", name, found);
                FUN_1000ed70(message, "", GIMMICK_MESSAGE_FRAMES);
                int sound;
                if (DAT_104c033a == 0)
                    sound = FUN_1013f880() % 3 + 0x8a;
                else
                    sound = FUN_1013f880() % 2 + 0x92 + (unsigned char)DAT_104c033a * 0xf;
                if (DAT_104c0332 == 0)
                    FUN_1003ad20(sound, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
                int slotVariable = CURRENT_GIMMICK.id;
                DAT_10c62edc[slotVariable] = 1;
                DAT_10c62edc[slotVariable] |= GIMMICK_FLAG_SEEN;
                FUN_1003b670(shown);
                goto done;
            }
            strcpy(name, DAT_1088e650[item].name);
            if (found != count)
            {
                sprintf(message, "\x81w%s\x81x\x82\xf0\x8e\xe8\x82\xc9\x93\xfc\x82\xea\x82\xbd\x81" "B", name, found - count);
                FUN_1000ed70(message, "", GIMMICK_MESSAGE_FRAMES);
                int sound;
                if (DAT_104c033a == 0)
                    sound = FUN_1013f880() % 3 + 0x8a;
                else
                    sound = FUN_1013f880() % 2 + 0x92 + (unsigned char)DAT_104c033a * 0xf;
                if (DAT_104c0332 == 0)
                    FUN_1003ad20(sound, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
                int packed = ((item << 8) | count) << 5;
                int slotVariable = CURRENT_GIMMICK.id;
                DAT_10c62edc[slotVariable] = packed | 1;
                DAT_10c62edc[slotVariable] |= GIMMICK_FLAG_SEEN;
                FUN_1003b670(shown);
                goto done;
            }
            sprintf(message, "\x81w%s\x81x\x82\xcd\x82\xb1\x82\xea\x88\xc8\x8f\xe3\x8e\x9d\x82\xc4\x82\xdc\x82\xb9\x82\xf1\x81" "B", name, count);
            FUN_1000ed70(message, "", GIMMICK_MESSAGE_FRAMES);
            int packed = ((item << 8) | count) << 5;
            int slotVariable = CURRENT_GIMMICK.id;
            DAT_10c62edc[slotVariable] = packed | 1;
            DAT_10c62edc[slotVariable] |= GIMMICK_FLAG_SEEN;
        }
    }
    {
        int sound;
        if (DAT_104c033a == 0)
            sound = FUN_1013f880() % 2 + 0x8d;
        else
            sound = FUN_1013f880() % 2 + 0x94 + (unsigned char)DAT_104c033a * 0xf;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(sound, &CURRENT_GIMMICK.node->worldMatrix.GetTranslation(), 1.0f);
    }
done:
    ((Latch *)&CURRENT_GIMMICK)->M_1000b9c0();
}
