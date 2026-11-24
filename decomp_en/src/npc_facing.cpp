// NPC idle and talk facing, and the enemy watch behaviour (EN build). JP twins: npc_facing.cpp, enemy_moves.cpp.
#include <math.h>

#include "engine.h"
#include "npc_facing.h"

// Idle in motion 2 facing the configured direction; talking in motion 3 turned to the player (unless the facing value
// is 100 or more). Like Npc_TalkFacing with talk motion 3. Returns 0. Called every frame for NPCs.
extern "C" int Npc_TalkFacingSimple(int index)
{
    if (g_charState[index].values[NPC_VALUE_IDLE_STARTED] == 0)
    {
        g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 1, NPC_MOTION_BLEND, 1.0f);
        g_charState[index].values[NPC_VALUE_IDLE_STARTED] = 1;
    }
    int state = g_charState[index].state;
    g_characterModels[index].mode = 0;
    if (state == 0)
        g_characterModels[index].targetHeading =
            (float)(g_charState[index].values[NPC_VALUE_FACING] % NPC_FACING_RANGE) * NPC_FACING_DEGREES_PER_UNIT * NPC_TURN_HALF /
            NPC_FACING_DEGREES;
    else if (state == 1)
    {
        g_characterModels[index].PlayMotion(NPC_MOTION_TALK, 0, NPC_MOTION_BLEND, 1.0f);
        if (g_charState[index].values[NPC_VALUE_FACING] / NPC_FACING_RANGE == 0)
        {
            Vec4 player;
            Vec4 self;
            player.Copy(g_characterModels[0].matrix.GetTranslation());
            self.Copy(g_characterModels[index].matrix.GetTranslation());
            g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + NPC_TURN_HALF;
        }
    }
    if (g_talkingNpc == index)
    {
        if (g_charState[index].state == 0)
        {
            g_charState[index].state = 1;
            g_characterModels[index].PlayMotion(NPC_MOTION_TALK, 1, NPC_MOTION_BLEND, 1.0f);
            return 0;
        }
    }
    else if (g_charState[index].state == 1)
    {
        g_charState[index].state = 0;
        g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 1, NPC_MOTION_BLEND, 1.0f);
    }
    return 0;
}

// Keeps NPC `index` in idle motion 2. Idle, it faces its configured direction; while talking it keeps motion 2 going
// and, unless its facing value is 100 or more, turns to the player. The state follows g_talkingNpc (talking while
// this NPC is the one being talked to), restarting the motion on each change. Returns 0. Called every frame for NPCs.
extern "C" int Npc_TalkFacing(int index)
{
    if (g_charState[index].values[NPC_VALUE_IDLE_STARTED] == 0)
    {
        g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 1, NPC_MOTION_BLEND, 1.0f);
        g_charState[index].values[NPC_VALUE_IDLE_STARTED] = 1;
    }
    int state = g_charState[index].state;
    g_characterModels[index].mode = 0;
    if (state == 0)
        g_characterModels[index].targetHeading =
            (float)(g_charState[index].values[NPC_VALUE_FACING] % NPC_FACING_RANGE) * NPC_FACING_DEGREES_PER_UNIT * NPC_TURN_HALF /
            NPC_FACING_DEGREES;
    else if (state == 1)
    {
        g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 0, NPC_MOTION_BLEND, 1.0f);
        int current = g_characterModels[index].playback->layer;
        int next = g_characterModels[index].playback->nextLayer;
        if (current != NPC_MOTION_IDLE && current != NPC_MOTION_TALK && next != NPC_MOTION_IDLE && next != NPC_MOTION_TALK)
        {
            g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 1, NPC_MOTION_BLEND, 1.0f);
            if (g_characterModels[index].playback->nextLayer != NPC_MOTION_IDLE)
                g_characterModels[index].PlayMotion(NPC_MOTION_WALK, 1, NPC_MOTION_BLEND, 1.0f);
        }
        if (g_charState[index].values[NPC_VALUE_FACING] / NPC_FACING_RANGE == 0)
        {
            Vec4 player;
            Vec4 self;
            player.Copy(g_characterModels[0].matrix.GetTranslation());
            self.Copy(g_characterModels[index].matrix.GetTranslation());
            g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + NPC_TURN_HALF;
        }
    }
    if (g_talkingNpc == index)
    {
        if (g_charState[index].state == 0)
        {
            g_charState[index].state = 1;
            g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 1, NPC_MOTION_BLEND, 1.0f);
        }
    }
    else if (g_charState[index].state == 1)
    {
        g_charState[index].state = 0;
        g_characterModels[index].PlayMotion(NPC_MOTION_IDLE, 1, NPC_MOTION_BLEND, 1.0f);
    }
    return 0;
}

// Enemy_WatchPlayer: starts motion 7 (unless it is already playing or being blended into) and then keeps the enemy
// turned towards the player until a 15-in-3000 chance per frame ends the behaviour. Returns 1 on the frame it ends,
// else 0. Called every frame by enemy behaviours.
extern "C" int Enemy_WatchPlayer(int index)
{
    Vec4 move;
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    g_charState[index].values[WATCH_VALUE_ACTIVE] = 1;
    move.Fill(0);
    if (g_charState[index].values[WATCH_VALUE_PHASE] == 0)
    {
        Playback *motion = g_characterModels[index].playback;
        g_charState[index].values[WATCH_VALUE_FRAMES] = WATCH_START_FRAMES;
        if (motion->layer != WATCH_MOTION && (motion->blending == 0 || motion->nextLayer != WATCH_MOTION))
            g_characterModels[index].PlayMotion(WATCH_MOTION, 1, NPC_MOTION_BLEND, 1.0f);
        g_charState[index].values[WATCH_VALUE_PHASE] = 1;
    }
    else if (g_charState[index].values[WATCH_VALUE_PHASE] == 1)
    {
        g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + NPC_TURN_HALF;
        if (Random() % WATCH_END_RANGE < WATCH_END_BELOW)
        {
            g_charState[index].values[WATCH_VALUE_PHASE] = 0;
            return 1;
        }
    }
    return 0;
}
