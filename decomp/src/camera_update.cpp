// The per-frame game camera: picks the eye and the target for this frame from the active camera mode (a scripted
// camera on key banks, an effect camera that follows a character, a path camera, the free follow camera that
// swings round walls, or the event camera) and publishes them in the camera globals. C++ rather than extern "C":
// the function keeps an exception frame for the matrices of its event camera.

#include <math.h>
#include <string.h>

#include "vec4.h"

#include "matrix4.h"
#include "key_track.h"
#include "node.h"
#include "anim_model.h"
#include "camera_path_select.h"

#include "camera_update.h"
#include "camera_update_vars.h"
#include "camera_vars.h"
#include "char_vars.h"
#include "map_setup.h"
#include "map_vars.h"
#include "render_vars.h"

extern "C" float FUN_1016b6c0(void);                       // Camera_TrackPathFrame
extern "C" int FUN_1016be80(Vec4 *target, Vec4 *point);    // Camera_LiftOverGround
extern "C" int FUN_1015c9e0(int mode);                     // Screen_StartTransition
float SnapAngle(float radians, int step);                  // 0x1016c340

#define FRAME_COUNT_LIMIT 100
#define CUT_BLEND_FRAMES 60               // frames a camera cut blend runs
#define SCRIPT_KEYED 1
#define SCRIPT_EASED 2
#define MODE_EVENT_AT_END 5               // camera mode 5 also follows the player
#define EFFECT_CAMERA_HEIGHT 60.0f
#define EFFECT_CAMERA_BLEND 0.2f
#define PATH_TARGET_HEIGHT 70.0f
#define PATH_SECOND_TARGET_HEIGHT 50.0f
#define PATH_BLEND 0.5f
#define PATH_CUT_DOT 0.2
#define FOLLOW_BLEND 0.3f
#define FOLLOW_BACK 1050.0f               // how far behind the target the free camera stands
#define FOLLOW_HEIGHT_BASE 300.0f
#define FOLLOW_RAY_MASK 4
#define FOLLOW_NEAR_WALL 450.0f           // a wall nearer than this to the target pushes the camera round
#define FOLLOW_SNAP_DEGREES 15
#define FOLLOW_PEEK_STEP 12.0f
#define FOLLOW_PUSH_DECAY 5.0f
#define FOLLOW_PUSH_TURN 0.08f
#define FOLLOW_PUSH_PEEK 30.0f
#define FOLLOW_MIN_DISTANCE 400.0f
#define FOLLOW_MAX_DISTANCE 1400.0f
#define FOLLOW_HEIGHT_GAP 100.0
#define FOLLOW_CLEAR_RADIUS 230.0f
#define FOLLOW_LIFT_FACTOR 0.7f
#define FOLLOW_LIFT_MIN 0.01
#define FOLLOW_SEARCH_STEP 0.26179168f    // 15 degrees
#define FOLLOW_SEARCH_EASE 0.1f
#define FOLLOW_EYE_LIMIT_MARGIN 50.0f
#define FOLLOW_RETRY_PUSH_FRAMES 5
#define EVENT_FRAME_MARGIN 10
#define EVENT_EYE_LEFT_LIMIT -150.0f
#define PATH_EYE_SPEED_MIN 3.0f
#define PATH_EYE_SPEED_MAX 60.0f
#define PATH_EYE_SPEED_GROW 1.15f
#define PATH_EYE_SPEED_THRESHOLD 5.0f
#define PATH_EYE_BLEND_STEPS 4.0f
#define PATH_EYE_BLEND_REST 0.4f
#define PATH_EYE_TURN_LIMIT 1.57075
#define PATH_FAR_DISTANCE 300.0f
#define PATH_BLEND_STEPS 0xd
#define SHAKE_ZERO '5'
#define SHAKE_STEP 5

inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Reads the next entry of a shake string (the chapter scripts' camera bumps): the prefix letters L (restart),
// a/b/c (half, quarter, eighth) and A/B/C (double, 4x, 8x) set the scale, the next character is the height
// ('5' = 0, 5 units per step). Advances *position past the prefix and returns the scaled height; the caller
// steps past the height character.
static float ReadShake(const char *text, int *position)
{
    float scale = 1.0f;
    if (text[*position] == 'L')
        *position = 0;
    if (text[*position] == 'a')
    {
        scale = 0.5f;
        (*position)++;
    }
    if (text[*position] == 'b')
    {
        scale = 0.25f;
        (*position)++;
    }
    if (text[*position] == 'c')
    {
        scale = 0.125f;
        (*position)++;
    }
    if (text[*position] == 'A')
    {
        scale = 2.0f;
        (*position)++;
    }
    if (text[*position] == 'B')
    {
        scale = 4.0f;
        (*position)++;
    }
    if (text[*position] == 'C')
    {
        scale = 8.0f;
        (*position)++;
    }
    float height = (float)((text[*position] - SHAKE_ZERO) * SHAKE_STEP);
    return height * scale;
}

// Plays the shake strings of the four scripted camera slots: every slot advances one entry per frame, the slot
// in use (DAT_1101de34) lifts `eye` and `target` by its entry.
static void PlayScriptShakes(Vec4 *eye, Vec4 *target)
{
    for (int slot = 0; slot < CAMERA_SCRIPT_SLOTS; slot++)
    {
        const char *text = DAT_1101d9c0[slot];
        if (text[0] != 0 && (int)strlen(text) > DAT_1101ddc0[slot])
        {
            float height = ReadShake(text, &DAT_1101ddc0[slot]);
            if (slot == DAT_1101de34)
            {
                target->z += height;
                eye->z += height;
            }
            DAT_1101ddc0[slot]++;
        }
    }
}

// Advances the caption shake string (DAT_1101d8b8) by one entry and returns its height, or 0 when it has ended.
static float StepCaptionShake(void)
{
    if (DAT_1101d8b8[0] != 0 && DAT_1101d9b8 < (int)strlen(DAT_1101d8b8))
    {
        float height = ReadShake(DAT_1101d8b8, &DAT_1101d9b8);
        DAT_1101d9b8++;
        return height;
    }
    return 0;
}

// Publishes the scripted camera: eye and target as the camera, the smoothed pair and the sound/view copies.
static int PublishScripted(Vec4 *eye, Vec4 *target)
{
    DAT_11af3e28.Copy(*eye);
    DAT_118e6c78.Copy(*target);
    DAT_118e69e8.Copy(DAT_11af3e28);
    DAT_118e6cb8.Copy(DAT_118e6c78);
    DAT_10f668e8.Copy(*target);
    DAT_10f65688.Copy(*eye);
    return 1;
}

// The end shared by the path camera and the free camera: restores the previous camera when the mode says so,
// damps the smoothed pair towards the new eye and target (0.3 per frame, at once while a cut is flagged), adds the
// caption shake and publishes. Clears the cut flag.
static int FinishFollowCamera(Vec4 *eye, Vec4 *target, const Vec4 &previousEye, const Vec4 &previousTarget)
{
    if (DAT_1101de1c == 1)
        DAT_11af3e28.Copy(previousEye);
    if (DAT_1101de1c == 2)
    {
        DAT_11af3e28.Copy(previousEye);
        DAT_118e6c78.Copy(previousTarget);
    }
    DAT_11af3f58.Fill(0);
    if (DAT_11018734 != 0)
    {
        DAT_118e69e8.Copy(DAT_11af3e28);
        DAT_118e6cb8.Copy(DAT_118e6c78);
    }
    else
    {
        DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_11af3e28 - DAT_118e69e8) * FOLLOW_BLEND);
        DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_118e6c78 - DAT_118e6cb8) * FOLLOW_BLEND);
    }
    float shake = StepCaptionShake();
    DAT_118e6cb8.z += shake;
    DAT_118e69e8.z += shake;
    eye->Copy(DAT_118e69e8);
    target->Copy(DAT_118e6cb8);
    DAT_10f668e8.Copy(DAT_118e6cb8);
    DAT_10f65688.Copy(DAT_118e69e8);
    if (DAT_11018734 != 0)
        DAT_11018734 = 0;
    return 1;
}

// Starts the blend of a camera cut (DAT_1101d88c): remembers the camera of the frame before; when a cut was
// flagged the blend starts from the new camera instead.
static void StartCutBlend(const Vec4 &previousEye, const Vec4 &previousTarget, int cutFlagged)
{
    DAT_1101d88c = 1;
    DAT_11af3f08.Copy(previousEye);
    DAT_11af3f18.Copy(previousTarget);
    if (cutFlagged != 0)
    {
        DAT_11af3f08.Copy(DAT_11af3e28);
        DAT_11af3f18.Copy(DAT_118e6c78);
    }
}

// Moves the second path follow's eye one step towards `sample` (the path eye): the step is `fraction` of the way,
// its length is smoothed by DAT_1101872c (grows 15 per cent per frame, between 3 and 60, never past the distance).
// A turn of more than a quarter circle about the target starts a cut blend.
static void StepPathEye(const Vec4 &sample, float fraction, float heading, const Vec4 &previousEye,
                        const Vec4 &previousTarget, int cutFlagged)
{
    Vec4 move;
    move.Copy((sample - DAT_11af3e28) * fraction);
    float moveLength = move.Length();
    Vec4 look = sample - DAT_118e6c78;
    if (fabs(Atan2(look.x, look.y) - heading) > PATH_EYE_TURN_LIMIT && DAT_1101d88c == 0)
        StartCutBlend(previousEye, previousTarget, cutFlagged);
    if (moveLength > PATH_EYE_SPEED_THRESHOLD)
    {
        if (DAT_1101872c < PATH_EYE_SPEED_MIN)
            DAT_1101872c = PATH_EYE_SPEED_MIN;
        if (moveLength <= DAT_1101872c)
            DAT_1101872c = moveLength;
        else
        {
            DAT_1101872c = DAT_1101872c * PATH_EYE_SPEED_GROW;
            if (moveLength < DAT_1101872c)
                DAT_1101872c = moveLength;
            if (DAT_1101872c > PATH_EYE_SPEED_MAX)
                DAT_1101872c = PATH_EYE_SPEED_MAX;
        }
        move.Copy(Vec3_Normalized(&move));
        move.Copy(move * DAT_1101872c);
    }
    DAT_11af3e28.Copy(DAT_11af3e28 + move);
}

// 0x1016c3e0 (Camera_Update). Called by Camera_Refresh every frame. Modes, in the order they are tried:
// a scripted camera (DAT_1101ddf4: slot DAT_1101de34 samples its eye and target key banks at the slot frame, or at
// a frame eased by pow() between the first and last key), the effect camera (DAT_10ff1df0: moves 20 per cent
// towards the effect character and the effect eye), the path camera (DAT_1101ddf0 = 1, DAT_1101d890: eye on the
// path track, target on the player, 0.5 blend), the free follow camera (DAT_1101ddf0 = 1 otherwise: stands 1050
// behind the player, peeks round walls, searches the 15-degree headings for a clear one, lifts over uneven ground),
// the second path follow (mode 0 and DAT_1101ddf0 = 0 with camera paths: eye and target samples, 0.4 steps) and
// the event camera (the other modes: samples the event key banks, or the eye and target nodes of a character at
// the last frame). Returns 1 when it placed a camera, 0 for the second path follow's no-path case and the event
// camera.
int Camera_Update(Vec4 *eye, Vec4 *target)
{
    if (DAT_1101d8b0 < FRAME_COUNT_LIMIT)
        DAT_1101d8b0++;
    Vec4 previousEye = DAT_11af3e28;
    int cutFlagged = DAT_11018734;
    Vec4 previousTarget = DAT_118e6c78;
    CameraTarget *camera = DAT_1101d884;
    DAT_10203440 = 1;
    Vec4 toEye;
    toEye.Copy(DAT_11af3e28 - DAT_118e6c78);
    float heading = Atan2(toEye.x, toEye.y);
    if (DAT_1101d88c != 0 && ++DAT_1101d88c > CUT_BLEND_FRAMES - 1)
        DAT_1101d88c = 0;

    if (DAT_1101ddf4 != 0)
    {
        int slot = DAT_1101de34;
        if (DAT_1101ddf8[slot] == SCRIPT_KEYED)
        {
            Vec4 sample;
            DAT_118e6ce8[slot].Sample(CameraScriptFrame(slot), &sample.x, &sample.y, &sample.z, &sample.w, 0);
            eye->Copy(sample);
            DAT_111322b8[slot].Sample(CameraScriptFrame(slot), &sample.x, &sample.y, &sample.z, &sample.w, 0);
            target->Copy(sample);
            PlayScriptShakes(eye, target);
            return PublishScripted(eye, target);
        }
        if (DAT_1101ddf8[slot] == SCRIPT_EASED)
        {
            KeyBank &eyeKeys = DAT_118e6ce8[slot];
            KeyBank &targetKeys = DAT_111322b8[slot];
            int eyeFirst = eyeKeys.frames[0];
            int eyeLast = eyeKeys.frames[eyeKeys.count - 1];
            float eyeProgress = (float)(CameraScriptFrame(slot) - eyeFirst) / (float)(eyeLast - eyeFirst);
            float eyeFrame = (float)pow(eyeProgress, CameraScriptExponent(slot)) * (float)(eyeLast - eyeFirst) + eyeFirst;
            int targetFirst = targetKeys.frames[0];
            int targetLast = targetKeys.frames[targetKeys.count - 1];
            float targetProgress = (float)(CameraScriptFrame(slot) - targetFirst) / (float)(targetLast - targetFirst);
            float targetFrame =
                (float)pow(targetProgress, DAT_10203464[slot]) * (float)(targetLast - targetFirst) + targetFirst;
            Vec4 sample;
            eyeKeys.Sample(eyeFrame, &sample.x, &sample.y, &sample.z, &sample.w, 0);
            eye->Copy(sample);
            targetKeys.Sample(targetFrame, &sample.x, &sample.y, &sample.z, &sample.w, 0);
            target->Copy(sample);
            PlayScriptShakes(eye, target);
            return PublishScripted(eye, target);
        }
    }

    if (DAT_1101de24 == 0 && DAT_10ff1df0 != 0)
    {
        Vec4 effectPosition = DAT_10db6ee8[DAT_11018730].matrix.GetTranslation();
        effectPosition.z += EFFECT_CAMERA_HEIGHT;
        DAT_118e6c78.Copy(DAT_118e6c78 + (effectPosition - DAT_118e6c78) * EFFECT_CAMERA_BLEND);
        DAT_11af3e28.Copy(DAT_11af3e28 + (DAT_11af3f48 - DAT_11af3e28) * EFFECT_CAMERA_BLEND);
        DAT_118e69e8.Copy(DAT_11af3e28);
        DAT_118e6cb8.Copy(DAT_118e6c78);
        eye->Copy(DAT_11af3e28);
        target->Copy(DAT_118e6c78);
        DAT_10f668e8.Copy(*target);
        DAT_10f65688.Copy(*eye);
        return 1;
    }

    if (DAT_1101de24 == 0 || DAT_1101de24 == MODE_EVENT_AT_END)
    {
        if (DAT_1101ddf0 == 1)
        {
            Vec4 pathEye;
            Vec4 pathEyeBack;
            Vec4 playerTop;
            playerTop.Copy(camera->world.GetTranslation());
            playerTop.z += PATH_TARGET_HEIGHT;
            if (DAT_1101d890 != 0)
            {
                DAT_1101d8a0 = 1;
                FUN_1016b6c0();
                DAT_11af3928[DAT_1101d8a4].Sample(DAT_1101c800, &pathEye.x, &pathEye.y, &pathEye.z, &pathEye.w, 0);
                if (DAT_10fdeb10[DAT_1101d8a4] != 0)
                {
                    DAT_118e96f8[DAT_1101d8a4].Sample(DAT_1101c800, &pathEyeBack.x, &pathEyeBack.y, &pathEyeBack.z,
                                                      &pathEyeBack.w, 0);
                    Vec4 lookBefore = previousTarget - previousEye;
                    lookBefore.Normalized3();
                    Vec4 lookFront = DAT_118e6c78 - pathEye;
                    lookFront.w = 0;
                    lookFront.Normalized3();
                    Vec4 lookBack = DAT_118e6c78 - pathEyeBack;
                    lookBack.w = 0;
                    lookBack.Normalized3();
                    if (DAT_1101d894 == 2)
                        pathEye.Copy(pathEyeBack);
                    else if (DAT_1101d894 != 1)
                    {
                        float frontDot = Vec3_Dot(&lookBefore, &lookFront);
                        if (Vec3_Dot(&lookBefore, &lookBack) <= frontDot)
                            DAT_1101d894 = 1;
                        else
                        {
                            DAT_1101d894 = 2;
                            pathEye.Copy(pathEyeBack);
                        }
                    }
                }
                DAT_11af3e28.Copy(pathEye);
                DAT_118e6c78.Copy(playerTop);
                if (DAT_10faa678 != 0)
                {
                    DAT_10faa678 = 0;
                    Vec4 lookNow;
                    lookNow.Fill(0);
                    lookNow.x = DAT_118e6c78.x - DAT_11af3e28.x;
                    lookNow.y = DAT_118e6c78.y - DAT_11af3e28.y;
                    Vec4 lookBefore;
                    lookBefore.Fill(0);
                    lookBefore.x = previousTarget.x - previousEye.x;
                    lookBefore.y = previousTarget.y - previousEye.y;
                    lookNow.Normalized3();
                    lookBefore.Normalized3();
                    if (Vec3_Dot(&lookNow, &lookBefore) < PATH_CUT_DOT)
                    {
                        DAT_1101d88c = 1;
                        DAT_11af3f08.Copy(previousEye);
                        DAT_11af3f18.Copy(previousTarget);
                    }
                }
                if (DAT_11018734 != 0)
                {
                    DAT_11af3e28.Copy(pathEye);
                    DAT_118e6c78.Copy(playerTop);
                }
                else
                {
                    DAT_11af3e28.Copy(previousEye + (pathEye - previousEye) * PATH_BLEND);
                    DAT_118e6c78.Copy(previousTarget + (playerTop - previousTarget) * PATH_BLEND);
                }
                return FinishFollowCamera(eye, target, previousEye, previousTarget);
            }

            // the free follow camera
            bool retried = false;
            if (DAT_1101de30 == 0)
            {
                Vec4 hit;
                Vec4 candidate;
                candidate.Copy(DAT_11af3e28 - DAT_118e6c78);
                float peek = (DAT_10203448 > 1.0f ? 1.0f : DAT_10203448);
                candidate.RotateZ(peek * 3.1415f / FOLLOW_PEEK_STEP);
                candidate.Copy(candidate + DAT_118e6c78);
                if (camera->collision->CastSegment("CA", FOLLOW_RAY_MASK, &candidate, &DAT_118e6c78, &hit, 0) != 0 &&
                    hit.Distance(DAT_118e6c78) < FOLLOW_NEAR_WALL)
                {
                    candidate.Copy(DAT_11af3e28 - DAT_118e6c78);
                    peek = (DAT_10203448 > 1.0f ? 1.0f : DAT_10203448);
                    candidate.RotateZ(peek * -3.1415f / FOLLOW_PEEK_STEP);
                    candidate.Copy(candidate + DAT_118e6c78);
                    if (camera->collision->CastSegment("CA", FOLLOW_RAY_MASK, &candidate, &DAT_118e6c78, &hit, 0) == 0 ||
                        hit.Distance(DAT_118e6c78) >= FOLLOW_NEAR_WALL)
                    {
                        DAT_1101de38 = 1;
                        peek = (DAT_10203448 > 1.0f ? 1.0f : DAT_10203448);
                        DAT_11af3f58.x = peek * FOLLOW_PUSH_PEEK + DAT_11af3f58.x;
                    }
                }
                else
                {
                    candidate.Copy(DAT_11af3e28 - DAT_118e6c78);
                    peek = (DAT_10203448 > 1.0f ? 1.0f : DAT_10203448);
                    candidate.RotateZ(peek * -3.1415f / FOLLOW_PEEK_STEP);
                    candidate.Copy(candidate + DAT_118e6c78);
                    if (camera->collision->CastSegment("CA", FOLLOW_RAY_MASK, &candidate, &DAT_118e6c78, &hit, 0) != 0 &&
                        hit.Distance(DAT_118e6c78) < FOLLOW_NEAR_WALL)
                    {
                        DAT_1101de38 = 1;
                        peek = (DAT_10203448 > 1.0f ? 1.0f : DAT_10203448);
                        DAT_11af3f58.x = DAT_11af3f58.x - peek * FOLLOW_PUSH_PEEK;
                    }
                }
            }
            for (;;)
            {
                if (DAT_1101de20 != 0)
                {
                    DAT_11af3f58.x = 0;
                    DAT_11af3f58.y = 0;
                    DAT_1101de20 = 0;
                }
                DAT_118e6cc8.Copy(DAT_11af3e28);
                DAT_11af3e68.Copy(DAT_118e6c78);
                DAT_118e6cd8.Fill(0);
                DAT_118e6cd8.y = FOLLOW_BACK;
                DAT_118e6cd8.z = DAT_1101dddc - FOLLOW_HEIGHT_BASE;
                Vec4 heading2;
                heading2.Copy(DAT_11af3e28 - DAT_118e6c78);
                if (DAT_10fabe8c != 0 && DAT_11af3f58.x > 0)
                    DAT_11af3f58.x = 0;
                if (DAT_10faaa80 != 0 && DAT_11af3f58.x < 0)
                    DAT_11af3f58.x = 0;
                if (DAT_10fabe8c != 0)
                    DAT_10fabe8c--;
                if (DAT_10faaa80 != 0)
                    DAT_10faaa80--;
                float searchStep = FOLLOW_SEARCH_STEP;
                if (DAT_11af3f58.x < 0)
                    searchStep = -FOLLOW_SEARCH_STEP;
                if (DAT_11af3f58.x < 0)
                {
                    DAT_11af3f58.x = DAT_11af3f58.x + FOLLOW_PUSH_DECAY;
                    if (DAT_11af3f58.x > 0)
                        DAT_11af3f58.x = 0;
                }
                else if (DAT_11af3f58.x > 0)
                {
                    DAT_11af3f58.x = DAT_11af3f58.x - FOLLOW_PUSH_DECAY;
                    if (DAT_11af3f58.x < 0)
                        DAT_11af3f58.x = 0;
                }
                float angle;
                DAT_118e6cd8.RotateX(DAT_1101de18);
                if (DAT_11af3f58.x != 0)
                {
                    float length = (float)sqrt(DAT_11af3f58.x * DAT_11af3f58.x + DAT_11af3f58.y * DAT_11af3f58.y);
                    float ratio = DAT_11af3f58.x / length;
                    angle = Atan2(heading2.x, -heading2.y) - ratio * FOLLOW_PUSH_TURN;
                }
                else
                    angle = Atan2(heading2.x, -heading2.y);
                DAT_118e6cd8.RotateZ(angle);
                float distance = (playerTop - previousEye).Length2D();
                Vec4 ideal = playerTop - DAT_118e6cd8 * DAT_1020344c;
                Vec4 eyeGoal;
                if (DAT_11018734 != 0)
                    eyeGoal.Copy(ideal);
                else
                {
                    float heightGap = (float)fabs(ideal.z - previousEye.z);
                    if (heightGap > FOLLOW_HEIGHT_GAP || distance < FOLLOW_MIN_DISTANCE ||
                        distance > FOLLOW_MAX_DISTANCE)
                    {
                        DAT_1101de38 = 1;
                        eyeGoal.Copy(ideal * 0.3f + previousEye * 0.7f);
                    }
                    else if (DAT_1101de38 == 0)
                        eyeGoal.Copy(ideal * 0.05f + previousEye * 0.95f);
                    else
                    {
                        eyeGoal.Copy(ideal * 0.4f + previousEye * 0.6f);
                        if (eyeGoal.Distance(ideal) < 10.0f)
                            DAT_1101de38 = 0;
                    }
                }
                DAT_1101d894 = 0;
                DAT_11af3e28.Copy(eyeGoal);
                DAT_118e6c78.Copy(playerTop);
                FUN_1016be80(&DAT_11af3e28, &DAT_118e6c78);
                if (DAT_1101de18 != 0)
                {
                    float lift = DAT_1101de18 * FOLLOW_LIFT_FACTOR;
                    Vec4 offset;
                    offset.Fill(0);
                    offset.y = FOLLOW_BACK;
                    offset.z = DAT_1101dddc - FOLLOW_HEIGHT_BASE;
                    offset.RotateX(lift);
                    offset.RotateZ(Atan2(heading2.x, -heading2.y));
                    Vec4 eyeLifted = playerTop - offset * DAT_1020344c;
                    FUN_1016be80(&eyeLifted, &playerTop);
                    Vec4 hitLift;
                    if (camera->collision->CastSegment("CA", FOLLOW_RAY_MASK, &eyeLifted, &playerTop, &hitLift, 0) == 0 ||
                        hitLift.Distance(playerTop) > FOLLOW_CLEAR_RADIUS)
                    {
                        DAT_1101de18 = lift;
                        if (lift < FOLLOW_LIFT_MIN)
                            DAT_1101de18 = 0;
                    }
                }
                if (DAT_1101de30 == 0)
                {
                    DAT_10203448 = (float)(DAT_10203448 + 0.01);
                    if (DAT_10203448 > 1.0)
                        DAT_10203448 = 1.0f;
                }
                else
                    DAT_10203448 = 0;
                Node *collision = camera->collision;
                if (collision == 0)
                    break;

                Vec4 hit;
                int blocked = collision->CastSegment("CA", FOLLOW_RAY_MASK, &DAT_11af3e28, &DAT_118e6c78, &hit, 0);
                if (blocked == 0)
                {
                    DAT_1101de3c = 0;
                    DAT_11af3ef8.Copy(DAT_11af3f58);
                    if (DAT_1101de28 != 0 &&
                        (DAT_118e6c78.InRect(&DAT_10f668a8) != 0 || DAT_11af3e28.InRect(&DAT_10f668a8) != 0))
                    {
                        float limit = DAT_10f668a8.max.z + FOLLOW_EYE_LIMIT_MARGIN;
                        if (DAT_11af3e28.z < limit)
                            DAT_11af3e28.z = limit;
                    }
                    break;
                }

                Vec4 wallEye = hit;
                int searching = 0;
                if (DAT_1101de30 == 0)
                {
                    float wallDistance = hit.Distance(DAT_118e6c78);
                    if (wallDistance < FOLLOW_NEAR_WALL)
                    {
                        searching = 4;
                        if (DAT_10203448 < 0.9)
                        {
                            if (wallDistance >= 700.0f || DAT_10203448 <= 1.0)
                            {
                                if (DAT_10203448 > 10.0f)
                                    DAT_10203448 = 10.0f;
                            }
                            else
                                DAT_10203448 = 1.0f;
                        }
                        else if (wallDistance >= 300.0f)
                        {
                            if (wallDistance >= 350.0f)
                            {
                                if (wallDistance < 400.0f)
                                    DAT_10203448 = 2.0f;
                            }
                            else
                                DAT_10203448 = 5.0f;
                        }
                        else
                        {
                            searching = 8;
                            DAT_10203448 = 10.0f;
                        }
                        hit.Copy(DAT_11af3e28);
                    }
                }
                int found = 0;
                int moved = 0;
                if (DAT_1101de30 == 0 && searching != 0)
                {
                    float snapped = SnapAngle(Atan2(heading2.x, -heading2.y), FOLLOW_SNAP_DEGREES);
                    int tries = searching * 2 + 4;
                    int foundStep = 0;
                    Vec4 trial;
                    Vec4 trialHit;
                    for (int step = 0; step < tries; step++)
                    {
                        int half = step / 2;
                        float sideSign = (step % 2 == 0) ? 1.0f : -1.0f;
                        float stepAngle;
                        if (step < 16)
                            stepAngle = snapped - sideSign * searchStep * (float)(half + 1) * DAT_10203448 * FOLLOW_SEARCH_EASE;
                        else
                            stepAngle = SnapAngle(snapped - sideSign * (float)half * searchStep, FOLLOW_SNAP_DEGREES);
                        trial.Fill(0);
                        trial.y = FOLLOW_BACK;
                        trial.z = DAT_1101dddc - FOLLOW_HEIGHT_BASE;
                        trial.RotateX(DAT_1101de18);
                        trial.RotateZ(stepAngle);
                        Vec4 trialEye = playerTop - trial * DAT_1020344c;
                        FUN_1016be80(&trialEye, &playerTop);
                        if (camera->collision->CastSegment("CA", FOLLOW_RAY_MASK, &trialEye, &playerTop, &trialHit, 0) == 0)
                        {
                            DAT_1101de2c = 1;
                            DAT_11af3e28.Copy(trialEye);
                            DAT_118e6c78.Copy(playerTop);
                            found = 1;
                            moved = 1;
                            foundStep = step;
                            break;
                        }
                    }
                    if (found != 0 && foundStep > 2 && DAT_1101d88c == 0)
                        StartCutBlend(previousEye, previousTarget, cutFlagged);
                }
                if (found == 0)
                    DAT_11af3e28.Copy(wallEye);
                bool retry = false;
                if (!retried)
                {
                    if (DAT_1101de3c == 0 || DAT_118e6cc8.Distance(DAT_11af3e68) > DAT_11af3e28.Distance(DAT_118e6c78))
                    {
                        if (moved != 0 && DAT_11af3ef8.x != DAT_11af3f58.x)
                        {
                            retry = true;
                            retried = true;
                            DAT_11af3e28.Copy(playerTop + (DAT_118e6cc8 - DAT_11af3e68));
                            DAT_118e6c78.Copy(playerTop);
                            if (DAT_11af3f58.x > 0)
                                DAT_10fabe8c = FOLLOW_RETRY_PUSH_FRAMES;
                            if (DAT_11af3f58.x < 0)
                                DAT_10faaa80 = FOLLOW_RETRY_PUSH_FRAMES;
                        }
                    }
                }
                DAT_1101de3c = moved;
                if (!retry)
                {
                    if (found != 0)
                        camera->collision->CastSegment("CA", FOLLOW_RAY_MASK, &DAT_11af3e28, &DAT_118e6c78, &hit, 0);
                    hit.Distance(DAT_118e6c78);
                    if (DAT_1101de28 != 0 &&
                        (DAT_118e6c78.InRect(&DAT_10f668a8) != 0 || DAT_11af3e28.InRect(&DAT_10f668a8) != 0))
                    {
                        float limit = DAT_10f668a8.max.z + FOLLOW_EYE_LIMIT_MARGIN;
                        if (DAT_11af3e28.z < limit)
                            DAT_11af3e28.z = limit;
                    }
                    break;
                }
            }
            return FinishFollowCamera(eye, target, previousEye, previousTarget);
        }

        if (DAT_1101de24 == 0 && DAT_1101ddf0 == 0)
        {
            if (DAT_1101d89c == 0)
                return 0;
            Matrix4 pathFrame;
            Vec4 sample;
            Vec4 sampleBack;
            Vec4 playerTop;
            FUN_1016b6c0();
            playerTop.Copy(camera->world.GetTranslation());
            playerTop.z += PATH_SECOND_TARGET_HEIGHT;
            if (DAT_11018628[DAT_1101d8a4] == 0)
            {
                DAT_10fabe88 = 0;
                DAT_11018738 = DAT_1101d8a4;
                DAT_118e6c78.Copy(playerTop);
            }
            else
            {
                if (DAT_11018738 != -1 && DAT_11018738 != DAT_1101d8a4)
                    DAT_10fabe88 = PATH_BLEND_STEPS;
                DAT_11018738 = DAT_1101d8a4;
                DAT_11af3ba8[DAT_1101d8a4].Sample(DAT_1101c800, &playerTop.x, &playerTop.y, &playerTop.z, &playerTop.w, 0);
                if (DAT_10fabe88 != 0)
                {
                    DAT_118e6c78.Copy(DAT_118e6c78 + (playerTop - DAT_118e6c78).Divided((float)DAT_10fabe88));
                    DAT_10fabe88--;
                }
                else
                    DAT_118e6c78.Copy(playerTop);
            }
            DAT_11af3928[DAT_1101d8a4].Sample(DAT_1101c800, &sample.x, &sample.y, &sample.z, &sample.w, 0);
            if (DAT_10fdeb10[DAT_1101d8a4] != 0)
            {
                DAT_118e96f8[DAT_1101d8a4].Sample(DAT_1101c800, &sampleBack.x, &sampleBack.y, &sampleBack.z,
                                                  &sampleBack.w, 0);
                Vec4 lookBefore = previousTarget - previousEye;
                lookBefore.Normalized3();
                Vec4 lookFront = DAT_118e6c78 - sample;
                lookFront.w = 0;
                lookFront.Normalized3();
                Vec4 lookBack = DAT_118e6c78 - sampleBack;
                lookBack.w = 0;
                lookBack.Normalized3();
                float frontDot = Vec3_Dot(&lookBefore, &lookFront);
                if (Vec3_Dot(&lookBefore, &lookBack) > frontDot)
                    sample.Copy(sampleBack);
            }
            if (DAT_1101d8a8 < 1)
            {
                if (DAT_11018734 == 0)
                    StepPathEye(sample, PATH_EYE_BLEND_REST, heading, previousEye, previousTarget, cutFlagged);
                else
                    DAT_10fabe88 = 0;
            }
            else
            {
                if (DAT_11018734 == 0)
                    StepPathEye(sample, PATH_EYE_BLEND_STEPS / (float)DAT_1101d8a8, heading, previousEye,
                                previousTarget, cutFlagged);
                else
                    DAT_10fabe88 = 0;
                DAT_1101d8a8--;
            }
            float shake = StepCaptionShake();
            DAT_118e6c78.z += shake;
            DAT_11af3e28.z += shake;
            Vec4 down;
            down.Set(0, 0, -1.0f, 0);
            Vec4 lookDown = DAT_11af3e28 - DAT_118e6c78;
            lookDown.Normalized3();
            Vec3_Dot(&lookDown, &down);
            if (DAT_11018734 == 0)
            {
                DAT_11af3e28.Copy(previousEye + (DAT_11af3e28 - previousEye) * 0.5f);
                DAT_118e6c78.Copy(previousTarget + (DAT_118e6c78 - previousTarget) * 0.5f);
            }
            else
            {
                if (DAT_11af3e28.Distance(DAT_118e6c78) > PATH_FAR_DISTANCE)
                {
                    if (DAT_1101d8b0 > 1 && DAT_10f654c0 == 0 && DAT_10203440 != 0)
                        FUN_1015c9e0(0xf);
                    if (DAT_1101d88c == 0)
                        StartCutBlend(previousEye, previousTarget, cutFlagged);
                }
                DAT_11018734 = 0;
            }
            float captionShake = StepCaptionShake();
            target->z += captionShake;
            eye->z += captionShake;
            DAT_118e69e8.Copy(DAT_11af3e28);
            DAT_118e6cb8.Copy(DAT_118e6c78);
            eye->Copy(DAT_11af3e28);
            target->Copy(DAT_118e6c78);
            DAT_10f668e8.Copy(*target);
            DAT_10f65688.Copy(*eye);
            return 1;
        }
    }

    // the event camera
    DAT_10fabe88 = 0;
    float spare;
    if (DAT_1101ddd4 == 0 || DAT_11018728 != DAT_10ff2ff4)
    {
        DAT_11af2420.Sample(DAT_11018728, &DAT_118e69e8.x, &DAT_118e69e8.y, &DAT_118e69e8.z, &spare, 0);
        DAT_11af2ea4.Sample(DAT_11018728, &DAT_118e6cb8.x, &DAT_118e6cb8.y, &DAT_118e6cb8.z, &spare, 0);
    }
    else if (DAT_1101ddd4 == 1)
    {
        AnimModel &actor = DAT_10db6ee8[DAT_1101ddd8];
        if (actor.targetNode->frameMatrixCount != 0)
        {
            Node *node = actor.targetNode;
            int frame = actor.trackA.frame;
            if (node->frameMatrixCount - EVENT_FRAME_MARGIN <= frame)
                frame = node->frameMatrixCount - EVENT_FRAME_MARGIN;
            Matrix4 frameMatrix = node->frameMatrices[frame];
            Vec4 offset;
            offset.Copy(frameMatrix.GetTranslation());
            offset.RotateZ(-actor.heading);
            DAT_118e6cb8.Copy(offset + actor.matrix.GetTranslation());
            DAT_118e6cb8.z += actor.m_2494;
        }
        if (actor.eyeNode->frameMatrixCount != 0)
        {
            Node *node = actor.eyeNode;
            int frame = actor.trackA.frame;
            if (node->frameMatrixCount - EVENT_FRAME_MARGIN <= frame)
                frame = node->frameMatrixCount - EVENT_FRAME_MARGIN;
            Matrix4 frameMatrix = node->frameMatrices[frame];
            Vec4 offset;
            offset.Copy(frameMatrix.GetTranslation());
            offset.RotateZ(-actor.heading);
            DAT_118e69e8.Copy(offset + actor.matrix.GetTranslation());
            if (DAT_118e69e8.x < EVENT_EYE_LEFT_LIMIT)
                DAT_118e69e8.x = EVENT_EYE_LEFT_LIMIT;
            DAT_118e69e8.z += actor.m_2494;
        }
    }
    if (DAT_11018728 < DAT_10ff2ff4)
        DAT_11018728++;
    DAT_11af3e28.Copy(DAT_118e69e8);
    DAT_118e6c78.Copy(DAT_118e6cb8);
    eye->Copy(DAT_11af3e28);
    target->Copy(DAT_118e6c78);
    DAT_10f668e8.Copy(DAT_118e6c78);
    DAT_10f65688.Copy(DAT_11af3e28);
    return 0;
}
