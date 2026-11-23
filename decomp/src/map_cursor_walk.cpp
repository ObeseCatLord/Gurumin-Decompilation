// The map cursor character (behaviour 8 of the per-kind update 0x100f9670): a character that walks the route of its
// map record (MapCursor_Step picks the node to head for), turns to face that node and keeps itself level with
// the pit area: inside it the character rises to the pit floor and the model is put on its own mode. When it has
// been hit (state 1) it faces the player, tilts a matrix towards him and starts the 0x5e0 effect at its head.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_slot.h"

#include "char_state.h"

#include "effect_manager.h"

#include "char_tables.h"
#include "char_vars.h"
#include "map_vars.h"
#include "world.h"

#include "map_cursor.h"

extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const EffectOptions *options);

// Behaviour states (CharState::state).
#define CURSOR_STATE_WALK 0
#define CURSOR_STATE_HIT 1

// Pit phases (CharState::pitPhase) and the frames a switch takes.
#define PIT_OUTSIDE 0
#define PIT_ENTERING 1
#define PIT_INSIDE 2
#define PIT_LEAVING 3
#define PIT_SWITCH_FRAMES 9

// Hit frames (CharState::hitFrames): the effect starts at 50, and in a two-player game at 30 and 70 as well; the
// state ends after 170 frames.
#define HIT_EFFECT_FRAME 0x32
#define HIT_EFFECT_FRAME_EARLY 0x1e
#define HIT_EFFECT_FRAME_LATE 0x46
#define HIT_END_FRAME 0xaa
#define HIT_EFFECT 0x5e0
#define HIT_EFFECT_SCALE 2.0f
// How far above the head node the hit effect starts.
#define HIT_EFFECT_LIFT 25.0f

// Motions of the cursor model.
#define MOTION_WALK 0
#define MOTION_HIT 2
#define MOTION_BLEND_FRAMES 5

// Rider detection around the cursor (as in MapCursor_Step) and the speed scale it gives.
#define RIDER_HEIGHT 100
#define RIDER_DISTANCE_SQUARED 90000
#define SPEED_SCALE_RIDDEN 1.75f
#define SPEED_SCALE_NORMAL 1.0f

// Distance the cursor walks per frame at speed scale 1, the rise per frame in the pit and the clearance above the
// pit floor.
#define WALK_STEP 3.0f
#define PIT_RISE 2.5f
#define PIT_CLEARANCE 10.0f
#define SINK_LIMIT_IDLE (-40)
// Half a turn: the model faces away from the heading of its walk.
#define HALF_TURN 3.1415f

// 0x10051570 (MapCursor_Walk). One frame of the cursor character `index`: marks its model locked, returns 1 at once
// while its slot is claimed (DAT_10210d30), clears a pending reaction, picks its speed scale (1.75 while a rider
// stands near), works out the distances to the player and the pit area phase (the cursor inside the pit area's box
// and under its floor sets the model mode 2, else 1), asks MapCursor_Step for the next route node and turns
// towards it. Walking (state 0) it moves along its heading unless the enemies are frozen, rising to the pit
// floor in the pit; hit (state 1) it faces the player and, at its effect frames, tilts a matrix towards him and
// starts the hit effect at its head. Returns 0.
int MapCursor_Walk(int index)
{
    int twoPlayers = 0;
    if (DAT_10c635f8 == 2)
        twoPlayers = 1;
    DAT_10db6ee8[index].locked = 1;
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].state = 0;
    }
    if (FUN_100249c0(index, 0, RIDER_HEIGHT, RIDER_DISTANCE_SQUARED) != 0)
        DAT_10db6ee8[index].speedScale = SPEED_SCALE_RIDDEN;
    else
        DAT_10db6ee8[index].speedScale = SPEED_SCALE_NORMAL;
    DAT_10218fc0[index] = 0;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    Vec4 move;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    float distance2D = (self - player).Length2D();
    float distance3D = (self - player).Length();
    float turnDelta = DAT_10db6ee8[index].heading - (float)atan2(toPlayer.x, toPlayer.y) - HALF_TURN;
    fabs((double)turnDelta);
    if (DAT_1020b430[index].lastHealth != DAT_10daaae8[index].stats[STAT_HEALTH])
        DAT_1020b430[index].lastHealth = DAT_10daaae8[index].stats[STAT_HEALTH];
    DAT_1020b430[index].pitFrames++;
    int inPit = 0;
    if (self.z < DAT_104bfe24 + PIT_CLEARANCE)
    {
        int y = (int)self.y;
        int x = (int)self.x;
        if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x &&
            y <= (int)DAT_104d29f8.y && DAT_104bfe28 != 0)
            inPit = 1;
    }
    if (inPit != 0)
    {
        int phase = DAT_1020b430[index].pitPhase;
        if (phase == PIT_OUTSIDE || phase == PIT_LEAVING)
        {
            DAT_1020b430[index].pitFrames = 0;
            DAT_1020b430[index].pitPhase = PIT_ENTERING;
        }
        else if (phase == PIT_ENTERING && DAT_1020b430[index].pitFrames > PIT_SWITCH_FRAMES)
            DAT_1020b430[index].pitPhase = PIT_INSIDE;
        DAT_10db6ee8[index].mode = 2;
        DAT_10db6ee8[index].m_238c = 1;
    }
    else
    {
        int phase = DAT_1020b430[index].pitPhase;
        if (phase == PIT_INSIDE || phase == PIT_ENTERING)
        {
            DAT_1020b430[index].pitFrames = 0;
            DAT_1020b430[index].pitPhase = PIT_LEAVING;
        }
        else if (phase == PIT_LEAVING && DAT_1020b430[index].pitFrames > PIT_SWITCH_FRAMES)
            DAT_1020b430[index].pitPhase = PIT_OUTSIDE;
        DAT_10db6ee8[index].mode = 1;
    }
    Vec4 node;
    node.Fill(0.0f);
    node.x = DAT_10db6ee8[index].matrix.translation.x;
    node.y = DAT_10db6ee8[index].matrix.translation.y;
    FUN_10051190(index, DAT_1020b430[index].cursorNode, &DAT_1020b430[index].cursorNode, &node.x, &node.y, 0, 0, 0,
                 &DAT_1020b430[index].cursorKind, &DAT_1020b430[index].cursorPrevious,
                 &DAT_1020b430[index].behaviour);
    node.Copy(node - DAT_10db6ee8[index].matrix.GetTranslation());
    DAT_10db6ee8[index].targetHeading = (float)atan2(node.x, node.y) + HALF_TURN;
    move.Fill(0.0f);
    move.y = -WALK_STEP;
    move.RotateZ(-DAT_10db6ee8[index].targetHeading);
    if (DAT_1020b430[index].state == CURSOR_STATE_WALK)
    {
        DAT_10db6ee8[index].PlayMotion(MOTION_WALK, 0, MOTION_BLEND_FRAMES, 1.0f);
        if (DAT_104bfd08 == 0)
        {
            DAT_10db6ee8[index].matrix.translation.x += move.x * DAT_10db6ee8[index].speedScale;
            DAT_10db6ee8[index].matrix.translation.y += move.y * DAT_10db6ee8[index].speedScale;
        }
        if (DAT_10db6ee8[index].mode == 2)
        {
            float height = DAT_10db6ee8[index].matrix.translation.z + PIT_RISE;
            DAT_10db6ee8[index].matrix.translation.z = height;
            float ceiling = DAT_104bfe24 - PIT_CLEARANCE;
            if (height > ceiling)
                DAT_10db6ee8[index].matrix.translation.z = ceiling;
        }
        else
            DAT_1020b430[index].sinkLimit = SINK_LIMIT_IDLE;
        DAT_10802bf8[index].position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        DAT_10802bf8[index].step.Copy(move);
        return 0;
    }
    if (DAT_1020b430[index].state != CURSOR_STATE_HIT)
        return 0;
    DAT_1020b430[index].hitFrames++;
    DAT_10db6ee8[index].PlayMotion(MOTION_HIT, 0, MOTION_BLEND_FRAMES, 1.0f);
    DAT_10db6ee8[index].targetHeading = (float)atan2(toPlayer.x, toPlayer.y) + HALF_TURN;
    if (DAT_1020b430[index].hitFrames == HIT_EFFECT_FRAME || (DAT_1020b430[index].hitFrames == HIT_EFFECT_FRAME_EARLY && twoPlayers) ||
        (DAT_1020b430[index].hitFrames == HIT_EFFECT_FRAME_LATE && twoPlayers))
    {
        Vec4 offset;
        Vec4 head;
        EffectOptions options;
        Matrix4 matrix;
        matrix.SetIdentity();
        offset.Fill(0.0f);
        head.Copy(DAT_10db6ee8[index].headNode->worldMatrix.GetTranslation());
        head.z += HIT_EFFECT_LIFT;
        if (self.z < player.z)
        {
            if (distance3D != 0.0f)
                matrix = RotationX(acosf(distance2D / distance3D)) * RotationZ(DAT_10db6ee8[index].heading);
            else
                matrix = RotationZ(DAT_10db6ee8[index].heading);
        }
        else if (distance3D != 0.0f)
            matrix = RotationX(-acosf(distance2D / distance3D)) * RotationZ(DAT_10db6ee8[index].heading);
        else
            matrix = RotationZ(DAT_10db6ee8[index].heading);
        matrix.SetTranslation(head);
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = HIT_EFFECT_SCALE;
        options.m_0000 = 0;
        FUN_1001d8b0(index, HIT_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
    }
    if (DAT_1020b430[index].hitFrames == HIT_END_FRAME)
    {
        DAT_1020b430[index].hitFrames = 0;
        DAT_1020b430[index].state = 0;
    }
    return 0;
}
