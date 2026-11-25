// Drawing the effect model slots (EN build). Ported from the matched JP source decomp/src/mesh_slots.cpp, where it is
// EffectModel::Draw run for each slot.
#include "effect_model.h"
#include "map_view.h"

// Own camera of a slot: 480 units back and 120 up, looking at the origin, a 45 degree perspective from 20 to 5000 units
// squeezed to 4:3.
// The own camera looks at the origin shifted by these amounts (the original added them, both 0).
const float OWN_CAMERA_TARGET_RAISE = 0.0f;
const float OWN_CAMERA_TARGET_SHIFT = 0.0f;
const double OWN_CAMERA_BACK = 480.0;
const double OWN_CAMERA_UP = 120.0;
const float OWN_CAMERA_NEAR = 20.0f;
const float OWN_CAMERA_FAR = 5000.0f;
const float OWN_CAMERA_FIELD_OF_VIEW = 0.785375f;
const float OWN_CAMERA_ASPECT = 1.333f;
const float NO_ASPECT_CHANGE = 1.0f;
// Pass mask meaning every pass.
const int MESH_SLOT_ALL_PASSES = -1;

// MeshSlots_DrawAll: queues the model of every slot for drawing once it is visible and its frame has reached the start
// frame: with the camera view, or with its own camera and an identity placement. Returns 0. Called every frame.
int MeshSlots_DrawAll(void)
{
    // Left over from the original: an identity placement that nothing reads any more.
    Matrix4 identity;
    identity.SetIdentity();
    for (int i = 0; i < EFFECT_MODEL_COUNT; i++)
    {
        EffectModel &slot = g_effectModels[i];
        if (slot.node != 0 && slot.visible != 0 && slot.frame >= slot.start)
        {
            if (slot.ownCamera != 0)
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
                target.z += OWN_CAMERA_TARGET_RAISE;
                target.x -= OWN_CAMERA_TARGET_SHIFT;
                eye.Copy(target);
                eye.z += OWN_CAMERA_UP;
                eye.y -= OWN_CAMERA_BACK;
                camera = LookAt(eye, target, &up, 0);
                projection = Multiply(ScaleMatrix(1.0f, OWN_CAMERA_ASPECT, 1.0f),
                                      PerspectiveMatrix(OWN_CAMERA_NEAR, OWN_CAMERA_FAR, OWN_CAMERA_FIELD_OF_VIEW,
                                                        NO_ASPECT_CHANGE));
                result = Multiply(projection, camera);
                slot.matrix.SetIdentity();
                slot.node->QueueDraw(&slot.matrix, &result, 0, MESH_SLOT_ALL_PASSES, &slot.color, 0);
            }
            else
                slot.node->QueueDraw(&slot.matrix, &g_cameraViewProjection, 0, MESH_SLOT_ALL_PASSES, &slot.color, 0);
        }
    }
    return 0;
}
