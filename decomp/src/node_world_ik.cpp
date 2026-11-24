// The update pass of a model tree with inverse kinematics (IK): samples the animation plugs, runs the IK state the node
// is in, sets its world matrix, builds the skin matrices of skinned nodes and does the same for the children. The IK
// states are the flags the "ikn", "ikg", "ikcoll", "iko" and "ikp" node commands set (node_commands.cpp):
//   0x2000  goal node: the thigh of a two-bone limb. Its saved block holds the goal point (vec0) and the two bone
//           lengths (angle.x, angle.y); the knee is placed with the circle intersection and the thigh is aimed at it.
//   0x40000 the knee (set by the goal node on its first child): aimed so that its child, the ankle, points at the goal.
//   0x80000 the ankle (set by the knee): its rotation is cleared so that the limb drives it.
//   0x1000  chasing node (tail, hair): a point (vec10) follows the tip with damping and is pushed out of the collision
//           spheres linked to the node; the node is aimed at it with limited steps.
//   0x4000  collision sphere: keeps its world position and speed in its saved block for the chasing nodes.
//   0x400000 target tracking (head look): the node turns towards the target point set by EnterTargetState.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "matrix_transform.h"

#include "node_data.h"

#include "node.h"

#include "node_channel.h"

#include "circle_intersect.h"

#include "render_vars.h"

#include "frame_vars.h"

#include "anim_step_vars.h"

#define FLAG_BILLBOARD 0x1
#define FLAG_IK_NODE 0x1000
#define FLAG_IK_GOAL 0x2000
#define FLAG_IK_COLLISION 0x4000
#define FLAG_IK_MIDDLE 0x40000
#define FLAG_IK_END 0x80000
#define FLAG_IK_ORIGIN 0x200000
#define FLAG_TRACK_TARGET 0x400000
#define FLAG_IK_PLANE 0x2000000
#define IK_STATE_FLAGS 0x4c3000

#define PLUG_COLOUR_TYPE 0x4ee33
#define PLUG_TRANSFORM_TYPE 0x674d3
#define FRAME_PLUG_FIRST 6
#define FRAME_PLUG_END 8
#define TRANSFORM_PLUG_FIRST 8
#define TRANSFORM_PLUG_END 12

#define ANGLE_PI 3.1415f
#define ANGLE_TWO_PI 6.283f
#define CHASE_STEP_LIMIT 0.26179168f
#define CHASE_STEP_LIMIT_PLANE 0.11219643f
#define CHASE_MAX_RETRIES 5
#define ANGLE_ABSURD 10000.0f
#define RAMP_LIMIT 3.0f
#define RAMP_GROWTH 0.15f
#define RAMP_STEP 0.12566f
#define STEADY_STEP 0.03f
#define MIN_LENGTH 0.001
#define POSE_COUNT_LAST 15

static float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Brings an angle into -pi..pi.
static float WrapAngle(float angle)
{
    while (ANGLE_PI < angle)
        angle -= ANGLE_TWO_PI;
    while (angle < -ANGLE_PI)
        angle += ANGLE_TWO_PI;
    return angle;
}

static float ClampAngle(float angle, float lowest, float highest)
{
    float atLeast = lowest <= angle ? angle : lowest;
    if (atLeast <= highest)
        return atLeast;
    return highest;
}

// The node's local matrix at `frame` the way UpdateWorld picks it: a negative frame takes a stored pose, an animated
// node samples its extra data, a node with frame matrices takes the one for the frame (clamped to the last).
static void LocalMatrixAtFrame(Node *node, int frame, Matrix4 &out)
{
    if (frame < 0)
    {
        if (node->frameMatrices != 0)
        {
            int back = -1 - frame;
            if (back > POSE_COUNT_LAST)
                back = POSE_COUNT_LAST;
            out = node->poses[back];
        }
        else
            out = node->matrix;
    }
    else if ((node->flags & NODE_FLAG_ANIMATED_BY_EXTRA) != 0 && node->extra != 0)
        out = node->extra->AnimatedMatrix(frame);
    else if (node->frameMatrices != 0)
    {
        int index = frame;
        if (node->frameMatrixCount <= frame)
            index = node->frameMatrixCount - 1;
        out = node->frameMatrices[index];
    }
    else
        out = node->matrix;
}

// Turns `joint` (whose rest matrix is held in its saved block) so that the point its child sits at, `childLocal` in the
// joint's space, swings towards `goal`: first about y, then about z, each angle wrapped and kept inside the saved limits.
static void AimJoint(Node *joint, Matrix4 &parentMatrix, Matrix4 &childLocal, const Vec4 &goal)
{
    NodeSaved *held = joint->saved;
    Matrix4 world = held->matrix * parentMatrix;
    Vec4 toChild = (childLocal * world).GetTranslation() - world.GetTranslation();
    Vec4 toGoal = goal - world.GetTranslation();
    Matrix4 toLocal = InverseMatrix(world);
    toChild = TransformNormal(toChild, toLocal);
    toGoal = TransformNormal(toGoal, toLocal);
    float yaw = WrapAngle(Atan2(toChild.x, toChild.z) - Atan2(toGoal.x, toGoal.z));
    yaw = ClampAngle(yaw, held->limitMin.y, held->limitMax.y);
    joint->matrix = RotationY(yaw) * held->matrix;
    world = joint->matrix * parentMatrix;
    toChild = (childLocal * world).GetTranslation() - world.GetTranslation();
    toGoal = goal - world.GetTranslation();
    toLocal = InverseMatrix(world);
    toChild = TransformNormal(toChild, toLocal);
    toGoal = TransformNormal(toGoal, toLocal);
    float roll = WrapAngle(Atan2(-toChild.x, toChild.y) - Atan2(-toGoal.x, toGoal.y));
    roll = ClampAngle(roll, held->limitMin.x, held->limitMax.x);
    joint->matrix = RotationZ(roll) * RotationY(yaw) * held->matrix;
}

// Eases the tracked angle held->angle.y towards `aim`: the step grows by 0.15 per frame up to 3 (counted in angle.z while
// the angle rises, in vec0.y while it falls) and is 0.12566 times the counter.
static void EaseTrackedAngle(NodeSaved *held, float aim)
{
    if (aim <= held->angle.y)
    {
        if (held->angle.y <= aim)
        {
            held->angle.z = 0.0f;
            held->vec0.y = 0.0f;
        }
        else
        {
            held->angle.z = 0.0f;
            if (held->vec0.y < RAMP_LIMIT)
                held->vec0.y += RAMP_GROWTH;
            float next = held->angle.y - held->vec0.y * RAMP_STEP;
            if (next - aim <= 0.0f)
                held->angle.y = aim;
            else
                held->angle.y = next;
        }
    }
    else
    {
        held->vec0.y = 0.0f;
        if (held->angle.z < RAMP_LIMIT)
            held->angle.z += RAMP_GROWTH;
        float next = held->angle.z * RAMP_STEP + held->angle.y;
        if (0.0f <= next - aim)
            held->angle.y = aim;
        else
            held->angle.y = next;
    }
}

// Moves the tracked angle held->angle.x towards `aim` by at most 0.03 per frame.
static void StepTrackedAngle(NodeSaved *held, float aim)
{
    if (aim <= held->angle.x)
    {
        if (aim < held->angle.x)
        {
            double distance = fabs(aim - held->angle.x);
            double step = distance <= STEADY_STEP ? distance : STEADY_STEP;
            held->angle.x = held->angle.x - (float)step;
        }
    }
    else
    {
        double distance = fabs(aim - held->angle.x);
        double step = distance <= STEADY_STEP ? distance : STEADY_STEP;
        held->angle.x = held->angle.x + (float)step;
    }
}

// 0x10194f40 (Node::UpdateWorldWithIk). One node of the update pass; see the top of the file for the IK states.
// `parentWorld` is the parent's world matrix, `view` the view-projection matrix used to cull skinned nodes and
// `billboard` the matrix billboard nodes (flag 0x1) copy their rotation from. `skinParent` is 0 in the normal pass; the
// skin pass of a skinned node passes the matrix its chain is carried in (a Matrix4 pointer). `forceChildren` lets a
// skinned node without a bone reach its children. A node already done in this pass (hasCollision) only passes the
// call on. Returns 0.
int Node::UpdateWorldWithIk(float *parentWorld, Matrix4 *view, Matrix4 *billboard, int frame, int skinParent,
                            int forceChildren)
{
    Matrix4 plugMatrix;
    Matrix4 &parentMatrix = *(Matrix4 *)parentWorld;
    int hasPlugMatrix = 0;
    int ikLimited = 0;
    if (frameSource != 0)
        frame = *frameSource;
    if ((flags & NODE_FLAG_SKIP_SUBTREE) || (skinParent == 0 && skinned != 0))
        return 0;
    if (hasCollision != 0)
    {
        if (skinned == 0 || skinParent == 0)
        {
            for (int i = 0; i < childCount; i++)
                children[i]->UpdateWorldWithIk(worldMatrix.m, view, billboard, frame, skinParent, 0);
        }
        else
        {
            for (int i = 0; i < childCount; i++)
                children[i]->UpdateWorldWithIk(worldMatrix.m, view, billboard, frame, (int)&skin->chainMatrix, 0);
        }
        return 0;
    }
    DAT_11b002b0++;
    for (int i = FRAME_PLUG_FIRST; i < FRAME_PLUG_END; i++)
    {
        if (plugType[i] != 0)
        {
            if (DAT_11b044ec == 0)
            {
                if (plugType[i] == PLUG_COLOUR_TYPE)
                    frame = plugs[i]->keys.FrameAt(frame, 0, 0);
            }
            else
                DAT_11b044ec(this, plugType[i], plugs[i], parentWorld, view, billboard, &plugMatrix, &frame);
        }
    }
    if (extra == 0)
    {
        tint.Fill(1.0f);
        m_02bc.Fill(0.0f);
    }
    else
    {
        if (extra->m_0070[0] != 0)
            extra->tracks[NODE_TRACK_COLOUR].Sample(frame, &tint.x, &tint.y, &tint.z, &tint.w, 0);
        else
            tint.Fill(1.0f);
        if (extra->m_0070[2] != 0)
            extra->tracks[NODE_TRACK_EXTRA].Sample(frame, &m_02bc.x, &m_02bc.y, &m_02bc.z, &m_02bc.w, 0);
        else
            m_02bc.Fill(0.0f);
    }
    for (int i = TRANSFORM_PLUG_FIRST; i < TRANSFORM_PLUG_END; i++)
    {
        if (plugType[i] != 0)
        {
            if (DAT_11b044ec == 0 && plugType[i] == PLUG_TRANSFORM_TYPE)
            {
                TransformChannel *plug = (TransformChannel *)plugs[i];
                Vec4 translation;
                Vec4 rotation;
                plug->position.Sample(frame, &translation.x, &translation.y, &translation.z, &translation.w, 0);
                plug->rotation.Sample(frame, &rotation.x, &rotation.y, &rotation.z, &rotation.w, 1);
                plugMatrix = RotationMatrix(rotation);
                plugMatrix.SetTranslation(translation);
                plugMatrix = InverseMatrix(matrix) * plugMatrix;
                hasPlugMatrix = 1;
            }
            else
                DAT_11b044ec(this, plugType[i], plugs[i], parentWorld, view, billboard, &plugMatrix, &hasPlugMatrix);
        }
    }
    this->frame = frame;
    if (skinParent != 0)
    {
        if ((flags & FLAG_IK_END) && saved->state == SAVED_EMPTY)
        {
            saved->matrix = matrix;
            saved->state = SAVED_HOLDING;
            Matrix4 upright;
            upright.SetIdentity();
            upright.SetTranslation(matrix.GetTranslation());
            matrix = upright;
        }
        if ((flags & FLAG_IK_MIDDLE) && childCount != 0)
        {
            if (saved->state == SAVED_EMPTY)
            {
                saved->matrix = matrix;
                saved->state = SAVED_HOLDING;
                if ((children[0]->flags & FLAG_IK_END) && children[0]->saved->state != SAVED_EMPTY)
                {
                    children[0]->matrix = children[0]->saved->matrix;
                    children[0]->saved->state = SAVED_EMPTY;
                }
                children[0]->flags |= FLAG_IK_END;
                children[0]->AllocSaved();
            }
            else
                AimJoint(this, parentMatrix, children[0]->matrix, parent->saved->vec0);
        }
        if ((flags & FLAG_IK_GOAL) && childCount != 0 && children[0]->childCount != 0)
        {
            if (saved->state == SAVED_EMPTY)
            {
                if ((children[0]->flags & FLAG_IK_MIDDLE) && children[0]->saved->state != SAVED_EMPTY)
                {
                    children[0]->matrix = children[0]->saved->matrix;
                    children[0]->saved->state = SAVED_EMPTY;
                }
                children[0]->AllocSaved();
                children[0]->flags |= FLAG_IK_MIDDLE;
                saved->matrix = matrix;
                saved->state = SAVED_HOLDING;
                Vec4 bone;
                bone.Copy(children[0]->matrix.GetTranslation());
                saved->angle.x = bone.Length();
                bone.Copy(children[0]->children[0]->matrix.GetTranslation());
                saved->angle.y = bone.Length();
                saved->vec0.Set(100.0f, 100.0f, 100.0f, 0.0f);
            }
            else
            {
                Matrix4 hipWorld = saved->matrix * parentMatrix;
                Vec4 kneeTarget;
                if (saved->angle.y + saved->angle.x <= hipWorld.GetTranslation().Distance(saved->vec0))
                    kneeTarget.Copy(saved->vec0);
                else
                {
                    Vec4 hip = hipWorld.GetTranslation();
                    double hipCircle[3];
                    double goalCircle[3];
                    double meeting[4];
                    hipCircle[0] = hip.x;
                    hipCircle[1] = hip.y;
                    hipCircle[2] = saved->angle.x;
                    goalCircle[0] = saved->vec0.x;
                    goalCircle[1] = saved->vec0.y;
                    goalCircle[2] = saved->angle.y;
                    FUN_10194a10(hipCircle, goalCircle, 0, meeting);
                    Vec4 first;
                    Vec4 second;
                    Vec4 kneeNow = children[0]->worldMatrix.GetTranslation();
                    first.x = (float)meeting[0];
                    first.y = (float)meeting[1];
                    float spread = saved->angle.x * saved->angle.x - (first.x - hip.x) * (first.x - hip.x) -
                                   (first.y - hip.y) * (first.y - hip.y);
                    if (fabs(spread) < 0.01)
                        spread = 0.0f;
                    first.z = (float)sqrt(spread) + hip.z;
                    second.x = (float)meeting[2];
                    second.y = (float)meeting[3];
                    float otherSpread = saved->angle.x * saved->angle.x - (second.x - hip.x) * (second.x - hip.x) -
                                        (second.y - hip.y) * (second.y - hip.y);
                    if (fabs(otherSpread) < 0.01)
                        otherSpread = 0.0f;
                    second.z = (float)sqrt(otherSpread) + hip.z;
                    if (spread < 0.0f)
                        first.Copy(second);
                    else if (0.0f <= otherSpread)
                    {
                        float firstDistance = kneeNow.Distance(first);
                        if (kneeNow.Distance(second) < firstDistance)
                            first.Copy(second);
                    }
                    kneeTarget.x = first.x;
                    kneeTarget.y = first.y;
                    kneeTarget.z = first.z;
                }
                if (children[0]->saved == 0)
                    children[0]->AllocSaved();
                AimJoint(this, parentMatrix, children[0]->saved->matrix, kneeTarget);
            }
        }
    }
    if (flags & FLAG_IK_COLLISION)
    {
        Matrix4 world;
        if (saved->state == SAVED_EMPTY)
        {
            saved->state = SAVED_HOLDING;
            saved->matrix = matrix;
            saved->angle.Set(0.0f, 0.0f, 0.0f, 0.0f);
            world = matrix * parentMatrix;
            saved->vec10.Copy(world.GetTranslation());
        }
        else
        {
            world = matrix * parentMatrix;
            saved->angle.Copy(world.GetTranslation() - saved->vec10);
            saved->vec10.Copy(world.GetTranslation());
        }
        saved->vec0.Copy(world.GetTranslation());
    }
    if (skinParent != 0)
    {
        if ((flags & FLAG_IK_NODE) && childCount != 0)
        {
            if (saved->state == SAVED_EMPTY)
            {
                saved->vec10.Copy((matrix * parentMatrix).GetTranslation());
                saved->matrix = matrix;
                saved->state = SAVED_HOLDING;
                saved->vec0.x = 0.0f;
                saved->vec0.y = 0.0f;
                saved->vec0.z = 0.0f;
                saved->angle.x = 0.0f;
                saved->angle.y = 0.0f;
                saved->angle.z = 0.0f;
                saved->m_00bc = 10000;
                saved->m_00c0 = 10000;
            }
            else
            {
                Vec4 still;
                still.Fill(0.0f);
                saved->angle.Copy(saved->angle * 0.8f);
                saved->vec10.Copy(saved->vec10 + still * 2.0f);
                Matrix4 world = matrix * parentMatrix;
                int retries = 0;
                bool again;
                do
                {
                    again = false;
                    Vec4 toChild = (children[0]->matrix * world).GetTranslation() - world.GetTranslation();
                    Vec4 toGoal = saved->vec10 - world.GetTranslation();
                    Matrix4 toLocal = saved->matrix * parentMatrix;
                    toLocal = InverseMatrix(toLocal);
                    toChild = TransformNormal(toChild, toLocal);
                    toGoal = TransformNormal(toGoal, toLocal);
                    float limit = (flags & FLAG_IK_PLANE) ? CHASE_STEP_LIMIT_PLANE : CHASE_STEP_LIMIT;
                    float yawStep = ClampAngle(Atan2(toChild.x, toChild.z) - Atan2(toGoal.x, toGoal.z), -limit, limit);
                    float yaw = ClampAngle(yawStep + saved->angle.y, saved->limitMin.y, saved->limitMax.y);
                    float rollStep = ClampAngle(Atan2(-toChild.x, toChild.y) - Atan2(-toGoal.x, toGoal.y), -limit, limit);
                    float roll = ClampAngle(rollStep + saved->angle.x, saved->limitMin.x, saved->limitMax.x);
                    matrix = RotationZ(roll) * RotationY(yaw) * saved->matrix;
                    if (-ANGLE_ABSURD < roll && roll < ANGLE_ABSURD)
                        saved->angle.x = roll;
                    if (-ANGLE_ABSURD < yaw && yaw < ANGLE_ABSURD)
                        saved->angle.y = yaw;
                    if (saved->m_00b8 != 0)
                    {
                        world = matrix * parentMatrix;
                        Vec4 tip = (children[0]->matrix * world).GetTranslation();
                        for (int i = 0; i < saved->m_00b8; i++)
                        {
                            Node *sphere = saved->linkedNodes[i];
                            if (sphere->saved != 0 && sphere->saved->state != SAVED_EMPTY &&
                                tip.Distance(sphere->saved->vec0) < sphere->bounds.w * 0.5f)
                            {
                                Vec4 away;
                                away.Copy(tip - sphere->saved->vec0);
                                away.Normalized3();
                                saved->vec10.Copy(tip + away * 2.0f + saved->angle * 0.5f);
                                retries++;
                                ikLimited = 1;
                                if (retries < CHASE_MAX_RETRIES)
                                {
                                    again = true;
                                    break;
                                }
                            }
                        }
                    }
                } while (again);
            }
        }
        if (flags & FLAG_TRACK_TARGET)
        {
            if (saved == 0)
                AllocSaved();
            if (saved->state == SAVED_EMPTY)
            {
                saved->vec10.Copy((matrix * parentMatrix).GetTranslation());
                saved->matrix = matrix;
                saved->state = SAVED_HOLDING;
                saved->vec0.x = 0.0f;
                saved->vec0.y = 0.0f;
                saved->vec0.z = 0.0f;
                saved->angle.x = 0.0f;
                saved->angle.y = 0.0f;
                saved->angle.z = 0.0f;
            }
            matrix = saved->matrix;
            if (frame < 0)
            {
                int back = -1 - frame;
                if (back > POSE_COUNT_LAST)
                    back = POSE_COUNT_LAST;
                matrix = poses[back];
            }
            else if ((flags & NODE_FLAG_ANIMATED_BY_EXTRA) == 0 || extra == 0)
            {
                if (frameMatrixCount != 0)
                {
                    int index = frame;
                    if (frameMatrixCount <= frame)
                        index = frameMatrixCount - 1;
                    matrix = frameMatrices[index];
                }
            }
            else
                matrix = extra->AnimatedMatrix(frame);
            float pitch = 0.0f;
            float turn = 0.0f;
            if (saved->state == SAVED_HOLDING)
            {
                Matrix4 world = matrix * parentMatrix;
                Vec4 target;
                target.Copy(*(Vec4 *)m_0098);
                float distance = target.Distance(world.GetTranslation());
                Matrix4 ahead = TranslationMatrix(0.0f, distance, 0.0f) * world;
                Vec4 forward = ahead.GetTranslation() - world.GetTranslation();
                Vec4 toTarget = target - world.GetTranslation();
                forward.Normalized3();
                toTarget.Normalized3();
                Matrix4 toLocal = world;
                toLocal = InverseMatrix(toLocal);
                forward = TransformNormal(forward, toLocal);
                toTarget = TransformNormal(toTarget, toLocal);
                EaseTrackedAngle(saved, WrapAngle(Atan2(forward.z, forward.y) - Atan2(toTarget.z, toTarget.y)));
                pitch = ClampAngle(saved->angle.y, saved->limitMin.x, saved->limitMax.x);
                StepTrackedAngle(saved, WrapAngle(Atan2(-forward.x, forward.y) - Atan2(-toTarget.x, toTarget.y)));
                turn = ClampAngle(saved->angle.x, saved->limitMin.y, saved->limitMax.y);
            }
            else if (saved->state == SAVED_RETURNING)
            {
                EaseTrackedAngle(saved, 0.0f);
                pitch = ClampAngle(saved->angle.y, saved->limitMin.x, saved->limitMax.x);
                StepTrackedAngle(saved, 0.0f);
                turn = ClampAngle(saved->angle.x, saved->limitMin.y, saved->limitMax.y);
            }
            matrix = matrix * (RotationX(pitch) * RotationZ(turn));
        }
    }
    if (skinned == 0 || skinParent == 0)
    {
        Matrix4 local;
        if (hasPlugMatrix == 0)
        {
            if (frame < 0)
            {
                if (frameMatrices != 0)
                {
                    int back = -1 - frame;
                    if (back > POSE_COUNT_LAST)
                        back = POSE_COUNT_LAST;
                    local = poses[back];
                }
                else
                    local = matrix;
            }
            else
                LocalMatrixAtFrame(this, frame, local);
            local.translation.x += m_0298.x;
            local.translation.y += m_0298.y;
            local.translation.z += m_0298.z;
            worldMatrix = local * parentMatrix;
        }
        else
        {
            if (frame < 0)
            {
                int back = -1 - frame;
                if (back > POSE_COUNT_LAST)
                    back = POSE_COUNT_LAST;
                local = frameMatrices[back];
            }
            else
                LocalMatrixAtFrame(this, frame, local);
            local.translation.x += m_0298.x;
            local.translation.y += m_0298.y;
            local.translation.z += m_0298.z;
            worldMatrix = local * plugMatrix * parentMatrix;
        }
        hasCollision = 1;
        if (skin != 0 && skin->boneCount != 0 && DAT_10203474 != 0)
        {
            if ((flags & NODE_FLAG_NEVER_CULLED) != 0 ||
                FUN_101ae4d0(&(bounds + worldMatrix.GetTranslation()), view, cullMargin) != 0)
            {
                if (skin->bones[0] != 0)
                {
                    UpdateWorldWithIk(parentWorld, view, billboard, frame, (int)&inverseMatrix, 1);
                    for (int i = 0; i < skin->boneCount; i++)
                        skin->skinMatrices[i] = skin->links[i]->skin->skinMatrix;
                    return 0;
                }
            }
            else
                hasCollision = 0;
        }
        if ((flags & FLAG_BILLBOARD) && billboard != 0)
            worldMatrix.SetRotationTransposed(*billboard);
        if (flags & NODE_FLAG_SPECIAL_PASSES)
            worldMatrix.ResetRotation();
        for (int i = 0; i < childCount; i++)
            children[i]->UpdateWorldWithIk(worldMatrix.m, view, billboard, frame, skinParent, forceChildren);
    }
    else
    {
        Matrix4 *skinBase = (Matrix4 *)skinParent;
        int hasFrameData = ((flags & NODE_FLAG_ANIMATED_BY_EXTRA) != 0 && extra != 0) || frameMatrices != 0;
        if (hasFrameData)
        {
            Vec4 firstTranslation;
            Vec4 bindTranslation;
            if ((flags & NODE_FLAG_ANIMATED_BY_EXTRA) != 0 && extra != 0)
                firstTranslation.Copy(extra->AnimatedMatrix(0).GetTranslation());
            else
                firstTranslation.Copy(frameMatrices->GetTranslation());
            bindTranslation.Copy(skin->bindPoses[0].GetTranslation());
            float bindLength = bindTranslation.Length();
            float firstLength = firstTranslation.Length();
            float ratio;
            if (MIN_LENGTH <= firstLength || MIN_LENGTH <= bindLength)
            {
                if (MIN_LENGTH <= firstLength)
                    ratio = bindLength / firstLength;
                else
                    ratio = 0.0f;
            }
            else
                ratio = 1.0f;
            if ((flags & IK_STATE_FLAGS) != 0)
                skin->chainMatrix = matrix;
            else
            {
                if (frame < 0)
                {
                    int back = -1 - frame;
                    if (back > POSE_COUNT_LAST)
                        back = POSE_COUNT_LAST;
                    skin->chainMatrix = poses[back];
                }
                else if ((flags & NODE_FLAG_ANIMATED_BY_EXTRA) == 0 || extra == 0)
                {
                    int index = frame;
                    if (frameMatrixCount <= frame)
                        index = frameMatrixCount - 1;
                    skin->chainMatrix = frameMatrices[index];
                }
                else
                    skin->chainMatrix = extra->AnimatedMatrix(frame);
                skin->chainMatrix.translation.x += m_0298.x;
                skin->chainMatrix.translation.y += m_0298.y;
                skin->chainMatrix.translation.z += m_0298.z;
            }
            Vec4 shifted;
            shifted.Copy(skin->chainMatrix.GetTranslation());
            if (0.0f < ratio && ratio != 1.0f)
            {
                shifted.x = shifted.x * ratio;
                shifted.y = shifted.y * ratio;
                shifted.z = shifted.z * ratio;
            }
            skin->chainMatrix.SetTranslation(shifted);
            if (hasPlugMatrix != 0)
            {
                worldMatrix = skin->chainMatrix * plugMatrix * parentMatrix;
                skin->chainMatrix = skin->chainMatrix * plugMatrix * *skinBase;
            }
            else
            {
                worldMatrix = skin->chainMatrix * parentMatrix;
                skin->chainMatrix = skin->chainMatrix * *skinBase;
            }
        }
        else
        {
            Matrix4 local = matrix;
            local.translation.x += m_0298.x;
            local.translation.y += m_0298.y;
            local.translation.z += m_0298.z;
            if (hasPlugMatrix != 0)
            {
                worldMatrix = local * plugMatrix * parentMatrix;
                skin->chainMatrix = matrix * plugMatrix * *skinBase;
            }
            else
            {
                worldMatrix = local * parentMatrix;
                skin->chainMatrix = matrix * *skinBase;
            }
        }
        if (skin->isLink != 0)
        {
            if (skin->bone == 0)
                skin->skinMatrix = skin->chainMatrix;
            else
                skin->skinMatrix = skin->inverseBone * skin->chainMatrix;
        }
        if (skin->bone == 0 && forceChildren == 0)
            return 0;
        for (int i = 0; i < childCount; i++)
            children[i]->UpdateWorldWithIk(worldMatrix.m, view, billboard, frame, (int)&skin->chainMatrix, 0);
    }
    if (skinParent == 0)
    {
        if (flags & FLAG_IK_COLLISION)
        {
            saved->vec0.Copy(worldMatrix.GetTranslation() - saved->vec10);
            saved->vec10.Copy(worldMatrix.GetTranslation());
        }
    }
    else if ((flags & FLAG_IK_NODE) && ikLimited == 0)
    {
        Vec4 tip;
        tip.Copy(children[0]->worldMatrix.GetTranslation());
        if (0.0f < saved->vec10.DistanceSquared(tip))
        {
            if (flags & FLAG_IK_PLANE)
            {
                saved->vec10.Copy(saved->vec10 + (tip - saved->vec10) * 0.4f);
                saved->vec0.Copy(saved->vec0 + (tip - saved->vec10) * 0.6f);
            }
            else
            {
                saved->vec10.Copy(saved->vec10 + (tip - saved->vec10) * 0.2f);
                saved->vec0.Copy(saved->vec0 + (tip - saved->vec10) * 0.2f);
            }
        }
        if (2.0f < saved->vec0.LengthSquared3())
            saved->vec10.Copy(saved->vec10 + saved->vec0);
        if (flags & FLAG_IK_PLANE)
        {
            saved->vec0.Copy(saved->vec0 * 0.95f);
            if (!(flags & FLAG_IK_ORIGIN))
                saved->vec10.z -= 4.0f;
        }
        else
        {
            saved->vec0.Copy(saved->vec0 * 0.9f);
            if (!(flags & FLAG_IK_ORIGIN))
                saved->vec10.z -= 12.0f;
        }
    }
    return 0;
}
