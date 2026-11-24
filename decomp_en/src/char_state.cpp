// Character slot helpers (EN build): defeat marking, model reload, groups, idle motion, follower placement, effects.
// Ported from the matched JP source decomp/src/app_0010.cpp, app_0011.cpp, app_0012.cpp and char_effects.cpp.
#include "char.h"
#include "effect_manager.h"
#include "engine.h"

// Char_MarkDefeated: marks character `index` as defeated unless it is busy; returns 1 when busy. Called through the
// script command table.
extern "C" int Char_MarkDefeated(int index)
{
    if (g_charPhase[index] != 0)
        return 1;
    g_charRecords[index].defeated = 1;
    return 0;
}

// Char_ReloadModel: reloads the model of character `index` from the model path table. Called through the script
// command table.
extern "C" int Char_ReloadModel(int index)
{
    return g_characterModels[index].LoadModel(Path_Data(g_modelPaths[g_charRecords[index].params[PARAM_MODEL]].path, 0));
}

// Char_UpdateGroup: stores the formation number of the character as its group (100 means "no group", stored as -1)
// and refreshes the group links. Called through the script command table.
extern "C" int Char_UpdateGroup(int index)
{
    int value = g_charState[index].values[1];
    if (value == CHAR_FORMATION_NONE)
        g_charGroup[index] = CHAR_GROUP_NONE;
    else
        g_charGroup[index] = value;
    Char_ResetState(index);
    return 0;
}

// Char_RandomIdle: random idle animation: a 1 % chance per call to play the idle motion (speed 30) on character
// `index` when it has one. Called every frame by the idle behaviours.
extern "C" int Char_RandomIdle(int index)
{
    if (g_characterModels[index].idleMotionEnabled != 0)
    {
        if (Random() % 10000 < 100)
            g_characterModels[index].AddMotionSpeed(30.0f);
    }
    return 0;
}

// Char_FollowLeader: moves character `index` to the position of character `index - 2` (a follower placed on its
// leader). Called by the follower behaviours.
extern "C" int Char_FollowLeader(int index)
{
    g_characterModels[index].SetPosition(g_characterModels[index - 2].matrix.m[MATRIX_TRANSLATION_X],
                                         g_characterModels[index - 2].matrix.m[MATRIX_TRANSLATION_Y],
                                         g_characterModels[index - 2].matrix.m[MATRIX_TRANSLATION_Z]);
    return 0;
}

// Char_StartEffect: starts effect `effect` at `position` for character slot `index` and records its handle in that
// slot. Called by the character behaviours.
extern "C" int Char_StartEffect(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                                int value, int frame, const Matrix4 *matrix, const EffectOptions *options)
{
    g_effectManager.Start(effect, position, parent, unused, owner, value, frame, matrix, options,
                          &g_charEffectHandle[index]);
    return 0;
}

// GameOver_StartQuiet: like the game-over start without the sound: stores `value`, sets the scene timer and plays the
// defeat motion of the player. Called through the script command table.
extern "C" int GameOver_StartQuiet(int value)
{
    g_gameOverValue = value;
    g_sceneTimer = 0x816;
    g_characterModels[0].PlayMotion(0x59, 1, 5, 1.0f);
    return 0;
}
