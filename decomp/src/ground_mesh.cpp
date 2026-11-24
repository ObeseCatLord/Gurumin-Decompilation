// The mesh of a ground object: a flat grid of triangles over its height map whose vertices are lifted to the height
// samples and shaded by the slope of the ground against a fixed light.

#include "vec4.h"

#include "matrix4.h"
#include "mesh.h"
#include "node.h"

#include "game_vars.h"
#include "ground_height.h"
#include "ground_mesh.h"
#include "map_vars.h"
#include "render_vars.h"

#define GROUND_SLOT_COUNT 0x60       // ground object slots
#define GRID_FIRST (-5)              // the first height sample index of the grid (the map starts 5 steps before the object)
#define GRID_CELLS 12                // cells per side
#define GRID_STEP 25                 // distance between samples
#define GRID_VERTICES (14 * 14 * 6)  // room for the six vertices of each cell
#define SHADE_UV_SCALE 0.125f        // texture coordinate per two samples
#define NEUTRAL_COLOUR 0.5f
#define NODE_FLAGS_REBUILD 0x14      // drawn, and the other bit set while the mesh is rebuilt
#define NODE_FLAGS_BUILT 0x4000010   // drawn, and the mesh was built
#define LIGHT_X 0.2f
#define LIGHT_Y 0.2f
#define LIGHT_Z 1.2f
#define SHADE_WEIGHT 0.7f            // brightness = 0.3 + 0.7 * how much the ground faces the light
#define SHADE_FLOOR 0.3f
#define HEIGHT_RED_DIVISOR 1600.0f   // the height brightens the red and green channels
#define HEIGHT_BLUE_DIVISOR 2000.0f  // and, less, the blue channel

// 0x10016ec0 (Ground_BuildMesh). For `slot` (or every slot when it is negative) that holds a ground object: converts
// `area`, when given, from world to object coordinates (the saved copy is restored before each object), marks the
// node, and unless `shadeOnly` builds a 12 x 12 cell grid (six vertices per cell, grid coordinates stored in the
// vertex) and gives it to the node. Then every vertex is placed at its height sample and shaded: the normal is the
// sum of the eight triangle normals around the sample, the brightness 0.3 + 0.7 * dot(normal, light) (0 when facing
// away) is kept in DAT_1031568c and added to the colour with the height. Finishes with the bounds and the buffers.
extern "C" void FUN_10016ec0(IntRect *area, int slot, int shadeOnly)
{
    Vertex vertices[GRID_VERTICES];
    int indices[GRID_VERTICES];
    IntRect saved = {0, 0, 0, 0};
    if (area != 0)
        saved = *area;

    int last;
    if (slot < 0)
    {
        slot = 0;
        last = GROUND_SLOT_COUNT;
    }
    else
    {
        last = slot + 1;
    }
    for (; slot < last; slot++)
    {
        if (DAT_1020d860[slot] == 0)
            continue;
        if (area != 0)
        {
            *area = saved;
            area->top -= (int)DAT_1020d860[slot]->y;
            area->bottom -= (int)DAT_1020d860[slot]->y;
            area->left -= (int)DAT_1020d860[slot]->x;
            area->right -= (int)DAT_1020d860[slot]->x;
        }
        Node *ground = (Node *)DAT_1020d860[slot];
        if (ground == 0)
            continue;
        int count = 0;
        int indexCount = 0;
        ground->flags |= NODE_FLAGS_REBUILD;
        if (shadeOnly == 0)
        {
            for (int row = GRID_FIRST; row < GRID_FIRST + GRID_CELLS; row++)
            {
                float rowNext = (float)(row + 1);
                float v1 = rowNext / 2.0f * SHADE_UV_SCALE;
                float bottom = (float)(row * GRID_STEP + GRID_STEP);
                float rowThis = (float)row;
                float v0 = rowThis / 2.0f * SHADE_UV_SCALE;
                float top = (float)(row * GRID_STEP);
                for (int column = GRID_FIRST; column < GRID_FIRST + GRID_CELLS; column++)
                {
                    float columnThis = (float)column;
                    float u0 = columnThis / 2.0f * SHADE_UV_SCALE;
                    float left = (float)(column * GRID_STEP);
                    vertices[count].Set(left, top, 0, 0, 0, 1.0f, u0, v0, NEUTRAL_COLOUR, NEUTRAL_COLOUR,
                                        NEUTRAL_COLOUR, 1.0f);
                    vertices[count].m_0060 = columnThis;
                    vertices[count].m_0064[0] = rowThis;
                    indices[indexCount++] = count;
                    vertices[count].sourceMaterial = 0;
                    vertices[count].material = 0;
                    count++;
                    vertices[count].Set(left, bottom, 0, 0, 0, 1.0f, u0, v1, NEUTRAL_COLOUR, NEUTRAL_COLOUR,
                                            NEUTRAL_COLOUR, 1.0f);
                    vertices[count].m_0060 = columnThis;
                    vertices[count].m_0064[0] = rowNext;
                    indices[indexCount++] = count;
                    vertices[count].sourceMaterial = 0;
                    vertices[count].material = 0;
                    count++;
                    float columnNext = (float)(column + 1);
                    float u1 = columnNext / 2.0f * SHADE_UV_SCALE;
                    float right = (float)(column * GRID_STEP + GRID_STEP);
                    vertices[count].Set(right, bottom, 0, 0, 0, 1.0f, u1, v1, NEUTRAL_COLOUR, NEUTRAL_COLOUR,
                                            NEUTRAL_COLOUR, 1.0f);
                    vertices[count].m_0060 = columnNext;
                    vertices[count].m_0064[0] = rowNext;
                    indices[indexCount++] = count;
                    vertices[count].sourceMaterial = 0;
                    vertices[count].material = 0;
                    count++;
                    vertices[count].Set(left, top, 0, 0, 0, 1.0f, u0, v0, NEUTRAL_COLOUR, NEUTRAL_COLOUR,
                                            NEUTRAL_COLOUR, 1.0f);
                    vertices[count].sourceMaterial = 0;
                    vertices[count].material = 0;
                    vertices[count].m_0060 = columnThis;
                    vertices[count].m_0064[0] = rowThis;
                    indices[indexCount++] = count;
                    count++;
                    vertices[count].Set(right, bottom, 0, 0, 0, 1.0f, u1, v1, NEUTRAL_COLOUR, NEUTRAL_COLOUR,
                                            NEUTRAL_COLOUR, 1.0f);
                    vertices[count].m_0060 = columnNext;
                    vertices[count].m_0064[0] = rowNext;
                    indices[indexCount++] = count;
                    vertices[count].sourceMaterial = 0;
                    vertices[count].material = 0;
                    count++;
                    vertices[count].Set(right, top, 0, 0, 0, 1.0f, u1, v0, NEUTRAL_COLOUR, NEUTRAL_COLOUR,
                                            NEUTRAL_COLOUR, 1.0f);
                    vertices[count].m_0060 = columnNext;
                    vertices[count].m_0064[0] = rowThis;
                    indices[indexCount++] = count;
                    vertices[count].sourceMaterial = 0;
                    vertices[count].material = 0;
                    count++;
                }
            }
        }
        ground->m_1894 = 1;
        if (shadeOnly == 0)
        {
            DAT_11b04500 = 0;
            ground->BuildMesh(vertices, indices, count);
            DAT_11b04500 = 0;
        }
        Vec4 around[9];
        Vec4 normal;
        Vec4 light;
        light.Set(LIGHT_X, LIGHT_Y, LIGHT_Z, 0);
        for (int pack = 0; pack < ground->packCount; pack++)
        {
            for (int index = 0; index < ground->packs[pack].vertexCount; index++)
            {
                int column = (int)ground->packs[pack].vertices[index].m_0060;
                int row = (int)ground->packs[pack].vertices[index].m_0064[0];
                ground->packs[pack].vertices[index].position[0] = (float)(column * GRID_STEP);
                ground->packs[pack].vertices[index].position[1] = (float)(row * GRID_STEP);
                ground->packs[pack].vertices[index].position[2] = DAT_1049f3dc[slot][column][row];
                ground->packs[pack].vertices[index].normal[0] = 0;
                ground->packs[pack].vertices[index].normal[1] = 0;
                ground->packs[pack].vertices[index].normal[2] = 1.0f;
                around[0].Set(-25.0f, -25.0f, DAT_1049f3dc[slot][column - 1][row - 1], 0);
                around[1].Set(0, -25.0f, DAT_1049f3dc[slot][column][row - 1], 0);
                around[2].Set(25.0f, -25.0f, DAT_1049f3dc[slot][column + 1][row - 1], 0);
                around[3].Set(-25.0f, 0, DAT_1049f3dc[slot][column - 1][row], 0);
                around[4].Set(0, 0, DAT_1049f3dc[slot][column][row], 0);
                around[5].Set(25.0f, 0, DAT_1049f3dc[slot][column + 1][row], 0);
                around[6].Set(-25.0f, -25.0f, DAT_1049f3dc[slot][column - 1][row + 1], 0);
                around[7].Set(0, -25.0f, DAT_1049f3dc[slot][column][row + 1], 0);
                around[8].Set(25.0f, -25.0f, DAT_1049f3dc[slot][column + 1][row + 1], 0);
                normal.Copy(Triangle_Normal(around[0], around[4], around[1]));
                normal.Copy(normal + Triangle_Normal(around[1], around[4], around[2]));
                normal.Copy(normal + Triangle_Normal(around[3], around[4], around[0]));
                normal.Copy(normal + Triangle_Normal(around[2], around[4], around[5]));
                normal.Copy(normal + Triangle_Normal(around[3], around[4], around[6]));
                normal.Copy(normal + Triangle_Normal(around[6], around[4], around[7]));
                normal.Copy(normal + Triangle_Normal(around[7], around[4], around[8]));
                normal.Copy(normal + Triangle_Normal(around[8], around[4], around[5]));
                normal.Copy(Vec3_Normalized(&normal));
                float facing;
                if (Vec3_Dot(&normal, &light) > 0.0f)
                    facing = Vec3_Dot(&normal, &light);
                else
                    facing = 0.0f;
                float brightness = facing * SHADE_WEIGHT + SHADE_FLOOR;
                DAT_1031568c[slot][column][row] = brightness;
                ground->packs[pack].vertices[index].colour[0] =
                    DAT_1049f3dc[slot][column][row] / HEIGHT_RED_DIVISOR + brightness;
                ground->packs[pack].vertices[index].colour[1] =
                    DAT_1049f3dc[slot][column][row] / HEIGHT_RED_DIVISOR + brightness;
                ground->packs[pack].vertices[index].colour[2] =
                    DAT_1049f3dc[slot][column][row] / HEIGHT_BLUE_DIVISOR + brightness;
            }
        }
        ground->ComputeBounds();
        DAT_10f655c0 = 1;
        ground->BuildMesh(1, 0);
        DAT_10f655c0 = 0;
        ground->flags |= NODE_FLAGS_BUILT;
    }
}
