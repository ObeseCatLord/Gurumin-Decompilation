// An area zone (kind 0x23 character, run once a frame by the scene frame 0x100f5a80): a box of the map (two corners in
// the map record) that acts on the player while the player stands inside it. Its type (record row 3) picks the action:
// 0..4 push the player along a force vector (type 0 only over the pit area, 1..3 depending on the player's mode, with
// a push strength that grows while the player is inside and fades while not), 5 sets the camera's value and speed
// scale, 7 and 8 set the camera value from the player's height inside the box (8 follows the height of a node as
// well), 6 is run by another function. The record may also start an effect and a sound at one frame of a cycle.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "last_position.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"

#include "map_records.h"

#include "map_vars.h"

#include "camera_vars.h"

#include "game.h"

#include "game_vars.h"

#include "char_vars.h"

#include "doppler_loops.h"

#include "area_zone.h"

#include "laser_zone.h"

#define ZONE_TYPE_PUSH_PIT 0
#define ZONE_TYPE_PUSH_RESTRICTED 1
#define ZONE_TYPE_PUSH_FREE 2
#define ZONE_TYPE_PUSH_NORMAL 3
#define ZONE_TYPE_CAMERA 5
#define ZONE_TYPE_OTHER 6
#define ZONE_TYPE_CAMERA_HEIGHT 7
#define ZONE_TYPE_CAMERA_NODE 8
#define ZONE_PLAYER_MODE_NORMAL 1
#define ZONE_PLAYER_MODE_OVER_PIT 2
#define ZONE_MARGIN 20.0f                    // the player counts as a box of this half size
#define ZONE_PARK_HEIGHT -3000.0f            // the zone character itself waits this low
#define ZONE_FADE 0.95f                      // push strength kept each frame outside the zone
#define ZONE_FORCE_UNIT 0.01f
#define ZONE_SPEED_SCALE_UNIT 100.0f
#define ZONE_NODE_LIFT 10.0f
#define ZONE_FRACTION_MIN 0.001f

extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);   // Char_StartEffect
extern "C" int FUN_10054550(int sound, int delay, const Vec4 *position);                          // queued sound

// 0x1007ad20 (Char_AreaZone). One frame of the area zone character `index`; its map record (the model's zone number)
// holds the box (rows 0 and 1), the force (row 2), the push growth (row 3) and the effect and sound (rows 4 to 6).
// A zone of type 6 is run by another function. On its first frame the zone copies the record into its slots and parks
// itself below the map. Later frames advance the cycle (the zone acts only in the frames between two limits), start
// the effect and sound at their frame, and act on the player: types 5, 7 and 8 set the camera's values when the
// player is in the box (and clear them when the player leaves), the other types push the player along the force
// vector with the push strength that grows inside the box and fades outside, as far as the player's mode allows.
// Returns 0.
extern "C" int FUN_1007ad20(int index)
{
    int zone = DAT_10db6ee8[index].zone;
    int idle = 1;
    float value;
    if (DAT_10210fc0[DAT_10db6ee8[index].zone].points[3].z == ZONE_TYPE_OTHER)
    {
        Char_LaserZone(index);
        return 0;
    }
    if (DAT_1020b430[index].state == 0)
    {
        const MapRecord &record = DAT_10210fc0[zone];
        DAT_1020b430[index].state = 1;
        DAT_10802bf8[index].position.x = (float)record.points[0].x;
        DAT_1020b430[index].zoneRampStep = record.points[3].x;
        DAT_10802bf8[index].position.y = (float)record.points[0].y;
        DAT_1020b430[index].zoneRampMax = record.points[3].y;
        DAT_1020b430[index].zoneType = record.points[3].z;
        DAT_10802bf8[index].position.z = (float)record.points[0].z;
        DAT_1020b430[index].zoneValue = record.points[3].code;
        DAT_1020b430[index].zoneActiveFrom = record.points[1].code;
        DAT_10802bf8[index].step.x = (float)record.points[1].x;
        DAT_1020b430[index].zoneRamp = 0;
        DAT_1020b430[index].zoneActive = 0;
        DAT_1020b430[index].zoneFrame = 0;
        DAT_1020b430[index].zonePeriod = record.points[0].code;
        DAT_1020b430[index].zoneActiveTo = record.points[2].code;
        DAT_10802bf8[index].step.y = (float)record.points[1].y;
        DAT_10802bf8[index].step.z = (float)record.points[1].z;
        if (record.points[0].code != 0)
        {
            DAT_1020b430[index].zoneEffect = record.points[4].x;
            DAT_1020b430[index].zoneEffectFrame = record.points[4].y;
            DAT_1020b430[index].zoneSound = record.points[5].code;
            DAT_1020b430[index].zoneSoundDelay = record.points[6].code;
            DAT_10868838[index][0].SetIdentity();
            DAT_10868838[index][0].translation.x = (float)record.points[5].x;
            DAT_10868838[index][0].translation.y = (float)record.points[5].y;
            DAT_10868838[index][0].translation.z = (float)record.points[5].z;
            DAT_10868838[index][1].SetIdentity();
            DAT_10868838[index][1].translation.x = (float)record.points[6].x;
            DAT_10868838[index][1].translation.y = (float)record.points[6].y;
            DAT_10868838[index][1].translation.z = (float)record.points[6].z;
        }
        else
        {
            DAT_1020b430[index].zoneEffect = 0;
        }
        DAT_10db6ee8[index].SetPosition(DAT_10db6ee8[index].matrix.translation.x,
                                        DAT_10db6ee8[index].matrix.translation.y, ZONE_PARK_HEIGHT);
        return 0;
    }
    Vec4 position;
    Vec4 force;
    if (++DAT_1020b430[index].zoneFrame > DAT_1020b430[index].zonePeriod)
        DAT_1020b430[index].zoneFrame = 0;
    if (DAT_1020b430[index].zoneFrame >= DAT_1020b430[index].zoneActiveFrom &&
        DAT_1020b430[index].zoneFrame <= DAT_1020b430[index].zoneActiveTo)
        idle = 0;
    if (DAT_1020b430[index].zoneEffect != 0 && DAT_1020b430[index].zoneEffectFrame == DAT_1020b430[index].zoneFrame)
    {
        Vec4 origin;
        FUN_1001d8b0(index, DAT_1020b430[index].zoneEffect, origin, -1, -1, 0, 0, 0, &DAT_10868838[index][0], 0);
        if (DAT_1020b430[index].zoneSound != 0)
        {
            if (DAT_1020b430[index].zoneSoundDelay > 0)
                FUN_10054550(DAT_1020b430[index].zoneSound, DAT_1020b430[index].zoneSoundDelay,
                             &(const Vec4 &)DAT_10868838[index][1].GetTranslation());
            else
                FUN_1003ad20(DAT_1020b430[index].zoneSound, &(const Vec4 &)DAT_10868838[index][1].GetTranslation(), 1.0f);
        }
    }
    if (DAT_1020b430[index].zoneType == ZONE_TYPE_CAMERA)
    {
        position.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        if (position.x - ZONE_MARGIN < DAT_10802bf8[index].step.x &&
            position.x + ZONE_MARGIN > DAT_10802bf8[index].position.x &&
            position.y - ZONE_MARGIN < DAT_10802bf8[index].step.y &&
            position.y + ZONE_MARGIN > DAT_10802bf8[index].position.y &&
            position.z - ZONE_MARGIN < DAT_10802bf8[index].step.z &&
            position.z + ZONE_MARGIN > DAT_10802bf8[index].position.z)
        {
            if (DAT_1020b430[index].zoneActive == 0)
            {
                DAT_1020b430[index].zoneActive = 1;
                DAT_1101dddc = (float)DAT_1020b430[index].zoneValue;
                DAT_1020344c = (float)DAT_10210fc0[DAT_10db6ee8[index].zone].points[3].x / ZONE_SPEED_SCALE_UNIT;
                return 0;
            }
        }
        else if (DAT_1020b430[index].zoneActive != 0)
        {
            DAT_1020b430[index].zoneActive = 0;
            DAT_1101dddc = 0;
            DAT_1020344c = 1.0f;
        }
        return 0;
    }
    if (DAT_1020b430[index].zoneType == ZONE_TYPE_CAMERA_HEIGHT)
    {
        position.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        if (position.x - ZONE_MARGIN < DAT_10802bf8[index].step.x &&
            position.x + ZONE_MARGIN > DAT_10802bf8[index].position.x &&
            position.y - ZONE_MARGIN < DAT_10802bf8[index].step.y &&
            position.y + ZONE_MARGIN > DAT_10802bf8[index].position.y)
        {
            int current = DAT_10db6ee8[index].zone;
            float span = DAT_10802bf8[index].step.z - DAT_10802bf8[index].position.z;
            DAT_1020b430[index].zoneActive = 1;
            value = position.z - DAT_10802bf8[index].position.z;
            if (value < 0.0f)
                value = 0.0f;
            else if (value > span)
                value = 1.0f;
            else
                value = value / span;
            DAT_1101dddc = (float)(DAT_10210fc0[current].points[3].y - DAT_10210fc0[current].points[3].x) * value +
                           (float)DAT_10210fc0[current].points[3].x;
            return 0;
        }
        if (DAT_1020b430[index].zoneActive != 0)
        {
            DAT_1020b430[index].zoneActive = 0;
            DAT_1101dddc = 0;
        }
        return 0;
    }
    if (DAT_1020b430[index].zoneType == ZONE_TYPE_CAMERA_NODE)
    {
        position.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        if (DAT_1021bdf8[index][0] != 0)
        {
            if (DAT_1020b430[index].zoneTracking == 0)
            {
                DAT_1020b430[index].zoneTracking = 1;
                DAT_10802bf8[index].history2.z = DAT_1021bdf8[index][0]->worldMatrix.translation.z;
            }
            value = DAT_1021bdf8[index][0]->worldMatrix.translation.z - DAT_10802bf8[index].history2.z;
            DAT_10db6ee8[0].SetPosition(position.x, position.y,
                                        value + DAT_1021bdf8[index][0]->worldMatrix.translation.z + ZONE_NODE_LIFT);
            DAT_10802bf8[index].history2.z = DAT_1021bdf8[index][0]->worldMatrix.translation.z;
        }
        else
        {
            value = 0.0f;
            DAT_10802bf8[index].history2.Fill(value);
            DAT_1020b430[index].zoneTracking = 0;
        }
        if (position.x - ZONE_MARGIN < DAT_10802bf8[index].step.x &&
            position.x + ZONE_MARGIN > DAT_10802bf8[index].position.x &&
            position.y - ZONE_MARGIN < DAT_10802bf8[index].step.y &&
            position.y + ZONE_MARGIN > DAT_10802bf8[index].position.y &&
            position.z - ZONE_MARGIN < DAT_10802bf8[index].step.z &&
            position.z + ZONE_MARGIN > DAT_10802bf8[index].position.z)
        {
            int current = DAT_10db6ee8[index].zone;
            float span = DAT_10802bf8[index].step.z - DAT_10802bf8[index].position.z;
            DAT_1020b430[index].zoneActive = 1;
            value = position.z - DAT_10802bf8[index].position.z;
            if (value < 0.0f)
                value = 0.0f;
            else if (value > span)
                value = 2.0f;
            else
                value = value / span + value / span;
            value = 1.0f - floorf(1.0f - value);
            float share = (span - (float)DAT_1020b430[index].zoneValue) / span;
            share = share + share;
            float rest = 1.0f - floorf(1.0f - share);
            if (rest > ZONE_FRACTION_MIN)
                value = value / rest;
            DAT_1101dddc = (float)(DAT_10210fc0[current].points[3].y - DAT_10210fc0[current].points[3].x) * value +
                           (float)DAT_10210fc0[current].points[3].x;
            return 0;
        }
        if (DAT_1020b430[index].zoneActive != 0)
        {
            DAT_1020b430[index].zoneActive = 0;
            DAT_1101dddc = 0;
        }
        return 0;
    }
    value = 0.0f;
    force.Fill(value);
    force.x = (float)DAT_10210fc0[DAT_10db6ee8[index].zone].points[2].x;
    force.y = (float)DAT_10210fc0[DAT_10db6ee8[index].zone].points[2].y;
    force.z = (float)DAT_10210fc0[DAT_10db6ee8[index].zone].points[2].z;
    if (force.x == 0.0f && force.y == 0.0f && force.z == 0.0f)
        return 0;
    force.Normalized3();
    position.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    if (idle == 0 && position.x - ZONE_MARGIN < DAT_10802bf8[index].step.x &&
        position.x + ZONE_MARGIN > DAT_10802bf8[index].position.x &&
        position.y - ZONE_MARGIN < DAT_10802bf8[index].step.y &&
        position.y + ZONE_MARGIN > DAT_10802bf8[index].position.y &&
        (DAT_1020b430[index].zoneValue == 0 ||
         (position.z - ZONE_MARGIN < DAT_10802bf8[index].step.z &&
          position.z + ZONE_MARGIN > DAT_10802bf8[index].position.z)))
    {
        DAT_1020b430[index].zoneRamp += DAT_1020b430[index].zoneRampStep;
        if (DAT_1020b430[index].zoneRamp > DAT_1020b430[index].zoneRampMax)
            DAT_1020b430[index].zoneRamp = DAT_1020b430[index].zoneRampMax;
    }
    else if (DAT_1020b430[index].zoneRamp > 0)
    {
        DAT_1020b430[index].zoneRamp = (int)((float)DAT_1020b430[index].zoneRamp * ZONE_FADE);
    }
    int type = DAT_1020b430[index].zoneType;
    int mode = DAT_10db6ee8[0].mode;
    int restricted = DAT_10db6ee8[0].m_238c;
    if ((type != ZONE_TYPE_PUSH_PIT ||
         ((position.z > DAT_104bfe24 ? (DAT_1020b430[index].zoneActive != 0 && restricted == 0 && DAT_10447dd0 == 0)
                                     : (restricted == 0 || mode == ZONE_PLAYER_MODE_OVER_PIT)) &&
          DAT_104bfe28 != 0 &&
          DAT_104b7200.PointInRect((int)position.x, (int)position.y, (int)DAT_104d29e8.x, (int)DAT_104d29e8.y,
                                   (int)DAT_104d29f8.x, (int)DAT_104d29f8.y) != 0)) &&
        (type != ZONE_TYPE_PUSH_RESTRICTED || (restricted != 0 && mode == ZONE_PLAYER_MODE_NORMAL)) &&
        (type != ZONE_TYPE_PUSH_FREE || (restricted == 0 && mode == ZONE_PLAYER_MODE_NORMAL)) &&
        (type != ZONE_TYPE_PUSH_NORMAL || mode == ZONE_PLAYER_MODE_NORMAL))
    {
        value = (float)DAT_1020b430[index].zoneRamp * ZONE_FORCE_UNIT;
        force.Copy(force * value);
        DAT_10db6ee8[0].matrix.translation.z = force.z + DAT_10db6ee8[0].matrix.translation.z;
        force.z = 0.0f;
        if (DAT_10db6ee8[0].IsClearOfMapLimited(&force) != 0)
        {
            DAT_10db6ee8[0].matrix.translation.x = DAT_10db6ee8[0].matrix.translation.x + force.x;
            DAT_10db6ee8[0].matrix.translation.y = DAT_10db6ee8[0].matrix.translation.y + force.y;
            DAT_10db6ee8[0].matrix.translation.z = DAT_10db6ee8[0].matrix.translation.z + force.z;
        }
        DAT_1020b430[index].zoneActive = 1;
        return 0;
    }
    DAT_1020b430[index].zoneRamp = 0;
    DAT_1020b430[index].zoneActive = 0;
    return 0;
}
