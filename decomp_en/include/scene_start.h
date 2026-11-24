// Starting and finishing a scene (map entry) in the EN build (JP: decomp/include/render_vars.h, game_vars.h,
// char_vars.h scene globals). This is the one declaration of these scene globals and functions.
#ifndef GURUMIN_EN_SCENE_START_H
#define GURUMIN_EN_SCENE_START_H

#include "vec4.h"

// Entries of the ring of recent player positions, and its index mask.
const int POSITION_HISTORY_SIZE = 16;
const int POSITION_HISTORY_MASK = POSITION_HISTORY_SIZE - 1;
// Value of the game mode (g_guardedValue9) of the costume that needs WorldRenderer::m_00974.
const int MODE_COSTUME_SIX = 6;

extern "C" Vec4 g_savedPlayerPosition;       // EN 0x0147b248 (JP DAT_10da9150): player position saved when a scene starts
extern "C" Vec4 g_savedPlayerVelocity;       // EN 0x00f445e0 (JP DAT_10868818): player velocity saved when a scene starts
extern "C" int g_movieFramesDecoded;         // EN 0x0091f614 (JP DAT_104bfe70): frames of the movie decoded so far
extern "C" int g_sceneState;                 // EN 0x0091f530 (JP DAT_104bfe68): state value handed to the world binding
extern "C" int g_sceneStartValue;            // EN 0x00920464 (JP DAT_104bfeb4): value given to Scene_Start
extern "C" int g_sceneStartPending;          // EN 0x00920460 (JP DAT_104bfeb0): non-zero from Scene_Start until Scene_Finish
extern "C" char g_sceneByte;                 // EN 0x00b91ea8 (JP DAT_104c0309): cleared by Scene_Start
extern "C" int g_greetingFrames;             // EN 0x009e33ac (JP DAT_104c00e0): non-zero = greetings are on
extern "C" int g_attackOverload;             // EN 0x009e33d4 (JP DAT_104c0108): attack overload counter
extern "C" int g_historyCount;               // EN 0x00a207bc (JP DAT_104c0228): cleared by Scene_Finish
extern "C" unsigned char g_historyWriteIndex;  // EN 0x00a1d9e6 (JP DAT_104c022c): write index of the player position history
extern "C" unsigned char g_historyFlag;      // EN 0x00a1d9e7 (JP DAT_104c022d): cleared by Scene_Finish
extern "C" Vec4 g_playerPositionHistory[POSITION_HISTORY_SIZE];   // EN 0x00da7a00 (JP DAT_10802ad8): recent player positions

// Battle and HUD state that Scene_ResetBattleState clears (their uses are not recovered yet, except where noted).
extern "C" int g_battleStateBc8860;          // EN 0x00bc8860 (JP DAT_10468128): tested together with the model's m_229c by the frame code
extern "C" char g_battleStateA207c7;         // EN 0x00a207c7 (JP DAT_104c023e): attack step counter (set to 1 by the attack start)
extern "C" int g_battleStateA150b4;          // EN 0x00a150b4 (JP DAT_104c0194)
extern "C" int g_battleStateA14f7c;          // EN 0x00a14f7c (JP DAT_104c0168)
extern "C" int g_battleStateA14f80;          // EN 0x00a14f80 (JP DAT_104c016c)
extern "C" int g_battleStateA1513c;          // EN 0x00a1513c (JP DAT_104c01a4): set to 1 by the frame code, read by the knockback code
extern "C" char g_battleStateA14f87;         // EN 0x00a14f87 (JP DAT_104c0190)
// g_attackOverload value from which the scene reset triggers the white fade of the combo finish.
const int COMBO_FINISH_OVERLOAD = 0x45;
// Frames of that fade.
const int COMBO_FINISH_FADE_FRAMES = 5;
extern "C" int Scene_ResetBattleState(void);   // EN 0x63f560 (JP 0x100114a0)
extern "C" int Scene_HostCallback(int code, int value);   // EN 0x5c8130 (JP 0x10092af0): the world's host callback

// Starts the scene with player character file `characterFile`; `value` is kept in g_sceneStartValue. EN 0x63e760.
extern "C" void Scene_Start(const char *characterFile, int value);
// Finishes a pending scene start unless a cut-scene runs. EN 0x5a8f90.
extern "C" void Scene_Finish(void);

#endif
