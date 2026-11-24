// AnimModel parameter setters (EN build): heading, speed, push, hold and snapping helpers. Ported from the matched JP
// source decomp/src/model_params.cpp.
#include "anim_model.h"
#include "node.h"
#include <math.h>
#include <stdlib.h>

// AnimModel::SnapHeadingToSide: wraps the heading into 0..2pi and snaps it to the nearer side view: pi/2 when it points
// into the first half turn, 3pi/2 otherwise. Returns 0.
int AnimModel::SnapHeadingToSide()
{
    while (targetHeading > ANGLE_FULL_TURN)
        targetHeading -= ANGLE_FULL_TURN;
    while (targetHeading < 0.0f)
        targetHeading += ANGLE_FULL_TURN;
    if (!(targetHeading < 0.0f) && targetHeading < ANGLE_HALF_TURN)
    {
        targetHeading = ANGLE_HALF_TURN / 2;
        return 0;
    }
    targetHeading = ANGLE_HALF_TURN * 1.5f;
    return 0;
}

// AnimModel::FaceAway: turns the model away from `point`: the heading is atan2(dx, dy) of the vector from `point` to the
// model. Returns 0.
int AnimModel::FaceAway(const Vec4 &point)
{
    Vec4 delta;
    Vec4 unused;  // constructed but never used (the original has it too)
    delta.Copy(matrix.GetTranslation());
    delta.Copy(delta - point);
    targetHeading = (float)atan2(delta.x, delta.y);
    return 0;
}

// AnimModel::FaceAwayFrom: turns the model away from model `other` (the heading of the vector from `other` to this
// model). Returns 0.
int AnimModel::FaceAwayFrom(AnimModel *other)
{
    Vec4 position;
    Vec4 otherPosition;
    position.Copy(matrix.GetTranslation());
    otherPosition.Copy(other->matrix.GetTranslation());
    position.Copy(position - otherPosition);
    targetHeading = (float)atan2(position.x, position.y);
    return 0;
}

// AnimModel::ScreenPosition: projects a point 50 units above the model's position to 640x480 screen pixels (y grows
// downwards) and stores them in *x and *y. Used to place balloons and markers over characters. Returns 0.
int AnimModel::ScreenPosition(int *x, int *y, const Matrix4 &viewProjection)
{
    Vec4 position;
    position.Copy(matrix.GetTranslation());
    position.z += ANIM_MARKER_HEIGHT;
    position.Copy(Project(position, viewProjection));
    position.x = (position.x + 1.0f) * (ANIM_SCREEN_WIDTH / 2);
    position.y = (2.0f - (position.y + 1.0f)) * (ANIM_SCREEN_HEIGHT / 2);
    *x = (int)position.x;
    *y = (int)position.y;
    return 0;
}

// AnimModel::AddMotionSpeed: adds `speed` (scaled, and divided by the model's speed divisor when it has one) to the
// vertical speed, capped at 60. Used for idle animations. Returns 0.
int AnimModel::AddMotionSpeed(float speed)
{
    speed = speed * MOTION_SPEED_SCALE;
    if (speedDivisor != 0)
        speed = speed / speedDivisor;
    speedChangeFrames = 0;
    idleMotionEnabled = 0;
    speed = speed * MOTION_SPEED_DAMPING + verticalSpeed;
    verticalSpeed = speed;
    if (blendFrames == 0)
        blendFrames = DEFAULT_BLEND_FRAMES;
    speedChangeFrames = 1;
    if (speed > MAX_VERTICAL_SPEED)
        verticalSpeed = MAX_VERTICAL_SPEED;
    return 0;
}

// AnimModel::PushMotionSpeed: gentler version of AddMotionSpeed used every frame while a push continues: a fifth of
// `speed` (divided by the model's speed divisor) is added only when a speed change is pending, the model has no divisor
// and fewer than 16 blend frames are left. The speed is capped at 60 and the frame counter advances. Returns 0.
int AnimModel::PushMotionSpeed(float speed)
{
    speed = speed * PUSH_SPEED_SCALE;
    if (speedDivisor != 0)
        speed = speed / speedDivisor;
    if (idleMotionEnabled == 1 && speedDivisor == 0)
    {
        speedChangeFrames = 0;
        if (blendFrames < PUSH_BLEND_LIMIT)
            verticalSpeed = speed * MOTION_SPEED_DAMPING + verticalSpeed;
        idleMotionEnabled = 0;
        if (blendFrames == 0)
            blendFrames = DEFAULT_BLEND_FRAMES;
    }
    if (verticalSpeed > MAX_VERTICAL_SPEED)
        verticalSpeed = MAX_VERTICAL_SPEED;
    speedChangeFrames++;
    return 0;
}

// AnimModel::MoveTowards: sets the movement step to the way from the model's position to (x, y), each axis limited to
// +-40, and stores `z` as the third component. Returns 0.
int AnimModel::MoveTowards(float x, float y, float z)
{
    x = x - matrix.m[MATRIX_TRANSLATION_X];
    y = y - matrix.m[MATRIX_TRANSLATION_Y];
    moveStep.x = x;
    moveStep.y = y;
    moveStep.z = z;
    if (x > MOVE_STEP_LIMIT)
        moveStep.x = MOVE_STEP_LIMIT;
    if (moveStep.x < -MOVE_STEP_LIMIT)
        moveStep.x = -MOVE_STEP_LIMIT;
    if (y > MOVE_STEP_LIMIT)
        moveStep.y = MOVE_STEP_LIMIT;
    if (moveStep.y < -MOVE_STEP_LIMIT)
        moveStep.y = -MOVE_STEP_LIMIT;
    return 0;
}

// AnimModel::Push: gives the model a movement impulse (x, y, z). While a speed change is pending the impulse is 0.7 of
// the value (x and y also scaled by the model's speed scale), or 1.4 of it when the model is nearly at rest. Otherwise
// only x and y are set, at 0.55. Returns 0.
int AnimModel::Push(float x, float y, float z)
{
    if (idleMotionEnabled == 1)
    {
        move.x = x * PUSH_MOVING_FACTOR * speedScale;
        move.y = y * PUSH_MOVING_FACTOR * speedScale;
        move.z = z * PUSH_MOVING_FACTOR;
        if (velocity.LengthSquared3() < 1.0f)
        {
            move.x = x * PUSH_AT_REST_FACTOR;
            move.y = y * PUSH_AT_REST_FACTOR;
            move.z = z * PUSH_AT_REST_FACTOR;
        }
    }
    else
    {
        move.x = x * PUSH_IDLE_FACTOR;
        move.y = y * PUSH_IDLE_FACTOR;
    }
    return 0;
}

// AnimModel::SnapHeight: moves the model to the nearest height that is a multiple of `step` (worked out in tenths of
// a unit, so step and height are rounded towards zero to 0.1), plus `offset`. Returns 0.
int AnimModel::SnapHeight(float step, float offset)
{
    int candidates[SNAP_CANDIDATE_COUNT];  // the multiples just below, above and two below; the fourth entry is unused
    Vec4 position = matrix.GetTranslation();
    int height = (int)(position.z * SNAP_UNITS_PER_WORLD_UNIT);
    int grid = (int)(step * SNAP_UNITS_PER_WORLD_UNIT);
    int below = height / grid;
    candidates[0] = below * grid;
    candidates[1] = (below + 1) * grid;
    candidates[2] = (below - 1) * grid;
    int best = 0;
    for (int i = 1; i < 3; i++)
    {
        if (abs(candidates[best] - height) > abs(candidates[i] - height))
            best = i;
    }
    position.z = (float)candidates[best] * 0.1f + offset;
    matrix.SetTranslation(position);
    return 0;
}

// AnimModel::SnapPosition: like SnapHeight for x and y: both are moved to the nearest multiple of `step` (in tenths of a
// unit) plus `offset`; the height stays. Returns 0.
int AnimModel::SnapPosition(float step, float offset)
{
    int candidates[SNAP_CANDIDATE_COUNT];  // the multiples just below, above and two below; the fourth entry is unused
    Vec4 position = matrix.GetTranslation();
    int x = (int)(position.x * SNAP_UNITS_PER_WORLD_UNIT);
    int grid = (int)(step * SNAP_UNITS_PER_WORLD_UNIT);
    int below = x / grid;
    candidates[0] = below * grid;
    candidates[1] = (below + 1) * grid;
    candidates[2] = (below - 1) * grid;
    int best = 0;
    int i;
    for (i = 1; i < 3; i++)
    {
        if (abs(candidates[best] - x) > abs(candidates[i] - x))
            best = i;
    }
    position.x = (float)candidates[best] * 0.1f + offset;
    int y = (int)(position.y * SNAP_UNITS_PER_WORLD_UNIT);
    below = y / grid;
    candidates[0] = below * grid;
    candidates[1] = (below + 1) * grid;
    candidates[2] = (below - 1) * grid;
    best = 0;
    for (i = 1; i < 3; i++)
    {
        if (abs(candidates[best] - y) > abs(candidates[i] - y))
            best = i;
    }
    position.y = (float)candidates[best] * 0.1f + offset;
    matrix.SetTranslation(position);
    return 0;
}

// AnimModel::Hold: starts or continues holding on to model `other` (on = 1): on the first call the offset to `other` is
// recorded (only for models with a "Bip01 Spine" bone), then the hold weight grows by `amount` up to 0.9. With on = 0 a
// running hold is marked released. Returns 0.
int AnimModel::Hold(int on, float amount, AnimModel *other)
{
    if (on != 0)
    {
        if (holdState == HOLD_FREE)
        {
            holdOffset.Fill(0);
            if (root != 0 && root->Find("Bip01 Spine", 0) != 0)
            {
                holdOffset.Copy(other->holdOffset + other->matrix.GetTranslation());
                holdOffset.Copy(holdOffset - matrix.GetTranslation());
            }
        }
        float weight = amount + holdWeight;
        holdWeight = weight;
        if (weight > HOLD_WEIGHT_MAX)
            holdWeight = HOLD_WEIGHT_MAX;
        holdState = HOLD_HOLDING;
        return 0;
    }
    if (holdState == HOLD_HOLDING)
        holdState = HOLD_RELEASED;
    return 0;
}

// AnimModel::HoldAtSpine: like Hold, but the offset recorded on the first call is the model's own "Bip01 Spine" bone
// relative to its position (0 when there is no such bone). Returns 0.
int AnimModel::HoldAtSpine(int on, float amount)
{
    if (on != 0)
    {
        if (holdState == HOLD_FREE)
        {
            holdOffset.Fill(0);
            if (root != 0)
            {
                Node *spine = root->Find("Bip01 Spine", 0);
                if (spine != 0)
                {
                    holdOffset.Copy(spine->worldMatrix.GetTranslation());
                    holdOffset.Copy(holdOffset - matrix.GetTranslation());
                }
            }
        }
        float weight = amount + holdWeight;
        holdWeight = weight;
        if (weight > HOLD_WEIGHT_MAX)
            holdWeight = HOLD_WEIGHT_MAX;
        holdState = HOLD_HOLDING;
        return 0;
    }
    if (holdState == HOLD_HOLDING)
        holdState = HOLD_RELEASED;
    return 0;
}

// AnimModel::IsClear: 1 when the model, moved by `offset` (0 = not moved), touches none of the other loaded models
// (collision test mode -1). Called by the characters before they move.
int AnimModel::IsClear(const Vec4 *offset)
{
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
    {
        float zero = 0;
        probe[0].Fill(zero);
    }
    Vec4 hits[MODEL_TOUCH_LIMIT];
    int hitOther = 0;
    int hitCount = 0;
    AnimModel *hitIds[MODEL_TOUCH_LIMIT];
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
    {
        if (g_modelFileRoots[i] != 0 && root != g_modelFileRoots[i] && g_modelFileRoots[i]->hasCollision != 0)
        {
            if (root->CollideModel(&matrix, g_modelFileRoots[i], g_modelFileMatrices[i], probe, 1, &hitCount, hitIds,
                                   hits, &hitOther, COLLIDE_ANY_MODE, 0, 0, 0, 0) != 0)
                return 0;
        }
    }
    return 1;
}

// AnimModel::IsClearFlagged: like IsClear, testing only nodes with the collision-probe flag (0x400).
int AnimModel::IsClearFlagged(const Vec4 *offset)
{
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
    {
        float zero = 0;
        probe[0].Fill(zero);
    }
    Vec4 hits[MODEL_TOUCH_LIMIT];
    int hitOther = 0;
    int hitCount = 0;
    AnimModel *hitIds[MODEL_TOUCH_LIMIT];
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
    {
        if (g_modelFileRoots[i] != 0 && root != g_modelFileRoots[i] && g_modelFileRoots[i]->hasCollision != 0 &&
            root->CollideModel(&matrix, g_modelFileRoots[i], g_modelFileMatrices[i], probe, 1, &hitCount, hitIds, hits,
                               &hitOther, COLLIDE_ANY_MODE, 0, 0, 0, NODE_FLAG_SHAPE_SPHERE) != 0)
            return 0;
    }
    return 1;
}

// AnimModel::IsClearAny: like IsClear with collision test mode 0.
int AnimModel::IsClearAny(const Vec4 *offset)
{
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
    {
        float zero = 0;
        probe[0].Fill(zero);
    }
    Vec4 hits[MODEL_TOUCH_LIMIT];
    int hitOther = 0;
    int hitCount = 0;
    AnimModel *hitIds[MODEL_TOUCH_LIMIT];
    for (int i = 0; i < MODEL_FILE_ENTRIES; i++)
    {
        if (g_modelFileRoots[i] != 0 && root != g_modelFileRoots[i] && g_modelFileRoots[i]->hasCollision != 0 &&
            root->CollideModel(&matrix, g_modelFileRoots[i], g_modelFileMatrices[i], probe, 1, &hitCount, hitIds, hits,
                               &hitOther, 0, 0, 0, 0, 0) != 0)
            return 0;
    }
    return 1;
}

// AnimModel::IsClearOfMap: 1 when the model, moved by `offset` (0 = not moved), does not touch the map collision of kind
// 0x4000000.
int AnimModel::IsClearOfMap(const Vec4 *offset)
{
    if (collider == 0)
        return 1;
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
        probe[0].Fill(0);
    probe[1].Fill(0);
    Vec4 hitPoint;
    Vec4 hitNormal;
    float nearest;
    int hitFlag;
    g_collisionFilter = collisionFilter;
    g_modelCollisionActive = 1;
    int hitCount = 0;
    int hit = collider->CollideProbesWithMesh(MAP_COLLISION_KIND, &matrix, dropCollider, colliderReach, &nearest, probe,
                                              1, &hitCount, &hitPoint, &hitNormal, 0, &hitFlag, 0, 0, 0);
    g_modelCollisionActive = 0;
    g_collisionFilter = 0;
    return hit == 0;
}

// AnimModel::IsClearOfMapLimited: like IsClearOfMap against all collision kinds, with the model's collision limit; also
// clear when the model has no drop collider.
int AnimModel::IsClearOfMapLimited(const Vec4 *offset)
{
    if (collider == 0 || dropCollider == 0)
        return 1;
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
        probe[0].Fill(0);
    probe[1].Fill(0);
    Vec4 hitPoint;
    Vec4 hitNormal;
    float nearest;
    int hitFlag;
    g_collisionFilter = collisionFilter;
    g_modelCollisionActive = 1;
    int hitCount = 0;
    int hit = collider->CollideProbesWithMesh(MAP_COLLISION_ANY, &matrix, dropCollider, colliderReach, &nearest, probe,
                                              1, &hitCount, &hitPoint, &hitNormal, collisionLimit, &hitFlag, 0, 0, 0);
    g_modelCollisionActive = 0;
    g_collisionFilter = 0;
    return hit == 0;
}
