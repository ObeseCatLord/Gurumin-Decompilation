// A sleeping NPC (0x10045830, called from the behaviour dispatch 0x100dfcf0): it holds its sleep motion until the
// player wakes it, then looks around and turns to the player. Views the character records from their model part
// (0x10db6ee8) like char_move.cpp.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

// A model's motion playback state (model_state.cpp Playback), only the fields used here.
struct Playback
{
    char pad_0000[0x1c];
    int layer;                // 0x1c: layer playing (-1 = none)
    int frame;                // 0x20: current frame
    char pad_0024[0x7c50];
    int blending;             // 0x7c74: non-zero while blending into another layer
};

// A character record seen from its model part (char_move.cpp CharRecord), only the fields used here.
struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    char pad_0000[0x294];
    int holding;              // 0x294: 1 while the sleeper holds its sleeping frame
    Playback *motion;         // 0x298: the model's motion playback state
    char pad_029c[0x208c];
    Matrix4 matrix;           // 0x2328: world matrix
    int heldFrame;            // 0x2368: the frame held while sleeping
    char pad_236c[0x20];
    int m_238c;               // 0x238c: Character::m_239c (non-zero while the player runs)
    char pad_2390[0x28];
    Vec4 speed;               // 0x23b8: current speed vector
    char pad_23c8[0xb4];
    float heading;            // 0x247c: facing angle in radians
    float targetHeading;      // 0x2480: angle the character turns towards
    char pad_2484[0x14];
    int m_2498;               // 0x2498: set to 1 by the scene start for the player (cleared here)
};

typedef char check_charrecord[sizeof(CharRecord) == 0x249c ? 1 : -1];

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = start, 1 = sleeping, 2 = woken
    int wait;                 // 0x04: frames since waking
    int talk;                 // 0x08: talk id: hundreds = wakes when it saw the player, line = wake distance / 10
    char pad_000c[0x2c];
    int m_0038;               // 0x38: 1 once it saw the player
    int m_003c;
};

// The last position and step of each character (route_walk.cpp StepRecord).
struct LastPosition
{
    Vec4 position;
    Vec4 step;
    char pad_0020[0x20];
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" NpcState DAT_1020b430[0x80];
extern "C" int DAT_10210b30[0x80];            // non-zero = told to react
extern "C" char DAT_10255a88[0x80];
extern "C" float DAT_10255160[0x80][4];
extern "C" LastPosition DAT_10802bf8[0x80];
extern "C" unsigned char DAT_102550e0[0x80];  // non-zero = wake at once (the character's level elsewhere)
extern "C" int DAT_104c00e0;
extern "C" int DAT_104bf8f8[0x7c];            // key prompt shown (app_0008.cpp)
extern "C" int DAT_1021d138[0x7c];            // key prompt id
extern "C" int DAT_1021f740[0x7c];            // key prompt frames left
extern "C" int FUN_1000d620(int index, int key, int frames);                // KeyPrompt_Show
extern "C" int FUN_10043cc0(int index);                                     // Char_ResetState

// 0x10045830 (Npc_SleepUntilWoken). A sleeper (motion 2 held at its current frame, prompt 10 shown) that wakes
// when it saw the player and its talk id has hundreds, when the player comes within the talk id's line * 10 units
// running fast (speed over 16) or while DAT_104c00e0 is set, when told to react or when DAT_102550e0 is set: it
// shows prompt 4, looks left and right (40 degrees either side of where it faced, every 6 frames for 35 frames),
// turns to the player at frame 45 and shows prompt 9 before its state is reset. Returns 0.
extern "C" int FUN_10045830(int index)
{
    int awake = 0;
    DAT_10db6ee8[index].m_2498 = 0;
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    (player - self).Length();      // computed and not used
    float distance = (player - self).Length2D();
    DAT_10255a88[index] = 0;
    if ((DAT_1020b430[index].m_0038 != 0 && DAT_1020b430[index].talk / 100 != 0) ||
        (distance < (float)(DAT_1020b430[index].talk % 100 * 10) && DAT_10db6ee8[0].m_238c != 0 &&
         DAT_10db6ee8[0].speed.LengthSquared3() > 256.0f) ||
        (distance < (float)(DAT_1020b430[index].talk % 100 * 10) && DAT_104c00e0 != 0) ||
        DAT_10210b30[index] != 0 || DAT_102550e0[index] != 0)
    {
        awake = 1;
        DAT_10210b30[index] = 0;
    }
    if (DAT_1020b430[index].state == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 1, 0, 1.0f);
        DAT_1020b430[index].state = 1;
    }
    else if (DAT_1020b430[index].state == 1)
    {
        if (DAT_10db6ee8[index].motion->layer == 2 && DAT_10db6ee8[index].motion->blending == 0)
        {
            DAT_10db6ee8[index].heldFrame = DAT_10db6ee8[index].motion->frame;
            DAT_10db6ee8[index].holding = 1;
        }
        FUN_1000d620(index, 10, 15);
        if (awake != 0)
        {
            DAT_1020b430[index].state = 2;
            FUN_1000d620(index, 4, 30);
            DAT_10255160[index][0] = DAT_10db6ee8[index].heading;
            DAT_10db6ee8[index].holding = 0;
        }
        DAT_10802bf8[index].position.Copy(player);
    }
    else if (DAT_1020b430[index].state == 2)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if (++DAT_1020b430[index].wait < 35)
        {
            if (DAT_1020b430[index].wait % 12 == 0)
                DAT_10db6ee8[index].targetHeading = DAT_10255160[index][0] + 0.69813f;
            else if (DAT_1020b430[index].wait % 12 == 6)
                DAT_10db6ee8[index].targetHeading = DAT_10255160[index][0] - 0.69813f;
        }
        if (DAT_1020b430[index].wait == 45)
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + 3.1415f;
        if (DAT_1020b430[index].wait == 45)
        {
            DAT_10255a88[index] = 1;
            FUN_1000d620(index, 9, 60);
            FUN_10043cc0(index);
        }
    }
    if (DAT_102550e0[index] != 0)
    {
        // the key prompt helper (0x1000d620) written out for prompt 9, 60 frames
        DAT_10255a88[index] = 1;
        if (DAT_104bf8f8[index] == 0 || DAT_1021d138[index] != 9)
        {
            DAT_104bf8f8[index] = 1;
            DAT_1021d138[index] = 9;
        }
        DAT_1021f740[index] = 60;
        FUN_10043cc0(index);
    }
    return 0;
}
