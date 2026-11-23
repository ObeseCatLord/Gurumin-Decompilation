// A lookout NPC (0x100b7b10, behaviour 0x2a of Char_RunBehaviour 0x100dfcf0): a character with a lamp whose "eye"
// lights up and raises a game event when the player does something it watches for.

#include "last_position.h"
#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_walker[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "node.h"

#include "map_vars.h"
#include "game_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "npc_lookout.h"
#include "world.h"
#include "game.h"
#include "action_select_vars.h"
#include "enemy_queue.h"
#include "camera_vars.h"

#define LOOKOUT_LAMP_RANGE 75.0f
#define LOOKOUT_REACT_RANGE 550.0f
#define LOOKOUT_BOX_HALF_WIDTH 70.0f
#define LOOKOUT_BOX_HALF_DEPTH 50.0f
#define LOOKOUT_BOX_HEIGHT 230.0f
#define LOOKOUT_PROMPT_FRAMES 60
#define LOOKOUT_PROMPT_SHOWN 0x41
#define LOOKOUT_PROMPT_KEY 2
#define LOOKOUT_PROMPT_SOUND 0x356
#define LOOKOUT_REACT_FRAMES 30
#define LOOKOUT_EYE_FRAMES 10
#define LOOKOUT_EVENT 0x12d
#define LOOKOUT_STATE_WAITING 1
#define LOOKOUT_STATE_EYE_OPENING 2
#define LOOKOUT_STATE_EYE_OFF 3
#define LOOKOUT_BEST_NEEDED 5
#define LOOKOUT_EVENT_KIND_OTHER 0x2a
#define LOOKOUT_ITEM_ONES 0x14
#define LOOKOUT_ITEM_TENS 0x1e
#define LOOKOUT_ITEM_HUNDREDS 0x1f

extern "C" int FUN_1000d620(int index, int key, int frames);                  // KeyPrompt_Show
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound

// 0x100b7b10 (Npc_Lookout). Finds the "eye" of the nearest "@O_068" lamp node within 75 units, then keeps the eye
// lit while the player talks to it (state 2, lit after 10 frames) and gives it back to the lamp otherwise (state 3).
// The mode picks what it watches for: 1 = the player within 550 units in a greeting area, 2 = the enemy at the front
// of the queue within 550 units, 3 = the player standing in a 140 x 100 box more than 230 units above it, 4 = the
// state DAT_10210e1c = 2, 5 = a change in the stock of items 0x14, 0x1e and 0x1f (kept as three digits) with the
// player within 550 units. A reaction shows a key prompt with sound 0x356 and raises event 0x12d when its countdown
// ends. DAT_10c63390 lets one lookout react at a time. Returns 0.
extern "C" int FUN_100b7b10(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    if (DAT_1020b430[index].state == 0)
    {
        int found = 0;
        DAT_1020b430[index].watchedCode = -1;
        DAT_1021bdf8[index][0] = 0;
        Node *lamp;
        do
        {
            if (DAT_10255f08 == 0)
                break;
            lamp = DAT_10255f08->Find("@O_068", found);
            if (lamp != 0)
            {
                if ((lamp->worldMatrix.GetTranslation() - self).Length2D() < LOOKOUT_LAMP_RANGE)
                {
                    Node *eye = lamp->Find("eye", 0);
                    if (eye != 0)
                    {
                        DAT_1021bdf8[index][0] = eye;
                        break;
                    }
                }
            }
            found++;
        } while (lamp != 0);
        DAT_1020b430[index].state = (DAT_1021bdf8[index][0] != 0) * 2 + LOOKOUT_STATE_WAITING;
    }
    else if (DAT_1020b430[index].state != LOOKOUT_STATE_WAITING)
    {
        if (DAT_1020b430[index].state == LOOKOUT_STATE_EYE_OPENING)
        {
            if (DAT_1020b430[index].eyeOpen == 0)
            {
                if (++DAT_1020b430[index].wait >= LOOKOUT_EYE_FRAMES)
                {
                    DAT_1020b430[index].eyeOpen = 1;
                    DAT_1021bdf8[index][0]->RestartColourPlugs("off", DAT_104bf8a4);
                }
            }
            else if (--DAT_1020b430[index].wait <= 0)
            {
                DAT_1020b430[index].eyeOpen = 0;
                if (DAT_10c6351c != index)
                {
                    DAT_1020b430[index].state = LOOKOUT_STATE_EYE_OFF;
                    DAT_1021bdf8[index][0]->RestartColourPlugs("off0", DAT_104bf8a4);
                }
                else
                {
                    DAT_1021bdf8[index][0]->RestartColourPlugs("on", DAT_104bf8a4);
                }
            }
        }
        else if (DAT_1020b430[index].state == LOOKOUT_STATE_EYE_OFF && DAT_10c6351c == index)
        {
            DAT_1020b430[index].state = LOOKOUT_STATE_EYE_OPENING;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].eyeOpen = 0;
            DAT_1021bdf8[index][0]->RestartColourPlugs("on", DAT_104bf8a4);
        }
    }
    if (DAT_1020b430[index].lookoutMode == 1)
    {
        if (DAT_104c00e0 != 0 &&
            (DAT_104c00e8 == 0x17 || DAT_104c00e8 == 0x19 || DAT_104c00e8 == 0x16 || DAT_104c00e8 == 0x10 ||
             DAT_104c00e8 == 0x14 || DAT_104c00e8 == 0x18 || DAT_104c00e8 == 0x28 || DAT_104c00e8 == 0x15))
        {
            if ((DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D() <
                LOOKOUT_REACT_RANGE)
            {
                if (DAT_10c63390 == 0)
                {
                    DAT_1020b430[index].reactDelay = 1;
                    DAT_10c63390 = 1;
                }
                if (DAT_1020b430[index].promptFrames == 0)
                {
                    FUN_1000d620(index, LOOKOUT_PROMPT_KEY, LOOKOUT_PROMPT_FRAMES);
                    FUN_10065b10(index, LOOKOUT_PROMPT_SOUND, 1, 0);
                    DAT_1020b430[index].promptFrames = LOOKOUT_PROMPT_SHOWN;
                }
                goto finish;
            }
        }
        if (DAT_1020b430[index].reactDelay == 0)
            goto finish;
        DAT_1020b430[index].reactDelay = 0;
        DAT_10874848.RegisterEvent(LOOKOUT_EVENT);
    }
    else
    {
        if (DAT_1020b430[index].lookoutMode == 2)
        {
            if (DAT_10c66cf4 != 0 && DAT_10c63390 != DAT_10c66cf4 && DAT_10c66cf4 != 0x19)
            {
                if ((DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation())
                        .Length2D() < LOOKOUT_REACT_RANGE)
                {
                    DAT_1020b430[index].reactDelay = LOOKOUT_REACT_FRAMES;
                    DAT_10c63390 = DAT_10c66cf4;
                    if (DAT_1020b430[index].promptFrames == 0)
                    {
                        FUN_1000d620(index, LOOKOUT_PROMPT_KEY, LOOKOUT_PROMPT_FRAMES);
                        FUN_10065b10(index, LOOKOUT_PROMPT_SOUND, 1, 0);
                        DAT_1020b430[index].promptFrames = LOOKOUT_PROMPT_SHOWN;
                    }
                }
            }
            if (DAT_1020b430[index].reactDelay > 0 && --DAT_1020b430[index].reactDelay == 0)
            {
                DAT_10874848.RegisterEvent(LOOKOUT_EVENT);
                DAT_10c63390 = DAT_10c66cf4;
                DAT_10c66cf4 = 0;
                DAT_10c66cd4 = 0;
            }
            goto finish;
        }
        if (DAT_1020b430[index].lookoutMode == 3)
        {
            if (self.x - LOOKOUT_BOX_HALF_WIDTH < player.x && player.x < self.x + LOOKOUT_BOX_HALF_WIDTH &&
                self.y - LOOKOUT_BOX_HALF_DEPTH < player.y && player.y < self.y + LOOKOUT_BOX_HALF_DEPTH &&
                DAT_10db9274 != 0 && self.z + LOOKOUT_BOX_HEIGHT < player.z && DAT_10c63390 == 0)
            {
                DAT_10c63390 = 1;
                DAT_1020b430[index].reactDelay = LOOKOUT_REACT_FRAMES;
            }
        }
        else
        {
            if (DAT_1020b430[index].lookoutMode == 4)
            {
                int area = DAT_104c00e8;
                int streak = DAT_10467798;
                int amount = DAT_10db0544;
                if (DAT_10210e1c == 2)
                {
                    if (DAT_1020b430[index].reactDelay == 0 && DAT_10c63390 == 0 &&
                        DAT_10db6ee8[0].matrix.translation.y > 0.0f)
                    {
                        if (area == 0x17 || area == 0x19 || area == 0x16 || area == 0x10 || area == 0x14 ||
                            area == 0x28)
                        {
                            if (streak == 0 && DAT_1020b430[index].bestValue == LOOKOUT_BEST_NEEDED &&
                                amount != DAT_1020b430[index].bestAmount)
                                DAT_10c63394 = LOOKOUT_EVENT_KIND_OTHER;
                            else
                                DAT_10c63394 = area;
                            DAT_1020b430[index].reactDelay = LOOKOUT_REACT_FRAMES;
                        }
                        else if (streak == 0 && DAT_1020b430[index].bestValue == LOOKOUT_BEST_NEEDED &&
                                 amount != DAT_1020b430[index].bestAmount)
                        {
                            DAT_10c63394 = LOOKOUT_EVENT_KIND_OTHER;
                            DAT_1020b430[index].reactDelay = LOOKOUT_REACT_FRAMES;
                        }
                    }
                    DAT_10c63390 = 1;
                }
                if (DAT_1020b430[index].bestValue < streak)
                {
                    DAT_1020b430[index].bestValue = streak;
                    DAT_1020b430[index].bestAmount = amount;
                }
                if (amount > 0)
                    DAT_1020b430[index].bestValue = 0;
                if (DAT_1020b430[index].reactDelay > 0 && --DAT_1020b430[index].reactDelay == 0)
                    DAT_10874848.RegisterEvent(LOOKOUT_EVENT);
                if (area != 0)
                    DAT_10c6356c = area;
                goto finish;
            }
            if (DAT_1020b430[index].lookoutMode != 5)
                goto finish;
            char ones = DAT_103150c0[LOOKOUT_ITEM_ONES];
            if (DAT_1020b430[index].watchedCode == -1)
                DAT_1020b430[index].watchedCode =
                    ones + (DAT_103150c0[LOOKOUT_ITEM_TENS] + DAT_103150c0[LOOKOUT_ITEM_HUNDREDS] * 10) * 10;
            int code = DAT_1020b430[index].watchedCode;
            if ((code % 10 != ones || code / 10 % 10 != DAT_103150c0[LOOKOUT_ITEM_TENS] ||
                 code / 100 % 10 != DAT_103150c0[LOOKOUT_ITEM_HUNDREDS]) &&
                DAT_10c63390 == 0)
            {
                if ((DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation())
                        .Length2D() <= LOOKOUT_REACT_RANGE)
                {
                    DAT_10c63390 = 1;
                    DAT_1020b430[index].reactDelay = 1;
                    if (DAT_1020b430[index].promptFrames == 0)
                    {
                        FUN_1000d620(index, LOOKOUT_PROMPT_KEY, LOOKOUT_PROMPT_FRAMES);
                        FUN_10065b10(index, LOOKOUT_PROMPT_SOUND, 1, 0);
                        DAT_1020b430[index].promptFrames = LOOKOUT_PROMPT_SHOWN;
                    }
                }
                else
                {
                    DAT_1020b430[index].watchedCode = -1;
                }
            }
        }
        if (DAT_1020b430[index].reactDelay > 0 && --DAT_1020b430[index].reactDelay == 0)
            DAT_10874848.RegisterEvent(LOOKOUT_EVENT);
    }
finish:
    if (DAT_1020b430[index].promptFrames > 0)
        DAT_1020b430[index].promptFrames--;
    return 0;
}
