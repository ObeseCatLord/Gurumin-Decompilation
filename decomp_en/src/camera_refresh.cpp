// Per-frame camera refresh (EN build). Ported from the matched JP source decomp/src/camera_refresh.cpp.
#include "camera.h"
#include "camera_refresh.h"
#include "char.h"
#include "game_vars.h"
#include "map_view.h"
#include "sound_listener.h"
#include "switches.h"
#include "ui_anim.h"

// Frames the UI animation camera looks ahead or back of the animation frame.
const int UI_ANIM_STEP = 8;
// Pi as the game writes it, used to turn the roll table (degrees) into radians.
const float CAMERA_ROLL_PI = 3.1415f;
// Degrees in a half turn.
const float DEGREES_PER_HALF_TURN = 180.0f;
// Lowest distance of the sound listener from the camera in the normal mode.
const float SOUND_DISTANCE_MIN = -195.0f;
// Distance added to the sound distance (the listener sits this far in front of the camera in the normal mode).
const float SOUND_DISTANCE_OFFSET = 200.0f;
// Distance in front of the camera the listener sits at while sounds are muted (cut-scene mode).
const float SOUND_AHEAD = 5.0f;
// Scale from camera units to the sound world.
const float LISTENER_SCALE = 0.15f;
// Height added to the player's height for the camera height value.
const float CAMERA_HEIGHT_OFFSET = 50.0f;
// UI animation camera: eye and target in the frame of the animated node (looking forwards, looking backwards).
const float UI_EYE_DEPTH = 200.0f;
const float UI_TARGET_DEPTH = 600.0f;
const float UI_CAMERA_HEIGHT = 170.0f;
// Listener command arguments: volume and up-vector z.
const float LISTENER_VOLUME = 1.0f;
const float LISTENER_UP_Z = -1.0f;

// Rebuilds g_cameraView (the camera matrix) and its two view-projections. While a UI animation node plays
// (g_uiAnimStep = 1 and at least 8 frames in, or -1 and at least 8 before the end) the eye and target are placed
// relative to the node's frame matrix, one step (8 frames) behind or ahead; otherwise Camera_Update gives them and,
// in the muted-sound mode, the roll comes from the roll table. Then it publishes eye, target, the sound target, the
// player's position and its height plus 50, and the sound listener (volume 1, looking at the target, unless sound
// is muted). Returns 0. Called by the frame update.
int Camera_Refresh(void)
{
    Vec4 up;
    up.x = 0;
    up.y = 1.0f;
    up.z = 0;
    Vec4 eye;
    Vec4 target;
    if (g_uiAnimStep == 1 && g_uiAnimFrame > UI_ANIM_STEP - 1)
    {
        g_uiAnimFrame -= UI_ANIM_STEP;
        eye.Set(0, -UI_EYE_DEPTH, UI_CAMERA_HEIGHT, 0);
        target.Set(0, UI_TARGET_DEPTH, UI_CAMERA_HEIGHT, 0);
        Matrix4 relative = g_uiAnimNode->frameMatrices[g_uiAnimFrame > 0 ? g_uiAnimFrame : 0] *
                           InverseMatrix(g_uiAnimNode->frameMatrices[g_uiAnimFrame > g_uiAnimNode->frameMatrixCount - 1
                                                                         ? g_uiAnimNode->frameMatrixCount - 1
                                                                         : g_uiAnimFrame]);
        eye.Copy(Project(eye, relative));
        target.Copy(Project(target, relative));
        Matrix4 frame = g_uiAnimNode->frameMatrices[g_uiAnimFrame > 0 ? g_uiAnimFrame : 0];
        frame.m[MATRIX_TRANSLATION_X] = 0;
        frame.m[MATRIX_TRANSLATION_Y] = 0;
        frame.m[MATRIX_TRANSLATION_Z] = 0;
        frame.SetTranslation(g_uiAnimNode->frameMatrices[g_uiAnimFrame > 0 ? g_uiAnimFrame : 0].GetTranslation());
        Matrix4 frameInverse = InverseMatrix(frame);
        g_cameraView = LookAt(eye, target, &up, 0);
        g_cameraView = frameInverse * g_cameraView;
        eye.Copy(Project(eye, frame));
        target.Copy(Project(target, frame));
        g_cameraViewProjection = Multiply(g_projection, g_cameraView);
        g_cameraViewBase = Multiply(g_baseProjection, g_cameraView);
        g_uiAnimFrame += UI_ANIM_STEP;
    }
    else if (g_uiAnimStep == -1 && g_uiAnimFrame < g_uiAnimNode->frameMatrixCount - UI_ANIM_STEP)
    {
        g_uiAnimFrame += UI_ANIM_STEP;
        eye.Set(0, UI_EYE_DEPTH, UI_CAMERA_HEIGHT, 0);
        target.Set(0, -UI_TARGET_DEPTH, UI_CAMERA_HEIGHT, 0);
        Matrix4 relative = g_uiAnimNode->frameMatrices[g_uiAnimFrame > 0 ? g_uiAnimFrame : 0] *
                           InverseMatrix(g_uiAnimNode->frameMatrices[g_uiAnimFrame > g_uiAnimNode->frameMatrixCount - 1
                                                                         ? g_uiAnimNode->frameMatrixCount - 1
                                                                         : g_uiAnimFrame]);
        eye.Copy(Project(eye, relative));
        target.Copy(Project(target, relative));
        Matrix4 frame = g_uiAnimNode->frameMatrices[g_uiAnimFrame > 0 ? g_uiAnimFrame : 0];
        frame.m[MATRIX_TRANSLATION_X] = 0;
        frame.m[MATRIX_TRANSLATION_Y] = 0;
        frame.m[MATRIX_TRANSLATION_Z] = 0;
        frame.SetTranslation(g_uiAnimNode->frameMatrices[g_uiAnimFrame > 0 ? g_uiAnimFrame : 0].GetTranslation());
        Matrix4 frameInverse = InverseMatrix(frame);
        g_cameraView = LookAt(eye, target, &up, 0);
        g_cameraView = frameInverse * g_cameraView;
        eye.Copy(Project(eye, frame));
        target.Copy(Project(target, frame));
        g_cameraViewProjection = Multiply(g_projection, g_cameraView);
        g_cameraViewBase = Multiply(g_baseProjection, g_cameraView);
        g_uiAnimFrame -= UI_ANIM_STEP;
    }
    else
    {
        Camera_Update(&eye, &target);
        if (g_soundsMuted != 0)
            g_cameraView = LookAt(eye, target, &up, g_cameraRollTable[g_cameraRollIndex] * CAMERA_ROLL_PI / DEGREES_PER_HALF_TURN);
        else
            g_cameraView = LookAt(eye, target, &up, 0);
        g_cameraViewProjection = Multiply(g_projection, g_cameraView);
        g_cameraViewBase = Multiply(g_baseProjection, g_cameraView);
    }
    g_cameraEye.Copy(eye);
    g_cameraTarget.Copy(target);
    if (g_soundsMuted == 1)
    {
        if (g_cameraSoundTargetOverride != 0)
            g_cameraSoundTarget.Copy(g_cameraSoundTargetFixed);
        else
            g_cameraSoundTarget.Copy(target);
    }
    else
        g_cameraSoundTarget.Copy(target);
    g_cameraTargetMapPass.Copy(g_characterModels[0].matrix.GetTranslation());
    float height = g_characterModels[0].matrix.GetTranslation().z;
    g_cameraEyeHeight = height;
    g_cameraEyeHeight = g_cameraEyeHeight + CAMERA_HEIGHT_OFFSET;
    if (g_soundsMuted != 0)
    {
        Vec4 direction;
        direction.Copy(target - eye);
        direction.Normalized3();
        Vec4 position;
        float ahead = SOUND_AHEAD;
        position.Copy(eye + direction * ahead);
        if (g_soundMuted == 0)
            g_soundArcSetListener(LISTENER_VOLUME, position.x * LISTENER_SCALE, position.y * LISTENER_SCALE,
                                  position.z * LISTENER_SCALE, direction.x, direction.y, direction.z, 0, 0, LISTENER_UP_Z);
        g_listenerPosition.Set(position.x * LISTENER_SCALE, position.y * LISTENER_SCALE, position.z * LISTENER_SCALE, 0);
        g_listenerDirection.Set(direction.x, direction.y, direction.z, 0);
    }
    else
    {
        Vec4 direction;
        direction.Copy(target - eye);
        float distance = direction.Length();
        direction.Normalized3();
        Vec4 position;
        float listenerDistance;
        if (distance - g_cameraVectorSpare.Length() > SOUND_DISTANCE_MIN)
            listenerDistance = distance - g_cameraVectorSpare.Length();
        else
            listenerDistance = SOUND_DISTANCE_MIN;
        position.Copy(eye + direction * (listenerDistance + SOUND_DISTANCE_OFFSET));
        if (g_soundMuted == 0)
            g_soundArcSetListener(LISTENER_VOLUME, position.x * LISTENER_SCALE, position.y * LISTENER_SCALE,
                                  position.z * LISTENER_SCALE, direction.x, direction.y, direction.z, 0, 0, LISTENER_UP_Z);
        g_listenerPosition.Set(position.x * LISTENER_SCALE, position.y * LISTENER_SCALE, position.z * LISTENER_SCALE, 0);
        g_listenerDirection.Set(direction.x, direction.y, direction.z, 0);
    }
    return 0;
}
