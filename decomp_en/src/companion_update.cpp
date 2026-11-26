// The player's companions (EN build). Ported from the matched JP source decomp/src/companion_update.cpp. Each
// companion k of character `index` lives in character slot index + 2 + k.
#include "anim_model.h"
#include "anti_tamper.h"
#include "camera.h"
#include "char.h"
#include "char_attach.h"
#include "char_inline.h"
#include "char_status.h"
#include "companion_update.h"
#include "marker_rider.h"
#include "node.h"
#include "char_shots.h"
#include "scene_start.h"
#include "npc_greet.h"
#include "vec4.h"

// Keeps companion 2 of character `index` (slot index + 4) in range and runs its down / revive cycle: mode 0 = active
// (checks health, waits outside the areas 0x13 and 0x30, goes down below its maximum), 1 = counting to 60 then revives
// with effect 0x36d0, sound 0x194 and 30 points, 2 = counts down to active, 10 and above counts down with effect 0x36d3
// at 70 and comes back at 11. Returns 0. Called by the character updater.
int Companion2_Update(int index)
{
    int slot = index + 2;
    if (g_charCounter[slot] > CHAR_COUNTER_MAX)
        g_charCounter[slot] = CHAR_COUNTER_MAX;
    if (g_charLevel[slot] > CHAR_LEVEL_MAX)
        g_charLevel[slot] = CHAR_LEVEL_MAX;
    if (g_charSpawnFlag[index + 4] > 0)
        g_charSpawnFlag[index + 4] = 0;
    unsigned char level = g_charHitLevel[index + 4];
    if (level != 0)
    {
        g_charHitLevel[index] = g_charHitLevel[index] < level ? level : g_charHitLevel[index];
    }
    int mode = g_charState[slot + 2].values[COMPANION_DOWN_MODE];
    if (mode != COMPANION_ACTIVE)
    {
        if (mode == COMPANION_REVIVING)
        {
            g_charPoseState[index + 4] = COMPANION_POSE_DOWN;
            if (g_charState[slot + 2].values[COMPANION_DOWN_TIMER] == 1)
                Character_PlaySoundInline(index, COMPANION_DOWN_SOUND, 1, 0);
            int timer = g_charState[slot + 2].values[COMPANION_DOWN_TIMER] + 1;
            g_charState[slot + 2].values[COMPANION_DOWN_TIMER] = timer;
            if (timer == COMPANION_REVIVE_FRAMES)
            {
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_ACTIVE;
                Vec4 position;
                Matrix4 matrix;
                if (g_charNodes[slot + 2][1] != 0)
                {
                    g_charNodes[slot + 2][1]->SetFlags(COMPANION_NODE_DOWN_FLAG);
                    matrix = g_charNodes[slot + 2][1]->worldMatrix;
                    Char_StartEffectInline(index, 0x36d0, position, -1, -1, &g_charNodes[slot + 2][1]->worldMatrix, 0, 0,
                                           &matrix, 0);
                }
                else
                {
                    matrix = RotationZ(g_characterModels[index].heading);
                    Char_StartEffectInline(index, 0x36d0, position, -1, -1, 0, 0, 0, &matrix, 0);
                }
                Character_PlaySoundInline(index, COMPANION_REVIVE_SOUND, 1, 0);
                g_guardedValue7 += COMPANION_REVIVE_BONUS;
                AntiTamper_StoreValues();
            }
        }
        else if (mode == COMPANION_RECOVERING)
        {
            if (--g_charState[slot + 2].values[COMPANION_DOWN_TIMER] == 0)
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_ACTIVE;
            g_charPoseState[index + 4] = COMPANION_POSE_DOWN;
        }
        else if (mode >= COMPANION_GOING_DOWN)
        {
            g_charState[slot + 2].values[COMPANION_DOWN_MODE] = mode - 1;
            if (mode - 1 == COMPANION_DOWN_EFFECT_AT)
            {
                Vec4 position;
                Matrix4 matrix;
                if (g_charNodes[slot + 2][1] != 0)
                {
                    matrix = g_charNodes[slot + 2][1]->worldMatrix;
                    Char_StartEffect(index, 0x36d3, position, -1, -1, &g_charNodes[slot + 2][1]->worldMatrix, 0, 0,
                                     &matrix, 0);
                }
                else
                {
                    matrix = RotationZ(g_characterModels[index].heading);
                    Char_StartEffect(index, 0x36d3, position, -1, -1, 0, 0, 0, &matrix, 0);
                }
            }
            if (g_charState[slot + 2].values[COMPANION_DOWN_MODE] == COMPANION_COMES_BACK)
            {
                if (g_charNodes[slot + 2][1] != 0)
                {
                    g_charNodes[slot + 2][1]->ClearFlags(COMPANION_NODE_DOWN_FLAG);
                    int health = g_charState[index].values[COMPANION_HEALTH_CAP] + COMPANION_HEALTH_LOW;
                    int recovered = g_charRecords[slot + 2].stats[STAT_HEALTH] + COMPANION_RECOVER_HEALTH;
                    if (health > recovered)
                        health = recovered;
                    g_charState[slot + 2].values[COMPANION_REVIVE_HEALTH] = health;
                    g_charRecords[slot + 2].stats[STAT_HEALTH] = g_charRecords[slot + 2].stats[STAT_MAX_HEALTH];
                    g_characterModels[slot + 2].root->SetColourB(COMPANION_COLOUR_VALUE, COMPANION_COLOUR_MASK);
                    g_charRecords[slot + 2].defeated = 0;
                    g_charKind[index + 4] = COMPANION_KIND_ENEMY;
                }
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_RECOVERING;
                g_charPoseState[index + 4] = COMPANION_POSE_DOWN;
            }
        }
    }
    else
    {
        int health = g_charRecords[slot + 2].stats[STAT_HEALTH];
        if (health > COMPANION_HEALTH_FULL)
        {
            int special = g_greetingFrames;
            int area = g_greetingArea;
            g_charRecords[slot + 2].defeated = 0;
            if ((special == 0 || (area != COMPANION_HIDE_AREA_A && area != COMPANION_HIDE_AREA_B)) &&
                g_charState[slot + 2].values[COMPANION_HIDE_COUNT] == 0 && g_guardedValue9 != COMPANION_NO_HIDE_MODE)
            {
                g_charRecords[slot + 2].held = 1;
                g_charRecords[slot + 2].defeated = 0;
            }
            else
            {
                g_charRecords[slot + 2].held = 0;
                g_charRecords[slot + 2].stats[STAT_VALUE_4] = COMPANION_HIDDEN_VALUE;
                if (special != 0 && (area == COMPANION_HIDE_AREA_A || area == COMPANION_HIDE_AREA_B))
                    g_charState[slot + 2].values[COMPANION_HIDE_COUNT] = COMPANION_HIDE_FRAMES;
                else
                    g_charState[slot + 2].values[COMPANION_HIDE_COUNT]--;
            }
            if (g_charHitFlag[index + 4] != 0)
            {
                g_charRecords[slot + 2].stats[STAT_HEALTH]--;
                g_charHitFlag[index + 4] = 0;
                if (g_charPoseState[index + 4] > COMPANION_POSE_HIT)
                    g_charPoseState[index + 4] = COMPANION_POSE_HIT;
            }
            if (g_charRecords[slot + 2].stats[STAT_HEALTH] <
                    g_charRecords[slot + 2].stats[STAT_MAX_HEALTH] - COMPANION_DAMAGE_LIMIT ||
                (g_guardedValue8 == 1 &&
                 g_charRecords[slot + 2].stats[STAT_HEALTH] < g_charRecords[slot + 2].stats[STAT_MAX_HEALTH]))
            {
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_REVIVING;
                int revived = g_charState[slot + 2].values[COMPANION_REVIVE_HEALTH];
                g_charRecords[slot + 2].held = 0;
                if (revived != 0)
                    g_charRecords[slot + 2].stats[STAT_HEALTH] = revived;
                else
                    g_charRecords[slot + 2].stats[STAT_HEALTH] =
                        g_charRecords[index].stats[STAT_MAX_HEALTH] + COMPANION_HEALTH_LOW;
                g_charRecords[slot + 2].stats[STAT_VALUE_4] = g_charRecords[index].stats[STAT_VALUE_4];
                g_charPoseState[index + 4] = COMPANION_POSE_REVIVED;
            }
        }
        else if (health > COMPANION_HEALTH_LOW)
        {
            g_charRecords[slot + 2].defeated = 0;
            if (g_charPoseState[index + 4] == COMPANION_FOLLOW_POSE)
            {
                if (g_charDisplayState[index + 4] > COMPANION_FAR_STATE_LIMIT)
                {
                    if (g_scopePointNode != 0)
                        g_charOffset[index].Copy(g_referencePosition - g_characterModels[index].matrix.GetTranslation());
                    g_charDisplayState[index] = COMPANION_FAR_STATE;
                }
                else
                {
                    if (g_scopePointNode != 0)
                        g_charOffset[index].Copy(g_referencePosition - g_characterModels[index].matrix.GetTranslation());
                    g_charDisplayState[index] = COMPANION_NEAR_STATE;
                }
            }
        }
        else
        {
            g_charRecords[slot + 2].stats[STAT_HEALTH] = COMPANION_HEALTH_LOW;
            g_charRecords[slot + 2].defeated = 1;
        }
    }
    return 0;
}

// Keeps companion 1 of character `index` (slot index + 3) in range and runs its down / revive cycle like
// Companion2_Update, with effects 0x36d1 (revive) and 0x36d4 (going down, with sound 0x193) started on the second node
// of the companion, a damage limit of 5 and a revive pose of 0x1e. A hidden companion sinks 50 units unless its owner
// keeps it up. Returns 0. Called by the character updater.
int Companion1_Update(int index)
{
    int slot = index + 1;
    if (g_charCounter[slot] > CHAR_COUNTER_MAX)
        g_charCounter[slot] = CHAR_COUNTER_MAX;
    if (g_charLevel[slot] > CHAR_LEVEL_MAX)
        g_charLevel[slot] = CHAR_LEVEL_MAX;
    if (g_charSpawnFlag[index + 3] > 0)
        g_charSpawnFlag[index + 3] = 0;
    unsigned char level = g_charHitLevel[index + 3];
    if (level != 0)
        g_charHitLevel[index] = g_charHitLevel[index] < level ? level : g_charHitLevel[index];
    int mode = g_charState[slot + 2].values[COMPANION_DOWN_MODE];
    if (mode != COMPANION_ACTIVE)
    {
        if (mode == COMPANION_REVIVING)
        {
            g_charPoseState[index + 3] = COMPANION_POSE_DOWN;
            if (g_charState[slot + 2].values[COMPANION_DOWN_TIMER] == 1)
                Character_PlaySoundInline(index, COMPANION_DOWN_SOUND, 1, 0);
            int timer = g_charState[slot + 2].values[COMPANION_DOWN_TIMER] + 1;
            g_charState[slot + 2].values[COMPANION_DOWN_TIMER] = timer;
            if (timer == COMPANION_REVIVE_FRAMES)
            {
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_ACTIVE;
                Vec4 position;
                Matrix4 matrix;
                if (g_charNodes[slot + 2][1] != 0)
                {
                    g_charNodes[slot + 2][1]->SetFlags(COMPANION_NODE_DOWN_FLAG);
                    matrix = g_charNodes[slot + 2][1]->worldMatrix;
                }
                else
                    matrix = RotationZ(g_characterModels[index].heading);
                if (g_charNodes[slot + 2][2] != 0)
                    Char_StartEffectInline(index, COMPANION1_REVIVE_EFFECT, position, -1, -1,
                                           &g_charNodes[slot + 2][2]->worldMatrix, 0, 0, &matrix, 0);
                else
                    Char_StartEffectInline(index, COMPANION1_REVIVE_EFFECT, position, -1, -1, 0, 0, 0, &matrix, 0);
                Character_PlaySoundInline(index, COMPANION_REVIVE_SOUND, 1, 0);
                g_guardedValue7 += COMPANION_REVIVE_BONUS;
                AntiTamper_StoreValues();
            }
        }
        else if (mode == COMPANION_RECOVERING)
        {
            g_charState[slot + 2].values[COMPANION_DOWN_TIMER]--;
            g_charPoseState[index + 3] = COMPANION_POSE_DOWN;
            if (g_charState[slot + 2].values[COMPANION_DOWN_TIMER] == 0)
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_ACTIVE;
        }
        else if (mode >= COMPANION_GOING_DOWN)
        {
            g_charState[slot + 2].values[COMPANION_DOWN_MODE] = mode - 1;
            if (mode - 1 == COMPANION_DOWN_EFFECT_AT)
            {
                Vec4 position;
                Matrix4 matrix;
                if (g_charNodes[slot + 2][1] != 0)
                    matrix = g_charNodes[slot + 2][1]->worldMatrix;
                else
                    matrix = RotationZ(g_characterModels[index].heading);
                if (g_charNodes[slot + 2][2] != 0)
                    Char_StartEffectInline(index, COMPANION1_DOWN_EFFECT, position, -1, -1,
                                           &g_charNodes[slot + 2][2]->worldMatrix, 0, 0, &matrix, 0);
                else
                    Char_StartEffectInline(index, COMPANION1_DOWN_EFFECT, position, -1, -1, 0, 0, 0, &matrix, 0);
                Character_PlaySoundInline(index, COMPANION1_GOING_DOWN_SOUND, 1, 0);
            }
            if (g_charState[slot + 2].values[COMPANION_DOWN_MODE] == COMPANION_COMES_BACK)
            {
                if (g_charNodes[slot + 2][1] != 0)
                {
                    g_charNodes[slot + 2][1]->ClearFlags(COMPANION_NODE_DOWN_FLAG);
                    int health = g_charState[index].values[COMPANION_HEALTH_CAP] + COMPANION_HEALTH_LOW;
                    int recovered = g_charRecords[slot + 2].stats[STAT_HEALTH] + COMPANION_RECOVER_HEALTH;
                    if (health > recovered)
                        health = recovered;
                    g_charState[slot + 2].values[COMPANION_REVIVE_HEALTH] = health;
                    g_charRecords[slot + 2].stats[STAT_HEALTH] = g_charRecords[slot + 2].stats[STAT_MAX_HEALTH];
                    g_characterModels[slot + 2].root->SetColourB(COMPANION_COLOUR_VALUE, COMPANION_COLOUR_MASK);
                    g_charRecords[slot + 2].defeated = 0;
                    g_charKind[index + 3] = COMPANION_KIND_ENEMY;
                }
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_RECOVERING;
                g_charPoseState[index + 3] = COMPANION_POSE_DOWN;
            }
        }
    }
    else
    {
        int health = g_charRecords[slot + 2].stats[STAT_HEALTH];
        if (health > COMPANION_HEALTH_FULL)
        {
            int special = g_greetingFrames;
            int area = g_greetingArea;
            g_charRecords[slot + 2].defeated = 0;
            if ((special == 0 || (area != COMPANION_HIDE_AREA_A && area != COMPANION_HIDE_AREA_B)) &&
                g_charState[slot + 2].values[COMPANION_HIDE_COUNT] == 0 && g_guardedValue9 != COMPANION_NO_HIDE_MODE)
            {
                g_charRecords[slot + 2].held = 1;
                g_charRecords[slot + 2].defeated = 0;
                g_shotSlots[slot + 2].position.z = 0;
            }
            else
            {
                g_charRecords[slot + 2].held = 0;
                g_charRecords[slot + 2].stats[STAT_VALUE_4] = COMPANION_HIDDEN_VALUE;
                if (g_charState[index].values[COMPANION_KEEP_UP] == 0)
                    g_shotSlots[slot + 2].position.z = COMPANION1_HIDDEN_Z;
                if (special != 0 && (area == COMPANION_HIDE_AREA_A || area == COMPANION_HIDE_AREA_B))
                    g_charState[slot + 2].values[COMPANION_HIDE_COUNT] = COMPANION_HIDE_FRAMES;
                else
                    g_charState[slot + 2].values[COMPANION_HIDE_COUNT]--;
            }
            if (g_charHitFlag[index + 3] != 0)
            {
                g_charRecords[slot + 2].stats[STAT_HEALTH]--;
                g_charHitFlag[index + 3] = 0;
                if (g_charPoseState[index + 3] > COMPANION_POSE_HIT)
                    g_charPoseState[index + 3] = COMPANION_POSE_HIT;
            }
            if (g_charRecords[slot + 2].stats[STAT_HEALTH] <
                    g_charRecords[slot + 2].stats[STAT_MAX_HEALTH] - COMPANION1_DAMAGE_LIMIT ||
                (g_guardedValue8 == 1 &&
                 g_charRecords[slot + 2].stats[STAT_HEALTH] < g_charRecords[slot + 2].stats[STAT_MAX_HEALTH]))
            {
                int revived = g_charState[slot + 2].values[COMPANION_REVIVE_HEALTH];
                g_charState[slot + 2].values[COMPANION_DOWN_MODE] = COMPANION_REVIVING;
                g_charRecords[slot + 2].held = 0;
                if (revived != 0)
                    g_charRecords[slot + 2].stats[STAT_HEALTH] = revived;
                else
                    g_charRecords[slot + 2].stats[STAT_HEALTH] =
                        g_charRecords[index].stats[STAT_MAX_HEALTH] + COMPANION_HEALTH_LOW;
                g_charRecords[slot + 2].stats[STAT_VALUE_4] = g_charRecords[index].stats[STAT_VALUE_4];
                g_charPoseState[index + 3] = COMPANION1_POSE_REVIVED;
                g_shotSlots[slot + 2].position.z = 0;
            }
        }
        else if (health > COMPANION_HEALTH_LOW)
        {
            g_charRecords[slot + 2].defeated = 0;
            if (g_charPoseState[index + 3] == COMPANION_FOLLOW_POSE)
            {
                if (g_charDisplayState[index + 3] > COMPANION_FAR_STATE_LIMIT)
                {
                    if (g_scopePointNode != 0)
                        g_charOffset[index].Copy(g_referencePosition - g_characterModels[index].matrix.GetTranslation());
                    g_charDisplayState[index] = COMPANION_FAR_STATE;
                }
                else
                {
                    if (g_scopePointNode != 0)
                        g_charOffset[index].Copy(g_referencePosition - g_characterModels[index].matrix.GetTranslation());
                    g_charDisplayState[index] = COMPANION_NEAR_STATE;
                }
            }
        }
        else
        {
            g_charRecords[slot + 2].stats[STAT_HEALTH] = COMPANION_HEALTH_LOW;
            g_charRecords[slot + 2].defeated = 1;
        }
    }
    return 0;
}
