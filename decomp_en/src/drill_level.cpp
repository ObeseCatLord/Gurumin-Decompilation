// The drill gauge (g_guardedValue7) and the drill level it makes (g_guardedValue6, one level per 100 points); EN build,
// JP twin: drill_level.cpp.
#include "anim_model.h"
#include "anti_tamper.h"
#include "char.h"
#include "char_attach.h"
#include "debris.h"
#include "drill_level.h"
#include "effect_manager.h"
#include "engine.h"
#include "pickups.h"
#include "scene_start.h"
#include "sound_effects.h"
#include "vec4.h"

// Gauge points per drill level, the gauge limits (base, with the bank, drill 5) and the level that grows claws.
const int GAUGE_PER_LEVEL = 100;
const int GAUGE_MAX_BASE = 300;
const int GAUGE_MAX_BANK = 400;
const int GAUGE_MAX_DRILL5 = 99;
const int LEVEL_FULL = 4;
// Costume whose lost levels throw its own debris piece, and the debris piece types.
const int COSTUME_POPON = 6;
const int DEBRIS_TYPE_LEVEL_BASE = 0x66;
const int DEBRIS_TYPE_POPON = 0x6e;
const int DEBRIS_COLOUR_DEFAULT = -1;
// Random spread of the thrown piece (each axis is the difference of two draws below this) and its upward direction.
const int DEBRIS_SPREAD = 0x32;
const float DEBRIS_HEIGHT = 15.0f;
// Owner argument of RemoveOwnedBy that stops every effect.
const int EFFECT_OWNER_ALL = 999;
const int SOUND_LEVEL_UP = 0x333;
const int SOUND_LEVEL_DOWN = 0x334;
// Alert motion of the player while its push lock (g_sceneTimer) is PLAYER_PUSH_LOCK.
const int MOTION_PLAYER_ALERT = 0x58;
const int MOTION_ALERT_BLEND = 5;
const int PLAYER_PUSH_LOCK = 0x41f;
// Where each claw sits relative to "@tume01", and the quarter and half turns that spread the claws.
const float TAIL_OFFSET_Y = -10.0f;
const float TAIL_OFFSET_Z = 35.0f;
const float TAIL_TURN_HALF = 3.1415f;
const float TAIL_TURN_QUARTER = TAIL_TURN_HALF / 2;

// UpdateDrillLevel: clamps the gauge to its maximum (300, 400 with the bank, 99 for drill 5) and to 0 and moves the
// level to match it. Going up a level plays sound 0x333 and, on reaching level 4, places the four "@tume01"
// attachments around the player's world matrix. Going down throws a debris piece with a random offset, plays sound
// 0x334 and removes the drill tool. Otherwise the stored level and gauge are refreshed (and the player raises its alert
// motion while its push lock is 0x41f). Returns 0. Called by the frame code.
int UpdateDrillLevel(void)
{
    int limit = g_pickupBankExtended != 0 ? GAUGE_MAX_BANK : GAUGE_MAX_BASE;
    if (g_drill5 != 0)
        limit = GAUGE_MAX_DRILL5;
    int clamped = 0;
    int gauge = g_guardedValue7;
    int level = g_guardedValue6;
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
    if (g_greetingFrames == 0)
    {
        if ((limit != gauge || (gauge - 1) / GAUGE_PER_LEVEL > level) && (level + 1) * GAUGE_PER_LEVEL <= gauge)
        {
            level++;
            g_guardedValue6 = level;
            g_guardedValue7 = gauge;
            g_effectManager.RemoveOwnedBy(EFFECT_OWNER_ALL);
            Char_SetHandTool(0, 0);
            Char_SetHandTool(1, 0);
            g_drillLevelChanged = 1;
            g_drillLevelWentDown = 0;
            Sound_PlayEffect(SOUND_LEVEL_UP, 0, 1.0f);
            if (level == LEVEL_FULL)
            {
                Node *tume = g_characterModels[0].root->Find("@tume01", 0);
                if (tume != 0)
                {
                    Vec4 unused0;
                    Vec4 unused1;
                    Vec4 unused2;
                    Vec4 unused3;
                    Matrix4 matrix;
                    for (int i = 0; i < DRILL_CLAW_COUNT; i++)
                    {
                        matrix = tume->worldMatrix;
                        if (i == 0)
                            matrix = TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * matrix;
                        else if (i == 1)
                            matrix = (TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * RotationZ(TAIL_TURN_QUARTER)) *
                                     matrix;
                        else if (i == 2)
                            matrix = (TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * RotationZ(-TAIL_TURN_QUARTER)) *
                                     matrix;
                        else if (i == 3)
                            matrix = (TranslationMatrix(0, TAIL_OFFSET_Y, TAIL_OFFSET_Z) * RotationZ(TAIL_TURN_HALF)) *
                                     matrix;
                        g_drillClaws[i]->matrix = matrix;
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
            spawn.Copy(g_characterModels[0].matrix.GetTranslation());
            spawn.x += (float)(Random() % DEBRIS_SPREAD - Random() % DEBRIS_SPREAD);
            spawn.y += (float)(Random() % DEBRIS_SPREAD - Random() % DEBRIS_SPREAD);
            if (g_guardedValue9 == COSTUME_POPON)
                Debris_SpawnFrom(spawn.x, spawn.y, spawn.z, &direction, DEBRIS_TYPE_POPON, DEBRIS_COLOUR_DEFAULT);
            else
                Debris_SpawnFrom(spawn.x, spawn.y, spawn.z, &direction, level + DEBRIS_TYPE_LEVEL_BASE,
                                 DEBRIS_COLOUR_DEFAULT);
            level--;
            g_drillLevelChanged = 1;
            g_drillLevelWentDown = 1;
            g_guardedValue6 = level;
            g_effectManager.RemoveOwnedBy(EFFECT_OWNER_ALL);
            g_guardedValue6 = level;
            g_guardedValue7 = gauge;
            Char_SetHandTool(0, 0);
            Char_SetHandTool(1, 0);
            Sound_PlayEffect(SOUND_LEVEL_DOWN, 0, 1.0f);
            changed = 1;
        }
    }
    if (changed)
    {
        g_guardedValue6 = level;
        g_guardedValue7 = gauge;
        AntiTamper_StoreValues();
    }
    if (g_sceneTimer == PLAYER_PUSH_LOCK)
        g_characterModels[0].PlayMotion(MOTION_PLAYER_ALERT, 1, MOTION_ALERT_BLEND, 1.0f);
    return 0;
}
