// The drill gauge (DAT_10c64ad8) and the drill level it makes (DAT_10c64ad4, one level per 100 points).

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "effect_manager.h"
#include "anim_model.h"
#include "char_vars.h"
#include "game_vars.h"

Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360

extern "C" int FUN_1013f880(void);                                                     // the game's random number
extern "C" int FUN_1000e130(float x, float y, float z, const Vec4 *direction, int type, int colour);  // Debris_SpawnFrom
extern "C" int FUN_1000e750(void);                                                     // anti-tamper guard refresh
extern "C" int FUN_10012510(int mode, int index);                                      // attach/remove the drill tool
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);            // Sound_PlayEffect

#define GAUGE_PER_LEVEL 100
#define GAUGE_MAX_BASE 300
#define GAUGE_MAX_BANK_BONUS 100
#define GAUGE_MAX_DRILL5 99
#define LEVEL_FULL 4
#define TAIL_COUNT 4
#define COSTUME_POPON 6
#define DEBRIS_TYPE_LEVEL_BASE 0x66
#define DEBRIS_TYPE_POPON 0x6e
#define DEBRIS_COLOUR_DEFAULT -1
#define DEBRIS_SPREAD 0x32
#define DEBRIS_HEIGHT 15.0f
#define EFFECT_OWNER_ALL 999
#define SOUND_LEVEL_UP 0x333
#define SOUND_LEVEL_DOWN 0x334
#define MOTION_PLAYER_ALERT 0x58
#define MOTION_ALERT_BLEND 5
#define PLAYER_PUSH_LOCK 0x41f
#define TAIL_OFFSET_Y -10.0f
#define TAIL_OFFSET_Z 35.0f
#define TAIL_TURN_HALF 3.1415f
#define TAIL_TURN_QUARTER (TAIL_TURN_HALF / 2)

// 0x10057ee0 (UpdateDrillLevel). Clamps the gauge to its maximum (300, 400 with the bank, 99 for drill 5) and to 0
// and moves the level to match it. Going up a level plays sound 0x333 and, on reaching level 4, places the four
// "@tume01" attachments around the player's world matrix. Going down throws a debris piece with a random offset, plays
// sound 0x334 and removes the drill tool. Otherwise the stored level and gauge are refreshed (and the player raises
// its alert motion while its push lock is 0x41f). Returns 0.
int UpdateDrillLevel(void)
{
    int gauge = DAT_10c64ad8;
    int limit = (DAT_10c64ad0 != 0 ? GAUGE_MAX_BANK_BONUS : 0) + GAUGE_MAX_BASE;
    int level = DAT_10c64ad4;
    if (DAT_10c6566c != 0)
        limit = GAUGE_MAX_DRILL5;
    int clamped = 0;
    if (gauge > limit)
    {
        gauge = limit;
        clamped = 1;
    }
    if (gauge < 0)
    {
        gauge = 0;
        clamped = 1;
    }
    int changed = clamped;
    if (DAT_104c00e0 == 0)
    {
        if ((limit != gauge || (gauge - 1) / GAUGE_PER_LEVEL > level) && (level + 1) * GAUGE_PER_LEVEL <= gauge)
        {
            level++;
            DAT_10c64ad4 = level;
            DAT_10c64ad8 = gauge;
            DAT_10daaa80.RemoveOwnedBy(EFFECT_OWNER_ALL);
            FUN_10012510(0, 0);
            FUN_10012510(1, 0);
            DAT_104c00bd = 1;
            DAT_104c01b4 = 0;
            FUN_1003ad20(SOUND_LEVEL_UP, 0, 1.0f);
            if (level == LEVEL_FULL)
            {
                Node *tume = DAT_10db6ee8[0].root->Find("@tume01", 0);
                if (tume != 0)
                {
                    Vec4 unused0;
                    Vec4 unused1;
                    Vec4 unused2;
                    Vec4 unused3;
                    Matrix4 matrix;
                    for (int i = 0; i < TAIL_COUNT; i++)
                    {
                        matrix = tume->worldMatrix;
                        if (i == 0)
                            matrix = TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * matrix;
                        else if (i == 1)
                            matrix = (TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * RotationZ(TAIL_TURN_QUARTER)) * matrix;
                        else if (i == 2)
                            matrix = (TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * RotationZ(-TAIL_TURN_QUARTER)) * matrix;
                        else if (i == 3)
                            matrix = (TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * RotationZ(TAIL_TURN_HALF)) * matrix;
                        DAT_102559f4[i]->matrix = matrix;
                    }
                }
            }
            changed = 1;
        }
        else if (level > 0 && level * GAUGE_PER_LEVEL > gauge)
        {
            Vec4 spawn;
            Vec4 direction;
            direction.Set(0, 0, DEBRIS_HEIGHT, 0);
            spawn.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            spawn.x += (float)(FUN_1013f880() % DEBRIS_SPREAD - FUN_1013f880() % DEBRIS_SPREAD);
            spawn.y += (float)(FUN_1013f880() % DEBRIS_SPREAD - FUN_1013f880() % DEBRIS_SPREAD);
            if (DAT_10c655f0 == COSTUME_POPON)
                FUN_1000e130(spawn.x, spawn.y, spawn.z, &direction, DEBRIS_TYPE_POPON, DEBRIS_COLOUR_DEFAULT);
            else
                FUN_1000e130(spawn.x, spawn.y, spawn.z, &direction, level + DEBRIS_TYPE_LEVEL_BASE,
                             DEBRIS_COLOUR_DEFAULT);
            level--;
            DAT_104c00bd = 1;
            DAT_104c01b4 = 1;
            DAT_10c64ad4 = level;
            DAT_10daaa80.RemoveOwnedBy(EFFECT_OWNER_ALL);
            DAT_10c64ad4 = level;
            DAT_10c64ad8 = gauge;
            FUN_10012510(0, 0);
            FUN_10012510(1, 0);
            FUN_1003ad20(SOUND_LEVEL_DOWN, 0, 1.0f);
            changed = 1;
        }
    }
    if (changed)
    {
        DAT_10c64ad4 = level;
        DAT_10c64ad8 = gauge;
        FUN_1000e750();
    }
    if (DAT_104c00d0 == PLAYER_PUSH_LOCK)
        DAT_10db6ee8[0].PlayMotion(MOTION_PLAYER_ALERT, 1, MOTION_ALERT_BLEND, 1.0f);
    return 0;
}
