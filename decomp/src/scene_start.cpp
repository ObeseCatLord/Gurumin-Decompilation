// Starting a scene (map entry): the player's position and velocity are saved, the world object (0x10c68000,
// world_frame.cpp) is bound to the loaded map and the player character file is loaded.

#include "font_renderer.h"
#include "node.h"
#include "map_vars.h"
#include "vec4.h"

#include "matrix4.h"

#include "effect_manager.h"

typedef int (*SceneCallback)(int code, int value);

// The world object (world_frame.cpp: WorldRenderer); only the members used here.
struct WorldRenderer
{
    char pad_0000[0x974];
    char m_0974;              // 0x974: set for costume 6
    int Bind(void *playerModel, void *mapRoot, int *state, SceneCallback callback, void *data);  // 0x1017c9c0
    int LoadCharacter(const char *path);                                                         // 0x10166950
    int Seek(int target);                                                                         // 0x10189000
};

extern "C" WorldRenderer DAT_10c68000;
extern "C" void FUN_100114a0(void);                   // Scene_ResetBattleState
extern "C" const char *FUN_1013f8a0(const char *name, int folder);
extern "C" int FUN_10092af0(int code, int value);     // scene callback
#include "anim_model.h"
#include "camera_vars.h"
#include "char_vars.h"
#include "game_vars.h"
#include "render_vars.h"
#include "sound_vars.h"

extern "C" int DAT_10db9380;

// Starts the scene with player character file `characterFile`; `value` is kept in DAT_104bfeb4.
extern "C" void FUN_100cc1d0(const char *characterFile, int value)
{
    FUN_100114a0();
    DAT_10da9150.Copy(DAT_118e69e8);
    DAT_10868818.Copy(DAT_118e6cb8);
    DAT_104bfe70 = 0;
    DAT_104bfeb4 = value;
    DAT_104bfe6c = 1;
    DAT_104bfeb0 = 1;
    DAT_104c0309 = 0;
    DAT_10c68000.Bind(DAT_10db6ee8, DAT_10255f08, &DAT_104bfe68, FUN_10092af0, &DAT_1020e538);
    DAT_10203420 = 1.0f;
    DAT_10c68000.LoadCharacter(FUN_1013f8a0(characterFile, 0));
    if (DAT_10c655f0 == 6)
        DAT_10c68000.m_0974 = 1;
    DAT_10c68000.Seek(0);
    DAT_104c00e0 = 0;
    DAT_104c0108 = 0;
    DAT_10db9380 = 1;
}

extern "C" void FUN_100123d0(void);              // Chars_ReloadModels
extern "C" int FUN_1015c9e0(int mode);           // Screen_StartTransition (0 = end the transition)
extern "C" int FUN_10001b40(void);               // SoundArc_Message221

// Finishes a pending scene start (DAT_104bfeb0) unless a cut-scene runs: reloads the character models, ends the
// screen transition, clears the effects, resets the per-object sound table (when DAT_104c01b0 is clear), fills the
// position history with the player's position and restores the player and camera vectors saved by the scene start.
extern "C" void FUN_1003a6c0(void)
{
    if (DAT_104bfeb0 != 0 && DAT_104bfe6c == 0)
    {
        FUN_100123d0();
        FUN_1015c9e0(0);
        DAT_10daaa80.Reset();
        DAT_104bfeb0 = 0;
        DAT_104c00e0 = 0;
        DAT_104c0108 = 0;
        if (DAT_104c01b0 == 0)
        {
            for (int i = 0; i < 0x44c; i++)
                DAT_10447338[i] = -11;
            if (FUN_10001b40() != 0)
                FUN_10001b40();
        }
        DAT_10db9380 = 1;
        DAT_104c0228 = 0;
        DAT_104c022c = 0;
        DAT_104c022d = 0;
        for (int i = 0; i < 16; i++)
        {
            DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
            DAT_104c022c = (DAT_104c022c + 1) & 0xf;
        }
        DAT_118e69e8.Copy(DAT_10da9150);
        DAT_118e6cb8.Copy(DAT_10868818);
        DAT_11af3e28.Copy(DAT_10da9150);
        DAT_118e6c78.Copy(DAT_10868818);
    }
}
