// Building the water regions of a map (the table at DAT_104c03c0): saves each vertex as built, finds each region's
// box, hides the regions that lie wholly under the map's collision and shades the vertices that lie close to the
// lit vertices of other regions.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"
#include "node.h"
#include "mesh.h"

#include "region_view.h"
#include "map_vars.h"

extern "C" int FUN_10020090(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit);  // Collide_RayThroughRegions

// The game's atan2 wrapper (float arguments).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

#define BOX_START 999999.0f      // the box starts inverted: corners at +/- this
#define RAY_LENGTH 1700.0f       // probe direction (0, -1700, 1700)
#define RAY_SCALE 10000.0f
#define SINK_DEPTH 350.0f        // how far a vertex under the collision is lowered
#define SINK_DECAY 0.99f         // decay of the lit value of a free vertex
#define NEIGHBOUR_RANGE 900.0f   // reach of one vertex's influence on another region's
#define LIGHT_RATE 18.0f
#define LIGHT_LIMIT 0.055555556f // nearness below which the neighbour is lit up (1/18)
#define PI_TRUNCATED 3.1415f
#define TWO_PI_TRUNCATED 6.283f

// 0x10020560 (Regions_Build). For every region: marks it hidden and unsettled, saves each vertex as built (position
// and tint) and copies the position to its point, builds the region's box, and probes each vertex with a ray through
// the "testatari" collision of the map: a vertex under it is lowered by 350 and loses its lit value, a vertex that
// is free keeps 0.99 of it and makes the region visible. Then every vertex without a lit value looks at the lit
// vertices of the overlapping visible regions within 900 units: the closest one gives it a lit value, a lowering
// that grows with the closeness, and, depending on the angle between the direction to that vertex and the direction
// of the "@Light" node, a darker or lighter tint and offset. Finally stores the first texture slot of the "@moya"
// node. Returns 0.
extern "C" int FUN_10020560()
{
    Vec4 point;
    Vec4 second;
    Vec4 result;
    Vec4 boxMin;
    Vec4 boxMax;
    Vec4 lightPos;
    float lightX;
    float lightY;
    DAT_104c03c4 = 1;
    if (DAT_10255f08 != 0)
    {
        Node *light = DAT_10255f08->Find("@Light", 0);
        if (light != 0)
            lightPos.Copy(light->worldMatrix.GetTranslation());
        else
            lightPos.Fill(0.0f);
        for (int region = 0; region < DAT_102564a0; region++)
        {
            DAT_104c03c0[region].settled = 0;
            DAT_104c03c0[region].hidden = 1;
            boxMin.Fill(BOX_START);
            boxMax.Fill(-BOX_START);
            for (int v = 0; v < DAT_104c03c0[region].pointCount; v++)
            {
                DAT_104c03c0[region].vertices[v].baseX = DAT_104c03c0[region].vertices[v].x;
                DAT_104c03c0[region].vertices[v].baseY = DAT_104c03c0[region].vertices[v].y;
                DAT_104c03c0[region].vertices[v].baseZ = DAT_104c03c0[region].vertices[v].z;
                DAT_104c03c0[region].points[v].m_0060 = DAT_104c03c0[region].vertices[v].baseX;
                DAT_104c03c0[region].points[v].m_0064 = DAT_104c03c0[region].vertices[v].baseY;
                DAT_104c03c0[region].points[v].m_0068 = DAT_104c03c0[region].vertices[v].baseZ;
                DAT_104c03c0[region].vertices[v].baseTint.x = DAT_104c03c0[region].vertices[v].tint.x;
                DAT_104c03c0[region].vertices[v].baseTint.y = DAT_104c03c0[region].vertices[v].tint.y;
                DAT_104c03c0[region].vertices[v].baseTint.z = DAT_104c03c0[region].vertices[v].tint.z;
                DAT_104c03c0[region].vertices[v].baseTint.w = DAT_104c03c0[region].vertices[v].tint.w;
                DAT_104c03c0[region].vertices[v].nearness = 1.0f;
                DAT_104c03c0[region].points[v].morphing = 0;
                if (boxMin.x > DAT_104c03c0[region].vertices[v].x)
                    boxMin.x = DAT_104c03c0[region].vertices[v].x;
                if (boxMin.y > DAT_104c03c0[region].vertices[v].y)
                    boxMin.y = DAT_104c03c0[region].vertices[v].y;
                if (boxMin.z > DAT_104c03c0[region].vertices[v].z)
                    boxMin.z = DAT_104c03c0[region].vertices[v].z;
                if (boxMax.x < DAT_104c03c0[region].vertices[v].x)
                    boxMax.x = DAT_104c03c0[region].vertices[v].x;
                if (boxMax.y < DAT_104c03c0[region].vertices[v].y)
                    boxMax.y = DAT_104c03c0[region].vertices[v].y;
                if (boxMax.z < DAT_104c03c0[region].vertices[v].z)
                    boxMax.z = DAT_104c03c0[region].vertices[v].z;
                point.x = DAT_104c03c0[region].vertices[v].x;
                point.y = DAT_104c03c0[region].vertices[v].y;
                point.z = DAT_104c03c0[region].vertices[v].z;
                second.Copy(point);
                Vec4 ray;
                ray.x = 0.0f;
                ray.y = -RAY_LENGTH;
                ray.z = RAY_LENGTH;
                second.Copy(second - ray * RAY_SCALE);
                Node *collision = DAT_10255f08->Find("testatari", 0);
                if (collision == 0 || FUN_10020090(collision, 0, 4, &point, &second, &result) == 0)
                {
                    DAT_104c03c0[region].vertices[v].tint.w *= SINK_DECAY;
                    DAT_104c03c0[region].hidden = 0;
                }
                else
                {
                    DAT_104c03c0[region].vertices[v].tint.w = 0.0f;
                    DAT_104c03c0[region].vertices[v].z -= SINK_DEPTH;
                }
                DAT_104c03c0[region].points[v].m_0068 = DAT_104c03c0[region].vertices[v].z;
            }
            DAT_104c03c0[region].min.Copy(boxMin);
            DAT_104c03c0[region].max.Copy(boxMax);
        }
        for (int region = 0; region < DAT_102564a0; region++)
        {
            if (DAT_104c03c0[region].hidden != 0)
                continue;
            for (int v = 0; v < DAT_104c03c0[region].pointCount; v++)
            {
                if (DAT_104c03c0[region].vertices[v].tint.w != 0.0f)
                    continue;
                point.x = DAT_104c03c0[region].vertices[v].x;
                point.y = DAT_104c03c0[region].vertices[v].y;
                point.z = DAT_104c03c0[region].vertices[v].z;
                for (int other = 0; other < DAT_102564a0; other++)
                {
                    if (DAT_104c03c0[other].min.x <= point.x + NEIGHBOUR_RANGE && point.x - NEIGHBOUR_RANGE <= DAT_104c03c0[other].max.x &&
                        DAT_104c03c0[other].min.y <= point.y + NEIGHBOUR_RANGE && point.y - NEIGHBOUR_RANGE <= DAT_104c03c0[other].max.y &&
                        DAT_104c03c0[other].hidden == 0)
                    {
                        for (int w = 0; w < DAT_104c03c0[other].pointCount; w++)
                        {
                            if ((v != w || region != other) && DAT_104c03c0[other].vertices[w].tint.w != 0.0f)
                            {
                                second.x = DAT_104c03c0[other].vertices[w].x;
                                second.y = DAT_104c03c0[other].vertices[w].y;
                                second.z = DAT_104c03c0[other].vertices[w].z;
                                float distance = (point - second).Length2D();
                                if (distance < NEIGHBOUR_RANGE)
                                {
                                    float closeness = distance / NEIGHBOUR_RANGE;
                                    if (DAT_104c03c0[other].vertices[w].nearness > closeness)
                                    {
                                        DAT_104c03c0[other].vertices[w].nearness = closeness;
                                        if (DAT_104c03c0[other].vertices[w].nearness < LIGHT_LIMIT)
                                        {
                                            DAT_104c03c0[other].vertices[w].tint.w =
                                                DAT_104c03c0[region].vertices[v].baseTint.w * DAT_104c03c0[other].vertices[w].nearness * LIGHT_RATE;
                                            DAT_104c03c0[other].points[w].tintTo.w = DAT_104c03c0[other].vertices[w].tint.w;
                                        }
                                        float lift = 1.0f - DAT_104c03c0[other].vertices[w].nearness;
                                        lift = ((1.0f - lift) * 0.6f + lift) * lift;
                                        DAT_104c03c0[other].vertices[w].z = DAT_104c03c0[other].vertices[w].baseZ - lift * SINK_DEPTH;
                                        DAT_104c03c0[other].points[w].m_0068 = DAT_104c03c0[other].vertices[w].z;
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
                                            float blend = 1.0f - DAT_104c03c0[other].vertices[w].nearness;
                                            if (side < 0.0f)
                                            {
                                                side = (-side * blend + 1.0f) * 0.5f;
                                                side = (1.0f - side) * 0.3f + side;
                                                DAT_104c03c0[other].vertices[w].tint.x = side * DAT_104c03c0[other].vertices[w].baseTint.x;
                                                DAT_104c03c0[other].vertices[w].tint.y = side * DAT_104c03c0[other].vertices[w].baseTint.y;
                                                DAT_104c03c0[other].vertices[w].tint.z = side * DAT_104c03c0[other].vertices[w].baseTint.z;
                                                DAT_104c03c0[other].vertices[w].offset.x = side * DAT_104c03c0[other].vertices[w].offset.x;
                                                DAT_104c03c0[other].vertices[w].offset.y = side * DAT_104c03c0[other].vertices[w].offset.y;
                                                DAT_104c03c0[other].vertices[w].offset.z = side * DAT_104c03c0[other].vertices[w].offset.z;
                                            }
                                            else
                                            {
                                                side = (1.0f - side) * blend * 0.2f;
                                                side = side * 0.35f;
                                                DAT_104c03c0[other].vertices[w].offset.x =
                                                    (1.0f - DAT_104c03c0[other].vertices[w].offset.x) * side + DAT_104c03c0[other].vertices[w].offset.x;
                                                DAT_104c03c0[other].vertices[w].offset.y =
                                                    (1.0f - DAT_104c03c0[other].vertices[w].offset.y) * side + DAT_104c03c0[other].vertices[w].offset.y;
                                                DAT_104c03c0[other].vertices[w].offset.z =
                                                    (1.0f - DAT_104c03c0[other].vertices[w].offset.z) * side + DAT_104c03c0[other].vertices[w].offset.z;
                                                DAT_104c03c0[other].vertices[w].tint.x = DAT_104c03c0[other].vertices[w].baseTint.x;
                                                DAT_104c03c0[other].vertices[w].tint.y = DAT_104c03c0[other].vertices[w].baseTint.y;
                                                DAT_104c03c0[other].vertices[w].tint.z = DAT_104c03c0[other].vertices[w].baseTint.z;
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
        Node *haze = DAT_10255f08->Find("@moya", 0);
        DAT_102028e0 = *haze->mesh->materials;
    }
    return 0;
}
