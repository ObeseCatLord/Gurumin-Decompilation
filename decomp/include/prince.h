// The prince (prince_update.cpp), a per-kind behaviour (kind 0x43) of 0x100f9670: a fighter with a swappable weapon
// and a gatling gun, with an ending scene when the game is over.
#ifndef GURUMIN_PRINCE_H
#define GURUMIN_PRINCE_H

#include "vec4.h"
#include "anim_model.h"
#include "char_slot.h"
#include "char_state.h"

// CharSlot::kind of the prince (it shares its voice cooldowns with the other boss kinds, see bs005.h).
#define PRINCE_KIND 0x43

// The state one frame of the prince works on (prince_update.cpp): the positions, the distances to the player, the motion
// the state asks for and the movement vector applied at the end of the frame.
struct PrinceFrame
{
    int index;
    AnimModel *model;
    CharSlot *slot;
    Kind43State *st;
    Vec4 playerPos;                              // position of character 0
    Vec4 ownPos;
    Vec4 move;                                   // the movement vector blended at the end of the frame
    float distance2D;                            // from the player, ignoring height
    float distance3D;
    int layer;                                   // layer the motion plays (Playback::layer)
    int nextLayer;                               // layer being blended into
    int moveMode;                                // PRINCE_MOVE_*
    int motion;                                  // motion the state wants (-1 = keep)
    int motionLoop;                              // second argument of PlayMotion
    int motionBlend;                             // third argument of PlayMotion
};


// 0x100edea0 (Prince_Update; C++ linkage: the original has an EH frame for its matrices and effect options). One
// frame of the prince in slot `index`. Returns 1 while the ending scene runs, else 0.
int Prince_Update(int index);

#endif
