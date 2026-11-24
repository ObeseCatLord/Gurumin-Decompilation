// AnimModel::UpdateFrame: the per-frame update of an animated model (0x1017a010).

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"
#include "anim_draw_vars.h"
#include "anim_step_vars.h"
#include "frame_vars.h"
#include "render_vars.h"

#define COUNTER_LIMIT 100                // idleFrames and activeFrames stop counting here
#define SHADOW_OPACITY_MAX 15            // shadowOpacity of a placed model
#define SHADOW_OPACITY_PIT 6             // shadowOpacity the shadow settles on in shadow mode 2
#define SHADOW_MODE_FADE_OUT 1           // m_2378: the shadow fades out
#define SHADOW_MODE_FADE_IN 0            // m_2378: the shadow fades in
#define SHADOW_MODE_SETTLE 2             // m_2378: the shadow settles on SHADOW_OPACITY_PIT
#define PENDING_PUSH_LIMIT 59.0f         // PushMotionSpeed's speed when the model is not slowed down
#define OUT_OF_WORLD_X -20000.0f         // a model left of this is not updated
#define WOBBLE_PERIOD 6.0f               // engine ticks per radian of the colour wobble
#define WOBBLE_DRAW_MODE 0x24            // the root's drawMode of the wobbling model
#define WOBBLE_STEP 0.030000001f                // the small turns that spin the wobbling colours
#define WOBBLE_STEP_BIG 0.060000002f
#define FOOT_PLANTED_HEIGHT 5.5f         // a toe this close to the model's height has the foot on the ground
#define FOOT_DEBOUNCE_FRAMES 4           // frames a lifted foot stays marked before it may step again
#define FOOTSTEP_SOUND_A 1
#define FOOTSTEP_SOUND_B 1
#define FULL_TURN 6.283f                 // 2 pi as the original wrote it
#define HEADING_EASING 4.0f              // the heading closes a quarter of the gap per frame
#define SPEED_GAP_LIMIT 1.0              // a vertical speed above this resets m_237c
#define PIT_SINK_SPEED -2.0f             // verticalSpeed of a model over the pit area
#define WALK_DAMPING 0.6f                // velocity factor while m_238c is 1
#define MOVE_SHARE_SLOW 0.1f             // share of the pending move applied when m_0074 is set
#define MOVE_SHARE 0.5f
#define FORCED_SHARE_VELOCITY 0.9f       // while the forced move is on: share of velocity kept
#define FORCED_SHARE_MOVE 0.25f          // and of the forced move applied
#define BLOCKED_VELOCITY_BOOST 1.6f      // velocity factor after a blocked move when m_0074 is set
#define GRAVITY 2.8f                     // vertical speed lost per frame
#define NO_SPIN 0.0f

// 0x1017a010 (AnimModel::UpdateFrame). Runs one frame of the model: counts the frames m_238c has been set or clear,
// starts the shadow, fades it, advances the motion playback and the position and speed keys, lets pending speed
// pushes take effect, plays a footstep when a toe touches down, animates the colours of the wobbling model and
// decides whether the model is on screen under `view`. The heading eases towards targetHeading by the shortest way
// round. When the model is visible (or `force`) it then adds up its pending moves (move, velocity, the push m_228c,
// m_23c8, m_23d8), tests the move against the collisions (MoveWithCollisions), settles the vertical speed, and refreshes
// the world matrices of its node tree with `frame` as the motion frame: root->UpdateWorld, or
// root->StartProximityPass under `view` and `reference` when `withView` is set; a held model is turned about its
// hold point and a spun one about y first. Returns 1 when it ran to the end (also when the model was not
// visible), 0 when it has no node tree or lies outside the world.
int AnimModel::UpdateFrame(Matrix4 *view, int withView, int force, int frame, Matrix4 *reference)
{
    if (m_238c != 0)
    {
        activeFrames = 0;
        if (idleFrames < COUNTER_LIMIT)
            idleFrames++;
    }
    else
    {
        idleFrames = 0;
        if (activeFrames < COUNTER_LIMIT)
            activeFrames++;
    }
    if (m_2379 == 0)
    {
        m_2379 = 1;
        if (m_2378 == 0)
            shadowOpacity = SHADOW_OPACITY_MAX;
        else
            shadowOpacity = (m_2378 == 1) - 1U & 3;
    }
    if (m_2288 != 0 && --m_2288 == 0)
    {
        float zero = 0.0f;
        m_228c.Fill(zero);
    }
    if (motion != 0 && m_0294 == 0)
    {
        motionFrame = motion->frame;
        DAT_11b04508 = 1;
        motion->Advance();
        DAT_11b04508 = 0;
    }
    else if (motion != 0 && m_0294 != 0)
        motion->frame = motionFrame;
    StepPath();
    StepTrackA();
    if (m_008c < 10)
        m_008c++;
    if (m_229c > 0)
    {
        PushMotionSpeed(speedDivisor == 0 ? PENDING_PUSH_LIMIT : PENDING_PUSH_LIMIT / (float)(speedDivisor * speedDivisor));
        m_229c--;
    }
    if (verticalSpeed > SPEED_GAP_LIMIT)
        m_237c = 1.0f;
    if (matrix.translation.x < OUT_OF_WORLD_X)
        return 0;
    if (loadedRoot == 0)
        return 0;
    if (m_2378 == SHADOW_MODE_FADE_OUT && shadowOpacity != 0)
        shadowOpacity--;
    if (m_2378 == SHADOW_MODE_FADE_IN && shadowOpacity < SHADOW_OPACITY_MAX)
        shadowOpacity++;
    if (m_2378 == SHADOW_MODE_SETTLE && shadowOpacity > SHADOW_OPACITY_PIT)
        shadowOpacity--;
    if (m_2378 == SHADOW_MODE_SETTLE && shadowOpacity < SHADOW_OPACITY_PIT)
        shadowOpacity++;
    if (m_0080 != 0 && mode != 0)
    {
        for (int foot = 0; foot < 2; foot++)
        {
            if (leftToeNode != 0 && DAT_1101d854 != 0)
            {
                Vec4 modelPosition = matrix.GetTranslation();
                Vec4 toePosition;
                toePosition.Copy(toeNodes[foot]->worldMatrix.GetTranslation());
                int steps;
                if (m_238c != 0 && modelPosition.z + FOOT_PLANTED_HEIGHT > toePosition.z)
                {
                    if (footState[foot] == 0)
                        DAT_1101d854(FOOTSTEP_SOUND_A, FOOTSTEP_SOUND_B, &toePosition);
                    steps = footState[foot];
                }
                else
                {
                    steps = footState[foot];
                    if (steps == 0)
                        continue;
                    if (steps >= FOOT_DEBOUNCE_FRAMES)
                    {
                        footState[foot] = 0;
                        continue;
                    }
                }
                footState[foot] = steps + 1;
            }
        }
    }
    if (DAT_10f6558c != 0)
    {
        unsigned int phase = m_025c + DAT_10f65504;
        float time = (float)phase;
        float wave = (float)sin(time / WOBBLE_PERIOD);
        m_024c.Set(1.0f, 1.0f, 1.0f, (wave + 1.0f) / 2.0f);
        if (DAT_10f6558c != 0 && loadedRoot->drawMode == WOBBLE_DRAW_MODE)
        {
            m_00b4[0].RotateY(-WOBBLE_STEP_BIG);
            m_00b4[0].RotateX(-WOBBLE_STEP);
            m_00b4[0].RotateZ(WOBBLE_STEP);
            m_00b4[2].RotateZ(-WOBBLE_STEP_BIG);
            m_00b4[2].RotateX(-WOBBLE_STEP);
            m_00b4[2].RotateY(WOBBLE_STEP);
            m_00b4[4].RotateY(-WOBBLE_STEP_BIG);
            m_00b4[4].RotateX(WOBBLE_STEP);
            m_00b4[4].RotateZ(WOBBLE_STEP);
            m_00b4[6].RotateZ(-WOBBLE_STEP);
            m_00b4[6].RotateX(-WOBBLE_STEP_BIG);
            m_00b4[6].RotateY(WOBBLE_STEP);
            m_00b4[8].RotateY(-WOBBLE_STEP);
            m_00b4[8].RotateZ(-WOBBLE_STEP);
            m_00b4[8].RotateX(-WOBBLE_STEP);
        }
    }
    DAT_10203474 = withView;
    int visible = 0;
    if (m_2394 != 0 || (loadedRoot->flags & NODE_FLAG_NEVER_CULLED) != 0 ||
        FUN_101ae4d0(&(loadedRoot->bounds + loadedRoot->worldMatrix.GetTranslation()), view, loadedRoot->cullMargin) != 0)
        visible = 1;
    visible |= force;
    updateCount++;
    m_2394 = 0;
    if (targetHeading != heading)
    {
        while (targetHeading < 0.0f)
            targetHeading += FULL_TURN;
        while (heading < 0.0f)
            heading += FULL_TURN;
        while (targetHeading > FULL_TURN)
            targetHeading -= FULL_TURN;
        while (heading > FULL_TURN)
            heading -= FULL_TURN;
        float nearest;
        if (fabs((targetHeading + FULL_TURN) - heading) > fabs((targetHeading - FULL_TURN) - heading))
        {
            nearest = targetHeading;
            if (!(fabs((targetHeading - FULL_TURN) - heading) > fabs(targetHeading - heading)))
                nearest = targetHeading - FULL_TURN;
        }
        else
        {
            nearest = targetHeading;
            if (!(fabs((targetHeading + FULL_TURN) - heading) > fabs(targetHeading - heading)))
                nearest = targetHeading + FULL_TURN;
        }
        heading = (nearest - heading) / HEADING_EASING + heading;
    }
    if (loadedRoot != 0 && visible != 0)
    {
        Vec4 oldPosition;
        oldPosition.Copy(matrix.GetTranslation());
        Vec4 impulse;
        if (m_238c == 1)
            velocity.Copy(velocity * WALK_DAMPING);
        if (mode == 0)
            verticalSpeed = 0;
        if (mode == 2 && DAT_10203430 != 0)
            verticalSpeed = PIT_SINK_SPEED;
        float stepX;
        float stepY;
        float stepZ;
        if (m_23d8.x == 0.0f && m_23d8.y == 0.0f && m_23d8.z == 0.0f)
        {
            float moveShareY;
            if (m_0074 != 0)
            {
                stepX = move.x * MOVE_SHARE_SLOW + velocity.x + m_228c.x + m_23c8.x + m_23d8.x;
                moveShareY = move.y * MOVE_SHARE_SLOW;
            }
            else
            {
                stepX = move.x * MOVE_SHARE + velocity.x + m_228c.x + m_23c8.x + m_23d8.x;
                moveShareY = move.y * MOVE_SHARE;
            }
            stepY = moveShareY + velocity.y + m_228c.y + m_23c8.y + m_23d8.y;
        }
        else
        {
            impulse.x = 0;
            impulse.y = 0;
            impulse.z = 0;
            move.x = 0;
            move.y = 0;
            move.z = 0;
            m_23c8.x = 0;
            m_23c8.y = 0;
            m_23c8.z = 0;
            verticalSpeed = 0;
            if (m_0074 != 0)
            {
                stepX = velocity.x * FORCED_SHARE_VELOCITY + m_228c.x + m_23d8.x * FORCED_SHARE_MOVE;
                stepY = velocity.y * FORCED_SHARE_VELOCITY + m_228c.y + m_23d8.y * FORCED_SHARE_MOVE;
            }
            else
            {
                stepX = m_228c.x + m_23d8.x;
                stepY = m_228c.y + m_23d8.y;
            }
        }
        stepZ = move.z * MOVE_SHARE + m_228c.z + verticalSpeed + m_23d8.z + m_23c8.z;
        if (m_2384 != 0 && MoveWithCollisions(stepX, stepY, stepZ, 1, 0) != 0)
        {
            if (m_0074 != 0)
            {
                velocity.x = velocity.x * BLOCKED_VELOCITY_BOOST;
                velocity.y = velocity.y * BLOCKED_VELOCITY_BOOST;
            }
        }
        else
        {
            velocity.x = 0;
            velocity.y = 0;
            velocity.z = 0;
            verticalSpeed = 0;
        }
        if (root == 0)
            return 0;
        m_23c8.x = 0;
        m_23c8.y = 0;
        m_23c8.z = 0;
        move.x = 0;
        move.y = 0;
        move.z = 0;
        if (m_238c == 1)
            verticalSpeed = 0;
        else
        {
            m_238c = 0;
            float fall = verticalSpeed - GRAVITY;
            verticalSpeed = fall;
            if (fall < 0.0f)
                verticalSpeed = 0.0f;
        }
        m_2370 = oldPosition.z;
        if (m_0076 == 0)
            matrix.SetRotationZ(heading);
        if (followNode != 0)
        {
            motionFrame = followNode->frame;
            matrix = followNode->worldMatrix;
        }
        if (m_0266 != 0)
            motionFrame = frame;
        DAT_1101d858++;
        if (shadowHidden == 0)
        {
            if (spinAngle != NO_SPIN)
            {
                Matrix4 spun;
                spun = RotationY(spinAngle) * matrix;
                if (withView != 0)
                    root->StartProximityPass(spun.m, view, 0, motionFrame, 0);
                else
                    root->UpdateWorld(spun, motionFrame);
            }
            else if (holdState != HOLD_FREE)
            {
                Matrix4 held;
                while (holdAngle < 0.0f)
                    holdAngle += FULL_TURN;
                while (holdAngle > FULL_TURN)
                    holdAngle -= FULL_TURN;
                float turned = holdAngle + holdWeight;
                holdAngle = turned;
                if (holdState == HOLD_RELEASED && (turned >= FULL_TURN || turned <= 0.0f))
                {
                    holdAngle = 0;
                    holdState = HOLD_FREE;
                    holdWeight = 0;
                }
                held = TranslationMatrix(-holdOffset.x, -holdOffset.y, -holdOffset.z) * RotationX(holdAngle) *
                       TranslationMatrix(holdOffset.x, holdOffset.y, holdOffset.z) * matrix;
                if (withView != 0)
                    root->StartProximityPass(held.m, view, reference, motionFrame, 0);
                else
                    root->UpdateWorld(held, motionFrame);
            }
            else if (withView != 0)
                root->StartProximityPass(matrix.m, view, reference, motionFrame, 0);
            else
                root->UpdateWorld(matrix, motionFrame);
            m_2384 = 1;
        }
        if (m_23d8.x != 0.0f || m_23d8.y != 0.0f || m_23d8.z != 0.0f)
        {
            move.x = 0;
            move.y = 0;
            move.z = 0;
            m_23c8.x = 0;
            m_23c8.y = 0;
            m_23c8.z = 0;
            m_23d8.x = 0;
            m_23d8.y = 0;
            m_23d8.z = 0;
            verticalSpeed = 0;
        }
    }
    DAT_10203474 = 1;
    return 1;
}
