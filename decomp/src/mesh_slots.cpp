// Attached mesh slots (see ai_pick.cpp for the slot table).

#include "vec4.h"

#include "matrix4.h"

Matrix4 LookAt(Vec4 eye, Vec4 target, const Vec4 *up, int a);                   // 0x101b1c20
Matrix4 Perspective(float nearPlane, float farPlane, float fieldOfView);          // 0x101b0c10
Matrix4 Scaling(float x, float y, float z);                                       // 0x101b13c0
Matrix4 Multiply(const Matrix4 &left, const Matrix4 &right);                        // 0x101b10f0

#include "node.h"

#include "mesh_slot.h"

extern "C" MeshSlot DAT_10edbe08[0x40];
extern "C" Matrix4 DAT_108392d8;

// 0x1000baf0 (MeshSlot::Draw). Queues the slot's model for drawing once it is visible and its delay has passed:
// with `view`, or with its own camera (480 units back and 120 up, looking at the origin, a 45 degree perspective from
// 20 to 5000 units squeezed to 4:3) and an identity placement. Returns 0 for a free slot, else 1.
int MeshSlot::Draw(const Matrix4 &view)
{
    if (mesh == 0)
        return 0;
    if (visible != 0 && frame >= delay)
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
            mesh->QueueDraw(&matrix, &result, 0, -1, &colour, 0);
        }
        else
            mesh->QueueDraw(&matrix, &view, 0, -1, &colour, 0);
    }
    return 1;
}

// Updates every used mesh slot with the world matrix DAT_108392d8. The identity matrix is unused.
int ResetMeshSlots()
{
    Matrix4 identity;
    identity.SetIdentity();
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10edbe08[i].mesh != 0)
            DAT_10edbe08[i].Draw(DAT_108392d8);
    }
    return 0;
}

