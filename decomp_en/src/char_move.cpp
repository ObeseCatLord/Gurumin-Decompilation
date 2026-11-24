// Character movement (EN build). Ported from the matched JP source decomp/src/char_move.cpp.
#include "char_move.h"
#include "anim_model.h"
#include "char.h"
#include "dialogs.h"
#include "engine.h"
#include "sound_effects.h"
#include "camera.h"
#include "collide_query.h"
#include "effect_manager.h"
#include "listbank.h"
#include "visibility.h"
#include <math.h>
#include <string.h>

// Character_SetMove: sets the movement impulse of character `index` from (x, y, z): half of it in the walking mode
// unless the idle motion is active, otherwise 0.7 of it, raised to 1.4 while the character is nearly standing still
// (speed shorter than 1). Called by the enemy and player movement code. Returns 0.
extern "C" int Character_SetMove(int index, float x, float y, float z)
{
    if (g_characterModels[index].idleMotionEnabled != IDLE_MOTION_ACTIVE && g_characterModels[index].mode == MODEL_MODE_WALKING)
    {
        g_characterModels[index].move.x = x * MOVE_SCALE_WALKING;
        g_characterModels[index].move.y = y * MOVE_SCALE_WALKING;
        g_characterModels[index].move.z = z * MOVE_SCALE_WALKING;
        return 0;
    }
    g_characterModels[index].move.x = x * MOVE_SCALE_NORMAL;
    g_characterModels[index].move.y = y * MOVE_SCALE_NORMAL;
    g_characterModels[index].move.z = z * MOVE_SCALE_NORMAL;
    if (g_characterModels[index].velocity.LengthSquared3() < STANDING_SPEED_SQUARED)
    {
        g_characterModels[index].move.x = x * MOVE_SCALE_STANDING;
        g_characterModels[index].move.y = y * MOVE_SCALE_STANDING;
        g_characterModels[index].move.z = z * MOVE_SCALE_STANDING;
    }
    return 0;
}

// Npc_Idle: keeps NPC `index` standing still in idle motion 2. Now and then (a 3 % chance per frame, then at least 75
// frames apart) an NPC with a talk line plays one of the voice sounds 0x209..0x20b and shows its talk prompt for 60
// frames. Called by the NPC behaviour update. Returns 0.
extern "C" int Npc_Idle(int index)
{
    if (g_charToldToReact[index] != 0 || g_charState[index].sawPlayer != 0)
    {
        g_charToldToReact[index] = 0;
        g_charState[index].sawPlayer = 0;
    }
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_charState[index].state = NPC_STATE_IDLE;
        return 0;
    }
    if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        g_characterModels[index].idling = 1;
        g_characterModels[index].moveStep.Fill(0);
        g_characterModels[index].move.Fill(0);
        g_characterModels[index].velocity.Fill(0);
        g_charTalkPending[index] = 0;
        if (g_charState[index].values[CHARSTATE_NPC_WAIT] > 0)
        {
            g_charState[index].values[CHARSTATE_NPC_WAIT]--;
            return 0;
        }
        if (g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE != 0 &&
            Random() % NPC_TALK_CHANCE_RANGE < NPC_TALK_CHANCE)
        {
            Character_PlaySound(index, Random() % NPC_TALK_SOUND_COUNT + NPC_TALK_SOUND_FIRST, 0, 0);
            KeyPrompt_Show(index,
                           g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE * NPC_PROMPT_KEY_SCALE - 1 +
                               g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE,
                           NPC_PROMPT_FRAMES);
            g_charState[index].values[CHARSTATE_NPC_WAIT] = NPC_TALK_GAP_FRAMES;
        }
    }
    return 0;
}

// Npc_WaitFacingAway: a character that waits with its back to the player: on the first frame it starts idle motion 2
// and takes its reaction distance from the talk id (line * 10). Then it keeps turning so that it faces away from the
// player (idle motion 2 once it is within about one degree, else motion 0). When the player comes closer than that
// distance (or the character was told to react, and has a script number) its state is reset to behaviour 0xf with
// talk id 999 and it starts walking away (16 units along its facing). Returns 0. Called by the NPC update.
extern "C" int Npc_WaitFacingAway(int index)
{
    float distance = (g_characterModels[0].matrix.GetTranslation() - g_characterModels[index].matrix.GetTranslation()).Length2D();
    CharState *state = &g_charState[index];
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_charState[index].state = NPC_STATE_IDLE;
        g_characterModels[index].mode = 0;
        g_charState[index].values[CHARSTATE_NPC_RANGE] =
            g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE * NPC_RANGE_PER_LINE;
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        return 0;
    }
    {
        float turn = g_characterModels[index].heading - g_characterModels[index].targetHeading;
        while (turn > ANGLE_HALF_TURN)
            turn -= 6.283f;  // a full turn in radians as the game rounds it
        while (turn < -ANGLE_HALF_TURN)
            turn += 6.283f;
        float squared = turn * turn;
        if (squared < TURN_SQUARED_LIMIT)
            g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        else
            g_characterModels[index].PlayMotion(NPC_WALK_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
    }
    g_characterModels[index].targetHeading =
        (float)atan2(g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                     g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y]) +
        ANGLE_HALF_TURN;
    if (distance < (float)g_charState[index].values[CHARSTATE_NPC_RANGE] ||
        (g_charState[index].sawPlayer != 0 && g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE != 0))
    {
        memset(state, 0, sizeof(CharState));
        g_charState[index].state = NPC_STATE_WALK_AWAY;
        g_charState[index].values[CHARSTATE_NPC_WAIT] = NPC_WALK_AWAY_WAIT;
        g_charState[index].values[CHARSTATE_NPC_TALK] = NPC_WALK_AWAY_TALK;
        g_charState[index].behaviour = NPC_WALK_AWAY_BEHAVIOUR;
        g_charToldToReact[index] = 0;
        g_characterModels[index].mode = MODEL_MODE_WALKING;
        Vec4 step;
        step.y = NPC_WALK_AWAY_STEP;
        step.RotateZ(-g_characterModels[index].heading);
        Character_SetMove(index, step.x, step.y, 0);
        g_characterModels[index].velocity.Copy(step);
        g_characterModels[index].AddMotionSpeed(NPC_WALK_AWAY_SPEED);
    }
    return 0;
}

// Npc_TalkWhenNear tuning. The player counts as near when closer than the talk line's hundreds (in units); the enemies
// woken by it are within this radius.
const float NPC_WAKE_RADIUS = 800.0f;
// Chance per frame (out of NPC_TALK_CHANCE_RANGE) that the waiting NPC speaks.
const int NPC_WAIT_TALK_CHANCE = 150;
// Prompt lines of the first talk (talk line 10 and any other), the voice played when it starts, and the distance
// (squared) within which that voice is heard when the NPC is not one of the enemy slots.
const int NPC_PROMPT_TALK_TEN = 9;
const int NPC_PROMPT_TALK_OTHER = 15;
const int NPC_TALK_LINE_TEN = 10;
const int NPC_START_VOICE = 0x20c;
const float NPC_START_VOICE_RANGE_SQUARED = 6250000.0f;
// Talk lines above this one use prompt keys offset by NPC_PROMPT_LATE_OFFSET.
const int NPC_PROMPT_LAST_EARLY_LINE = 20;
const int NPC_PROMPT_LATE_OFFSET = 1979;
// Speed added to the NPC's motion when it turns to the player.
const float NPC_TURN_SPEED = 20.0f;

// Npc_TalkWhenNear: idles NPC `index` like Npc_Idle (voice sounds now and then with the talk prompt) and starts talking
// once the player comes closer than the talk id's hundreds (or at once when told to react; `wakeEnemies` also wakes the
// enemies within 800): it turns to the player, speeds up its motion, plays voice 0x20c at its position (when the camera
// is near or the slot is below the enemy slots), shows prompt 9 (line 10) or 15 and its state is reset. Returns 0.
// Called by the NPC update.
extern "C" int Npc_TalkWhenNear(int index, int wakeEnemies)
{
    float distance = (g_characterModels[0].matrix.GetTranslation() - g_characterModels[index].matrix.GetTranslation()).Length2D();
    if (g_charToldToReact[index] != 0 ||
        distance < (float)(g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE * NPC_TALK_SCRIPT_SCALE))
    {
        g_charToldToReact[index] = 0;
        if (wakeEnemies != 0)
            Enemies_WakeNearby(index, NPC_WAKE_RADIUS, 0);
        g_charState[index].sawPlayer = 1;
    }
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_charState[index].state = NPC_STATE_IDLE;
        return 0;
    }
    if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        if (g_charState[index].values[CHARSTATE_NPC_WAIT] > 0)
            g_charState[index].values[CHARSTATE_NPC_WAIT]--;
        else if (g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE != 0 &&
                 Random() % NPC_TALK_CHANCE_RANGE < NPC_WAIT_TALK_CHANCE)
        {
            Character_PlaySound(index, Random() % NPC_TALK_SOUND_COUNT + NPC_TALK_SOUND_FIRST, 0, 0);
            int line = g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE;
            if (line > NPC_PROMPT_LAST_EARLY_LINE)
                KeyPrompt_Show(index, line + NPC_PROMPT_LATE_OFFSET, NPC_PROMPT_FRAMES);
            else
                KeyPrompt_Show(index, line - 1, NPC_PROMPT_FRAMES);
            g_charState[index].values[CHARSTATE_NPC_WAIT] = NPC_TALK_GAP_FRAMES;
        }
        if (g_charState[index].sawPlayer != 0)
        {
            g_characterModels[index].targetHeading =
                (float)atan2(g_characterModels[0].matrix.m[MATRIX_TRANSLATION_X] - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                             g_characterModels[0].matrix.m[MATRIX_TRANSLATION_Y] - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y]) +
                ANGLE_HALF_TURN;
            g_characterModels[index].AddMotionSpeed(NPC_TURN_SPEED);
            if ((g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3() <
                    NPC_START_VOICE_RANGE_SQUARED ||
                index < ENEMY_SLOT_FIRST)
            {
                Vec4 position;
                position.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                             g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                             g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
                Sound_PlayEffect(NPC_START_VOICE, &position, 1.0f);
            }
            if (g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE == NPC_TALK_LINE_TEN)
                KeyPrompt_Show(index, NPC_PROMPT_TALK_TEN, NPC_PROMPT_FRAMES);
            else
                KeyPrompt_Show(index, NPC_PROMPT_TALK_OTHER, NPC_PROMPT_FRAMES);
            Char_ResetState(index);
        }
    }
    return 0;
}

// Npc_AlertWhenSeen tuning. The NPC looks from this far above its origin to this far above the player's, every
// NPC_SIGHT_PERIOD frames; it reacts within the talk line times NPC_ALERT_RANGE_PER_LINE units.
const float NPC_SIGHT_HEIGHT = 50.0f;
const int NPC_SIGHT_PERIOD = 10;
const int NPC_ALERT_RANGE_PER_LINE = 10;
// Voice played, and effect started, when the NPC notices the player; the talk id's hundreds hold two bits that switch
// them off.
const int NPC_ALERT_VOICE = 0x2d0;
const int NPC_ALERT_EFFECT = 0x3f4;
const int NPC_ALERT_NO_VOICE_BIT = 2;
const int NPC_ALERT_NO_EFFECT_BIT = 1;
// Value of g_charGroupTimer set when the player is noticed.
const int NPC_ALERT_GROUP_TIMER = 2;
// Effect parent and unused arguments meaning "none".
const int NPC_ALERT_EFFECT_NO_PARENT = -1;

// Npc_AlertWhenSeen: places character `index` where it stands on its first frame; afterwards, once every ten frames
// (the frame counter's units equal the character's), it checks whether nothing blocks the line from its head to the
// player's head (50 above each origin). When the player is that visible and nearer than the talk id's line * 10 units,
// it sets its group timer to 2, plays voice 0x2d0 (unless the talk id's hundreds have bit 2), starts effect 0x3f4 at its
// position (unless they have bit 1) and its state is reset. Returns 0. Called by the NPC update.
int Npc_AlertWhenSeen(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    float distance = (player - self).Length2D();
    g_charRecords[index].defeated = 1;
    int visible = 0;
    if (g_frameCounter % NPC_SIGHT_PERIOD == index % NPC_SIGHT_PERIOD)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.Copy(self);
        from.z += NPC_SIGHT_HEIGHT;
        to.Copy(player);
        to.z += NPC_SIGHT_HEIGHT;
        if (g_mapRootNode == 0 || g_mapRootNode->Collide(0, COLLISION_MASK_SMALL_OBJECTS, &from, &to, &hit) == 0)
            visible = 1;
    }
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_characterModels[index].SetPosition(self.x, self.y, self.z);
        g_characterModels[index].alpha = 0;
        g_characterModels[index].mode = 0;
        g_charState[index].state = NPC_STATE_IDLE;
    }
    else if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].alpha = 0;
        if (distance < (float)(g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE * NPC_ALERT_RANGE_PER_LINE) &&
            visible != 0)
        {
            g_charGroupTimer[index] = NPC_ALERT_GROUP_TIMER;
            if ((g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE & NPC_ALERT_NO_VOICE_BIT) == 0)
                Character_PlaySound(index, NPC_ALERT_VOICE, 0, 0);
            if ((g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE & NPC_ALERT_NO_EFFECT_BIT) == 0)
            {
                Matrix4 matrix;
                Vec4 position;
                matrix.SetIdentity();
                matrix.SetTranslation(g_characterModels[index].matrix.GetTranslation());
                g_effectManager.Start(NPC_ALERT_EFFECT, position, NPC_ALERT_EFFECT_NO_PARENT, NPC_ALERT_EFFECT_NO_PARENT, 0, 0,
                                      0, &matrix, 0, 0);
            }
            Char_ResetState(index);
        }
    }
    return 0;
}

// Npc_TurnWhenSeen: Npc_AlertWhenSeen with one step more: when it reacts, the character also turns at once to face the
// player (heading and target heading). It places the character where it stands on its first frame; afterwards, once
// every ten frames (the frame counter's units equal the character's), it checks whether nothing blocks the line from
// its head to the player's head (50 above each origin). When the player is that visible and nearer than the talk id's
// line * 10 units, it sets its group timer to 2, turns, plays voice 0x2d0 (unless the talk id's hundreds have bit 2), starts effect 0x3f4 at its
// position (unless they have bit 1) and its state is reset. Returns 0. Called by the NPC update.
int Npc_TurnWhenSeen(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    float distance = (player - self).Length2D();
    g_charRecords[index].defeated = 1;
    int visible = 0;
    if (g_frameCounter % NPC_SIGHT_PERIOD == index % NPC_SIGHT_PERIOD)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.Copy(self);
        from.z += NPC_SIGHT_HEIGHT;
        to.Copy(player);
        to.z += NPC_SIGHT_HEIGHT;
        if (g_mapRootNode == 0 || g_mapRootNode->Collide(0, COLLISION_MASK_SMALL_OBJECTS, &from, &to, &hit) == 0)
            visible = 1;
    }
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_characterModels[index].SetPosition(self.x, self.y, self.z);
        g_characterModels[index].alpha = 0;
        g_characterModels[index].mode = 0;
        g_charState[index].state = NPC_STATE_IDLE;
    }
    else if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].alpha = 0;
        if (distance < (float)(g_charState[index].values[CHARSTATE_NPC_TALK] % NPC_TALK_SCRIPT_SCALE * NPC_ALERT_RANGE_PER_LINE) &&
            visible != 0)
        {
            g_charGroupTimer[index] = NPC_ALERT_GROUP_TIMER;
            float target = (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
            g_characterModels[index].targetHeading = target;
            g_characterModels[index].heading = target;
            if ((g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE & NPC_ALERT_NO_VOICE_BIT) == 0)
                Character_PlaySound(index, NPC_ALERT_VOICE, 0, 0);
            if ((g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE & NPC_ALERT_NO_EFFECT_BIT) == 0)
            {
                Matrix4 matrix;
                Vec4 position;
                matrix.SetIdentity();
                matrix.SetTranslation(g_characterModels[index].matrix.GetTranslation());
                g_effectManager.Start(NPC_ALERT_EFFECT, position, NPC_ALERT_EFFECT_NO_PARENT, NPC_ALERT_EFFECT_NO_PARENT, 0, 0,
                                      0, &matrix, 0, 0);
            }
            Char_ResetState(index);
        }
    }
    return 0;
}

// Npc_RiseWhenSeen tuning. The NPC waits this far below the ground it was placed on, and checks the line from this far
// below its origin to this far above the player's.
const float RISE_HIDDEN_DEPTH = 350.0f;
const float RISE_PLACE_DEPTH = 400.0f;
// Distance within which the NPC rises (talk id units digit 0) or moves back up and starts again (digits 1 and 2).
const float RISE_RANGE = 800.0f;
const float RISE_RANGE_REDIRECT = 1300.0f;
// Talk id divisor that splits the digit that selects the reaction from the talk id the NPC then starts with.
const int RISE_TALK_BASE = 10;
// CharState::state value while the NPC rises, and the frames the rise takes.
const int NPC_STATE_RISING = 2;
const int RISE_FRAMES = 15;
// Voice played when the NPC starts rising.
const int RISE_VOICE = 0x208;
// CharState::behaviour values the redirect reactions switch to.
const int RISE_BEHAVIOUR_FIRST = 5;
const int RISE_BEHAVIOUR_SECOND = 6;

// Npc_RiseWhenSeen: a character that waits 400 units below the ground where it was placed. Once every ten frames it
// checks the line from 350 below its origin to the player's head (50 above the player's origin). The talk id's units say
// what happens when that line is clear: 0 = within 800 units it rises (state 2: alpha grows to 1 over 15 frames, voice
// 0x208) and its state is reset; 1 or 2 = within 1300 units it moves back up and starts again with the talk id's remaining
// digits and behaviour 5 or 6. Returns 0. Called by the NPC update.
int Npc_RiseWhenSeen(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    float distance = (player - self).Length2D();
    g_charRecords[index].defeated = 1;
    int visible = 0;
    if (g_frameCounter % NPC_SIGHT_PERIOD == index % NPC_SIGHT_PERIOD)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.Copy(self);
        from.z -= RISE_HIDDEN_DEPTH;
        to.Copy(player);
        to.z += NPC_SIGHT_HEIGHT;
        if (g_mapRootNode == 0 || g_mapRootNode->Collide(0, COLLISION_MASK_SMALL_OBJECTS, &from, &to, &hit) == 0)
            visible = 1;
    }
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_characterModels[index].SetPosition(self.x, self.y, self.z + RISE_PLACE_DEPTH);
        g_characterModels[index].alpha = 0;
        g_characterModels[index].mode = 0;
        g_charState[index].state = NPC_STATE_IDLE;
        return 0;
    }
    if (g_charState[index].state == NPC_STATE_IDLE)
    {
        int rest = g_charState[index].values[CHARSTATE_NPC_TALK] / RISE_TALK_BASE;
        int digit = g_charState[index].values[CHARSTATE_NPC_TALK] % RISE_TALK_BASE;
        g_characterModels[index].alpha = 0;
        if (digit == 0)
        {
            if (distance < RISE_RANGE && visible != 0)
            {
                g_charState[index].state = NPC_STATE_RISING;
                g_charState[index].values[CHARSTATE_NPC_WAIT] = 0;
                g_characterModels[index].mode = MODEL_MODE_WALKING;
                Character_PlaySound(index, RISE_VOICE, 0, 0);
                return 0;
            }
        }
        else if (digit == 1)
        {
            if (distance < RISE_RANGE_REDIRECT && visible != 0)
            {
                g_charState[index].behaviour = RISE_BEHAVIOUR_FIRST;
                g_charState[index].state = NPC_STATE_START;
                g_charState[index].values[CHARSTATE_NPC_WAIT] = 0;
                g_charState[index].values[CHARSTATE_NPC_TALK] = rest;
                g_characterModels[index].SetPosition(self.x, self.y, self.z - RISE_PLACE_DEPTH);
                return 0;
            }
        }
        else if (digit == 2 && distance < RISE_RANGE_REDIRECT && visible != 0)
        {
            g_charState[index].behaviour = RISE_BEHAVIOUR_SECOND;
            g_charState[index].state = NPC_STATE_START;
            g_charState[index].values[CHARSTATE_NPC_WAIT] = 0;
            g_charState[index].values[CHARSTATE_NPC_TALK] = rest;
            g_characterModels[index].SetPosition(self.x, self.y, self.z - RISE_PLACE_DEPTH);
            return 0;
        }
    }
    else if (g_charState[index].state == NPC_STATE_RISING)
    {
        g_charState[index].values[CHARSTATE_NPC_WAIT]++;
        g_characterModels[index].alpha = (float)g_charState[index].values[CHARSTATE_NPC_WAIT] / (float)RISE_FRAMES;
        if (g_charState[index].values[CHARSTATE_NPC_WAIT] == RISE_FRAMES)
        {
            g_characterModels[index].alpha = 1.0f;
            Char_ResetState(index);
            g_charRecords[index].defeated = 0;
        }
    }
    return 0;
}

// Char_SwingDoor tuning. CharState::values indices of a door: its swing angle in degrees, the rising flag and the frames
// it still rises (the NPC state does not use them).
const int DOOR_VALUE_ANGLE = 3;
const int DOOR_VALUE_RISING = 4;
const int DOOR_VALUE_RISE_FRAMES = 5;
// Degrees the talk id's units digit opens the door by, per unit.
const int DOOR_DEGREES_PER_UNIT = 45;
// CharState::state value while the scripts close the door, degrees it turns a frame, the angles of the half and full turn,
// and how the radians are made.
const int NPC_STATE_CLOSING = 2;
const int DOOR_CLOSE_STEP = 10;
const int DOOR_HALF_TURN = 180;
const int DOOR_FULL_TURN = 360;
// Effect shown at the door's position while it is open.
const int DOOR_EFFECT = 0x2db;
// Node matrix slot the door's root matrix is saved in.
const int DOOR_SAVED_MATRIX = 0;

// Char_SwingDoor: a door on a character slot: its model's root matrix is saved on the first frame and then turned about Y
// by the talk id's units * 45 degrees. While open (state 1) it may rise (the rising flag set: half the remaining frames in
// units a frame, counting them down) and shows effect 0x2db at its position. Closing (state 2, set by the scripts) it
// swings 10 degrees a frame towards 0 or 360 degrees; once shut (with the scripts loaded) its state is reset. Returns 0.
// Called by the character update.
int Char_SwingDoor(int index)
{
    g_characterModels[index].mode = 0;
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_charState[index].values[DOOR_VALUE_ANGLE] =
            g_charState[index].values[CHARSTATE_NPC_TALK] % 10 * DOOR_DEGREES_PER_UNIT;
        g_characterNodeMatrices[index][DOOR_SAVED_MATRIX] = g_characterModels[index].root->matrix;
        g_charState[index].state = NPC_STATE_IDLE;
    }
    else if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].root->matrix =
            g_characterNodeMatrices[index][DOOR_SAVED_MATRIX] *
            RotationY((float)g_charState[index].values[DOOR_VALUE_ANGLE] * ANGLE_HALF_TURN / (float)DOOR_HALF_TURN);
        if (g_charState[index].values[DOOR_VALUE_RISING] != 0)
            g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] +=
                (float)(g_charState[index].values[DOOR_VALUE_RISE_FRAMES]-- / 2);
        Matrix4 matrix;
        Vec4 position;
        matrix.SetIdentity();
        matrix.SetTranslation(g_characterModels[index].matrix.GetTranslation());
        g_effectManager.Start(DOOR_EFFECT, position, NPC_ALERT_EFFECT_NO_PARENT, NPC_ALERT_EFFECT_NO_PARENT, 0, 0, 0, &matrix, 0, 0);
    }
    else if (g_charState[index].state == NPC_STATE_CLOSING)
    {
        g_characterModels[index].mode = MODEL_MODE_WALKING;
        if (g_charState[index].values[DOOR_VALUE_ANGLE] > DOOR_HALF_TURN)
        {
            if (g_charState[index].values[DOOR_VALUE_ANGLE] < DOOR_FULL_TURN)
            {
                g_charState[index].values[DOOR_VALUE_ANGLE] += DOOR_CLOSE_STEP;
                if (g_charState[index].values[DOOR_VALUE_ANGLE] >= DOOR_FULL_TURN)
                    g_charState[index].values[DOOR_VALUE_ANGLE] = 0;
            }
        }
        else if (g_charState[index].values[DOOR_VALUE_ANGLE] > 0)
        {
            g_charState[index].values[DOOR_VALUE_ANGLE] -= DOOR_CLOSE_STEP;
            if (g_charState[index].values[DOOR_VALUE_ANGLE] <= 0)
                g_charState[index].values[DOOR_VALUE_ANGLE] = 0;
        }
        g_characterModels[index].root->matrix =
            g_characterNodeMatrices[index][DOOR_SAVED_MATRIX] *
            RotationY((float)g_charState[index].values[DOOR_VALUE_ANGLE] * ANGLE_HALF_TURN / (float)DOOR_HALF_TURN);
        if (g_charState[index].values[DOOR_VALUE_ANGLE] == 0 && g_gameBuiltinsRegistered != 0)
        {
            g_characterModels[index].mode = MODEL_MODE_WALKING;
            Char_ResetState(index);
        }
    }
    return 0;
}
