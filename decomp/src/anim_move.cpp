// AnimModel::MoveWithCollisions: moves a model by a step against the map and the other models' collisions (0x10175f20).

#include <math.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"
#include "matrix_transform.h"

#include "node.h"

#include "anim_model.h"
#include "anim_move.h"
#include "anim_step_vars.h"
#include "node_collision_vars.h"
#include "render_vars.h"

#define MOVE_LIMIT_PLAYER 45.0f          // largest step on one axis for a player-controlled model (m_2390 == 1)
#define MOVE_LIMIT_OTHER 30.0f           // largest step on one axis for any other model
#define PULL_DIVISOR 12.0f               // the pull towards pullTarget covers this share of the distance
#define PULL_MOVE_SHARE 0.5f             // share of the model's own step kept while it is pulled
#define PULL_LIFT_MINIMUM 1.0f           // the pull lifts the model only when the target is higher than this
#define PULL_LIFT_SHARE 0.6f             // share of the height difference added to the step
#define FALL_FLOOR_Z -3500.0f            // a model below this is put back on the floor and stops falling
#define STILL_DISTANCE 0.1               // steps shorter than this count as standing still
#define STEP_SHRINK 0.95f                // the step the probes start from is shrunk by this
#define SPEED_LIMIT 26.0f                // longest horizontal step a probe is moved
#define NEAREST_START 1000000.0f         // the "nearest hit" the collision tests start from
#define GROUND_SPEED_LIMIT 20.0f         // m_237c, the ground probes' reach, never gets above this
#define GROUND_SPEED_LIMIT_PLAYER 7.0f   // ... or above this for player models
#define GROUND_REACH_FAR -4.0f           // the ground probes look this many m_237c down and ...
#define GROUND_REACH_NEAR -1.0f          // ... this many
#define GROUND_SETTLE_LIMIT 2.0f         // above this m_237c the model counts as in the air
#define GROUND_WALK_GAIN 0.7f            // m_237c grows by this per frame for a model that is not a player
#define GROUND_AIR_GAIN 0.8f             // ... and by this per frame for a player that is over no ground
#define GROUND_AIR_GAIN_FLAGGED 0.2f     // ... or this when m_0013 is set
#define QUARTER_TURN 0.785375f           // angle the ground probes are turned by
#define SIXTH_TURN 1.04716671f           // angle the side probes are turned by
#define FIFTH_TURN 0.628300011f          // angle the near side probes are turned by
#define AHEAD_REACH 1.20000005f          // how far ahead of the horizontal speed the turned probes look
#define TARGET_LIFT_SHARE 0.949999988f   // share of the vertical step added to the turned probes' height
#define SHORT_PROBE_SCALE 0.2f           // the probe that moves a fifth of the step
#define GROUND_HIT_SCALE -0.5f           // the probe under a landing spot is half the step back
#define RING_SPEED_SHARE 0.7f            // scale of the outer ring probes
#define RING_SPEED_SHARE_LOW 0.5f
#define WALL_SLIDE_MINIMUM 1.0           // a flat step of at least this on one axis can slide along a wall
#define WALL_ALIGNED_DOT 0.32            // map normals aligned with the step by more than this block it
#define FALL_DAMP -5.0f                  // verticalSpeed above this is damped
#define FALL_DAMP_SHARE 0.9f
#define FALL_DAMP_STEP 5.0f
#define LAND_SPEED_LIMIT -1.0f           // a vertical step below this marks a landing (m_0d48)
#define STEP_UP_LIMIT 1.0f               // step heights above this make a model take the free ring
#define SPEED_STILL_LIMIT 1.1f
#define VELOCITY_KEEP 0.82f              // velocity = previous push * this + step * VELOCITY_STEP
#define VELOCITY_STEP 0.34f
#define PUSH_KEEP 0.9f                   // the push (m_2408) keeps this much each frame
#define PUSH_STEP 0.05f
#define VELOCITY_SLOW 0.8f               // push = velocity * this while standing on ground that moves

#define PROBE_COUNT 32
#define PROBE_GROUND_A 30                // probes[30] and probes[31] look for ground under a landing spot
#define PROBE_GROUND_B 31
#define SLIDE_COUNT 16
#define HIT_COUNT 0x100

enum ProbeVariant
{
    VARIANT_HOVER = 1,        // m_02a1: probes spread by turns about z
    VARIANT_FLY = 2,          // m_02a0: probes turned about x and z
    VARIANT_AIR = 3,          // mode 2: the model is over the pit area
    VARIANT_WALK = 4          // everything else: the rings of the walking models
};

enum ContactKind
{
    CONTACT_MAP_HIT = 2,
    CONTACT_MODEL_HIT = 3
};

static const RingProbe RING_BLOCKED[] =
{
    { 1.0f, 1.0f, RING_SPEED_SHARE },
    { -1.0f, 1.0f, RING_SPEED_SHARE },
    { 1.0f, 0.5f, RING_SPEED_SHARE_LOW },
    { -1.0f, 0.5f, RING_SPEED_SHARE_LOW }
};

static const RingProbe RING_STUCK[] =
{
    { 1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f },
    { 2.0f, 1.0f, RING_SPEED_SHARE },
    { -2.0f, 1.0f, RING_SPEED_SHARE },
    { 2.0f, 0.5f, RING_SPEED_SHARE },
    { -2.0f, 0.5f, RING_SPEED_SHARE }
};

static const RingProbe RING_FREE[] =
{
    { 0.5f, 1.0f, 1.0f },
    { -0.5f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { -1.0f, 1.0f, 1.0f },
    { 2.0f, 1.0f, RING_SPEED_SHARE },
    { -2.0f, 1.0f, RING_SPEED_SHARE },
    { 2.0f, 0.5f, RING_SPEED_SHARE_LOW },
    { -2.0f, 0.5f, RING_SPEED_SHARE_LOW }
};

// Fills probes[first ..] with the ring: every entry's direction (x, 0, z) is turned into the frame of the move
// (`heading`) and scaled by the horizontal speed and the entry's own factor. The direction is kept in slideDirs for
// the probe the move ends on.
static void SetRingProbes(Vec4 *probes, Vec4 *slideDirs, int first, const RingProbe *ring, int count, const Matrix4 &heading,
                          float speed)
{
    for (int i = 0; i < count; i++)
    {
        Vec4 direction;
        direction.Set(ring[i].x, 0, ring[i].z, 0);
        direction.Normalized3();
        slideDirs[first + i].Copy(direction);
        probes[first + i].Copy(Project(direction, heading) * speed * ring[i].scale);
    }
}

// After a frame on the ground the shadow's matrix follows the model's: a full copy while m_0012 is 0, only the
// position while it is 2.
static void FollowWithShadow(AnimModel *model)
{
    if (model->m_238c == 1)
    {
        char followMode = model->m_0012;
        if (followMode == 0)
            model->shadowMatrix = model->matrix;
        if (followMode == 2)
        {
            model->shadowMatrix._41 = model->matrix._41;
            model->shadowMatrix._42 = model->matrix._42;
        }
    }
}

// Keeps the shadow under the model: its position is the model's, but never higher than the shadow already is.
static void PlaceShadow(AnimModel *model, const Vec4 &position)
{
    Vec4 shadowPosition;
    shadowPosition.Copy(model->shadowMatrix.GetTranslation());
    if (shadowPosition.z > position.z)
        shadowPosition.z = position.z;
    shadowPosition.x = position.x;
    shadowPosition.y = position.y;
    model->shadowMatrix.SetTranslation(shadowPosition);
}

// After a probe was chosen: moves the model to it and settles it on the ground under it. A landing spot is tested
// with two probes that look down from it (probes[PROBE_GROUND_A/B]); a hit puts the model back at `position`. Marks
// the landing in m_0d48 when it came down on something. `mode` is the collision test mode of the ground probes,
// `dampFall` lets the model slow its fall when a ground probe finds ground while it is in the air (verticalSpeed).
// Returns the probe index the move ended on, -1 when the ground blocks it.
static int SettleOnGround(AnimModel *model, Vec4 *probes, int probeIndex, const Vec4 &position, const Vec4 &move, int mode,
                          int dampFall)
{
    Vec4 groundHits[HIT_COUNT];
    AnimModel *groundIds[HIT_COUNT];
    int hitOther = 0;
    float nearest = NEAREST_START;
    int groundHit = 0;
    int result = probeIndex;
    model->matrix.SetTranslation(position + probes[probeIndex]);
    probes[PROBE_GROUND_B].Fill(0.0f);
    probes[PROBE_GROUND_A].Copy(probes[probeIndex] * GROUND_HIT_SCALE);
    if (model->m_2390 != 0)
        model->m_023c = 4;
    if (model->m_2390 == 1 && model->lifted == 0)
        groundHit = model->LiftProbes(&nearest, &probes[PROBE_GROUND_A], 1, groundIds, groundHits, &hitOther, 0, 1, 1, mode, 0,
                                      0, 0);
    model->m_023c = 0;
    if (groundHit != 0)
    {
        if (dampFall != 0 && model->m_238c == 0)
        {
            float speed = model->verticalSpeed;
            if (speed >= FALL_DAMP)
                model->verticalSpeed = speed * FALL_DAMP_SHARE;
            else
                model->verticalSpeed = speed + FALL_DAMP_STEP;
        }
        result = -1;
    }
    else
    {
        if (model->m_2390 == 1)
        {
            model->m_023c = 4;
            groundHit = model->LiftProbes(&nearest, &probes[PROBE_GROUND_B], 1, groundIds, groundHits, &hitOther, 0, 1, 1, mode,
                                          0, 0, 0);
            model->m_023c = 0;
        }
        if (groundHit != 0)
        {
            if (dampFall != 0 && model->m_238c == 0)
            {
                float speed = model->verticalSpeed;
                if (speed >= FALL_DAMP)
                    model->verticalSpeed = speed * FALL_DAMP_SHARE;
                else
                    model->verticalSpeed = speed + FALL_DAMP_STEP;
            }
            result = -1;
        }
        else
        {
            if (probeIndex > 0 && move.z < LAND_SPEED_LIMIT)
                model->m_0d48 = 1;
            if (probeIndex == 0 && probes[0].z != move.z)
                model->m_0d48 = 1;
        }
    }
    model->matrix.SetTranslation(position);
    if (result >= 0 && model->m_2390 == 1 && DAT_11b0450c > 0)
        DAT_11b0450c--;
    return result;
}

// 0x10175f20 (AnimModel::MoveWithCollisions). Moves the model by the step (x, y, z) and tests the move against the
// map and the other models: the step is limited to 30 (45 for a player) per axis and pulled towards pullTarget while
// that lasts, then a ring of probes (the step itself, then turns of it, then smaller steps, then standing still) is
// handed to LiftProbes, and the model moves along the first probe that is clear. Models that are on the ground first
// look for the ground below (and for a moving ground that carries them: shiftX/Y/Z); velocity, the push (m_2408),
// the contact direction (m_0090) and the shadow are updated from the move. Returns 0 without moving when the model
// may not move (or the world has no collision data), else 1. `wantHitOther` makes the tests report the model hit.
int AnimModel::MoveWithCollisions(float moveX, float moveY, float moveZ, int wantHitOther, int unused)
{
    contactState = 0;
    mapTested = 0;
    if (m_006c != 0 && moveZ < 0)
    {
        moveX = 0;
        moveY = 0;
    }
    Vec4 requested;
    requested.x = moveX;
    requested.y = moveY;
    requested.z = moveZ;
    if ((DAT_10203430 == 0 && (mode == 2 || mode == 0)) || m_0267 != 0)
        return 0;
    if (pullTarget != 0 && m_238c == 0)
    {
        Vec4 pull = (*pullTarget - matrix.GetTranslation()).Divided(PULL_DIVISOR);
        moveX = moveX * PULL_MOVE_SHARE + pull.x;
        moveY = moveY * PULL_MOVE_SHARE + pull.y;
        if (pull.z > PULL_LIFT_MINIMUM)
            moveZ = pull.z * PULL_LIFT_SHARE + moveZ;
        pullFrames--;
        if (pullFrames <= 0)
            pullTarget = 0;
    }
    Vec4 walkStep;
    Vec4 move;
    Vec4 position;
    Vec4 flat;
    Vec4 probes[PROBE_COUNT];
    Vec4 slideDirs[SLIDE_COUNT];
    memset(slideDirs, 0, sizeof(slideDirs));
    float limit = MOVE_LIMIT_OTHER;
    if (m_2390 == 1)
        limit = MOVE_LIMIT_PLAYER;
    if (moveZ > limit)
        moveZ = limit;
    else if (moveZ < -limit)
        moveZ = -limit;
    if (moveX > limit)
        moveX = limit;
    else if (moveX < -limit)
        moveX = -limit;
    if (moveY > limit)
        moveY = limit;
    else if (moveY < -limit)
        moveY = -limit;
    if (DAT_1020342c == 0)
    {
        m_238c = 1;
        flat.x = moveX;
        flat.y = moveY;
        flat.z = 0;
        if (moveX != 0 || moveY != 0)
            flat.z = 8.0f;
        position.Copy(matrix.GetTranslation());
        matrix.SetTranslation(position + flat);
        return 0;
    }
    if (moveZ < 0 && mode == 1)
        moveZ = 0;
    if (matrix.GetTranslation().z < FALL_FLOOR_Z)
    {
        m_238c = 1;
        matrix._43 = FALL_FLOOR_Z;
        verticalSpeed = 0;
        m_237c = 0;
    }
    Vec4 groundHits[HIT_COUNT];
    if (fabs(moveX) + fabs(moveY) < STILL_DISTANCE)
        m_2374 = 0;
    float nearest = NEAREST_START;
    int hitOther = 0;
    position.Copy(matrix.GetTranslation());
    flat.x = moveX;
    flat.y = moveY;
    flat.z = moveZ;
    flat.w = 0;
    move.Copy(flat);
    walkStep.Copy(flat * STEP_SHRINK);
    flat.z = 0;
    float speed = flat.Length();
    if (speed > SPEED_LIMIT)
        move.Copy((move * SPEED_LIMIT).Divided(speed));
    Vec4 hits[HIT_COUNT];
    if (m_2384 != 0)
        m_23f8.Copy(move);
    int savedGrounded = m_238c;
    m_2474 = 1;
    AnimModel *hitIds[HIT_COUNT];
    if (m_0240 != 0)
    {
        probes[0].x = 0;
        probes[0].y = 0;
        probes[0].z = 0;
        if (LiftProbes(&nearest, probes, 1, hitIds, hits, &hitOther, 0, m_2390, 0, m_0240, m_0240, 0, 0) == 0)
        {
            matrix.SetTranslation(position + probes[0]);
            position.Copy(matrix.GetTranslation());
        }
    }
    lifted = 0;
    int hitMatrix = 0;
    int hitFlag = 0;
    int probeResult = 0;
    if (m_229c == 0 && mode == 1)
    {
        if (m_237c > GROUND_SPEED_LIMIT)
            m_237c = GROUND_SPEED_LIMIT;
        if (m_2390 == 1 && m_237c > GROUND_SPEED_LIMIT_PLAYER)
            m_237c = GROUND_SPEED_LIMIT_PLAYER;
        if (speedDivisor != 0)
            m_237c = 1.0f;
        int groundProbes = 5;
        if (m_02a2 != 0 || m_2390 != 1)
            groundProbes = 1;
        if (m_02a0 != 0)
            groundProbes = 1;
        if (m_238c != 0)
            groundProbes = 1;
        probes[0].x = 0;
        probes[0].y = 0;
        probes[0].z = m_237c * GROUND_REACH_FAR;
        probes[1].x = 0;
        probes[1].y = 0;
        probes[1].z = m_237c * GROUND_REACH_NEAR;
        if (m_238c == 0 && groundProbes != 1)
        {
            probes[2].Copy(Project(probes[0], RotationY(-QUARTER_TURN)));
            probes[3].Copy(Project(probes[0], RotationY(QUARTER_TURN)));
            probes[4].Copy(Project(probes[0], RotationX(-QUARTER_TURN)));
            probes[5].Copy(Project(probes[0], RotationX(QUARTER_TURN)));
            for (int i = 0; i < 6; i++)
                probes[i].z = (float)(int)probes[i].z;
        }
        else
        {
            groundProbes = 1;
            probes[0].z = (float)(int)probes[0].z;
            probes[1].z = (float)(int)probes[1].z;
        }
        int probeCount = groundProbes + 1;
        m_023c = 4;
        hitFlag = 0;
        probeResult = LiftProbes(&nearest, probes, probeCount, hitIds, hits, &hitOther, 0, m_2390, 0, 2, 0, &hitMatrix, &hitFlag);
        m_023c = 0;
        bool fell = false;
        if (probeResult < 0 || probeResult >= probeCount)
            fell = true;
        else if (m_2390 == 1)
        {
            AnimModel *groundIds[HIT_COUNT];
            int groundOther = 0;
            float groundNearest = NEAREST_START;
            matrix.SetTranslation(position + probes[probeResult]);
            probes[PROBE_GROUND_B].Fill(0.0f);
            probes[PROBE_GROUND_A].Copy(probes[probeResult] * GROUND_HIT_SCALE);
            int groundHit = 0;
            m_023c = 4;
            if (lifted == 0)
                groundHit = LiftProbes(&groundNearest, &probes[PROBE_GROUND_A], 1, groundIds, groundHits, &groundOther, 0, m_2390,
                                       1, 0, 0, 0, 0);
            m_023c = 0;
            if (groundHit != 0)
            {
                matrix.SetTranslation(position);
                velocity.Set(0, 0, 0, 0);
                m_237c = 1.0f;
                m_238c = 1;
            }
            else
            {
                m_023c = 4;
                groundHit = LiftProbes(&groundNearest, &probes[PROBE_GROUND_B], 1, groundIds, groundHits, &groundOther, 0,
                                       m_2390, 1, 0, 0, 0, 0);
                m_023c = 0;
                if (groundHit != 0)
                    matrix.SetTranslation(position);
                else
                {
                    if (m_0013 != 0)
                        m_237c = m_237c + GROUND_AIR_GAIN_FLAGGED;
                    else
                        m_237c = m_237c + GROUND_AIR_GAIN;
                    m_238c = savedGrounded;
                    if (m_237c > GROUND_SETTLE_LIMIT)
                        m_238c = 0;
                    position.Copy(matrix.GetTranslation());
                }
            }
            if (probeResult == -1)
                m_237c = 1.0f;
        }
        else
        {
            matrix.SetTranslation(position + probes[probeResult]);
            m_237c = m_237c + GROUND_WALK_GAIN;
            m_238c = savedGrounded;
            if (m_237c > GROUND_SETTLE_LIMIT)
                m_238c = 0;
            position.Copy(matrix.GetTranslation());
        }
        if (fell)
        {
            m_238c = 1;
            m_237c = 1.0f;
            m_0d48 = 1;
            velocity.Set(0, 0, 0, 0);
        }
        if (hitFlag != 0)
        {
            m_0d48 = 1;
            m_237c = 1.0f;
            m_238c = 1;
            velocity.Set(0, 0, 0, 0);
        }
    }
    lifted = 0;
    float shiftX = 0;
    float shiftY = 0;
    float shiftZ = 0;
    if (m_238c == 0)
        m_01d4 = 0;
    else
    {
        if (hitMatrix != 0)
        {
            Matrix4 *ground = (Matrix4 *)hitMatrix;
            if (m_01d4 == hitMatrix)
            {
                Vec4 oldPosition = matrix.GetTranslation();
                Matrix4 inverse = InverseMatrix(m_01d8);
                Vec4 local = Project(oldPosition, inverse);
                Vec4 carried = Project(local, *ground);
                local.Copy(carried);
                shiftX = local.x - oldPosition.x;
                shiftY = local.y - oldPosition.y;
                shiftZ = local.z - oldPosition.z;
                if (shiftZ > 0)
                    shiftZ = 0;
                Matrix4 turned = UnscaledMatrix(ground);
                float turnedHeading = turned.M_101b0a80();
                float oldHeading = m_0018.M_101b0a80();
                m_0018 = turned;
                float groundTurn = oldHeading - turnedHeading;
                heading = heading - groundTurn;
                targetHeading = targetHeading - groundTurn;
                m_0058.Copy(oldPosition);
                m_01d8 = *ground;
            }
            else
            {
                m_01d4 = hitMatrix;
                m_01d8 = *ground;
                m_0058.Copy(matrix.GetTranslation());
                m_0018 = UnscaledMatrix(ground);
            }
        }
        flat.w = 0;
        moveX = shiftX + moveX;
        moveY = shiftY + moveY;
        moveZ = shiftZ + moveZ;
        flat.x = moveX;
        flat.y = moveY;
        flat.z = moveZ;
        move.Copy(flat);
        walkStep.Copy(flat * STEP_SHRINK);
        speed = flat.Length();
        if (speed > SPEED_LIMIT)
            move.Copy((move * SPEED_LIMIT).Divided(speed));
    }
    m_2474 = 0;
    hitOther = 0;
    if (m_008d != 0)
    {
        matrix.SetTranslation(position + move);
        m_23e8.Copy(move);
        m_00a0.Copy(move);
        if (m_238c == 1)
        {
            velocity.Copy(move);
            velocity.x = velocity.x - shiftX;
            velocity.y = velocity.y - shiftY;
            velocity.z = velocity.z - shiftZ;
            m_2408.Copy(velocity * VELOCITY_SLOW);
            m_2408.x = shiftX + m_2408.x;
            m_2408.y = shiftY + m_2408.y;
            m_2408.z = shiftZ + m_2408.z;
        }
        else
        {
            velocity.Copy(m_2408 * VELOCITY_KEEP + move * VELOCITY_STEP);
            m_2408.Copy(m_2408 * PUSH_KEEP + move * PUSH_STEP);
        }
        m_2374 = m_2374 + 1;
        FollowWithShadow(this);
        flat.Copy(matrix.GetTranslation());
        PlaceShadow(this, flat);
        return 1;
    }
    bool idle = m_02a2 != 0;
    if (!idle && fabs(moveX) < STILL_DISTANCE && fabs(moveY) < STILL_DISTANCE && fabs(moveZ) < STILL_DISTANCE)
        idle = true;
    if (idle)
    {
        probes[0].Fill(0.0f);
        if (m_238c == 1)
        {
            velocity.Copy(probes[0]);
            velocity.x = velocity.x - shiftX;
            velocity.y = velocity.y - shiftY;
            velocity.z = velocity.z - shiftZ;
            m_2408.Copy(velocity * VELOCITY_SLOW);
            m_2408.x = shiftX + m_2408.x;
            m_2408.y = shiftY + m_2408.y;
            m_2408.z = shiftZ + m_2408.z;
        }
        else
        {
            velocity.Copy(m_2408 * VELOCITY_KEEP + walkStep * VELOCITY_STEP);
            m_2408.Copy(m_2408 * PUSH_KEEP + walkStep * PUSH_STEP);
        }
        probeResult = -1;
    }
    else
    {
        float angle = (float)atan2(moveY, moveX);
        float flatSpeed = (float)sqrt(moveX * moveX + moveY * moveY);
        int variant = VARIANT_WALK;
        int probeCount = 0;
        bool wallAhead = false;
        Vec4 up;
        up.Set(0, 0, 1, 0);
        up.Normalized3();
        if (m_02a1 != 0)
        {
            variant = VARIANT_HOVER;
            for (int i = 0; i < 6; i++)
                probes[i].Copy(move);
            probes[1].RotateZ(FIFTH_TURN);
            probes[2].RotateZ(-FIFTH_TURN);
            probes[3].RotateZ(SIXTH_TURN);
            probes[4].RotateZ(-SIXTH_TURN);
            probes[5].Copy(move * SHORT_PROBE_SCALE);
            probes[6].Copy(move * 0.0f);
            probeCount = 7;
        }
        else if (m_02a0 != 0)
        {
            variant = VARIANT_FLY;
            Vec4 ahead;
            ahead.Set(0, flatSpeed * AHEAD_REACH, moveZ, 0);
            Vec4 high = Project(ahead, RotationX(-QUARTER_TURN));
            high = Project(high, RotationZ(angle));
            Vec4 low = Project(ahead, RotationX(-SIXTH_TURN));
            low = Project(low, RotationZ(angle));
            Vec4 mid = Project(ahead, RotationX(-FIFTH_TURN));
            mid = Project(mid, RotationZ(angle));
            probes[0].Copy(move);
            probes[1].Copy(mid);
            probes[2].Copy(high);
            probes[3].Copy(low);
            probes[4].Copy(move * SHORT_PROBE_SCALE);
            probes[5].Copy(move * 0.0f);
            probeCount = 7;
            if (m_238c == 0)
                for (int i = 1; i <= 6; i++)
                    if (probes[0].z < probes[i].z)
                        probes[i].z = probes[0].z;
        }
        else if (mode == 2)
        {
            variant = VARIANT_AIR;
            Vec4 ahead;
            ahead.Set(0, flatSpeed * AHEAD_REACH, moveZ, 0);
            Vec4 high = Project(ahead, RotationX(-QUARTER_TURN));
            high = Project(high, RotationZ(angle));
            Vec4 low = Project(ahead, RotationX(SIXTH_TURN));
            low = Project(low, RotationZ(angle));
            Vec4 mid = Project(ahead, RotationX(-QUARTER_TURN));
            mid = Project(mid, RotationZ(angle));
            probes[0].Copy(move);
            probes[1].Copy(mid);
            probes[2].Copy(high);
            probes[3].Copy(RotateAboutAxis(&up, SIXTH_TURN, &move));
            probes[4].Copy(RotateAboutAxis(&up, -SIXTH_TURN, &move));
            if (m_2390 == 1)
            {
                probeCount = 10;
                probes[5].Copy(low);
                probes[6].Copy(RotateAboutAxis(&high, SIXTH_TURN, &move));
                probes[7].Copy(RotateAboutAxis(&high, -SIXTH_TURN, &move));
                probes[8].Copy(move * SHORT_PROBE_SCALE);
                probes[9].Copy(move * 0.0f);
            }
            else
            {
                probeCount = 6;
                probes[5].Copy(move * 0.0f);
            }
            if (m_238c == 0)
                for (int i = 1; i <= 6; i++)
                    if (probes[0].z < probes[i].z)
                        probes[i].z = probes[0].z;
        }
        else if (m_2390 == 1)
        {
            if (speedDivisor == 0 && m_0013 == 0 && m_238c == 0)
            {
                probes[0].Copy(move);
                contactState = 1;
                int blockedResult = LiftProbes(&nearest, probes, 1, hitIds, hits, &hitOther, 0, m_2390, 0, 1, 0, 0, 0);
                if (blockedResult == -1)
                {
                    bool blockedByWall = true;
                    if (contactState != CONTACT_MODEL_HIT)
                    {
                        if (contactState != CONTACT_MAP_HIT)
                            blockedByWall = false;
                        else
                        {
                            Vec4 contact = m_0000;
                            contact.Normalized3();
                            Vec4 sideways;
                            sideways.Set(move.x, move.y, 0, move.w);
                            if (fabs(sideways.x) >= WALL_SLIDE_MINIMUM || fabs(sideways.y) >= WALL_SLIDE_MINIMUM)
                            {
                                sideways.Normalized3();
                                if (fabs(Vec3_Dot(&contact, &sideways)) <= WALL_ALIGNED_DOT)
                                    blockedByWall = false;
                            }
                        }
                    }
                    if (blockedByWall)
                        wallAhead = true;
                }
                contactState = 0;
            }
            if (speedDivisor != 0)
                wallAhead = true;
            Vec4 flatStep;
            flatStep.Set(moveX, moveY, 0, 0);
            Matrix4 headingMatrix;
            headingMatrix = DirectionMatrix(flatStep);
            probes[0].Copy(move);
            if (wallAhead)
            {
                SetRingProbes(probes, slideDirs, 4, RING_BLOCKED, 4, headingMatrix, flatSpeed);
                probes[9].Copy(move * 0.0f);
                for (int i = 4; i <= 7; i++)
                    probes[i].z = probes[0].z;
                float stepHeight = (float)fabs(move.z);
                Vec4 stepDirection;
                stepDirection.Copy(move);
                Matrix4 stepMatrix = DirectionMatrix(stepDirection);
                static const float verticalY[] = { -0.5f, -1.0f, -1.3f };
                for (int i = 0; i < 3; i++)
                {
                    Vec4 direction;
                    direction.Set(0, verticalY[i], 1.0f, 0);
                    direction.Normalized3();
                    slideDirs[1 + i].Copy(direction);
                    probes[1 + i].Copy(Project(direction, stepMatrix) * stepHeight);
                }
                probeCount = 10;
            }
            else if (move.z > STEP_UP_LIMIT || m_237c > SPEED_STILL_LIMIT || m_238c == 0)
            {
                SetRingProbes(probes, slideDirs, 1, RING_FREE, 8, headingMatrix, flatSpeed);
                probes[9].Copy(move * 0.0f);
                for (int i = 1; i <= 8; i++)
                    probes[i].z = probes[0].z;
                probeCount = 10;
            }
            else
            {
                SetRingProbes(probes, slideDirs, 2, RING_STUCK, 6, headingMatrix, flatSpeed);
                probes[8].Copy(move * 0.0f);
                for (int i = 1; i <= 7; i++)
                    probes[i].z = probes[0].z;
                Vec4 direction;
                direction.Set(0, -0.5f, 1.0f, 0);
                direction.Normalized3();
                slideDirs[1].Copy(direction);
                probes[1].Copy(Project(direction, headingMatrix) * flatSpeed);
                probeCount = 9;
            }
        }
        else
        {
            Vec4 ahead;
            ahead.Set(0, flatSpeed * AHEAD_REACH, moveZ * 0.6f, 0);
            Vec4 high = Project(ahead, RotationX(-QUARTER_TURN));
            high = Project(high, RotationZ(angle));
            Vec4 mid = Project(ahead, RotationX(-FIFTH_TURN));
            mid = Project(mid, RotationZ(angle));
            float lift = moveZ * TARGET_LIFT_SHARE;
            high.z = high.z + lift;
            mid.z = mid.z + lift;
            probes[0].Copy(move);
            probes[1].Copy(mid);
            probes[2].Copy(high);
            probes[3].Copy(RotateAboutAxis(&ahead, SIXTH_TURN, &move));
            probes[4].Copy(RotateAboutAxis(&ahead, -SIXTH_TURN, &move));
            probes[5].Copy(move * 0.0f);
            probeCount = 6;
        }
        int groundedBefore = m_238c;
        m_023c = 4;
        if (variant == VARIANT_WALK && m_2390 == 1)
            DAT_11b002b4 = 0;
        int *hitOtherArg = 0;
        if (wantHitOther != 0)
            hitOtherArg = &hitOther;
        probeResult = LiftProbes(&nearest, probes, probeCount, hitIds, hits, hitOtherArg, 0, m_2390, 0, 1, 0, 0, 0);
        if (probeResult != 0)
            m_0d44 = 1;
        m_023c = 0;
        if (probeResult == probeCount - 1)
            velocity.Set(0, 0, 0, 0);
        bool settle = probeResult >= 0 && probeResult < probeCount - 1;
        if (variant != VARIANT_WALK && m_2390 != 1)
            settle = false;
        if (settle)
        {
            int settleMode = 0;
            int dampFall = 1;
            if (variant == VARIANT_WALK)
            {
                settleMode = 1;
                dampFall = 0;
            }
            probeResult = SettleOnGround(this, probes, probeResult, position, move, settleMode, dampFall);
        }
        else if (moveZ < LAND_SPEED_LIMIT)
            m_0d48 = 1;
        m_238c = groundedBefore;
        if (probeResult != -1)
        {
            m_0090.Copy(slideDirs[probeResult]);
            hitSolid = DAT_11b002b4 == 0;
            m_23e8.Copy(move);
            m_00a0.Copy(probes[probeResult]);
            matrix.SetTranslation(position + probes[probeResult]);
            if (m_238c == 1)
            {
                velocity.Copy(probes[probeResult]);
                velocity.x = velocity.x - shiftX;
                velocity.y = velocity.y - shiftY;
                velocity.z = velocity.z - shiftZ;
                m_2408.Copy(velocity * VELOCITY_SLOW);
                m_2374 = m_2374 + 1;
                m_2408.x = shiftX + m_2408.x;
                m_2408.y = shiftY + m_2408.y;
                m_2408.z = shiftZ + m_2408.z;
            }
            else
            {
                velocity.Copy(m_2408 * VELOCITY_KEEP + move * VELOCITY_STEP);
                m_2408.Copy(m_2408 * PUSH_KEEP + move * PUSH_STEP);
                m_2374 = m_2374 + 1;
            }
        }
    }
    if (probeResult == -1)
    {
        m_23e8.Set(0, 0, 0, 0);
        m_00a0.Set(0, 0, 0, 0);
    }
    FollowWithShadow(this);
    flat.Copy(matrix.GetTranslation());
    PlaceShadow(this, flat);
    return 1;
}
