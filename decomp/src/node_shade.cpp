// Wave shading of the child copies a node makes of itself (Node::CreateChildCopies): each copy gets a different phase,
// so the 32 copies show the same mesh with different wave-shaped normals and colours.

#include <math.h>

#include "vec4.h"
#include "plane.h"
#include "matrix4.h"
#include "mesh.h"
#include "node.h"
#include "render_vars.h"

Plane PlaneFromTriangle(const Vec4 &a, const Vec4 &origin, const Vec4 &b);  // 0x101af360 (vec_ops.cpp)

#define WAVE_PHASES 32
#define WAVE_PHASE_STEP 0.19634375f      // 2 pi / WAVE_PHASES
#define WAVE_FREQUENCY_NEAR 0.05f        // per unit of distance to the box's lower corner
#define WAVE_FREQUENCY_FAR 0.02f         // per unit of distance to the box's upper corner
#define WAVE_NEAR_WEIGHT 1.5f
#define WAVE_NORMAL_SCALE 16.0f          // the wave height used for the vertex normals' z and the lighting triangle
#define LIGHT_TRIANGLE_STEP 10.0f        // size of the triangle the lighting normal is taken from
#define LIGHT_BRIGHTNESS 0.2f
#define COLOUR_ALPHA 0.62f
#define DARK_RED 0.2f
#define DARK_GREEN_BLUE 0.32f
#define LIGHT_RED 0.6f
#define LIGHT_GREEN_BLUE 0.82f

// One wave term: the cosine of `distance` scaled by `frequency` and shifted by `shift`.
inline float WaveTerm(float distance, float frequency, float shift)
{
    return (float)cos(distance * frequency + shift);
}

// 0x10193d10 (ShadeWaveCopy). Gives every vertex of this node the colours and normals of a wave pattern: the normal's
// x, y and z are sums of two cosines of the vertex's distance from the box's lower and upper corner (shifted by
// `phase`, 0..31, one per copy), the colour is a fixed grey-green (lighter when DAT_10203418 is set) and the second
// colour is the wave-lit brightness: the dot product of the normal of a small triangle on the wave surface with the
// light direction, or 0 when facing away. Sets render mode 0xc. Returns 1.
int Node::ShadeWaveCopy(int phase)
{
    drawMode = 0xc;
    Vec4 normal;
    Vec4 light;
    light.Set(0.5f, 0.6f, -0.4f, 0);
    for (int pack = 0; pack < packCount; pack++)
    {
        float wavePhase = (float)(phase % WAVE_PHASES) * WAVE_PHASE_STEP;
        Vec4 nearOffset;  // x, y = offset to the box's lower corner, z = its length
        Vec4 farOffset;   // the same for the upper corner
        for (int v = 0; v < packs[pack].vertexCount; v++)
        {
            nearOffset.x = packs[pack].vertices[v].position[0] - box[NODE_BOX_MIN].x;
            nearOffset.y = packs[pack].vertices[v].position[1] - box[NODE_BOX_MIN].y;
            nearOffset.z = (float)sqrt(nearOffset.x * nearOffset.x + nearOffset.y * nearOffset.y);
            farOffset.x = packs[pack].vertices[v].position[0] - box[NODE_BOX_MAX].x;
            farOffset.y = packs[pack].vertices[v].position[1] - box[NODE_BOX_MAX].y;
            farOffset.z = (float)sqrt(farOffset.x * farOffset.x + farOffset.y * farOffset.y);
            float nearWave0 = WaveTerm(nearOffset.z, WAVE_FREQUENCY_NEAR, wavePhase + wavePhase) * WAVE_NEAR_WEIGHT;
            packs[pack].vertices[v].normal[0] = nearWave0 + WaveTerm(farOffset.z, WAVE_FREQUENCY_FAR, wavePhase);
            float nearWave1 = WaveTerm(nearOffset.z, WAVE_FREQUENCY_NEAR, wavePhase + wavePhase) * WAVE_NEAR_WEIGHT;
            packs[pack].vertices[v].normal[1] = nearWave1 + WaveTerm(farOffset.z, WAVE_FREQUENCY_FAR, wavePhase);
            float nearWave2 = WaveTerm(nearOffset.z, WAVE_FREQUENCY_NEAR, wavePhase + wavePhase);
            packs[pack].vertices[v].normal[2] = nearWave2 + WaveTerm(farOffset.z, WAVE_FREQUENCY_FAR, wavePhase);
            int darkColours = DAT_10203418 == 0;
            packs[pack].vertices[v].colour[3] = COLOUR_ALPHA;
            float greenBlue;
            if (!darkColours)
            {
                packs[pack].vertices[v].colour[0] = LIGHT_RED;
                greenBlue = LIGHT_GREEN_BLUE;
            }
            else
            {
                packs[pack].vertices[v].colour[0] = DARK_RED;
                greenBlue = DARK_GREEN_BLUE;
            }
            packs[pack].vertices[v].colour[1] = greenBlue;
            packs[pack].vertices[v].colour[2] = greenBlue;
            packs[pack].vertices[v].emissive[0] = 0;
            packs[pack].vertices[v].emissive[1] = 0;
            packs[pack].vertices[v].emissive[2] = 0;
            Vec4 corner[3];
            corner[0].x = packs[pack].vertices[v].position[0];
            corner[0].y = packs[pack].vertices[v].position[1];
            corner[0].z = packs[pack].vertices[v].normal[2] * WAVE_NORMAL_SCALE;
            nearOffset.x = packs[pack].vertices[v].position[0] + LIGHT_TRIANGLE_STEP - box[NODE_BOX_MIN].x;
            nearOffset.y = packs[pack].vertices[v].position[1] - box[NODE_BOX_MIN].y;
            nearOffset.z = (float)sqrt(nearOffset.x * nearOffset.x + nearOffset.y * nearOffset.y);
            farOffset.x = packs[pack].vertices[v].position[0] + LIGHT_TRIANGLE_STEP - box[NODE_BOX_MAX].x;
            farOffset.y = packs[pack].vertices[v].position[1] - box[NODE_BOX_MAX].y;
            farOffset.z = (float)sqrt(farOffset.x * farOffset.x + farOffset.y * farOffset.y);
            corner[1].x = packs[pack].vertices[v].position[0] + LIGHT_TRIANGLE_STEP;
            corner[1].y = packs[pack].vertices[v].position[1];
            corner[1].z = (WaveTerm(farOffset.z, WAVE_FREQUENCY_FAR, wavePhase) +
                           WaveTerm(nearOffset.z, WAVE_FREQUENCY_NEAR, wavePhase + wavePhase)) * WAVE_NORMAL_SCALE;
            nearOffset.x = packs[pack].vertices[v].position[0] - box[NODE_BOX_MIN].x;
            nearOffset.y = packs[pack].vertices[v].position[1] + LIGHT_TRIANGLE_STEP - box[NODE_BOX_MIN].y;
            nearOffset.z = (float)sqrt(nearOffset.x * nearOffset.x + nearOffset.y * nearOffset.y);
            farOffset.x = packs[pack].vertices[v].position[0] - box[NODE_BOX_MAX].x;
            farOffset.y = packs[pack].vertices[v].position[1] + LIGHT_TRIANGLE_STEP - box[NODE_BOX_MAX].y;
            farOffset.z = (float)sqrt(farOffset.x * farOffset.x + farOffset.y * farOffset.y);
            corner[2].x = packs[pack].vertices[v].position[0];
            corner[2].y = packs[pack].vertices[v].position[1] + LIGHT_TRIANGLE_STEP;
            corner[2].z = (WaveTerm(farOffset.z, WAVE_FREQUENCY_FAR, wavePhase) +
                           WaveTerm(nearOffset.z, WAVE_FREQUENCY_NEAR, wavePhase + wavePhase)) * WAVE_NORMAL_SCALE;
            Plane plane = PlaneFromTriangle(corner[0], corner[1], corner[2]);
            normal.x = plane.a;
            normal.y = plane.b;
            normal.z = plane.c;
            normal.w = plane.d;
            normal.Normalized3();
            float brightness;
            if (Vec3_Dot(&normal, &light) > 0)
                brightness = Vec3_Dot(&normal, &light);
            else
                brightness = 0;
            brightness = brightness * LIGHT_BRIGHTNESS;
            packs[pack].vertices[v].emissive[0] = brightness;
            packs[pack].vertices[v].emissive[1] = brightness;
            packs[pack].vertices[v].emissive[2] = brightness;
        }
    }
    return 1;
}
