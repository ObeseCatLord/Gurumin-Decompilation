// Character motion files, model reload and pushes (EN build). JP twins: char_motion.cpp, char_turn.cpp.
#include "char_motion.h"
#include "engine.h"

// Data folder number of motion and model files.
const int FOLDER_DATA = 0;

// Loads motion file `path` (folder 0) on character `index` and plays frames start..end; `once` = 0 loops. Returns 0.
// Called through the script command table.
extern "C" int Char_PlayMotionFile(int index, const char *path, int start, int end, int once)
{
    g_characterModels[index].LoadModel(Path_Data(path, FOLDER_DATA));
    g_characterModels[index].ClearMotionKeys();
    g_characterModels[index].AddMotionKey(0, (float)start);
    g_characterModels[index].AddMotionKey(end - start, (float)end);
    g_characterModels[index].SetMotionOptions(1, once == 0);
    return 0;
}

// Reloads the model of every character slot 0..0x5b that is still in the world (slot 0, the player, always) and whose
// slot is not waiting on the previous slot, then resets its motion with Model_ApplyCommands(i, -1). Called after a
// scene start.
extern "C" void Chars_ReloadModels(void)
{
    for (int i = 0; i < RELOAD_SLOT_COUNT; i++)
    {
        if (i != 0 && g_characterModels[i].matrix.m[MATRIX_TRANSLATION_X] < CHAR_OFFWORLD_X)
            continue;
        if (g_charRecords[i].sharesModel != 0 && g_charRecords[i - 1].m_00a8 == 0)
            continue;
        g_characterModels[i].LoadModel(Path_Data(g_modelPaths[g_charRecords[i].params[PARAM_MODEL]].path, FOLDER_DATA));
        Model_ApplyCommands(i, MODEL_COMMANDS_RESET);
    }
}

// Pushes character `index` away from `point` with `strength` (in the x/y plane of the direction, z = 0), passing
// `flag` on with the push. Character 0 is not pushed while the scene timer is set, and nothing happens when the
// character stands exactly on the point. Returns 0. Called by hit and explosion effects.
extern "C" int Char_PushAwayFrom(int strength, char flag, int index, const Vec4 *point)
{
    Vec4 direction = g_characterModels[index].matrix.GetTranslation();
    Vec4 from;
    from.Copy(*point);
    direction.Copy(direction - from);
    direction.Normalized3();
    if ((index != 0 || g_sceneTimer == 0) && (direction.x != 0.0f || direction.y != 0.0f || direction.z != 0.0f))
    {
        direction.Copy(direction * (float)strength);
        g_characterModels[index].SetPendingPush(direction.x, direction.y, 0, flag);
    }
    return 0;
}

// Pushes character `index` away from character `otherIndex` (or from `point` when it is given) with the fixed strength
// PUSH_STRENGTH_FROM_CHARACTER, passing `flag` on with the push. Character 0 is not pushed while the scene timer is set,
// and nothing happens when the two stand exactly on the same spot. Returns 0. Called by the script command that pushes a
// character away and by the scene code (EN 0x5e30b0, 0x5c3430).
extern "C" int Char_PushAwayFromCharacter(char flag, int index, int otherIndex, const Vec4 *point)
{
    if (index == 0 && otherIndex == 0)
        return 0;
    Vec4 direction = g_characterModels[index].matrix.GetTranslation();
    Vec4 from = g_characterModels[otherIndex].matrix.GetTranslation();
    if (point != 0)
        from.Copy(*point);
    if (index != 0 || g_sceneTimer == 0)
    {
        direction.Copy(direction - from);
        if (direction.x != 0.0f || direction.y != 0.0f || direction.z != 0.0f)
        {
            direction.Normalized3();
            direction.Copy(direction * (float)PUSH_STRENGTH_FROM_CHARACTER);
            g_characterModels[index].SetPendingPush(direction.x, direction.y, 0, flag);
        }
    }
    return 0;
}
