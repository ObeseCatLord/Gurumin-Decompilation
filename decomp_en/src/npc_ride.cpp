// An NPC that rides behind a marker character (EN build; JP: decomp/src/npc_ride.cpp). It keeps the distance it
// started at behind the nearest marker character and follows the marker's heading.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_shots.h"
#include "engine.h"
#include "node.h"
#include "npc_hurl.h"
#include "npc_ride.h"
#include "vec4.h"

// CharState::values and CharState fields the rider keeps its state in.
#define RIDE_STATE_START 0
#define RIDE_STATE_FOLLOW 1
#define RIDE_MARKER_KIND ','              // kind of the characters it rides behind
#define RIDE_MARKER_FIRST 0x5c            // slots 0x5c..0x7b are searched for a marker
#define RIDE_MARKER_END 0x7c
#define RIDE_NO_MARKER (-1)
#define RIDE_FAR_DISTANCE 99999.0f        // start value of the nearest-marker search
#define RIDE_GONE_X (-20000.0f)           // a marker moved below this x is removed
#define RIDE_MAX_STEP 12.0f               // most it moves in one frame
#define RIDE_MARK_VALUE 30                // value written to its mark every frame
#define RIDE_TURN_OFFSET 3.1415f
#define RIDE_FULL_TURN 6.283f
#define RIDE_CUT_ANGLE 0.7854f            // within 45 degrees of the marker's way it may cut the turn
#define RIDE_CUT_TURN 0.17453f            // 10 degrees
#define RIDE_SETTLED_ANGLE 0.01756f       // about one degree
#define RIDE_SETTLED_DISTANCE_SQUARED 4.0f
#define RIDE_MOTION_IDLE 2
#define RIDE_MOTION_WALK 0
#define RIDE_MOTION_BLEND 5

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Npc_RideBehindMarker. State 0 plays the idle motion and picks the nearest marker character (kind ',' with a model,
// slots 0x5c..0x7b), remembering its distance in the shot parameters; with none the state is reset. State 1 moves at
// most 12 units a frame to the place that distance behind the marker (cutting 10 degrees inside a turn when it lags
// within 45 degrees of the marker's way and is farther from the marker than its place), faces the marker's heading
// (motion 2 when settled, else motion 0) and is reset once the marker is removed. Returns 0.
int Npc_RideBehindMarker(int index)
{
    Vec4 playerPosition;
    Vec4 selfPosition;
    playerPosition.Copy(g_characterModels[0].matrix.GetTranslation());
    selfPosition.Copy(g_characterModels[index].matrix.GetTranslation());
    (playerPosition - selfPosition).Length();
    (playerPosition - selfPosition).Length2D();
    g_charHurlMark[index] = RIDE_MARK_VALUE;
    if (g_charState[index].state == RIDE_STATE_START)
    {
        g_characterModels[index].PlayMotion(RIDE_MOTION_IDLE, 0, RIDE_MOTION_BLEND, 1.0f);
        g_charState[index].state = RIDE_STATE_FOLLOW;
        float nearestSquared = RIDE_FAR_DISTANCE;
        int nearest = RIDE_NO_MARKER;
        Vec4 markerPosition;
        for (int i = RIDE_MARKER_FIRST; i < RIDE_MARKER_END; i++)
        {
            if (g_charKind[i] == RIDE_MARKER_KIND && g_characterModels[i].root != 0)
            {
                markerPosition.Copy(g_characterModels[i].matrix.GetTranslation());
                if ((markerPosition - selfPosition).LengthSquared3() < nearestSquared)
                {
                    nearest = i;
                    nearestSquared = (markerPosition - selfPosition).LengthSquared3();
                }
            }
        }
        g_charState[index].values[RIDE_VALUE_MARKER] = nearest;
        g_shotParams[index][0] = sqrtf(nearestSquared);
        if (nearest == RIDE_NO_MARKER)
            Char_ResetState(index);
    }
    else if (g_charState[index].state == RIDE_STATE_FOLLOW)
    {
        g_charGroup[index] = -1;
        Vec4 place;
        Vec4 step;
        Vec4 markerPosition;
        if (g_characterModels[g_charState[index].values[RIDE_VALUE_MARKER]].matrix.m[MATRIX_TRANSLATION_X] < RIDE_GONE_X)
            markerPosition.Copy(g_characterModels[index].matrix.GetTranslation());
        else
            markerPosition.Copy(g_characterModels[g_charState[index].values[RIDE_VALUE_MARKER]].matrix.GetTranslation());
        place.y = -g_shotParams[index][0];
        place.RotateZ(RIDE_TURN_OFFSET - g_characterModels[g_charState[index].values[RIDE_VALUE_MARKER]].heading);
        place.Copy(place + markerPosition);
        step.Copy(place - selfPosition);
        float length = step.Length();
        if (length > RIDE_MAX_STEP)
        {
            step.Normalized3();
            step.Copy(step * RIDE_MAX_STEP);
            place.Copy(selfPosition + step);
        }
        float facing = atan2f(selfPosition.x - markerPosition.x, selfPosition.y - markerPosition.y);
        float turn = Atan2(selfPosition.x - place.x, selfPosition.y - place.y);
        turn -= facing;
        while (turn > RIDE_TURN_OFFSET)
            turn -= RIDE_FULL_TURN;
        while (turn < -RIDE_TURN_OFFSET)
            turn += RIDE_FULL_TURN;
        if ((float)fabs(turn) < RIDE_CUT_ANGLE && (markerPosition - selfPosition).Length() < length)
        {
            step.Fill(0.0f);
            step.y = -g_shotParams[index][0];
            step.RotateZ(RIDE_TURN_OFFSET - facing);
            if (turn < 0.0f)
                step.RotateZ(-RIDE_CUT_TURN);
            else
                step.RotateZ(RIDE_CUT_TURN);
            step.Copy(step + markerPosition);
            g_characterModels[index].SetPosition(step.x, step.y, markerPosition.z);
        }
        else
            g_characterModels[index].SetPosition(place.x, place.y, place.z);
        float heading = g_characterModels[g_charState[index].values[RIDE_VALUE_MARKER]].heading;
        g_characterModels[index].targetHeading = heading;
        turn = heading - g_characterModels[index].heading;
        while (turn > RIDE_TURN_OFFSET)
            turn -= RIDE_FULL_TURN;
        while (turn < -RIDE_TURN_OFFSET)
            turn += RIDE_FULL_TURN;
        float settled = (float)fabs(turn);

        if (settled > RIDE_SETTLED_ANGLE || (place - selfPosition).LengthSquared3() > RIDE_SETTLED_DISTANCE_SQUARED)
            g_characterModels[index].PlayMotion(RIDE_MOTION_WALK, 0, RIDE_MOTION_BLEND, 1.0f);
        else
            g_characterModels[index].PlayMotion(RIDE_MOTION_IDLE, 0, RIDE_MOTION_BLEND, 1.0f);
        if (g_characterModels[g_charState[index].values[RIDE_VALUE_MARKER]].matrix.m[MATRIX_TRANSLATION_X] < RIDE_GONE_X)
            Char_ResetState(index);
    }
    g_charToldToReact[index] = 0;
    return 0;
}
