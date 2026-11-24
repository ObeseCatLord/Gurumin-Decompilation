// The scene camera refresh: builds the camera matrices from the camera the game placed or, while a UI animation node
// plays, from that node's frame matrices, then publishes the camera position and target, the sound target and the
// sound listener. C++ rather than extern "C": the function keeps an exception frame for its matrix temporaries.

#include "vec4.h"

#include "matrix4.h"
#include "node.h"
#include "anim_model.h"

#include "camera_refresh_vars.h"
#include "camera_vars.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "sound_vars.h"

#include "matrix_transform.h"
Matrix4 LookAtMatrix(Vec4 eye, Vec4 target, const Vec4 *up, float roll);     // 0x101b1c20
Matrix4 Multiply(const Matrix4 &left, const Matrix4 &right);                 // 0x101b10f0

#include "camera_update.h"
extern "C" int FUN_100017c0(float volume, float x, float y, float z, float dx, float dy, float dz, float ux, float uy,
                            float uz);

#define UI_ANIM_STEP 8                    // frames the UI animation camera looks ahead or back
#define CAMERA_ROLL_PI 3.1415f
#define SOUND_DISTANCE_MIN -195.0f        // lowest sound distance
#define SOUND_DISTANCE_OFFSET 200.0f
#define SOUND_AHEAD 5.0f                  // sound distance in front of the camera in mode 1
#define LISTENER_SCALE 0.15f
#define CAMERA_HEIGHT_OFFSET 50.0f

// 0x10015960 (Camera_Refresh). Rebuilds DAT_10836330 (the camera matrix) and DAT_108392d8/DAT_10836cd8 (its
// view-projections). While a UI animation node plays (DAT_104c01e0 = 1 and at least 8 frames in, or -1 and at least 8
// before the end) the eye and target are placed relative to the node's frame matrix, one step (8 frames) behind or
// ahead; otherwise Camera_Update gives them and, in mode DAT_104bfe6c, the roll comes from the roll table. Then it
// publishes eye, target, the sound target, the player's position and its height + 50, and the sound listener (volume 1,
// looking at the target, unless DAT_104c01b0 is set). Returns 0.
int Camera_Refresh(void)
{
    Vec4 up;
    up.x = 0;
    up.y = 1.0f;
    up.z = 0;
    Vec4 eye;
    Vec4 target;
    if (DAT_104c01e0 == 1 && DAT_104c01e4 > UI_ANIM_STEP - 1)
    {
        DAT_104c01e4 -= UI_ANIM_STEP;
        eye.Set(0, -200.0f, 170.0f, 0);
        target.Set(0, 600.0f, 170.0f, 0);
        Matrix4 relative = DAT_104c01dc->frameMatrices[DAT_104c01e4 > 0 ? DAT_104c01e4 : 0] *
                           InverseMatrix(DAT_104c01dc->frameMatrices[DAT_104c01e4 <= DAT_104c01dc->frameMatrixCount - 1
                                                                         ? DAT_104c01e4
                                                                         : DAT_104c01dc->frameMatrixCount - 1]);
        eye.Copy(Project(eye, relative));
        target.Copy(Project(target, relative));
        Matrix4 frame = DAT_104c01dc->frameMatrices[DAT_104c01e4 > 0 ? DAT_104c01e4 : 0];
        frame.m[MATRIX_TRANSLATION_X] = 0;
        frame.m[MATRIX_TRANSLATION_Y] = 0;
        frame.m[MATRIX_TRANSLATION_Z] = 0;
        frame.SetTranslation(DAT_104c01dc->frameMatrices[DAT_104c01e4 > 0 ? DAT_104c01e4 : 0].GetTranslation());
        Matrix4 frameInverse = InverseMatrix(frame);
        DAT_10836330 = LookAtMatrix(eye, target, &up, 0);
        DAT_10836330 = frameInverse * DAT_10836330;
        eye.Copy(Project(eye, frame));
        target.Copy(Project(target, frame));
        DAT_108392d8 = Multiply(DAT_10866da8, DAT_10836330);
        DAT_10836cd8 = Multiply(DAT_104c03f8, DAT_10836330);
        DAT_104c01e4 += UI_ANIM_STEP;
    }
    else if (DAT_104c01e0 == -1 && DAT_104c01e4 < DAT_104c01dc->frameMatrixCount - UI_ANIM_STEP)
    {
        DAT_104c01e4 += UI_ANIM_STEP;
        eye.Set(0, 200.0f, 170.0f, 0);
        target.Set(0, -600.0f, 170.0f, 0);
        Matrix4 relative = DAT_104c01dc->frameMatrices[DAT_104c01e4 > 0 ? DAT_104c01e4 : 0] *
                           InverseMatrix(DAT_104c01dc->frameMatrices[DAT_104c01e4 <= DAT_104c01dc->frameMatrixCount - 1
                                                                         ? DAT_104c01e4
                                                                         : DAT_104c01dc->frameMatrixCount - 1]);
        eye.Copy(Project(eye, relative));
        target.Copy(Project(target, relative));
        Matrix4 frame = DAT_104c01dc->frameMatrices[DAT_104c01e4 > 0 ? DAT_104c01e4 : 0];
        frame.m[MATRIX_TRANSLATION_X] = 0;
        frame.m[MATRIX_TRANSLATION_Y] = 0;
        frame.m[MATRIX_TRANSLATION_Z] = 0;
        frame.SetTranslation(DAT_104c01dc->frameMatrices[DAT_104c01e4 > 0 ? DAT_104c01e4 : 0].GetTranslation());
        Matrix4 frameInverse = InverseMatrix(frame);
        DAT_10836330 = LookAtMatrix(eye, target, &up, 0);
        DAT_10836330 = frameInverse * DAT_10836330;
        eye.Copy(Project(eye, frame));
        target.Copy(Project(target, frame));
        DAT_108392d8 = Multiply(DAT_10866da8, DAT_10836330);
        DAT_10836cd8 = Multiply(DAT_104c03f8, DAT_10836330);
        DAT_104c01e4 -= UI_ANIM_STEP;
    }
    else
    {
        Camera_Update(&eye, &target);
        if (DAT_104bfe6c != 0)
            DAT_10836330 = LookAtMatrix(eye, target, &up, DAT_10c68944[DAT_10c6897c] * CAMERA_ROLL_PI / 180.0f);
        else
            DAT_10836330 = LookAtMatrix(eye, target, &up, 0);
        DAT_108392d8 = Multiply(DAT_10866da8, DAT_10836330);
        DAT_10836cd8 = Multiply(DAT_104c03f8, DAT_10836330);
    }
    DAT_10f65688.Copy(eye);
    DAT_10f668e8.Copy(target);
    if (DAT_104bfe6c == 1)
    {
        if (DAT_10c6de2c != 0)
            DAT_10f66938.Copy(DAT_10c6de30);
        else
            DAT_10f66938.Copy(target);
    }
    else
        DAT_10f66938.Copy(target);
    DAT_10f66908.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    float height = DAT_10db6ee8[0].matrix.GetTranslation().z;
    DAT_10f65590 = height;
    DAT_10f65590 = DAT_10f65590 + CAMERA_HEIGHT_OFFSET;
    if (DAT_104bfe6c != 0)
    {
        Vec4 direction;
        direction.Copy(target - eye);
        direction.Normalized3();
        Vec4 position;
        float ahead = SOUND_AHEAD;
        position.Copy(eye + direction * ahead);
        if (DAT_104c01b0 == 0)
            FUN_100017c0(1.0f, position.x * LISTENER_SCALE, position.y * LISTENER_SCALE, position.z * LISTENER_SCALE,
                         direction.x, direction.y, direction.z, 0, 0, -1.0f);
        DAT_10c67f40.Set(position.x * LISTENER_SCALE, position.y * LISTENER_SCALE, position.z * LISTENER_SCALE, 0);
        DAT_10c67fe0.Set(direction.x, direction.y, direction.z, 0);
    }
    else
    {
        Vec4 direction;
        direction.Copy(target - eye);
        float distance = direction.Length();
        direction.Normalized3();
        Vec4 position;
        if (distance - DAT_118e6cd8.Length() > SOUND_DISTANCE_MIN)
            distance = distance - DAT_118e6cd8.Length();
        else
            distance = SOUND_DISTANCE_MIN;
        position.Copy(eye + direction * (distance + SOUND_DISTANCE_OFFSET));
        if (DAT_104c01b0 == 0)
            FUN_100017c0(1.0f, position.x * LISTENER_SCALE, position.y * LISTENER_SCALE, position.z * LISTENER_SCALE,
                         direction.x, direction.y, direction.z, 0, 0, -1.0f);
        DAT_10c67f40.Set(position.x * LISTENER_SCALE, position.y * LISTENER_SCALE, position.z * LISTENER_SCALE, 0);
        DAT_10c67fe0.Set(direction.x, direction.y, direction.z, 0);
    }
    return 0;
}
