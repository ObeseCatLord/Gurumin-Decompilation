// Camera mode of the item stand pull (run by the scene frame, FUN_100f5a80): the camera circles the player while the
// player is drawn towards the item stand ("@kagudai").

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "map_vars.h"
#include "char_state.h"
#include "anim_model.h"

#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "camera_modes.h"

extern "C" void FUN_1016a8d0(void);
extern "C" int FUN_1016a8e0(void);
extern "C" int FUN_1013f880(void);   // the game's random number

// The orbit settings live in the first floats of the matrix DAT_10868838[0][0] (shared scratch memory).
#define ORBIT_DISTANCE DAT_10868838[0][0].m[0]
#define ORBIT_HEIGHT DAT_10868838[0][0].m[1]
#define ORBIT_UNUSED_STEP DAT_10868838[0][0].m[2]
#define ORBIT_SCALE_RATE DAT_10868838[0][0].m[4]
#define ORBIT_TURN DAT_10868838[0][0].m[6]
#define ORBIT_SCALE DAT_10868838[0][0].m[8]

#define ORBIT_START_DISTANCE 800.0f
#define ORBIT_TURN_PER_FRAME 0.008f
#define WALL_CLEARANCE 25.0f
#define WALL_NEAR_DISTANCE 50.0f
#define WALL_NEAR_SHRINK 0.75f
#define PULL_MAX_STEP 15.0f
#define TARGET_LIFT 70.0f
#define EASE_GOAL 0.15f
#define EASE_PREVIOUS 0.85f
#define STATE_ENDING 1
#define STATE_ENDED 2
#define MAP_MASK_CAMERA 4

// 0x10018870 (Camera_PullToItemStand). The camera position goal stays on the line from the target goal through the
// current camera, turned 0.008 rad a frame and set to ORBIT_DISTANCE * ORBIT_SCALE from the target goal (the settings
// are set up on the first call, which also rolls pullVariant for the player's slot). The goal is then moved in
// front of walls: a segment from 25 units beyond the goal back to the target is cast against the map's "CA" shapes
// (mask 4) and then its "CS" shapes, and a hit puts the goal 25 units short of it (or at 3/4 of the distance when
// that is less than 50). State DAT_104c035c = 1 ends the mode (control mode 0, state 2). While the map has an
// "@kagudai" node the player is moved towards it (at most 15 units a frame, when the map allows) and the target goal
// is 70 units above the player. The target and position then move 15 % towards their goals plus 85 % towards their
// previous values (only in the running state), and the copies are refreshed. Returns 0.
extern "C" int FUN_10018870(void)
{
    if (DAT_104c035c == 0)
    {
        if (DAT_104c0358 == 0)
        {
            FUN_1016a8d0();
            DAT_10866d98.Copy(DAT_118e69e8);
            DAT_10edbd48.Copy(DAT_118e6cb8);
            float scale = 1.0f;
            DAT_10866d98.Copy((DAT_10866d98 - DAT_10edbd48) * scale + DAT_10edbd48);
            DAT_104c0358 = 1;
            DAT_1020b430[0].pullVariant = 0;
            ZeroMatrix();   // builds and drops a zero matrix
            DAT_1020b430[0].pullVariant = ((FUN_1013f880() % 2 + 1) * 16) | (FUN_1013f880() % 2 * 4 + 4) |
                                          (FUN_1013f880() % 2 + 1);
            (DAT_10866d98 - DAT_10edbd48).Length2D();   // the original measures the distance and drops it
            ORBIT_HEIGHT = DAT_10866d98.z;
            ORBIT_UNUSED_STEP = ORBIT_TURN_PER_FRAME;
            ORBIT_SCALE_RATE = 0.0f;
            ORBIT_TURN = ORBIT_TURN_PER_FRAME;
            ORBIT_SCALE = 1.0f;
            ORBIT_DISTANCE = ORBIT_START_DISTANCE;
        }
        ORBIT_SCALE = ORBIT_SCALE + ORBIT_SCALE_RATE;
        Vec4 orbitOffset;
        orbitOffset.Copy(DAT_10866d98 - DAT_10edbd48);
        orbitOffset.RotateZ(-ORBIT_TURN);
        orbitOffset.Normalized3();
        orbitOffset.Copy(orbitOffset * ORBIT_DISTANCE * ORBIT_SCALE);
        DAT_10866d98.Copy(DAT_10edbd48 + orbitOffset);
        {
            Vec4 farPoint;
            Vec4 target;
            Vec4 hit;
            target.Copy(DAT_10edbd48);
            farPoint.Copy(DAT_10866d98 - DAT_10edbd48);
            {
                float distance = farPoint.Length();
                float reach = distance + WALL_CLEARANCE;
                farPoint.Normalized3();
                farPoint.Copy(farPoint * reach + DAT_10edbd48);
            }
            if (DAT_10255f08 != 0)
            {
                if (DAT_10255f08->CastSegment("CA", MAP_MASK_CAMERA, &farPoint, &target, &hit, 0))
                {
                    float clearance = (hit - target).Length();
                    if (clearance > WALL_NEAR_DISTANCE)
                        clearance -= WALL_CLEARANCE;
                    else
                        clearance *= WALL_NEAR_SHRINK;
                    hit.Copy(farPoint - target);
                    hit.Normalized3();
                    DAT_10866d98.Copy(target + hit * clearance);
                    farPoint.Copy(target + hit * (clearance + WALL_CLEARANCE));
                }
                if (DAT_10255f08 != 0)
                {
                    if (DAT_10255f08->CastSegment("CS", 0, &farPoint, &target, &hit, 0))
                    {
                        float clearance = (hit - target).Length();
                        if (clearance > WALL_NEAR_DISTANCE)
                            clearance -= WALL_CLEARANCE;
                        else
                            clearance *= WALL_NEAR_SHRINK;
                        hit.Copy(farPoint - target);
                        hit.Normalized3();
                        farPoint.Copy(target + hit * clearance);
                        DAT_10866d98.Copy(farPoint);
                    }
                }
            }
        }
        DAT_118e6cb8.Copy(DAT_118e6cb8 + (DAT_10edbd48 - DAT_118e6cb8) * EASE_GOAL +
                          (DAT_10f08aa0 - DAT_118e6cb8) * EASE_PREVIOUS);
        DAT_118e69e8.Copy(DAT_118e69e8 + (DAT_10866d98 - DAT_118e69e8) * EASE_GOAL +
                          (DAT_104d29d8 - DAT_118e69e8) * EASE_PREVIOUS);
        DAT_118e6c78.Copy(DAT_118e6cb8);
        DAT_11af3e28.Copy(DAT_118e69e8);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_10f65688.Copy(DAT_118e69e8);
        DAT_104d29d8.Copy(DAT_118e69e8);
        DAT_10f08aa0.Copy(DAT_118e6cb8);
    }
    else if (DAT_104c035c == STATE_ENDING)
    {
        FUN_1016a8e0();
        DAT_104c035c = STATE_ENDED;
    }
    if (DAT_10255f08 != 0)
    {
        Node *stand = DAT_10255f08->Find("@kagudai", 0);
        if (stand != 0)
        {
            Vec4 playerPosition;
            Vec4 toStand;
            playerPosition.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            toStand.Copy(stand->worldMatrix.GetTranslation());
            toStand.Copy(toStand - playerPosition);
            toStand.z = 0;
            if (toStand.Length2D() > PULL_MAX_STEP)
            {
                toStand.Normalized3();
                float step = PULL_MAX_STEP;
                toStand.Copy(toStand * step);
            }
            if (DAT_10db6ee8[0].IsClearOfMapLimited(&toStand))
            {
                DAT_10db6ee8[0].matrix._41 = toStand.x + DAT_10db6ee8[0].matrix._41;
                DAT_10db6ee8[0].matrix._42 = toStand.y + DAT_10db6ee8[0].matrix._42;
            }
            DAT_10edbd48.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            DAT_10edbd48.z += TARGET_LIFT;
        }
    }
    return 0;
}
