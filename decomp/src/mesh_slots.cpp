// Drawing the model slots (EffectModel, DAT_10edbe08).

#include "vec4.h"

#include "matrix4.h"
#include "render_vars.h"

Matrix4 LookAt(Vec4 eye, Vec4 target, const Vec4 *up, int a);                   // 0x101b1c20
Matrix4 Perspective(float nearPlane, float farPlane, float fieldOfView);          // 0x101b0c10
Matrix4 RotationX(float angle);                                                   // 0x101b1ad0
Matrix4 RotationY(float angle);                                                   // 0x101b1bb0
Matrix4 RotationZ(float angle);                                                   // 0x101b1b40
Matrix4 Scaling(float x, float y, float z);                                       // 0x101b13c0
Matrix4 Multiply(const Matrix4 &left, const Matrix4 &right);                        // 0x101b10f0

#include "node.h"

#include "effect_model.h"


// 0x1000baf0 (EffectModel::Draw). Queues the slot's model for drawing once it is visible and its frame has reached the start frame:
// with `view`, or with its own camera (480 units back and 120 up, looking at the origin, a 45 degree perspective from
// 20 to 5000 units squeezed to 4:3) and an identity placement. Returns 0 for a free slot, else 1.
int EffectModel::Draw(const Matrix4 &view)
{
    if (node == 0)
        return 0;
    if (visible != 0 && frame >= start)
    {
        if (ownCamera != 0)
        {
            Matrix4 result;
            Matrix4 unused1;
            Matrix4 camera;
            Matrix4 projection;
            Matrix4 unused2;
            Vec4 up;
            Vec4 target;
            Vec4 eye;
            up.x = 0;
            up.y = 1.0f;
            up.z = 0;
            target.Set(0, 0, 0, 0);
            eye.Copy(target);
            eye.z += 120.0;
            eye.y -= 480.0;
            camera = LookAt(eye, target, &up, 0);
            projection = Multiply(Scaling(1.0f, 1.333f, 1.0f), Perspective(20.0f, 5000.0f, 0.785375f));
            result = Multiply(projection, camera);
            matrix.SetIdentity();
            node->QueueDraw(&matrix, &result, 0, -1, &color, 0);
        }
        else
            node->QueueDraw(&matrix, &view, 0, -1, &color, 0);
    }
    return 1;
}

// Degrees to radians the way the game writes it (pi = 3.1415).
#define KEYED_PI 3.1415f
#define KEYED_HALF_TURN_DEGREES 180.0f

// 0x10037b40 (EffectModel::DrawKeyed). Plays one frame of the effect: when it has been changed (key or rotation), the
// matrix is rebuilt from the key frame at the current frame (position) and the rotation in degrees; then the model is
// started with `view` and `camera` (or its own camera, as Draw has) at the frame counted from `start`. Ends the effect
// (Free) once the last frame is reached and returns 0; the frame counter only advances while the effect is visible.
// Returns 1 while the slot is in use.
int EffectModel::DrawKeyed(Matrix4 &view, Matrix4 &camera)
{
    if (node == 0)
        return 0;
    if (visible != 0)
    {
        if (frame >= start)
        {
            if (changed != 0)
            {
                Vec4 position;
                keys.Sample(frame - start, &position.x, &position.y, &position.z, &position.w, 0);
                matrix = RotationX(rotation.x * KEYED_PI / KEYED_HALF_TURN_DEGREES) *
                         RotationY(rotation.y * KEYED_PI / KEYED_HALF_TURN_DEGREES) *
                         RotationZ(rotation.z * KEYED_PI / KEYED_HALF_TURN_DEGREES);
                matrix.SetTranslation(position);
            }
            if (ownCamera != 0)
            {
                Matrix4 result;
                Matrix4 unused1;
                Matrix4 ownView;
                Matrix4 projection;
                Matrix4 unused2;
                Vec4 up;
                Vec4 target;
                Vec4 eye;
                up.x = 0;
                up.y = 1.0f;
                up.z = 0;
                target.Set(0, 0, 0, 0);
                eye.Copy(target);
                eye.z += 120.0;
                eye.y -= 480.0;
                ownView = LookAt(eye, target, &up, 0);
                projection = Multiply(Scaling(1.0f, 1.333f, 1.0f), Perspective(20.0f, 5000.0f, 0.785375f));
                result = Multiply(projection, ownView);
                matrix.SetIdentity();
                node->StartProximityPass(matrix.m, &result, &ownView, frame - start, 0);
            }
            else
                node->StartProximityPass(matrix.m, &view, &camera, frame - start, 0);
            if (frame - start >= end)
            {
                visible = 0;
                Free();
                return 0;
            }
        }
        if (visible != 0)
            frame++;
    }
    return 1;
}

// Draws every used model slot with the world matrix DAT_108392d8. The identity matrix is unused.
int ResetMeshSlots()
{
    Matrix4 identity;
    identity.SetIdentity();
    for (int i = 0; i < EFFECT_MODEL_SLOTS; i++)
    {
        if (DAT_10edbe08[i].node != 0)
            DAT_10edbe08[i].Draw(DAT_108392d8);
    }
    return 0;
}

