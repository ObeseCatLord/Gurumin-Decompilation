// Starting a scene (map entry): the player's position and velocity are saved, the world object is bound to the loaded
// map and the player character file is loaded (EN build). Ported from the matched JP source decomp/src/scene_start.cpp.
#include "char.h"
#include "char_motion.h"
#include "dialogs.h"
#include "effect_manager.h"
#include "engine.h"
#include "font_renderer.h"
#include "game_vars.h"
#include "head_look.h"
#include "camera.h"
#include "node.h"
#include "scene_start.h"
#include "screen_transition.h"
#include "switches.h"
#include "texture_mgr.h"
#include "vec4.h"
#include "world_renderer.h"
#include "anim_model.h"
#include "anti_tamper.h"
#include "collide_regions.h"

// Mode of a character slot that makes it walk (the player's mode after a scene starts).
const int PLAYER_MODE_WALK = 1;
// Folder argument of Path_Data for the character files.
const int PATH_FOLDER_DEFAULT = 0;

// Starts the scene with player character file `characterFile`; `value` is kept in g_sceneStartValue. Called when a map
// is entered.
extern "C" void Scene_Start(const char *characterFile, int value)
{
    Scene_ResetBattleState();
    g_savedPlayerPosition.Copy(g_cameraPositionNow);
    g_savedPlayerVelocity.Copy(g_cameraTargetNow);
    g_movieFramesDecoded = 0;
    g_sceneStartValue = value;
    g_soundsMuted = 1;
    g_sceneStartPending = 1;
    g_sceneByte = 0;
    g_world.Bind(g_characterModels, g_mapRootNode, &g_sceneState, Scene_HostCallback, &g_textRenderer);
    g_lightScale = 1.0f;
    g_world.LoadCharacter(Path_Data(characterFile, PATH_FOLDER_DEFAULT));
    if (g_guardedValue9 == MODE_COSTUME_SIX)
        g_world.m_00974 = 1;
    g_world.Seek(0);
    g_greetingFrames = 0;
    g_characterModels[PLAYER_SLOT].mode = PLAYER_MODE_WALK;
    g_attackOverload = 0;
}

// Finishes a pending scene start (g_sceneStartPending) unless a cut-scene runs: reloads the character models, ends the
// screen transition, clears the effects, resets the looping sound table (when sound is on), fills the position history
// with the player's position and restores the player and camera vectors saved by the scene start.
extern "C" void Scene_Finish(void)
{
    if (g_sceneStartPending != 0 && g_soundsMuted == 0)
    {
        Chars_ReloadModels();
        Screen_StartTransition(0);
        g_effectManager.Reset();
        g_sceneStartPending = 0;
        g_greetingFrames = 0;
        g_attackOverload = 0;
        if (g_soundMuted == 0)
        {
            for (int i = 0; i < SOUND_LOOP_COUNT; i++)
                g_soundLoopHandle[i] = SOUND_LOOP_FREE;
            if (g_soundArcStopLoops() != 0)
                g_soundArcStopLoops();
        }
        g_characterModels[PLAYER_SLOT].mode = PLAYER_MODE_WALK;
        g_historyCount = 0;
        g_historyWriteIndex = 0;
        g_historyFlag = 0;
        for (int i = 0; i < POSITION_HISTORY_SIZE; i++)
        {
            g_playerPositionHistory[g_historyWriteIndex].Copy(g_characterModels[PLAYER_SLOT].matrix.GetTranslation());
            g_historyWriteIndex = (g_historyWriteIndex + 1) & POSITION_HISTORY_MASK;
        }
        g_cameraPositionNow.Copy(g_savedPlayerPosition);
        g_cameraTargetNow.Copy(g_savedPlayerVelocity);
        g_cameraPositionCopy.Copy(g_savedPlayerPosition);
        g_cameraTargetCopy.Copy(g_savedPlayerVelocity);
    }
}
