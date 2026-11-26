// Enemy AI updaters, one per enemy kind, called once per frame per enemy (EN build). Each skips a busy enemy, runs
// the current behaviour (CharState::state 0, 1 or 2) and marks it finished when the behaviour reports done; in the
// finished state a delay counter runs and a picker chooses the next behaviour.
#include "char.h"
#include "enemy_ai.h"
#include "enemy_flee.h"
#include "enemy_moves_long.h"
#include "npc_facing.h"

// Enemy_UpdateAI_A: kind A: wanders near its home, idles or turns to the player; picker 40/25/35. Returns 0.
extern "C" int Enemy_UpdateAI_A(int index)
{
    if (Enemy_IsBusy(index) != 0)
        return 0;
    int done;
    switch (g_charState[index].state)
    {
    case ENEMY_BEHAVIOUR_FIRST:
        done = Enemy_WanderNearHome(index, ENEMY_WANDER_RANGE);
        break;
    case ENEMY_BEHAVIOUR_SECOND:
        done = Enemy_IdleMotion(index);
        break;
    case ENEMY_BEHAVIOUR_THIRD:
        done = Enemy_TurnToPlayer(index, ENEMY_TURN_END_CHANCE);
        break;
    default:
        if (++g_charState[index].values[ENEMY_VALUE_PICK_DELAY] > ENEMY_PICK_DELAY_FRAMES)
        {
            g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
            Enemy_PickAction_40_25_35(index);
        }
        return 0;
    }
    if (done != 0)
    {
        g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
        g_charState[index].state = ENEMY_STATE_FINISHED;
    }
    return 0;
}

// Enemy_UpdateAI_C: kind C: watches the player, idles or turns to the player; single picker. Returns 0.
extern "C" int Enemy_UpdateAI_C(int index)
{
    if (Enemy_IsBusy(index) != 0)
        return 0;
    int done;
    switch (g_charState[index].state)
    {
    case ENEMY_BEHAVIOUR_FIRST:
        done = Enemy_WatchPlayer(index);
        break;
    case ENEMY_BEHAVIOUR_SECOND:
        done = Enemy_IdleMotion(index);
        break;
    case ENEMY_BEHAVIOUR_THIRD:
        done = Enemy_TurnToPlayer(index, ENEMY_TURN_END_CHANCE);
        break;
    default:
        if (++g_charState[index].values[ENEMY_VALUE_PICK_DELAY] > ENEMY_PICK_DELAY_FRAMES)
        {
            g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
            EnemyAI_PickSingle(index);
        }
        return 0;
    }
    if (done != 0)
    {
        g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
        g_charState[index].state = ENEMY_STATE_FINISHED;
    }
    return 0;
}

// Enemy_UpdateAI_D: kind D: turns to the player, keeps its distance or idles; picker 60/10/30. Returns 0.
extern "C" int Enemy_UpdateAI_D(int index)
{
    if (Enemy_IsBusy(index) != 0)
        return 0;
    int done;
    switch (g_charState[index].state)
    {
    case ENEMY_BEHAVIOUR_FIRST:
        done = Enemy_TurnToPlayer(index, ENEMY_TURN_END_CHANCE);
        break;
    case ENEMY_BEHAVIOUR_SECOND:
        done = Enemy_KeepDistanceFromPlayer(index, ENEMY_TURN_END_CHANCE);
        break;
    case ENEMY_BEHAVIOUR_THIRD:
        done = Enemy_IdleMotion(index);
        break;
    default:
        if (++g_charState[index].values[ENEMY_VALUE_PICK_DELAY] > ENEMY_PICK_DELAY_FRAMES)
        {
            g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
            Enemy_PickAction_60_10_30(index);
        }
        return 0;
    }
    if (done != 0)
    {
        g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
        g_charState[index].state = ENEMY_STATE_FINISHED;
    }
    return 0;
}

// Enemy_UpdateAI_B: kind B (EN-only): turns to the player, backs away from it or chases it; picker 60/20/20. Returns 0. Called once per frame.
extern "C" int Enemy_UpdateAI_B(int index)
{
    if (Enemy_IsBusy(index) != 0)
        return 0;
    int done;
    switch (g_charState[index].state)
    {
    case ENEMY_BEHAVIOUR_FIRST:
        done = Enemy_TurnToPlayer(index, ENEMY_TURN_END_CHANCE);
        break;
    case ENEMY_BEHAVIOUR_SECOND:
        done = Enemy_BackAwayFromPlayer(index, ENEMY_BACK_AWAY_STOP_AFTER, 0);
        break;
    case ENEMY_BEHAVIOUR_THIRD:
        done = Enemy_ChasePlayer(index, 0);
        break;
    default:
        if (++g_charState[index].values[ENEMY_VALUE_PICK_DELAY] > ENEMY_PICK_DELAY_FRAMES)
        {
            g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
            Enemy_PickAction_60_20_20(index);
        }
        return 0;
    }
    if (done != 0)
    {
        g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
        g_charState[index].state = ENEMY_STATE_FINISHED;
    }
    return 0;
}

// Enemy_UpdateAI_E: kind E (EN-only): chases the player, turns to it or backs away from it; picker 20/60/20. Returns 0. Called once per frame.
extern "C" int Enemy_UpdateAI_E(int index)
{
    if (Enemy_IsBusy(index) != 0)
        return 0;
    int done;
    switch (g_charState[index].state)
    {
    case ENEMY_BEHAVIOUR_FIRST:
        done = Enemy_ChasePlayer(index, 0);
        break;
    case ENEMY_BEHAVIOUR_SECOND:
        done = Enemy_TurnToPlayer(index, ENEMY_TURN_END_CHANCE);
        break;
    case ENEMY_BEHAVIOUR_THIRD:
        done = Enemy_BackAwayFromPlayer(index, 0, 1);
        break;
    default:
        if (++g_charState[index].values[ENEMY_VALUE_PICK_DELAY] > ENEMY_PICK_DELAY_FRAMES)
        {
            g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
            Enemy_PickAction_20_60_20(index);
        }
        return 0;
    }
    if (done != 0)
    {
        g_charState[index].values[ENEMY_VALUE_PICK_DELAY] = 0;
        g_charState[index].state = ENEMY_STATE_FINISHED;
    }
    return 0;
}
