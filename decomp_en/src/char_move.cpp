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
const int NPC_STATE_CLOSING = 2;   // also: Enemy_DropInWhenNear is placing the enemy to drop in
const int NPC_STATE_WAITING_NEAR = 3;   // Enemy_DropInWhenNear: hidden until the player comes near
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

// Enemy_FollowAndBlast tuning. CharState::values indices: the state flag (1 = attacking), the frames to wait before the
// next attack and the frames the attack still lasts.
const int BLAST_VALUE_WAIT = 0;
const int BLAST_VALUE_ATTACKING = 1;
const int BLAST_VALUE_FRAMES = 2;
// Chance of an attack per frame (rolls up to this one, out of 100), frames between attacks and frames an attack lasts.
const int BLAST_ATTACK_ROLLS = 2;
const int BLAST_ROLL_RANGE = 100;
const int BLAST_WAIT_FRAMES = 100;
const int BLAST_ATTACK_FRAMES = 60;
// Attack frame countdown value at which the blast goes off, its sound and effect, and the effect's scale.
const int BLAST_FIRE_FRAME = 30;
const int BLAST_SOUND = 0x150;
const int BLAST_EFFECT = 0x46;
const float BLAST_EFFECT_SCALE = 1.5f;
// Motion layers: walking, idle and attack; the attack plays once.
const int BLAST_MOTION_WALK = 0;
const int BLAST_MOTION_ATTACK = 4;
// The enemy walks towards the player while farther than this, 5 units a frame (the speed is a literal below).
const float BLAST_FOLLOW_DISTANCE = 120.0f;

// Enemy_FollowAndBlast: an enemy that walks towards the player (motion 0, 5 units a frame) while farther than 120 units,
// else idles (motion 2). Now and then (3 in 100, at most every 100 frames) it turns away from the player and plays attack
// motion 4 for 60 frames: after 30 of them it plays sound 0x150 and starts effect 0x46 (scale 1.5) from its own matrix.
// Being told to react cancels the attack. Its motion playback is left in g_lastEnemyPlayback (cleared at the start).
// Returns 0. Called by the enemy update.
extern "C" int Enemy_FollowAndBlast(int index)
{
    g_lastEnemyPlayback = 0;
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (g_charToldToReact[index] != 0)
    {
        g_charToldToReact[index] = 0;
        g_charState[index].values[BLAST_VALUE_ATTACKING] = 0;
        g_charState[index].values[BLAST_VALUE_WAIT] = 0;
    }
    if (g_charState[index].values[BLAST_VALUE_ATTACKING] == 1)
    {
        g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
        if (g_charState[index].values[BLAST_VALUE_FRAMES] != 0)
            g_charState[index].values[BLAST_VALUE_FRAMES]--;
        if (g_charState[index].values[BLAST_VALUE_FRAMES] == 0)
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 0;
        if (g_charState[index].values[BLAST_VALUE_FRAMES] == BLAST_FIRE_FRAME)
        {
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = BLAST_EFFECT_SCALE;
            options.owner = index;
            options.m_000e = 1;
            if ((g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3() <
                    NPC_START_VOICE_RANGE_SQUARED ||
                index < ENEMY_SLOT_FIRST)
            {
                Vec4 soundPosition;
                soundPosition.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                                  g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                                  g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
                Sound_PlayEffect(BLAST_SOUND, &soundPosition, 1.0f);
            }
            g_effectManager.Start(BLAST_EFFECT, offset, NPC_ALERT_EFFECT_NO_PARENT, NPC_ALERT_EFFECT_NO_PARENT,
                                  0, 0, 0, &g_characterModels[index].matrix, &options, &g_charEffectHandle[index]);
        }
    }
    else
    {
        if (g_charState[index].values[BLAST_VALUE_WAIT] != 0)
            g_charState[index].values[BLAST_VALUE_WAIT]--;
        if (Random() % BLAST_ROLL_RANGE <= BLAST_ATTACK_ROLLS && g_charState[index].values[BLAST_VALUE_WAIT] == 0)
        {
            g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
            g_characterModels[index].PlayMotion(BLAST_MOTION_ATTACK, 1, NPC_IDLE_BLEND_FRAMES, 1.0f);
            g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 1;
            g_charState[index].values[BLAST_VALUE_FRAMES] = BLAST_ATTACK_FRAMES;
        }
        else if (toPlayer.Length() > BLAST_FOLLOW_DISTANCE)
        {
            g_characterModels[index].PlayMotion(BLAST_MOTION_WALK, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f);   // 5 units a frame (a literal: Vec4::operator* takes its factor by reference)
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2(toPlayer.x, toPlayer.y) + ANGLE_HALF_TURN;
        }
        else
            g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
    }
    g_lastEnemyPlayback = g_characterModels[index].playback;
    return 0;
}

// Npc_FaceAndTalk tuning. Frames a talk prompt stays up, frames before the next one, chance (out of NPC_TALK_CHANCE_RANGE)
// of a prompt per frame, the prompt shown when the NPC saw the player, and the radius in which it wakes enemies.
const int FACE_PROMPT_COOLDOWN = 120;
const int FACE_PROMPT_CHANCE = 50;
const int FACE_SAW_PLAYER_PROMPT = 15;
const float FACE_WAKE_RADIUS = 1500.0f;
// CharState::state value while the NPC turns to the player after it saw it, and the frames that takes.
const int NPC_STATE_TURNING = 2;
const int FACE_TURN_FRAMES = 30;
// Squared radians below which the NPC counts as facing the player while it turns (about one degree).
const float FACE_TURNING_LIMIT = 0.01745f;
// Motion layer played while the NPC is not facing the player.
const int FACE_MOTION_TURN = 0;

// Npc_FaceAndTalk: an NPC that keeps facing the player (motion 2 once within one degree, else motion 0) and now and then
// (when its talk line is set and no prompt is showing) shows talk prompt line - 1 for 60 frames, then waits 120 frames.
// Once it saw the player or is told to react, it shows prompt 15, plays voice 0x20c, wakes the enemies within 1500 and
// turns to the player for 30 frames before its state is reset. Returns 0. Called by the NPC update.
int Npc_FaceAndTalk(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    (player - self).Length();
    (player - self).Length2D();
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        g_charToldToReact[index] = 0;
        g_charState[index].state = NPC_STATE_IDLE;
        return 0;
    }
    if (g_charState[index].state == NPC_STATE_IDLE)
    {
        if (g_charState[index].sawPlayer != 0 || g_charToldToReact[index] != 0)
        {
            if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != FACE_SAW_PLAYER_PROMPT)
            {
                g_keyPromptShown[index] = 1;
                g_keyPromptKey[index] = FACE_SAW_PLAYER_PROMPT;
            }
            g_keyPromptFrames[index] = NPC_PROMPT_FRAMES;
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
            g_charState[index].state = NPC_STATE_TURNING;
            g_charState[index].values[CHARSTATE_NPC_WAIT] = 0;
            Enemies_WakeNearby(index, FACE_WAKE_RADIUS, 0);
        }
        g_characterModels[index].targetHeading =
            (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
        float turn = g_characterModels[index].heading - g_characterModels[index].targetHeading;
        while (turn > ANGLE_HALF_TURN)
            turn -= 6.283f;
        while (turn < -ANGLE_HALF_TURN)
            turn += 6.283f;
        float squared = turn * turn;
        if (squared < TURN_SQUARED_LIMIT)
            g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        else
            g_characterModels[index].PlayMotion(FACE_MOTION_TURN, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        if (g_charState[index].values[CHARSTATE_NPC_TALK] > 0 && g_charState[index].values[CHARSTATE_NPC_RANGE] == 0 &&
            Random() % NPC_TALK_CHANCE_RANGE < FACE_PROMPT_CHANCE)
        {
            g_charState[index].values[CHARSTATE_NPC_RANGE] = FACE_PROMPT_COOLDOWN;
            KeyPrompt_Show(index, g_charState[index].values[CHARSTATE_NPC_TALK] - 1, NPC_PROMPT_FRAMES);
        }
        if (g_charState[index].values[CHARSTATE_NPC_RANGE] > 0)
        {
            g_charToldToReact[index] = 0;
            g_charState[index].values[CHARSTATE_NPC_RANGE]--;
            return 0;
        }
    }
    else if (g_charState[index].state == NPC_STATE_TURNING)
    {
        g_characterModels[index].targetHeading =
            (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
        float turn = g_characterModels[index].heading - g_characterModels[index].targetHeading;
        g_charState[index].values[CHARSTATE_NPC_WAIT]++;
        while (turn > ANGLE_HALF_TURN)
            turn -= 6.283f;
        while (turn < -ANGLE_HALF_TURN)
            turn += 6.283f;
        if (turn < FACE_TURNING_LIMIT)
            g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        else
            g_characterModels[index].PlayMotion(FACE_MOTION_TURN, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        if (g_charState[index].values[CHARSTATE_NPC_WAIT] == FACE_TURN_FRAMES)
            Char_ResetState(index);
    }
    g_charToldToReact[index] = 0;
    return 0;
}

// Enemy_DropIn tuning. An enemy placed below z = DROP_GONE_Z has been moved away: nothing happens. It is placed this many
// units ahead of and this far below where it stands, drops in over DROP_FRAMES frames, rising 4 units a remaining frame
// (DROP_RISE_PER_FRAME) and moving back up to 16 units (DROP_BACK_DISTANCE), then moves on 1.25 times as fast.
const float DROP_GONE_Z = -20000.0f;
const float DROP_AHEAD = 200.0f;
const float DROP_BELOW = -650.0f;
const int DROP_FRAMES = 20;
const float DROP_RISE_PER_FRAME = 4.0f;
const float DROP_BACK_DISTANCE = -16.0f;
const float DROP_SPEED_UP = 1.25f;
// g_charReleaseNow value of the talk id's units digits 1 and 2, and the group timer set once placed.
const short DROP_RELEASE_NOW = 150;
const int DROP_GROUP_TIMER = 1;
// Prompt shown when the talk id has hundreds.
const int DROP_PROMPT = 15;

// Enemy_DropIn: an enemy that drops in from above: on its first frame it is refilled if defeated and placed 200 units
// ahead and 650 below where it stands (talk id's units 1 or 2 also set its release value to 150), with its group timer
// set to 1. Then it rises in an arc over 20 frames (up 4 * frames left, back 16 * (t/20)^2) and keeps moving that way
// (1.25 times as fast) until it stands on the map: then it turns to the player when the talk id has tens, shows prompt 15
// when it has hundreds, and its state is reset. Nothing happens while it is moved away. Returns 0. Called by the enemy
// update.
int Enemy_DropIn(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    g_charRespawned[index] = 0;
    g_charTalkPending[index] = 0;
    g_charHitLevel[index] = 0;
    if (g_charState[index].state != NPC_STATE_START && g_charRecords[index].stats[STAT_HEALTH] <= 0)
        g_charRespawned[index] = 1;
    if (g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] < DROP_GONE_Z)
        return 0;
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_characterModels[index].alpha = 1.0f;
        g_characterModels[index].mode = 0;
        switch (g_charState[index].values[CHARSTATE_NPC_TALK] % 10)
        {
        case 1:
            g_charReleaseNow[index] = DROP_RELEASE_NOW;
            break;
        case 2:
            g_charReleaseNow[index] = DROP_RELEASE_NOW;
            break;
        }
        if (g_charRecords[index].stats[STAT_HEALTH] <= 0)
        {
            g_charRecords[index].stats[STAT_HEALTH] = g_charRecords[index].stats[STAT_MAX_HEALTH];
            g_charPhase[index] = 0;
        }
        g_charState[index].state = NPC_STATE_IDLE;
        Vec4 place;
        place.y = DROP_AHEAD;
        place.RotateZ(-g_characterModels[index].heading);
        place.z = DROP_BELOW;
        place.Copy(place + self);
        g_characterModels[index].SetPosition(place.x, place.y, place.z);
        g_charGroupTimer[index] = DROP_GROUP_TIMER;
        return 0;
    }
    if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].childRunning = 1;
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        Vec4 step;
        int t = DROP_FRAMES;
        if (g_charState[index].values[CHARSTATE_NPC_WAIT] < DROP_FRAMES)
            t = g_charState[index].values[CHARSTATE_NPC_WAIT];
        step.y = (float)t / (float)DROP_FRAMES;
        step.y *= step.y;
        step.y *= DROP_BACK_DISTANCE;
        step.RotateZ(-g_characterModels[index].heading);
        step.z = (float)(DROP_FRAMES - g_charState[index].values[CHARSTATE_NPC_WAIT]) * DROP_RISE_PER_FRAME;
        if (++g_charState[index].values[CHARSTATE_NPC_WAIT] == DROP_FRAMES)
        {
            g_characterModels[index].mode = MODEL_MODE_WALKING;
            g_characterModels[index].idleMotionEnabled = 0;
        }
        else if (g_charState[index].values[CHARSTATE_NPC_WAIT] < DROP_FRAMES)
        {
            step.Copy(step + self);
            g_characterModels[index].SetPosition(step.x, step.y, step.z);
            g_characterModels[index].velocity.Copy(step);
            return 0;
        }
        step.Copy(step * 1.25f);   // a literal: Vec4::operator* takes its factor by reference
        Character_SetMove(index, step.x, step.y, 0);
        if (g_characterModels[index].idleMotionEnabled != 0 && g_characterModels[index].IsClearOfMapLimited(0) != 0)
        {
            g_characterModels[index].childRunning = 0;
            if (g_charState[index].values[CHARSTATE_NPC_TALK] / 10 % 10 != 0)
                g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
            if (g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE != 0)
                KeyPrompt_Show(index, DROP_PROMPT, NPC_PROMPT_FRAMES);
            g_charRespawned[index] = 1;
            g_charTalkPending[index] = 1;
            Char_ResetState(index);
        }
    }
    return 0;
}

// Enemy_ChaseAndBlast tuning. Distance beyond which the enemy runs (motion 1, 15 units a frame: written as 5 times 3 in
// the second Vec4::operator* argument), sound and effect of its blast, and the frames it then stands idle.
const float CHASE_RUN_DISTANCE = 300.0f;
const int CHASE_MOTION_RUN = 1;
const int CHASE_BLAST_SOUND = 0x14f;
const int CHASE_BLAST_EFFECT = 0x5e;
const int CHASE_IDLE_FRAMES = 30;

// Enemy_ChaseAndBlast: like Enemy_FollowAndBlast: it runs to the player (motion 1, 15 units a frame) while farther than
// 300 units, walks (motion 0, 5 units) while farther than 120, else idles. Now and then (3 in 100, at most every 100
// frames) it turns away from the player, plays sound 0x14f and starts effect 0x5e (scale 1.5) from its own matrix, then
// stands idle for 30 frames. Returns 0. Called by the enemy update.
extern "C" int Enemy_ChaseAndBlast(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (g_charState[index].values[BLAST_VALUE_ATTACKING] == 1)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        if (g_charState[index].values[BLAST_VALUE_FRAMES] != 0)
            g_charState[index].values[BLAST_VALUE_FRAMES]--;
        if (g_charState[index].values[BLAST_VALUE_FRAMES] == 0)
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 0;
    }
    else
    {
        if (g_charState[index].values[BLAST_VALUE_WAIT] != 0)
            g_charState[index].values[BLAST_VALUE_WAIT]--;
        if (Random() % BLAST_ROLL_RANGE <= BLAST_ATTACK_ROLLS && g_charState[index].values[BLAST_VALUE_WAIT] == 0)
        {
            g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = BLAST_EFFECT_SCALE;
            options.owner = index;
            options.m_000e = 1;
            if ((g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3() <
                    NPC_START_VOICE_RANGE_SQUARED ||
                index < ENEMY_SLOT_FIRST)
            {
                Vec4 soundPosition;
                soundPosition.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                                  g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                                  g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
                Sound_PlayEffect(CHASE_BLAST_SOUND, &soundPosition, 1.0f);
            }
            g_effectManager.Start(CHASE_BLAST_EFFECT, offset, NPC_ALERT_EFFECT_NO_PARENT, NPC_ALERT_EFFECT_NO_PARENT,
                                  0, 0, 0, &g_characterModels[index].matrix, &options, &g_charEffectHandle[index]);
            g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 1;
            g_charState[index].values[BLAST_VALUE_FRAMES] = CHASE_IDLE_FRAMES;
        }
        else if (toPlayer.Length() > CHASE_RUN_DISTANCE)
        {
            g_characterModels[index].PlayMotion(CHASE_MOTION_RUN, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f * 3.0f);   // 15 units a frame (literals: operator* takes its factor by reference)
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2(toPlayer.x, toPlayer.y) + ANGLE_HALF_TURN;
        }
        else if (toPlayer.Length() > BLAST_FOLLOW_DISTANCE)
        {
            g_characterModels[index].PlayMotion(BLAST_MOTION_WALK, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f);   // 5 units a frame
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2(toPlayer.x, toPlayer.y) + ANGLE_HALF_TURN;
        }
        else
            g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
    }
    return 0;
}

// Enemy_DropInWhenNear: Enemy_DropIn that first waits hidden (alpha 0, the record's defeated flag set) until the player
// comes within 800 units, then drops in the same way (without waiting to stand on the map, only for its motion to
// settle). Returns 0. Called by the enemy update.
int Enemy_DropInWhenNear(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    g_charRespawned[index] = 0;
    g_charTalkPending[index] = 0;
    g_charHitLevel[index] = 0;
    if (g_charState[index].state != NPC_STATE_START && g_charRecords[index].stats[STAT_HEALTH] <= 0)
        g_charRespawned[index] = 1;
    if (g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z] < DROP_GONE_Z)
        return 0;
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_characterModels[index].alpha = 0;
        g_charState[index].state = NPC_STATE_WAITING_NEAR;
        return 0;
    }
    if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        Vec4 step;
        int t = DROP_FRAMES;
        if (g_charState[index].values[CHARSTATE_NPC_WAIT] < DROP_FRAMES)
            t = g_charState[index].values[CHARSTATE_NPC_WAIT];
        step.y = (float)t / (float)DROP_FRAMES;
        step.y *= step.y;
        step.y *= DROP_BACK_DISTANCE;
        step.RotateZ(-g_characterModels[index].heading);
        step.z = (float)(DROP_FRAMES - g_charState[index].values[CHARSTATE_NPC_WAIT]) * DROP_RISE_PER_FRAME;
        if (++g_charState[index].values[CHARSTATE_NPC_WAIT] == DROP_FRAMES)
        {
            g_characterModels[index].mode = MODEL_MODE_WALKING;
            g_characterModels[index].idleMotionEnabled = 0;
        }
        else if (g_charState[index].values[CHARSTATE_NPC_WAIT] < DROP_FRAMES)
        {
            step.Copy(step + self);
            g_characterModels[index].SetPosition(step.x, step.y, step.z);
            g_characterModels[index].velocity.Copy(step);
            return 0;
        }
        step.Copy(step * 1.25f);   // a literal: Vec4::operator* takes its factor by reference
        Character_SetMove(index, step.x, step.y, 0);
        if (g_characterModels[index].idleMotionEnabled != 0)
        {
            if (g_charState[index].values[CHARSTATE_NPC_TALK] / 10 % 10 != 0)
                g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
            if (g_charState[index].values[CHARSTATE_NPC_TALK] / NPC_TALK_SCRIPT_SCALE != 0)
                KeyPrompt_Show(index, DROP_PROMPT, NPC_PROMPT_FRAMES);
            g_charRespawned[index] = 1;
            g_charTalkPending[index] = 1;
            Char_ResetState(index);
            return 0;
        }
    }
    else
    {
        if (g_charState[index].state == NPC_STATE_CLOSING)
        {
            g_charRecords[index].defeated = 0;
            g_characterModels[index].mode = 0;
            switch (g_charState[index].values[CHARSTATE_NPC_TALK] % 10)
            {
            case 1:
                g_charReleaseNow[index] = DROP_RELEASE_NOW;
                break;
            case 2:
                g_charReleaseNow[index] = DROP_RELEASE_NOW;
                break;
            }
            if (g_charRecords[index].stats[STAT_HEALTH] <= 0)
            {
                g_charRecords[index].stats[STAT_HEALTH] = g_charRecords[index].stats[STAT_MAX_HEALTH];
                g_charPhase[index] = 0;
            }
            g_charState[index].state = NPC_STATE_IDLE;
            Vec4 place;
            place.y = DROP_AHEAD;
            place.RotateZ(-g_characterModels[index].heading);
            place.z = DROP_BELOW;
            place.Copy(place + self);
            g_characterModels[index].SetPosition(place.x, place.y, place.z);
            g_charGroupTimer[index] = DROP_GROUP_TIMER;
            g_characterModels[index].alpha = 1.0f;
            return 0;
        }
        if (g_charState[index].state == NPC_STATE_WAITING_NEAR)
        {
            g_characterModels[index].alpha = 0;
            g_charRecords[index].defeated = 1;
            if ((player - self).Length() < RISE_RANGE)
                g_charState[index].state = NPC_STATE_CLOSING;
        }
    }
    return 0;
}

// Enemy_ShoveAndBlast tuning. CharState::values index of the attack chain (2 = a second attack follows, 1 = last attack),
// the talk value of walking, the frames an attack lasts, the attack frame countdown at which the blast goes off, its
// sound and effect, the range the enemy follows the player in, and the idle blend frames.
const int SHOVE_VALUE_CHAIN = 4;
const int SHOVE_CHAIN_MORE = 2;
const int SHOVE_CHAIN_LAST = 1;
const int SHOVE_TALK_WALKING = 2;
const int SHOVE_ATTACK_FRAMES = 35;
const int SHOVE_FIRE_FRAME = 15;
const int SHOVE_SOUND = 0x151;
const int SHOVE_EFFECT = 0x61;
const float SHOVE_FOLLOW_MAX_DISTANCE = 800.0f;
const int SHOVE_IDLE_BLEND_FRAMES = 14;

// Enemy_ShoveAndBlast: an enemy that idles (motion 2) and now and then (3 in 100, at most every 100 frames) either
// attacks or starts walking: walking it follows the player (motion 0, 5 units a frame) between 120 and 800 units away,
// else goes back to idling. An attack (motion 4 for 35 frames) keeps it turned away from the player and starts effect
// 0x61 with sound 0x151 after 20 frames; being told to react starts an attack that chains into a second one. Returns
// 0. Called by the enemy update.
extern "C" int Enemy_ShoveAndBlast(int index)
{
    Vec4 toPlayer;
    Vec4 self;
    Vec4 player;
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    g_characterModels[index].important = 1;
    toPlayer.Copy(player - self);
    if (g_charToldToReact[index] != 0)
    {
        g_charToldToReact[index] = 0;
        g_charState[index].values[BLAST_VALUE_ATTACKING] = 0;
        g_charState[index].values[BLAST_VALUE_WAIT] = 0;
        g_charState[index].values[SHOVE_VALUE_CHAIN] = SHOVE_CHAIN_MORE;
        g_characterModels[index].PlayMotion(BLAST_MOTION_ATTACK, 1, NPC_IDLE_BLEND_FRAMES, 1.0f);
        g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
        g_charState[index].values[BLAST_VALUE_ATTACKING] = 1;
        g_charState[index].values[BLAST_VALUE_FRAMES] = SHOVE_ATTACK_FRAMES;
        g_charState[index].values[SHOVE_VALUE_CHAIN] = SHOVE_CHAIN_MORE;
    }
    if (g_charState[index].values[BLAST_VALUE_ATTACKING] == 1)
    {
        if (g_charState[index].values[BLAST_VALUE_FRAMES] != 0)
            g_charState[index].values[BLAST_VALUE_FRAMES]--;
        g_characterModels[index].FaceAwayFrom(&g_characterModels[0]);
        if (g_charState[index].values[BLAST_VALUE_FRAMES] == SHOVE_FIRE_FRAME)
        {
            Vec4 offset;
            offset.Fill(0.0f);
            EffectOptions options;
            options.m_0000 = 0;
            options.colour = g_charRecords[index].stats[STAT_EFFECT_COLOUR];
            options.scale = BLAST_EFFECT_SCALE;
            options.owner = index;
            options.m_000e = 1;
            if ((g_characterModels[index].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3() <
                    NPC_START_VOICE_RANGE_SQUARED ||
                index < ENEMY_SLOT_FIRST)
            {
                Vec4 soundPosition;
                soundPosition.Set(g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X],
                                  g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Y],
                                  g_characterModels[index].matrix.m[MATRIX_TRANSLATION_Z], 0);
                Sound_PlayEffect(SHOVE_SOUND, &soundPosition, 1.0f);
            }
            g_effectManager.Start(SHOVE_EFFECT, offset, NPC_ALERT_EFFECT_NO_PARENT, NPC_ALERT_EFFECT_NO_PARENT, 0, 0, 0,
                                  &g_characterModels[index].matrix, &options, &g_charEffectHandle[index]);
        }
        if (g_charState[index].values[BLAST_VALUE_FRAMES] == 0 &&
            g_charState[index].values[SHOVE_VALUE_CHAIN] == SHOVE_CHAIN_MORE)
        {
            g_characterModels[index].PlayMotion(BLAST_MOTION_ATTACK, 1, NPC_IDLE_BLEND_FRAMES, 1.0f);
            g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 1;
            g_charState[index].values[BLAST_VALUE_FRAMES] = SHOVE_ATTACK_FRAMES;
            g_charState[index].values[SHOVE_VALUE_CHAIN] = SHOVE_CHAIN_LAST;
        }
        else if (g_charState[index].values[BLAST_VALUE_FRAMES] == 0)
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 0;
    }
    else if (g_charState[index].values[BLAST_VALUE_ATTACKING] == 0)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        if (g_charState[index].values[BLAST_VALUE_WAIT] != 0)
            g_charState[index].values[BLAST_VALUE_WAIT]--;
        g_charState[index].values[SHOVE_VALUE_CHAIN] = SHOVE_CHAIN_MORE;
        if (Random() % BLAST_ROLL_RANGE <= BLAST_ATTACK_ROLLS && g_charState[index].values[BLAST_VALUE_WAIT] == 0)
        {
            g_characterModels[index].PlayMotion(BLAST_MOTION_ATTACK, 1, NPC_IDLE_BLEND_FRAMES, 1.0f);
            g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 1;
            g_charState[index].values[BLAST_VALUE_FRAMES] = SHOVE_ATTACK_FRAMES;
            return 0;
        }
        if (Random() % BLAST_ROLL_RANGE <= BLAST_ATTACK_ROLLS && g_charState[index].values[BLAST_VALUE_WAIT] == 0)
        {
            g_charState[index].values[BLAST_VALUE_ATTACKING] = SHOVE_TALK_WALKING;
            g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
            return 0;
        }
    }
    else
    {
        if (g_charState[index].values[BLAST_VALUE_WAIT] != 0)
            g_charState[index].values[BLAST_VALUE_WAIT]--;
        g_charState[index].values[SHOVE_VALUE_CHAIN] = SHOVE_CHAIN_MORE;
        if (Random() % BLAST_ROLL_RANGE <= BLAST_ATTACK_ROLLS && g_charState[index].values[BLAST_VALUE_WAIT] == 0)
        {
            g_characterModels[index].PlayMotion(BLAST_MOTION_ATTACK, 1, NPC_IDLE_BLEND_FRAMES, 1.0f);
            g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
            g_charState[index].values[BLAST_VALUE_ATTACKING] = 1;
            g_charState[index].values[BLAST_VALUE_FRAMES] = SHOVE_ATTACK_FRAMES;
            return 0;
        }
        if (toPlayer.Length() > BLAST_FOLLOW_DISTANCE && toPlayer.Length() < SHOVE_FOLLOW_MAX_DISTANCE)
        {
            g_characterModels[index].PlayMotion(BLAST_MOTION_WALK, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
            toPlayer.Normalized3();
            toPlayer.Copy(toPlayer * 5.0f);   // 5 units a frame (a literal: operator* takes its factor by reference)
            g_characterModels[index].Push(toPlayer.x, toPlayer.y, 0);
            g_characterModels[index].targetHeading = (float)atan2(toPlayer.x, toPlayer.y) + ANGLE_HALF_TURN;
            return 0;
        }
        g_charState[index].values[BLAST_VALUE_ATTACKING] = 0;
        g_charState[index].values[BLAST_VALUE_WAIT] = BLAST_WAIT_FRAMES;
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, SHOVE_IDLE_BLEND_FRAMES, 1.0f);
    }
    return 0;
}

// Npc_RunToMarker tuning. The marker characters are the slots MARKER_FIRST up to MARKER_END; a free one has the kind
// MARKER_KIND_FREE, a claimed one MARKER_KIND_TAKEN. CharState::values index of the marker slot the NPC runs to.
const int MARKER_FIRST = 0x5c;
const int MARKER_END = 0x7c;
const char MARKER_KIND_FREE = ',';
const char MARKER_KIND_TAKEN = 'Q';
const int RUN_VALUE_MARKER = 3;
// CharState::state value while the NPC waits and runs to its marker.
const int NPC_STATE_RUNNING = 2;
// Distance reported when no marker has been looked at yet.
const float RUN_NO_DISTANCE = 99999.0f;
// Behaviour the NPC restarts with when no marker is free, and the one it takes after reaching its marker.
const int RUN_BEHAVIOUR_NO_MARKER = 1;
const int RUN_BEHAVIOUR_ARRIVED = 0x1f;
// Sight: heights above the origins, the talk line's reach per line, the view angle (radians) and the range seen at any angle.
const float RUN_SIGHT_PLAYER_HEIGHT = 50.0f;
const float RUN_SIGHT_SELF_HEIGHT = 75.0f;
const float RUN_SIGHT_ANGLE = 1.099525f;
const float RUN_SIGHT_NEAR_RANGE = 250.0f;
// Start of the run: the prompt shown (for NPC_PROMPT_FRAMES), the radius of enemies woken, frames before it runs.
const int RUN_PROMPT = 15;
const float RUN_WAKE_RADIUS = 1500.0f;
const int RUN_START_FRAMES = 10;
// Running: motion number, distance covered per frame, and distance from the marker at which it stops, placed this far short.
const int RUN_MOTION = 1;
const float RUN_STEP = 24.0f;
const float RUN_ARRIVE_DISTANCE = 150.0f;
const float RUN_STOP_SHORT = 75.0f;
// A marker moved below this x was removed from the map.
const float RUN_MARKER_GONE_X = -20000.0f;

// Npc_RunToMarker: a messenger. On its first frame it claims the nearest free marker (slots 0x5c to 0x7b of kind ','
// become 'Q'; with none its state restarts with behaviour 1). It idles (motion 2) until it sees the player or is told to
// react: then it shows prompt 15, plays voice 0x20c (when the camera is near), wakes the enemies within 1500, turns to
// the player and speeds up its motion. After 10 frames it runs (motion 1) towards the marker 24 units a frame; within
// 150 units it is placed 75 units short of the marker, frees it and starts over with behaviour 0x1f. A marker that was
// moved away resets the NPC. Returns 0. Called by the NPC update.
extern "C" int Npc_RunToMarker(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    if (g_charState[index].state == NPC_STATE_START)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        g_charState[index].state = NPC_STATE_IDLE;
        Vec4 marker;
        float nearestDistance = RUN_NO_DISTANCE;
        int nearest = -1;
        for (int i = MARKER_FIRST; i < MARKER_END; i++)
        {
            if (g_charKind[i] == MARKER_KIND_FREE)
            {
                marker.Copy(g_characterModels[i].matrix.GetTranslation());
                if (nearest == -1)
                {
                    nearest = i;
                    nearestDistance = (self - marker).Length2D();
                }
                else if ((self - marker).Length2D() < nearestDistance)
                {
                    nearest = i;
                    nearestDistance = (self - marker).Length2D();
                }
            }
        }
        if (nearest != -1)
        {
            g_charToldToReact[index] = 0;
            g_charState[index].values[RUN_VALUE_MARKER] = nearest;
            g_charKind[nearest] = MARKER_KIND_TAKEN;
            return 0;
        }
        g_charToldToReact[index] = 0;
        g_charState[index].state = NPC_STATE_START;
        g_charState[index].behaviour = RUN_BEHAVIOUR_NO_MARKER;
        return 0;
    }
    else if (g_charState[index].state == NPC_STATE_IDLE)
    {
        g_characterModels[index].PlayMotion(NPC_IDLE_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        player.z += RUN_SIGHT_PLAYER_HEIGHT;
        self.z += RUN_SIGHT_SELF_HEIGHT;
        g_charState[index].sawPlayer |= Char_CanSee(&self, &player, g_characterModels[index].heading,
                                                  (float)(g_charState[index].values[CHARSTATE_NPC_TALK] * 10),
                                                  RUN_SIGHT_ANGLE, RUN_SIGHT_NEAR_RANGE, 1);
        if (g_charState[index].sawPlayer != 0 || g_charToldToReact[index] != 0)
        {
            if (g_keyPromptShown[index] == 0 || g_keyPromptKey[index] != RUN_PROMPT)
            {
                g_keyPromptShown[index] = 1;
                g_keyPromptKey[index] = RUN_PROMPT;
            }
            g_keyPromptFrames[index] = NPC_PROMPT_FRAMES;
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
            g_charState[index].state = NPC_STATE_RUNNING;
            g_charState[index].values[CHARSTATE_NPC_WAIT] = 0;
            Enemies_WakeNearby(index, RUN_WAKE_RADIUS, 0);
            g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
            g_characterModels[index].AddMotionSpeed(NPC_TURN_SPEED);
            g_charToldToReact[index] = 0;
            return 0;
        }
    }
    else if (g_charState[index].state == NPC_STATE_RUNNING &&
             ++g_charState[index].values[CHARSTATE_NPC_WAIT] > RUN_START_FRAMES)
    {
        Vec4 marker;
        Vec4 step;
        marker.Copy(g_characterModels[g_charState[index].values[RUN_VALUE_MARKER]].matrix.GetTranslation());
        step.Copy(marker - self);
        step.Normalized3();
        step.Copy(step * 24.0f);   // RUN_STEP (a literal: operator* takes its factor by reference)
        g_characterModels[index].PlayMotion(RUN_MOTION, 0, NPC_IDLE_BLEND_FRAMES, 1.0f);
        g_characterModels[index].targetHeading = (float)atan2(step.x, step.y) + ANGLE_HALF_TURN;
        if ((self - marker).Length2D() < RUN_ARRIVE_DISTANCE)
        {
            step.Copy(self - marker);
            step.Normalized3();
            step.Copy(step * 75.0f);   // RUN_STOP_SHORT
            step.Copy(step + marker);
            g_characterModels[index].SetPosition(step.x, step.y, marker.z);
            g_charKind[g_charState[index].values[RUN_VALUE_MARKER]] = MARKER_KIND_FREE;
            g_charState[index].state = NPC_STATE_START;
            g_charState[index].values[CHARSTATE_NPC_WAIT] = 0;
            g_charState[index].values[CHARSTATE_NPC_TALK] = 0;
            g_charState[index].sawPlayer = 0;
            g_charState[index].behaviour = RUN_BEHAVIOUR_ARRIVED;
        }
        else
            Character_SetMove(index, step.x, step.y, 0);
        if (marker.x < RUN_MARKER_GONE_X)
            Char_ResetState(index);
    }
    g_charToldToReact[index] = 0;
    return 0;
}
