// An enemy that appears out of nowhere (EN build). Ported from the matched JP source decomp/src/enemy_blink.cpp: it
// waits hidden far away until the player comes near its home, picks a random spot around the home that it can stand on
// and slides into place while fading in.
#include "anim_model.h"
#include "char.h"
#include "char_shots.h"
#include "engine.h"
#include "node.h"
#include "vec4.h"
#include "visibility.h"

// CharState::values indices of this behaviour: frames slid so far, and the distance (in units) at which it appears.
const int APPEAR_VALUE_WAIT = 0;
const int APPEAR_VALUE_REACH = 1;
// x position the enemy hides at.
const float APPEAR_HIDDEN_X = -200000.0f;
// Behaviour steps.
const int APPEAR_STATE_START = 0;
const int APPEAR_STATE_HIDDEN = 1;
const int APPEAR_STATE_SLIDING = 2;
// Motion layer, blend frames and speed of the idle motion it plays while hidden and sliding.
const int APPEAR_MOTION_LAYER = 2;
const int APPEAR_MOTION_FRAMES = 5;
// Frames the slide into place takes.
const int APPEAR_SLIDE_FRAMES = 10;
// Random spots tried around the home.
const int APPEAR_TRIES = 10;
// Spot search: distance from the home, random pitch range (degrees) and heading range (degrees).
const float APPEAR_SPOT_DISTANCE = -1000.0f;
const int APPEAR_PITCH_RANGE = 180;
const int APPEAR_PITCH_CENTRE = 90;
const int APPEAR_HEADING_RANGE = 360;
const float APPEAR_PI = 3.1415f;
const float APPEAR_HALF_TURN_DEGREES = 180.0f;
// Height above the home the search starts from and the ground probe length.
const float APPEAR_LIFT = 50.0f;
const float APPEAR_GROUND_DEPTH = 1000.0f;
// A spot is pulled to this distance short of what the ray hits; spots nearer than 500 units (squared) to the home are skipped.
const float APPEAR_WALL_MARGIN = 75.0f;
const float APPEAR_MIN_DISTANCE_SQUARED = 250000.0f;
// Fraction of the way home the enemy slides back each frame, and its alpha while appearing.
const float APPEAR_SLIDE_FRACTION = 0.1f;
const float APPEAR_ALPHA = 0.5f;
// AnimModel::mode values: 1 = walking, 0 while appearing.
const int APPEAR_MODE_APPEARING = 0;
const int APPEAR_MODE_WALKING = 1;

// Enemy_AppearNearby: remembers where it was placed (its home) and hides at x = -200000. Once the player is within
// the reach (values[1]) units of the home it tries up to ten random directions from the home (1000 units at a random
// pitch and heading, pulled back to 75 units short of what the ray hits when that is still 500 units away, and in a
// visible cell): where it can stand and finds ground below, it moves there and slides back a tenth of the way home each
// frame for 10 frames, half transparent, before its state is reset. Returns 0. Called by the behaviour dispatch.
extern "C" int Enemy_AppearNearby(int index)
{
    if (g_charState[index].state == APPEAR_STATE_START)
    {
        g_charState[index].state = APPEAR_STATE_HIDDEN;
        g_shotSlots[index].position.Copy(g_characterModels[index].matrix.GetTranslation());
        g_charInPlay[index] = 0;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] = APPEAR_HIDDEN_X;
        g_characterModels[index].PlayMotion(APPEAR_MOTION_LAYER, 0, APPEAR_MOTION_FRAMES, 1.0f);
        return 0;
    }
    if (g_charState[index].state == APPEAR_STATE_HIDDEN)
    {
        int reach = g_charState[index].values[APPEAR_VALUE_REACH];
        if ((g_shotSlots[index].position - g_characterModels[0].matrix.GetTranslation()).LengthSquared3() <
            (float)(reach * reach))
        {
            Vec4 spot;
            Vec4 ground;
            Vec4 probe;
            Vec4 hit;
            probe.Copy(g_shotSlots[index].position);
            probe.z += APPEAR_LIFT;
            int tries;
            for (tries = 0; tries < APPEAR_TRIES; tries++)
            {
                spot.Fill(0.0f);
                spot.y = APPEAR_SPOT_DISTANCE;
                spot.RotateX((float)(Random() % APPEAR_PITCH_RANGE - APPEAR_PITCH_CENTRE) * APPEAR_PI / APPEAR_HALF_TURN_DEGREES);
                spot.RotateZ((float)(Random() % APPEAR_HEADING_RANGE) * APPEAR_PI / APPEAR_HALF_TURN_DEGREES);
                spot.Copy(spot + g_shotSlots[index].position);
                if (g_mapRootNode != 0)
                {
                    if (g_mapRootNode->CastRay(0, 4, 0, &spot, &probe, &hit, 0) != 0)
                    {
                        spot.Copy(probe - hit);
                        spot.Normalized3();
                        spot.Copy(spot * 75.0f)   /* APPEAR_WALL_MARGIN: passed by reference, so it stays a literal */;
                        spot.Copy(hit + spot);
                        if ((spot - probe).LengthSquared3() < APPEAR_MIN_DISTANCE_SQUARED)
                            continue;
                    }
                    if (g_mapRootNode != 0 && g_mapRootNode->visibilityCells != 0 && Vis_IsVisible(spot) == 0)
                        continue;
                }
                g_characterModels[index].SetPosition(probe.x, probe.y, probe.z);
                if (g_characterModels[index].IsClearOfMapLimited(0) == 0)
                    continue;
                g_characterModels[index].SetPosition(APPEAR_HIDDEN_X, 0, 0);
                ground.Copy(probe);
                probe.z -= APPEAR_GROUND_DEPTH;
                if (g_mapRootNode != 0 && g_mapRootNode->Collide(0, 4, &ground, &probe, &hit) != 0)
                    break;
            }
            if (tries < APPEAR_TRIES)
            {
                g_charState[index].state = APPEAR_STATE_SLIDING;
                g_charState[index].values[APPEAR_VALUE_WAIT] = 0;
                g_characterModels[index].SetPosition(spot.x, spot.y, spot.z);
                g_shotSlots[index].position.Copy(g_shotSlots[index].position - spot);
                g_shotSlots[index].position.Copy(g_shotSlots[index].position * 0.1f)   /* APPEAR_SLIDE_FRACTION: passed by reference, so it stays a literal */;
                g_characterModels[index].mode = APPEAR_MODE_APPEARING;
                g_characterModels[index].alpha = APPEAR_ALPHA;
                return 0;
            }
        }
    }
    else if (g_charState[index].state == APPEAR_STATE_SLIDING)
    {
        g_characterModels[index].PlayMotion(APPEAR_MOTION_LAYER, 0, APPEAR_MOTION_FRAMES, 1.0f);
        g_charState[index].values[APPEAR_VALUE_WAIT]++;
        Vec4 position;
        position.Copy(g_characterModels[index].matrix.GetTranslation());
        position.Copy(position + g_shotSlots[index].position);
        g_characterModels[index].SetPosition(position.x, position.y, position.z);
        g_characterModels[index].alpha = APPEAR_ALPHA;
        if (g_charState[index].values[APPEAR_VALUE_WAIT] == APPEAR_SLIDE_FRAMES)
        {
            g_characterModels[index].alpha = 1.0f;
            g_characterModels[index].mode = APPEAR_MODE_WALKING;
            Char_ResetState(index);
        }
    }
    return 0;
}
