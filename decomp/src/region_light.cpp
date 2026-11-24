// The lights that flood and drain the water regions of a map (the table at DAT_104c03c0, see region_view.h): the
// vertices around the given light points start to morph towards a lit (flooded) or a drained state, and the effects
// that go with it are started.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"
#include "node.h"
#include "effect_manager.h"

#include "region_view.h"
#include "game_vars.h"
#include "map_vars.h"

extern "C" int FUN_10020090(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit);  // Collide_RayThroughRegions
extern "C" int FUN_1013f880(void);                                              // the game's random number
extern "C" int FUN_10054550(int sound, int delay, const Vec4 *position);        // queued sound

// The game's atan2 wrapper (float arguments).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// acos on floats (the argument is rounded to a float first).
inline float Acos(float x)
{
    return (float)acos(x);
}

#define VERTEX(region, index) DAT_104c03c0[region].vertices[index]
#define POINT(region, index) DAT_104c03c0[region].points[index]

#define LIGHT_ANCHOR_X -185.0f           // the fixed spot the lamp effects are tilted away from
#define LIGHT_ANCHOR_Y -510.0f
#define LIGHT_ANCHOR_Z 8000.0f
#define HALF_PI_TRUNCATED 1.57075f
#define PI_TRUNCATED 3.1415f
#define TWO_PI_TRUNCATED 6.283f
#define LAMP_DROP 150.0f                 // the lamp effect sits this far below the light point
#define LAMP_EFFECT 0x761                // started three times at each light point
#define LAMP_SOUND_A 0x18a
#define LAMP_SOUND_B 0x189
#define LAMP_DELAY_SKIP 30               // the light's delay minus this (at least 0) delays the lamp effects
#define RAY_LENGTH 1700.0f               // probe direction (0, -1700, 1700) for the collision test
#define RAY_SCALE 10000.0f
#define SINK_DEPTH 350.0f
#define MORPH_ALL_FRAMES 900             // morph start of a point that no light reaches
#define LIGHT_REACH 900.0f               // reach of a lit vertex on the vertices of other regions
#define LIGHT_RATE 18.0f                 // distance units per frame of the flood front
#define LIGHT_LIMIT 0.055555556f
#define FLOOD_LIMIT 0.99f                // a vertex whose lit value is below this can be flooded
#define SPLASH_EFFECT 0x760
#define SPLASH_SPREAD 400                // random offset range of a splash in x and y
#define SPLASH_HEIGHT 40                 // and in z and in frames
#define SPLASH_SHARE 20                  // splashes of a region in all, shared between its points
#define SPLASH_NEIGHBOUR_COUNT 10
#define FLOAT_ONE_BITS 0x3f800000        // IEEE bits of 1.0f (the lit value is compared as an integer)
#define SPLASH_LIT_LIMIT 0.9f            // a neighbour only splashes while its lit value is below this

// Random offset in [-range / 2, range / 2) as a float.
#define SPLASH_OFFSET(range) ((float)(FUN_1013f880() % (range) - (range) / 2))
// The frame an effect starts at: `frame` plus a random -20..19, or 0 when that is not positive.
#define SPLASH_FRAME(frame) \
    (FUN_1013f880() % SPLASH_HEIGHT - SPLASH_HEIGHT / 2 + (frame) > 0 ? FUN_1013f880() % SPLASH_HEIGHT - SPLASH_HEIGHT / 2 + (frame) : 0)
// The delay of the lamp effects of light `i`.
#define LAMP_DELAY(i) (DAT_1020c370[i] - LAMP_DELAY_SKIP > 0 ? DAT_1020c370[i] - LAMP_DELAY_SKIP : 0)

// 0x10062e70 (Regions_StartLightMorph). `lights` are `lightCount` points (16 bytes each). With `lighting` 0 the
// lamp effects (0x761 at each light, tilted towards it from a fixed spot, plus two sounds) are started first. Then
// every vertex of every region that is lit (lighting 0) or not yet lit (lighting != 0) and is under the map's
// "testatari" collision (lighting 0) or free of it (lighting != 0) gets its control point set to morph: the lit
// value (tint w) goes to 0 and the vertex sinks by 350 (lighting 0) or goes to 1 (lighting != 0). The morph starts
// at the frame the nearest light's front reaches the point (900 when none does), the region is marked unsettled
// and splash effects 0x760 start. Finally the same is spread from every such vertex to the points of other
// regions within 900 units, with a lowering that grows with the closeness and, depending on the angle to the
// "@Light" node, a darker or lighter tint and offset (like Regions_Build). Returns 0.
int FUN_10062e70(const Vec4 *lights, int lightCount, int lighting)
{
    Vec4 point;
    Vec4 second;
    Vec4 result;
    Vec4 unused1;
    Vec4 unused2;
    Vec4 lightPos;
    float lightX;
    float lightY;
    if (DAT_10255f08 != 0)
    {
        Node *light = DAT_10255f08->Find("@Light", 0);
        if (light != 0)
            lightPos.Copy(light->worldMatrix.GetTranslation());
        else
            lightPos.Fill(0.0f);
        for (int i = 0; i < lightCount; i++)
        {
            point.Set(LIGHT_ANCHOR_X, LIGHT_ANCHOR_Y, LIGHT_ANCHOR_Z, 0.0f);
            point.Copy(lights[i] - point);
            float heading = Atan2(point.x, point.y);
            float turn = heading + PI_TRUNCATED;
            float pitch = HALF_PI_TRUNCATED - Acos(point.Length2D() / point.Length());
            if (lighting == 0)
            {
                Matrix4 placement;
                second.Fill(0.0f);
                placement.SetIdentity();
                placement = RotationX(pitch) * RotationZ(turn) * TranslationMatrix(lights[i].x, lights[i].y, lights[i].z - LAMP_DROP);
                DAT_10daaa80.Start(LAMP_EFFECT, second, -1, -1, 0, 0, LAMP_DELAY(i), &placement, 0, 0);
                DAT_10daaa80.Start(LAMP_EFFECT, second, -1, -1, 0, 0, LAMP_DELAY(i), &placement, 0, 0);
                DAT_10daaa80.Start(LAMP_EFFECT, second, -1, -1, 0, 0, LAMP_DELAY(i), &placement, 0, 0);
                FUN_10054550(LAMP_SOUND_A, LAMP_DELAY(i), 0);
                FUN_10054550(LAMP_SOUND_B, LAMP_DELAY(i), 0);
            }
        }
        for (int region = 0; region < DAT_102564a0; region++)
        {
            for (int v = 0; v < DAT_104c03c0[region].pointCount; v++)
            {
                VERTEX(region, v).x = VERTEX(region, v).baseX;
                VERTEX(region, v).y = VERTEX(region, v).baseY;
                VERTEX(region, v).z = VERTEX(region, v).baseZ;
                POINT(region, v).positionTo.x = VERTEX(region, v).baseX;
                POINT(region, v).positionTo.y = VERTEX(region, v).baseY;
                POINT(region, v).positionTo.z = VERTEX(region, v).baseZ;
                POINT(region, v).positionFrom.Copy(POINT(region, v).positionTo);
                POINT(region, v).tintTo.x = VERTEX(region, v).tint.x;
                POINT(region, v).tintTo.y = VERTEX(region, v).tint.y;
                POINT(region, v).tintTo.z = VERTEX(region, v).tint.z;
                POINT(region, v).tintTo.w = VERTEX(region, v).tint.w;
                POINT(region, v).tintFrom.Copy(POINT(region, v).tintTo);
                POINT(region, v).offsetTo.x = VERTEX(region, v).offset.x;
                POINT(region, v).offsetTo.y = VERTEX(region, v).offset.y;
                POINT(region, v).offsetTo.z = VERTEX(region, v).offset.z;
                POINT(region, v).offsetTo.w = VERTEX(region, v).offset.w;
                POINT(region, v).offsetFrom.Copy(POINT(region, v).offsetTo);
                point.x = VERTEX(region, v).x;
                point.y = VERTEX(region, v).y;
                point.z = VERTEX(region, v).z;
                second.Copy(point);
                Vec4 ray;
                ray.x = 0.0f;
                ray.y = -RAY_LENGTH;
                ray.z = RAY_LENGTH;
                second.Copy(second - ray * RAY_SCALE);
                Node *collision = DAT_10255f08->Find("testatari", 0);
                if (lighting != 0)
                {
                    if (!(VERTEX(region, v).tint.w < FLOOD_LIMIT) || collision == 0 || FUN_10020090(collision, 0, 4, &point, &second, &result) != 0)
                    {
                        POINT(region, v).morphing = 0;
                        continue;
                    }
                    POINT(region, v).tintFrom.x = VERTEX(region, v).baseTint.x;
                    POINT(region, v).tintFrom.y = VERTEX(region, v).baseTint.y;
                    POINT(region, v).tintFrom.z = VERTEX(region, v).baseTint.z;
                    POINT(region, v).tintFrom.w = 1.0f;
                    POINT(region, v).positionFrom.x = VERTEX(region, v).baseX;
                    POINT(region, v).positionFrom.y = VERTEX(region, v).baseY;
                    POINT(region, v).positionFrom.z = VERTEX(region, v).baseZ;
                    POINT(region, v).offsetFrom.Fill(0.0f);
                    DAT_104c03c0[region].hidden = 0;
                    int *morphing = &POINT(region, v).morphing;
                    int starting = 1;
                    if (*morphing != 0)
                        starting = 0;
                    *morphing = 1;
                    POINT(region, v).morphStart = MORPH_ALL_FRAMES;
                    POINT(region, v).facing = 0;
                    DAT_104c03c0[region].settled = 1;
                    DAT_104c03c0[region].morphStart = DAT_104bf8a4;
                    for (int j = 0; j < lightCount; j++)
                    {
                        float step = (lights[j] - point).Length2D() / LIGHT_RATE;
                        int *start = &POINT(region, v).morphStart;
                        int delay = DAT_1020c370[j];
                        float current = (float)*start;
                        float lightDelay = (float)delay;
                        if (lightDelay + step < current)
                            *start = (int)step + delay;
                    }
                    if (starting)
                    {
                        Matrix4 placement;
                        Vec4 origin;
                        for (int n = 0; n < (SPLASH_SHARE / DAT_104c03c0[region].pointCount > 1 ? SPLASH_SHARE / DAT_104c03c0[region].pointCount : 1); n++)
                        {
                            placement.SetIdentity();
                            placement._41 = SPLASH_OFFSET(SPLASH_SPREAD) + VERTEX(region, v).x;
                            placement._42 = SPLASH_OFFSET(SPLASH_SPREAD) + VERTEX(region, v).y;
                            placement._43 = SPLASH_OFFSET(SPLASH_HEIGHT) + VERTEX(region, v).z;
                            DAT_10daaa80.Start(SPLASH_EFFECT, origin, -1, -1, 0, 0, SPLASH_FRAME(POINT(region, v).morphStart), &placement, 0, 0);
                        }
                    }
                }
                else
                {
                    if (!(VERTEX(region, v).tint.w > 0.0f) || collision == 0 || FUN_10020090(collision, 0, 4, &point, &second, &result) == 0)
                    {
                        POINT(region, v).morphing = 0;
                        continue;
                    }
                    POINT(region, v).tintFrom.x = VERTEX(region, v).baseTint.x;
                    POINT(region, v).tintFrom.y = VERTEX(region, v).baseTint.y;
                    POINT(region, v).tintFrom.z = VERTEX(region, v).baseTint.z;
                    POINT(region, v).tintFrom.w = 0.0f;
                    POINT(region, v).positionFrom.x = VERTEX(region, v).baseX;
                    POINT(region, v).positionFrom.y = VERTEX(region, v).baseY;
                    POINT(region, v).positionFrom.z = VERTEX(region, v).baseZ - SINK_DEPTH;
                    POINT(region, v).offsetFrom.Fill(0.0f);
                    int *morphing = &POINT(region, v).morphing;
                    int starting = 1;
                    if (*morphing != 0)
                        starting = 0;
                    *morphing = 1;
                    POINT(region, v).morphStart = MORPH_ALL_FRAMES;
                    POINT(region, v).facing = 0;
                    DAT_104c03c0[region].settled = 1;
                    DAT_104c03c0[region].morphStart = DAT_104bf8a4;
                    for (int j = 0; j < lightCount; j++)
                    {
                        float step = (lights[j] - point).Length2D() / LIGHT_RATE;
                        int *start = &POINT(region, v).morphStart;
                        int delay = DAT_1020c370[j];
                        float current = (float)*start;
                        float lightDelay = (float)delay;
                        if (lightDelay + step < current)
                            *start = (int)step + delay;
                    }
                    if (starting)
                    {
                        Matrix4 placement;
                        Vec4 origin;
                        for (int n = 0; n < (SPLASH_SHARE / DAT_104c03c0[region].pointCount > 1 ? SPLASH_SHARE / DAT_104c03c0[region].pointCount : 1); n++)
                        {
                            placement.SetIdentity();
                            placement._41 = SPLASH_OFFSET(SPLASH_SPREAD) + VERTEX(region, v).x;
                            placement._42 = SPLASH_OFFSET(SPLASH_SPREAD) + VERTEX(region, v).y;
                            placement._43 = SPLASH_OFFSET(SPLASH_HEIGHT) + VERTEX(region, v).z;
                            DAT_10daaa80.Start(SPLASH_EFFECT, origin, -1, -1, 0, 0, SPLASH_FRAME(POINT(region, v).morphStart), &placement, 0, 0);
                        }
                    }
                }
            }
        }
        for (int region = 0; region < DAT_102564a0; region++)
        {
            for (int v = 0; v < DAT_104c03c0[region].pointCount; v++)
            {
                if ((lighting != 0 && *(int *)&POINT(region, v).tintFrom.w == FLOAT_ONE_BITS) || (lighting == 0 && POINT(region, v).tintFrom.w == 0.0f))
                {
                    if (DAT_104c03c0[region].settled != 0)
                    {
                        point.Copy(POINT(region, v).positionFrom);
                        for (int other = 0; other < DAT_102564a0; other++)
                        {
                            if (!(point.x + LIGHT_REACH < DAT_104c03c0[other].min.x) && !(point.x - LIGHT_REACH > DAT_104c03c0[other].max.x) &&
                                !(point.y + LIGHT_REACH < DAT_104c03c0[other].min.y) && !(point.y - LIGHT_REACH > DAT_104c03c0[other].max.y))
                            {
                                for (int w = 0; w < DAT_104c03c0[other].pointCount; w++)
                                {
                                    if (v != w || region != other)
                                    {
                                        if ((lighting != 0 && *(int *)&VERTEX(other, w).tint.w != FLOAT_ONE_BITS && POINT(other, w).tintFrom.w != 1.0f) ||
                                            (lighting == 0 && VERTEX(other, w).tint.w != 0.0f && POINT(other, w).tintFrom.w != 0.0f))
                                        {
                                            second.x = VERTEX(other, w).x;
                                            second.y = VERTEX(other, w).y;
                                            second.z = VERTEX(other, w).z;
                                            float distance = (point - second).Length2D();
                                            if (distance < LIGHT_REACH)
                                            {
                                                POINT(other, w).positionFrom.Copy(POINT(other, w).positionTo);
                                                POINT(other, w).tintFrom.Copy(POINT(other, w).tintTo);
                                                POINT(other, w).offsetFrom.Copy(POINT(other, w).offsetTo);
                                                float closeness = distance / LIGHT_REACH;
                                                if (VERTEX(other, w).nearness > closeness)
                                                {
                                                    VERTEX(other, w).nearness = closeness;
                                                    if (lighting == 0 ? VERTEX(other, w).nearness < LIGHT_LIMIT : VERTEX(other, w).nearness > LIGHT_LIMIT)
                                                        POINT(other, w).tintFrom.w = VERTEX(region, v).baseTint.w * VERTEX(other, w).nearness * LIGHT_RATE;
                                                    float lift = 1.0f - VERTEX(other, w).nearness;
                                                    lift = ((1.0f - lift) * 0.6f + lift) * lift;
                                                    POINT(other, w).positionFrom.z = VERTEX(region, v).baseZ - lift * SINK_DEPTH;
                                                    if (lightPos.Length() > 1.0f)
                                                    {
                                                        result.Copy(second - point);
                                                        result.z = 0.0f;
                                                        lightX = lightPos.x;
                                                        lightY = lightPos.y;
                                                        float heading = Atan2(result.x, result.y);
                                                        float turn = heading - Atan2(lightX, lightY);
                                                        for (; turn > PI_TRUNCATED; turn -= TWO_PI_TRUNCATED)
                                                        {
                                                        }
                                                        for (; turn < -PI_TRUNCATED; turn += TWO_PI_TRUNCATED)
                                                        {
                                                        }
                                                        heading = (float)fabs(turn);
                                                        float side = (heading / PI_TRUNCATED - 0.5f) * 2.0f;
                                                        if (side < 0.0f)
                                                        {
                                                            side = (-side * (1.0f - VERTEX(other, w).nearness) + 1.0f) * 0.5f;
                                                            side = (1.0f - side) * 0.3f + side;
                                                            POINT(other, w).tintFrom.x = side * VERTEX(other, w).baseTint.x;
                                                            POINT(other, w).tintFrom.y = side * VERTEX(other, w).baseTint.y;
                                                            POINT(other, w).tintFrom.z = side * VERTEX(other, w).baseTint.z;
                                                            POINT(other, w).facing = 0;
                                                            POINT(other, w).offsetFrom.x = 0.0f;
                                                            POINT(other, w).offsetFrom.y = 0.0f;
                                                            POINT(other, w).offsetFrom.z = 0.0f;
                                                        }
                                                        else
                                                        {
                                                            side = (1.0f - side) * (1.0f - VERTEX(other, w).nearness) * 0.2f;
                                                            side = side * 0.35f;
                                                            POINT(other, w).offsetFrom.x = side;
                                                            POINT(other, w).offsetFrom.y = side;
                                                            POINT(other, w).offsetFrom.z = side;
                                                            POINT(other, w).facing = 1;
                                                            POINT(other, w).tintFrom.x = VERTEX(other, w).baseTint.x;
                                                            POINT(other, w).tintFrom.y = VERTEX(other, w).baseTint.y;
                                                            POINT(other, w).tintFrom.z = VERTEX(other, w).baseTint.z;
                                                        }
                                                    }
                                                    int *morphing = &POINT(other, w).morphing;
                                                    int starting = 1;
                                                    if (*morphing != 0)
                                                        starting = 0;
                                                    *morphing = 1;
                                                    POINT(other, w).morphStart = MORPH_ALL_FRAMES;
                                                    DAT_104c03c0[other].settled = 1;
                                                    DAT_104c03c0[other].morphStart = DAT_104bf8a4;
                                                    for (int j = 0; j < lightCount; j++)
                                                    {
                                                        float step = (lights[j] - point).Length2D() / LIGHT_RATE;
                                                        int *start = &POINT(other, w).morphStart;
                                                        int delay = DAT_1020c370[j];
                                                        float current = (float)*start;
                                                        float lightDelay = (float)delay;
                                                        if (lightDelay + step < current)
                                                            *start = (int)step + delay;
                                                    }
                                                    if (starting && POINT(other, w).tintFrom.w < SPLASH_LIT_LIMIT)
                                                    {
                                                        Matrix4 placement;
                                                        Vec4 origin;
                                                        for (int n = 0; n < SPLASH_NEIGHBOUR_COUNT; n++)
                                                        {
                                                            placement.SetIdentity();
                                                            placement._41 = SPLASH_OFFSET(SPLASH_SPREAD) + VERTEX(other, w).x;
                                                            placement._42 = SPLASH_OFFSET(SPLASH_SPREAD) + VERTEX(other, w).y;
                                                            placement._43 = SPLASH_OFFSET(SPLASH_HEIGHT) + VERTEX(other, w).z;
                                                            DAT_10daaa80.Start(SPLASH_EFFECT, origin, -1, -1, 0, 0, SPLASH_FRAME(POINT(other, w).morphStart), &placement, 0, 0);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
