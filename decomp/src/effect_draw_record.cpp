// Drawing one running effect (EffectManager::DrawRecord): every effect kind that is not a light or a sprite of the
// two sprite tables is drawn here as its own small piece of geometry: quads, discs, bursts, a cone, a flare, a
// ribbon built in a spare light and the models of the model effects. The tracks the definition declares give the
// colours, sizes and texture rectangles; a track the definition leaves out has its own default.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "texture_object.h"

#include "tri_batch.h"

#include "key_track.h"

#include "node.h"

#include "camera_vars.h"

#include "map_vars.h"

#include "render_state.h"

#include "render_vars.h"

#include "debug_shape.h"

#include "effect_manager.h"

#include "effect_trail.h"

#include "effect_draw.h"

// Where the 640x480 screen has its centre.
#define SCREEN_CENTRE_X 320.0f
#define SCREEN_CENTRE_Y 240.0f
// A disc of kind 0xc is already visible from this depth.
#define DISC_NEAR_DEPTH_C 0.5
// Depth 0.9 to 1.0 is in front of the camera; a sprite shrinks as its depth rises towards 1.
#define SPRITE_DEPTH_SCALE 8.0f
// Width of the streak points in the light, per unit of the track value.
#define STREAK_WIDTH_SCALE 10.0f
// A full turn of the cone's and the disc's rings (the game's own rounding of 2 pi).
#define RING_TURN 6.283f
// Default size of the flat shapes and of the disc rings.
#define SHAPE_DEFAULT_SIZE 10.0f
// Points of the streak of kind 0x17 and rings of the cone.
#define STREAK_POINT_COUNT 13
#define CONE_RING_COUNT 3
#define CONE_RING_POINTS 6
#define CONE_VERTEX_COUNT (CONE_RING_COUNT * CONE_RING_POINTS)
#define CONE_TRIANGLE_SHORTS 0x24
#define CONE_INDEX_COUNT 0x48
#define CONE_SHADER 0x24
#define CONE_MESH_VERTICES 0x15
// Disc: points on a ring, points in all, and the number of triangle corners (three bands of twelve triangles).
#define DISC_RING_POINTS 7
#define DISC_POINT_COUNT 0x20
#define DISC_BAND_CORNERS 0x24
#define DISC_VERTEX_COUNT 0x6c
// The outermost and innermost rings of a disc are drawn without alpha: points 0 to 6 and above 0x14.
#define DISC_LAST_INNER_POINT 0x14
// Vertices of the two bursts.
#define BURST_SQUARE_VERTICES 0x1e
#define BURST_HEXAGON_VERTICES 0x30
// The burst shapes are drawn in a 10 x 10 cell around (5, 5), a tenth of the cell per texture unit.
#define BURST_CELL_CENTRE 5.0f
#define BURST_UV_SCALE 0.1f
// Vertices of the flare strip.
#define FLARE_VERTEX_COUNT 8
// Where the sprite records of each draw pass live: pass n uses the records [start, end).
static const int SPRITE_RING_START[EFFECT_SPRITE_PASS_COUNT] = { 0, 0x1cc, 0x244, 0x2a8, 0x2f8 };
static const int SPRITE_RING_END[EFFECT_SPRITE_PASS_COUNT] = { 0x1cc, 0x244, 0x2a8, 0x2f8, EFFECT_SPRITE_COUNT };

// How far along the streak each of its points is.
static const float STREAK_FRACTIONS[STREAK_POINT_COUNT] = {
    0.0f, 0.01f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.99f, 1.0f
};
// Corners of the disc triangles: a band of twelve triangles between two rings, repeated for the three bands (+7).
static const int DISC_TRIANGLES[DISC_BAND_CORNERS] = {
    0, 7, 1, 1, 7, 8, 1, 8, 2, 2, 8, 9, 2, 9, 3, 3, 9, 10, 3, 10, 4, 4, 10, 11, 4, 11, 5, 5, 11, 12, 5, 12, 6, 6, 12, 13
};
// The cone's triangles between its three rings, as shorts (the second set is six points further).
static const short CONE_TRIANGLES[CONE_TRIANGLE_SHORTS] = {
    0, 6, 7, 0, 7, 1, 1, 7, 8, 1, 8, 2, 2, 8, 9, 2, 9, 3, 3, 10, 4, 3, 9, 10, 4, 10, 11, 4, 11, 5, 5, 11, 6, 5, 6, 0
};
// The outer and inner hexagon of the hexagon burst: x and y of its twelve points, and the triangle corners.
static const float HEXAGON_X[12] = { 3.0f, 7.0f, 10.0f, 7.0f, 3.0f, 0.0f, 4.0f, 6.0f, 8.0f, 6.0f, 4.0f, 2.0f };
static const float HEXAGON_Y[12] = { 0.0f, 0.0f, 5.0f, 10.0f, 10.0f, 5.0f, 2.0f, 2.0f, 5.0f, 8.0f, 8.0f, 5.0f };
static const int HEXAGON_TRIANGLES[BURST_HEXAGON_VERTICES] = {
    0, 1, 7, 0, 7, 6, 1, 2, 8, 1, 8, 7, 2, 3, 9, 2, 9, 8, 3, 4, 10, 3, 10, 9,
    4, 5, 11, 4, 11, 10, 5, 0, 6, 5, 6, 11, 10, 11, 6, 9, 10, 6, 9, 6, 7, 9, 7, 8
};
// The outer and inner square of the square burst.
static const float SQUARE_X[8] = { 0.0f, 10.0f, 10.0f, 0.0f, 2.5f, 7.5f, 7.5f, 2.5f };
static const float SQUARE_Y[8] = { 0.0f, 0.0f, 10.0f, 10.0f, 2.5f, 2.5f, 7.5f, 7.5f };
static const int SQUARE_TRIANGLES[BURST_SQUARE_VERTICES] = {
    0, 1, 5, 0, 5, 4, 1, 2, 5, 2, 6, 5, 2, 3, 7, 2, 7, 6, 3, 0, 4, 3, 4, 7, 4, 5, 6, 4, 6, 7
};
// Texture coordinates of the flare strip's vertices.
static const float FLARE_U[FLARE_VERTEX_COUNT] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f };
static const float FLARE_V[FLARE_VERTEX_COUNT] = { 0.0f, 0.0f, 0.4f, 0.4f, 0.6f, 0.6f, 1.0f, 1.0f };

// The sprite blend mode a definition's `blend` value stands for, -1 for an unknown value.
static int BlendMode(int blend)
{
    static const int modes[7] = { BLEND_ALPHA, BLEND_ADD, BLEND_MULTIPLY, 10, 11, 12, 13 };
    return blend >= 0 && blend < 7 ? modes[blend] : -1;
}

// Sets the blend mode of the next texture draw.
static void SetDrawBlend(int blend)
{
    int mode = BlendMode(blend);
    if (mode >= 0)
        FUN_1013f070(RENDER_BLEND_MODE, mode, 0);
}

// Sets the blend mode of the effect batch.
static void SetBatchBlend(int blend)
{
    int mode = BlendMode(blend);
    if (mode >= 0)
        DAT_1101c804.SetBlend(mode, 0);
}

// 1 when the effect's position is far enough outside the visible area that it is not drawn.
static int OutsideVisibleArea(const EffectRecord &rec)
{
    const Vec4 &point = rec.partPositions[rec.part];
    if (point.x <= DAT_10f668a8.min.x - TRAIL_CULL_MARGIN)
        return 1;
    if (point.y <= DAT_10f668a8.min.y - TRAIL_CULL_MARGIN)
        return 1;
    if (point.z <= DAT_10f668a8.min.z - TRAIL_CULL_MARGIN_NEAR)
        return 1;
    if (DAT_10f668a8.max.x + TRAIL_CULL_MARGIN <= point.x)
        return 1;
    if (DAT_10f668a8.max.y + TRAIL_CULL_MARGIN <= point.y)
        return 1;
    if (DAT_10f668a8.max.z + TRAIL_CULL_MARGIN_FAR <= point.z)
        return 1;
    return 0;
}

// Moves a projected point (-1..1) to the 640x480 screen.
static void ToScreen(Vec4 &point)
{
    point.x = point.x * TRAIL_SCREEN_WIDTH / 2.0f + SCREEN_CENTRE_X;
    point.y = SCREEN_CENTRE_Y - point.y * TRAIL_SCREEN_HEIGHT / 2.0f;
}

static int OffScreen(float x, float y)
{
    return x < -TRAIL_HIDE_DISTANCE || x > TRAIL_HIDE_DISTANCE || y < -TRAIL_HIDE_DISTANCE || y > TRAIL_HIDE_DISTANCE;
}

// A colour channel (1.0 = full) as a byte.
static unsigned int Channel(float value)
{
    float limited = 1.0f <= value ? 1.0f : value;
    return (unsigned int)(int)(limited * TRAIL_CHANNEL_SCALE);
}

// The diffuse colour with the given alpha.
static unsigned int PackArgb(const Vec4 &colour, float alpha)
{
    return (((Channel(alpha) << 8 | Channel(colour.x)) << 8 | Channel(colour.y)) << 8) | Channel(colour.z);
}

// The diffuse colour of a vertex that fades out completely (alpha 0).
static unsigned int PackRgb(const Vec4 &colour)
{
    return ((Channel(colour.x) << 8 | Channel(colour.y)) << 8 | Channel(colour.z)) & 0xffffff;
}

static unsigned int PackSpecular(const Vec4 &colour)
{
    return (Channel(colour.z) << 8 | Channel(colour.y)) << 8 | Channel(colour.x);
}

// Fills a vertex that is already on the screen.
static void SetVertex(ScreenVertex &vertex, float x, float y, float z, float rhw, unsigned int diffuse,
                      unsigned int specular, float u, float v)
{
    vertex.x = x;
    vertex.y = y;
    vertex.z = z;
    vertex.rhw = rhw;
    vertex.diffuse = diffuse;
    vertex.specular = specular;
    vertex.u = u;
    vertex.v = v;
}

// Reads track flag `flag` (the definition's tracks 4 to 10) at the effect's frame, or fills the vector with `fill`.
static void ReadTrack(const EffectDef &def, EffectDef &head, int flag, const EffectRecord &rec, Vec4 &out, float fill)
{
    if (def.trackFlags[flag] == 0)
        out.Fill(fill);
    else
        head.tracks[flag + 4].Sample(rec.m_0010, &out.x, &out.y, &out.z, &out.w, 0);
}

// The effect colour (track flag 1, default white), multiplied by the manager's colour when the definition says so.
static void ReadColour(EffectManager &manager, const EffectDef &def, EffectDef &head, const EffectRecord &rec,
                       Vec4 &colour)
{
    if (def.trackFlags[1] == 0)
        colour.Fill(1.0f);
    else
    {
        head.tracks[5].Sample(rec.m_0010, &colour.x, &colour.y, &colour.z, &colour.w, 0);
        if (def.m_0261 == 3)
            colour.Copy(colour * manager.colour);
    }
}

// The turn of the effect about x and y (track flag 6, none by default).
static void ReadTurn(const EffectDef &def, EffectDef &head, const EffectRecord &rec, float &turnX, float &turnY)
{
    turnX = 0.0f;
    turnY = 0.0f;
    if (def.trackFlags[6] != 0)
    {
        float rest;
        head.tracks[10].Sample(rec.m_0010, &turnX, &turnY, &rest, &rest, 0);
    }
}

// Leaves a sprite of the effect in a sprite table (it fades out over `lifetime` frames, drawn by DrawSpriteA/B):
// takes the next record of the pass's ring.
static void LeaveSprite(EffectSprite *table, int &cursor, int pass, const EffectRecord &rec, const Vec4 &position,
                        const Vec4 &colour, const Vec4 &colour2, float size, float rotation, float u0, float v0,
                        float u1, float v1, int lifetime)
{
    EffectSprite &sprite = table[cursor];
    sprite.type = rec.id;
    sprite.list = rec.options.list;
    sprite.position.Copy(position);
    sprite.size = size;
    sprite.u0 = u0;
    sprite.v0 = v0;
    sprite.u1 = u1;
    sprite.v1 = v1;
    sprite.colour2[0] = colour2.x;
    sprite.colour2[1] = colour2.y;
    sprite.colour2[2] = colour2.z;
    sprite.m_0078 = rotation;
    sprite.colour[0] = colour.x;
    sprite.colour[1] = colour.y;
    sprite.colour[2] = colour.z;
    sprite.alpha = colour.w;
    sprite.lifetime = (short)lifetime;
    sprite.age = 0;
    cursor = cursor + 1;
    if (SPRITE_RING_END[pass] <= cursor)
        cursor = SPRITE_RING_START[pass];
}

// The debug shapes of kinds 0x10 and 0xf and of definitions with a hit test (shown while the debug flag is set).
static void DrawDebugShape(EffectDef &def, EffectDef &head, EffectRecord &rec, const Matrix4 &view)
{
    if (def.kind == EFFECT_KIND_DEBUG_CYLINDER || def.kind == EFFECT_KIND_DEBUG_BOX)
    {
        float extent = 0.0f;
        float rest = 0.0f;
        head.tracks[1].Sample(rec.m_0010, &extent, &rest, &rest, &rest, 0);
        if (0.0f < extent)
        {
            float reachA;
            float reachB;
            float radius;
            if (def.trackFlags[0] == 0)
            {
                reachA = 100.0f;
                reachB = 100.0f;
                radius = 30.0f;
            }
            else
                head.tracks[4].Sample(rec.m_0010, &reachA, &reachB, &radius, &rest, 0);
            Vec4 ends[2];
            if (def.kind == EFFECT_KIND_DEBUG_CYLINDER)
            {
                ends[0].Set(0.0f, 0.0f, 0.0f, 0.0f);
                ends[1].Set(0.0f, 0.0f, 0.0f, 0.0f);
                ends[0].Copy(Project(ends[0], rec.startMatrix));
            }
            else
            {
                ends[0].Set(-reachA, 0.0f, 0.0f, 0.0f);
                ends[1].Set(reachB, 0.0f, 0.0f, 0.0f);
                ends[0].Copy(Project(ends[0], rec.worldMatrix));
            }
            ends[1].Copy(Project(ends[1], rec.worldMatrix));
            ends[0].w = radius;
            ends[1].w = radius;
            Matrix4 identity;
            identity.SetIdentity();
            FUN_1013f070(RENDER_BLEND_MODE, 0, 0);
            FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
            FUN_1013f070(RENDER_CULL_MODE, 1, 0);
            Debug_DrawShape(DEBUG_SHAPE_CYLINDER, &ends[0], &ends[1], &identity, &view);
        }
    }
    else if (def.m_0018[0] != 0)
    {
        Matrix4 placement;
        placement.SetIdentity();
        placement.Translate(rec.worldMatrix.translation.x, rec.worldMatrix.translation.y,
                            rec.worldMatrix.translation.z);
        float radius = 0.0f;
        float rest = 0.0f;
        head.tracks[1].Sample(rec.m_0010, &radius, &rest, &rest, &rest, 0);
        Vec4 radii;
        radii.x = radius;
        radii.y = radius;
        radii.z = radius;
        FUN_1013f070(RENDER_BLEND_MODE, 0, 0);
        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
        FUN_1013f070(RENDER_CULL_MODE, 1, 0);
        Debug_DrawShape(DEBUG_SHAPE_ELLIPSOID, &radii, 0, &placement, &view);
    }
}

// Kind 0x17: a streak from where the effect started to where it is, built in light EFFECT_LIGHT_STREAK and drawn
// by DrawLight.
static int DrawStreak(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec, int definition,
                      const Matrix4 &view, const Matrix4 *camera)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colour;
    Vec4 colour2;
    Vec4 width;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    ReadTrack(def, head, 0, rec, width, 1.0f);
    SetDrawBlend(def.blend);
    EffectLight &streak = DAT_118e99f8[EFFECT_LIGHT_STREAK];
    streak.type = definition;
    streak.lastFrame = -1;
    streak.frame = 0;
    streak.fade = 1.0f;
    streak.pointCount = STREAK_POINT_COUNT;
    Vec4 from;
    Vec4 to;
    from.Set(rec.startMatrix.translation.x, rec.startMatrix.translation.y, rec.startMatrix.translation.z, 0.0f);
    to.Set(rec.worldMatrix.translation.x, rec.worldMatrix.translation.y, rec.worldMatrix.translation.z, 0.0f);
    Vec4 along;
    along.Copy(to - from);
    for (int i = 0; i < streak.pointCount; i++)
    {
        streak.points[i].Copy(from + along * STREAK_FRACTIONS[i]);
        streak.widths[i].Fill(width.x * STREAK_WIDTH_SCALE);
        streak.colours[i].Copy(colour);
        streak.specular[i].Copy(colour2);
    }
    manager.DrawLight(EFFECT_LIGHT_STREAK, view, camera);
    return 0;
}

// Kind 0xe: a flare drawn as a strip of eight vertices between the effect's start and its position.
static int DrawFlare(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec, const Matrix4 &view)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colour;
    Vec4 colour2;
    Vec4 stretch;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    ReadTrack(def, head, 0, rec, stretch, 1.0f);
    SetDrawBlend(def.blend);
    Vec4 ends[2];
    Vec4 screen[2];
    ends[0].Set(rec.startMatrix.translation.x, rec.startMatrix.translation.y, rec.startMatrix.translation.z, 0.0f);
    ends[1].Set(rec.worldMatrix.translation.x, rec.worldMatrix.translation.y, rec.worldMatrix.translation.z, 0.0f);
    screen[0].Copy(TransformCoord(ends[0], view));
    screen[1].Copy(TransformCoord(ends[1], view));
    float alpha0 = 1.0f;
    float alpha1 = 1.0f;
    if (TRAIL_NEAR_DEPTH <= screen[0].z && 0.0f <= screen[0].w && screen[0].z <= TRAIL_FAR_DEPTH &&
        TRAIL_NEAR_DEPTH <= screen[1].z && 0.0f <= screen[1].w && screen[1].z <= TRAIL_FAR_DEPTH)
    {
        ToScreen(screen[0]);
        ToScreen(screen[1]);
        if (OffScreen(screen[0].x, screen[0].y))
            alpha0 = 0.0f;
        if (OffScreen(screen[1].x, screen[1].y))
            alpha1 = 0.0f;
        Vec4 along;
        along.Copy(screen[1] - screen[0]);
        Vec4 across;
        across.Copy(along.Normalized3());
        across.RotateZ(TRAIL_SIDE_TURN);
        across.x = across.x * stretch.x;
        across.y = across.y * stretch.x;
        across.z = 0.0f;
        across.w = 0.0f;
        along.x = along.x * stretch.y;
        along.y = along.y * stretch.y;
        along.z = 0.0f;
        along.w = 0.0f;
        Vec4 corner[FLARE_VERTEX_COUNT];
        corner[0].Copy((screen[0] - along) - across);
        corner[1].Copy((screen[0] - along) + across);
        corner[2].Copy(screen[0] - across);
        corner[3].Copy(screen[0] + across);
        corner[4].Copy(screen[1] - across);
        corner[5].Copy(screen[1] + across);
        corner[6].Copy((screen[1] + along) - across);
        corner[7].Copy((screen[1] + along) + across);
        ScreenVertex strip[FLARE_VERTEX_COUNT];
        for (int i = 0; i < FLARE_VERTEX_COUNT; i++)
        {
            int end = i < 4 ? 0 : 1;
            float alpha = i < 4 ? alpha0 : alpha1;
            SetVertex(strip[i], corner[i].x, corner[i].y, screen[end].z, 1.0f, PackArgb(colour, colour.w * alpha),
                      PackSpecular(colour2), FLARE_U[i], FLARE_V[i]);
        }
        DAT_118d0ba8[def.texture].DrawStrip(strip, FLARE_VERTEX_COUNT);
    }
    return 0;
}

// Kinds 0xd and 0xc: a disc of four rings (seven points each) round the effect's position, drawn as three bands of
// triangles; the outer and inner ring fade to transparent. `nearDepth` is where a point counts as in front of the
// camera.
static int DrawDisc(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec, const Matrix4 &view,
                    double nearDepth)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colour;
    Vec4 uvOrigin;
    Vec4 colour2;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 5, rec, uvOrigin, 0.0f);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    float innerRadius = SHAPE_DEFAULT_SIZE;
    float outerRadius = SHAPE_DEFAULT_SIZE;
    float height = SHAPE_DEFAULT_SIZE;
    if (def.trackFlags[0] != 0)
    {
        float rest;
        head.tracks[4].Sample(rec.m_0010, &innerRadius, &outerRadius, &height, &rest, 0);
    }
    Vec4 points[DISC_POINT_COUNT];
    Matrix4 placement = rec.worldMatrix * view;
    for (int i = 0; i < DISC_POINT_COUNT; i++)
    {
        float angle = (float)(i % DISC_RING_POINTS) / 6.0f * RING_TURN;
        float ring = (float)(i / DISC_RING_POINTS);
        float radius = outerRadius * ring + (float)(3 - i / DISC_RING_POINTS) * innerRadius;
        Vec4 point;
        point.x = (float)cos((double)angle) * radius;
        point.y = (float)sin((double)angle) * radius;
        point.z = height * ring;
        points[i].Copy(Project(point, placement));
        ToScreen(points[i]);
        if (def.flat)
            points[i].z = 0.0f;
    }
    ScreenVertex mesh[DISC_VERTEX_COUNT];
    float visible = 1.0f;
    for (int n = 0; n < DISC_VERTEX_COUNT; n++)
    {
        int index = DISC_TRIANGLES[n % DISC_BAND_CORNERS] + (n / DISC_BAND_CORNERS) * DISC_RING_POINTS;
        if (n % 3 == 0)
        {
            int second =
                DISC_TRIANGLES[(n + 1) % DISC_BAND_CORNERS] + ((n + 1) / DISC_BAND_CORNERS) * DISC_RING_POINTS;
            int third =
                DISC_TRIANGLES[(n + 2) % DISC_BAND_CORNERS] + ((n + 2) / DISC_BAND_CORNERS) * DISC_RING_POINTS;
            visible = 1.0f;
            if (points[index].z > 1.0f || points[second].z > 1.0f || points[third].z > 1.0f ||
                points[index].z < nearDepth || points[index].w < 0.0f || points[second].z < nearDepth ||
                points[second].w < 0.0f || points[third].z < nearDepth || points[third].w < 0.0f)
                visible = 0.0f;
            if (OffScreen(points[index].x, points[index].y) || OffScreen(points[second].x, points[second].y) ||
                OffScreen(points[third].x, points[third].y))
                visible = 0.0f;
        }
        unsigned int diffuse;
        if (index < DISC_RING_POINTS || index > DISC_LAST_INNER_POINT)
            diffuse = PackRgb(colour);
        else
            diffuse = PackArgb(colour, colour.w * visible);
        SetVertex(mesh[n], points[index].x, points[index].y, points[index].z, 1.0f / points[index].w, diffuse,
                  PackSpecular(colour2), (float)(index % DISC_RING_POINTS) / 6.0f + uvOrigin.x,
                  (float)(index / DISC_RING_POINTS) / 3.0f + uvOrigin.y);
    }
    SetBatchBlend(def.blend);
    DAT_1101c804.AddVertices(&DAT_118d0ba8[def.texture], mesh, DISC_VERTEX_COUNT);
    return 0;
}

// Kind 0x15: a cone of three rings of six points drawn as an indexed mesh through the effect vertex shader.
static int DrawCone(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec, const Matrix4 &view)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colour;
    Vec4 offset;
    Vec4 colour2;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 5, rec, offset, 0.0f);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    float turnX;
    float turnY;
    ReadTurn(def, head, rec, turnX, turnY);
    float radii[CONE_RING_COUNT] = { 10.0f, 15.0f, 20.0f };
    if (def.trackFlags[0] != 0)
    {
        float rest;
        head.tracks[4].Sample(rec.m_0010, &radii[0], &radii[1], &radii[2], &rest, 0);
    }
    EffectMeshVertex vertices[CONE_VERTEX_COUNT];
    for (int n = 0; n < CONE_VERTEX_COUNT; n++)
    {
        int ring = n / CONE_RING_POINTS;
        float angle = (float)(n % CONE_RING_POINTS) / 6.0f * RING_TURN;
        float middle = (float)(ring % 2);
        EffectMeshVertex &vertex = vertices[n];
        vertex.position[0] = (float)cos((double)angle) * radii[ring];
        vertex.position[1] = (float)sin((double)angle) * radii[ring];
        vertex.position[2] = 0.0f;
        vertex.position[3] = 1.0f;
        vertex.offset[0] = offset.x * middle;
        vertex.offset[1] = offset.y * middle;
        vertex.offset[2] = 0.0f;
        vertex.offset[3] = 1.0f;
        vertex.colour[0] = colour.x;
        vertex.colour[1] = colour.y;
        vertex.colour[2] = colour.z;
        vertex.colour[3] = colour.w * middle;
        vertex.spare[0] = 0.0f;
        vertex.spare[1] = 0.0f;
        vertex.spare[2] = 0.0f;
        vertex.spare[3] = 0.0f;
        vertex.colour2[0] = colour2.x;
        vertex.colour2[1] = colour2.y;
        vertex.colour2[2] = colour2.z;
        vertex.colour2[3] = colour2.w;
        vertex.spare2[0] = 0.0f;
        vertex.spare2[1] = 0.0f;
        vertex.spare2[2] = 0.0f;
        vertex.spare2[3] = 0.0f;
        vertex.one[0] = 1.0f;
        vertex.one[1] = 0.0f;
        vertex.one[2] = 0.0f;
        vertex.one[3] = 0.0f;
    }
    short indices[CONE_INDEX_COUNT];
    for (int n = 0; n < CONE_INDEX_COUNT; n++)
        indices[n] = (short)(CONE_TRIANGLES[n % CONE_TRIANGLE_SHORTS] + (n / CONE_TRIANGLE_SHORTS) * CONE_RING_POINTS);
    Matrix4 placement = rec.worldMatrix;
    if (turnX != 0.0f)
        placement = RotationX(turnX) * placement;
    if (turnY != 0.0f)
        placement = RotationY(turnY) * placement;
    SetDrawBlend(def.blend);
    DAT_118d0ba8[def.texture].DrawMesh(&placement, (Matrix4 *)&view, vertices, CONE_MESH_VERTICES, indices,
                                       CONE_INDEX_COUNT, CONE_SHADER, 0, 0);
    return 0;
}

// The quad kinds 5, 0xb and 0x13: four corners (in the quad's own plane) turned by the effect's roll and turns,
// placed with its world matrix, projected and drawn as a textured quad. Returns 0.
static int DrawPlacedQuad(EffectDef &def, EffectRecord &rec, const Matrix4 &view, const Vec4 corners[4],
                          const Vec4 uvs[4], const Vec4 colours[4], const Vec4 speculars[4], float turnX, float turnY)
{
    Matrix4 placement = RotationZ(rec.options.angle);
    if (turnX != 0.0f)
        placement = RotationX(turnX) * placement;
    if (turnY != 0.0f)
        placement = RotationY(turnY) * placement;
    placement = placement * rec.worldMatrix;
    Vec4 anchor;
    Vec4 screen[4];
    for (int i = 0; i < 4; i++)
    {
        Vec4 world = anchor + Project(corners[i], placement);
        screen[i].Copy(Project(world, view));
        ToScreen(screen[i]);
        screen[i].w = 1.0f / screen[i].w;
        if (OffScreen(screen[i].x, screen[i].y))
            return 0;
    }
    SetDrawBlend(def.blend);
    DAT_118d0ba8[def.texture].DrawQuad(screen, uvs, colours, speculars, 1.0f, 0);
    return 0;
}

// The colours of the four corners of a quad: the effect colour and the second colour.
static void FillQuadColours(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec,
                            Vec4 colours[4], Vec4 speculars[4])
{
    Vec4 colour;
    Vec4 colour2;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    for (int i = 0; i < 4; i++)
    {
        colours[i].Copy(colour);
        speculars[i].Copy(colour2);
    }
}

// The texture coordinates of a quad that shows the whole texture.
static void FillWholeTexture(Vec4 uvs[4])
{
    uvs[0].Set(0.0f, 0.0f, 0.0f, 0.0f);
    uvs[1].Set(1.0f, 0.0f, 0.0f, 0.0f);
    uvs[2].Set(0.0f, 1.0f, 0.0f, 0.0f);
    uvs[3].Set(1.0f, 1.0f, 0.0f, 0.0f);
}

// Kind 5: a square quad of half size `size` (track flag 0, 10 by default), the whole texture.
static int DrawQuadCentred(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec,
                           const Matrix4 &view)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colours[4];
    Vec4 speculars[4];
    Vec4 uvs[4];
    FillQuadColours(manager, def, head, rec, colours, speculars);
    FillWholeTexture(uvs);
    float turnX;
    float turnY;
    ReadTurn(def, head, rec, turnX, turnY);
    Vec4 size;
    ReadTrack(def, head, 0, rec, size, SHAPE_DEFAULT_SIZE);
    Vec4 corners[4];
    corners[0].Set(-size.x, size.x, 0.0f, 0.0f);
    corners[1].Set(size.x, size.x, 0.0f, 0.0f);
    corners[2].Set(-size.x, -size.x, 0.0f, 0.0f);
    corners[3].Set(size.x, -size.x, 0.0f, 0.0f);
    return DrawPlacedQuad(def, rec, view, corners, uvs, colours, speculars, turnX, turnY);
}

// Kind 0xb: a quad standing on its bottom edge: width 2 x size.x, height 2 x size.y (track flag 0, 10 by default).
static int DrawQuadStanding(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec,
                            const Matrix4 &view)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colours[4];
    Vec4 speculars[4];
    Vec4 uvs[4];
    FillQuadColours(manager, def, head, rec, colours, speculars);
    FillWholeTexture(uvs);
    Vec4 size;
    ReadTrack(def, head, 0, rec, size, SHAPE_DEFAULT_SIZE);
    Vec4 corners[4];
    corners[0].Set(-size.x, size.y + size.y, 0.0f, 0.0f);
    corners[1].Set(size.x, size.y + size.y, 0.0f, 0.0f);
    corners[2].Set(-size.x, 0.0f, 0.0f, 0.0f);
    corners[3].Set(size.x, 0.0f, 0.0f, 0.0f);
    return DrawPlacedQuad(def, rec, view, corners, uvs, colours, speculars, 0.0f, 0.0f);
}

// Kind 0x13: like kind 5 with the texture rectangle from the tracks (flags 5, 3 and 4: offset, shift and size).
static int DrawQuadTextured(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec,
                            const Matrix4 &view)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colours[4];
    Vec4 speculars[4];
    FillQuadColours(manager, def, head, rec, colours, speculars);
    float turnX;
    float turnY;
    ReadTurn(def, head, rec, turnX, turnY);
    Vec4 size;
    ReadTrack(def, head, 0, rec, size, SHAPE_DEFAULT_SIZE);
    Vec4 offset;
    Vec4 shift;
    Vec4 extent;
    ReadTrack(def, head, 5, rec, offset, 0.0f);
    ReadTrack(def, head, 3, rec, shift, 0.0f);
    ReadTrack(def, head, 4, rec, extent, 1.0f);
    Vec4 uvs[4];
    uvs[0].Set(offset.x + shift.x, offset.y + shift.y, 0.0f, 0.0f);
    uvs[1].Set(extent.x + offset.x, offset.y + shift.y, 0.0f, 0.0f);
    uvs[2].Set(offset.x + shift.x, extent.y + offset.y, 0.0f, 0.0f);
    uvs[3].Set(extent.x + offset.x, extent.y + offset.y, 0.0f, 0.0f);
    Vec4 corners[4];
    corners[0].Set(-size.x, size.x, 0.0f, 0.0f);
    corners[1].Set(size.x, size.x, 0.0f, 0.0f);
    corners[2].Set(-size.x, -size.x, 0.0f, 0.0f);
    corners[3].Set(size.x, -size.x, 0.0f, 0.0f);
    return DrawPlacedQuad(def, rec, view, corners, uvs, colours, speculars, turnX, turnY);
}

// Kinds 1 and 0x12: a textured square at the effect's position, a sprite that shrinks with the depth. The first
// time it is drawn (with no culling) it also leaves a sprite in each sprite table that its definition asks for.
static int DrawSpriteKind(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec,
                          const Matrix4 &view, int pass)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colour;
    Vec4 colour2;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    Vec4 turn;
    ReadTrack(def, head, 6, rec, turn, 0.0f);
    float rotation = def.trackFlags[6] == 0 ? 0.0f : turn.x;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    if (def.kind == EFFECT_KIND_SPRITE_RECT)
    {
        Vec4 uvOrigin;
        Vec4 uvShift;
        Vec4 uvSize;
        ReadTrack(def, head, 5, rec, uvOrigin, 0.0f);
        ReadTrack(def, head, 3, rec, uvShift, 0.0f);
        ReadTrack(def, head, 4, rec, uvSize, 1.0f);
        u0 = uvOrigin.x + uvShift.x;
        v0 = uvOrigin.y + uvShift.y;
        u1 = uvSize.x + uvOrigin.x;
        v1 = uvSize.y + uvOrigin.y;
    }
    Vec4 position;
    position.Copy(Project(rec.partPositions[rec.part], view));
    ToScreen(position);
    if (OffScreen(position.x, position.y) || position.z > TRAIL_FAR_DEPTH || position.z < TRAIL_NEAR_DEPTH)
        return 0;
    float size;
    if (def.trackFlags[0] == 0)
        size = (1.0f - position.z) * SPRITE_DEPTH_SCALE;
    else
    {
        Vec4 scale;
        ReadTrack(def, head, 0, rec, scale, 1.0f);
        size = (1.0f - position.z) * scale.x * SPRITE_DEPTH_SCALE;
    }
    if (manager.m_0050 == 0 && rec.spriteLeft[0] == 0 && def.spawnWorldSprite != 0)
    {
        rec.spriteLeft[0] = 1;
        LeaveSprite(DAT_11134cc8, manager.m_0010[pass], pass, rec, rec.partPositions[rec.part], colour, colour2, size,
                    rotation, u0, v0, u1, v1, def.worldSpriteLifetime);
    }
    if (def.flat)
        position.z = 0.0f;
    if (manager.m_0050 == 0 && rec.spriteLeft[1] == 0 && def.m_007c != 0)
    {
        rec.spriteLeft[1] = 1;
        LeaveSprite(DAT_118a8888, manager.m_0030[pass], pass, rec, position, colour, colour2, size, rotation, u0, v0,
                    u1, v1, def.lightFrames);
    }
    SetBatchBlend(def.blend);
    DAT_1101c804.AddQuad(&DAT_118d0ba8[def.texture], position.x - size, position.y - size, position.x + size,
                         position.y + size, u0, v0, u1, v1, position.z, colour.w, colour.x, colour.y, colour.z,
                         colour2.x, colour2.y, colour2.z, rotation);
    return 0;
}

// Kinds 2 and 3: two nested shapes (squares or hexagons) at the effect's position whose outer ring is transparent.
static int DrawBurst(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec, const Matrix4 &view)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colour;
    Vec4 colour2;
    Vec4 uvOrigin;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 5, rec, uvOrigin, 0.0f);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    Vec4 position;
    position.Copy(Project(rec.partPositions[rec.part], view));
    ToScreen(position);
    if (OffScreen(position.x, position.y) || position.z > TRAIL_FAR_DEPTH || position.z < TRAIL_NEAR_DEPTH ||
        position.w < 0.0f)
        return 0;
    float scaleX;
    float scaleY;
    if (def.trackFlags[0] == 0)
    {
        scaleX = (1.0f - position.z) * SPRITE_DEPTH_SCALE;
        scaleY = scaleX;
    }
    else
    {
        Vec4 scale;
        ReadTrack(def, head, 0, rec, scale, 1.0f);
        float depth = 1.0f - position.z;
        scaleX = depth * scale.x * SPRITE_DEPTH_SCALE;
        scaleY = depth * scale.y * SPRITE_DEPTH_SCALE;
    }
    if (def.flat)
        position.z = 0.0f;
    float rhw = 1.0f / position.w;
    ScreenVertex vertices[BURST_HEXAGON_VERTICES];
    int count;
    if (def.kind != EFFECT_KIND_BURST_SQUARE)
    {
        count = BURST_HEXAGON_VERTICES;
        for (int n = 0; n < count; n++)
        {
            int point = HEXAGON_TRIANGLES[n];
            unsigned int diffuse = point < 6 ? PackRgb(colour) : PackArgb(colour, colour.w);
            SetVertex(vertices[n], (HEXAGON_X[point] - BURST_CELL_CENTRE) * scaleX + position.x,
                      (HEXAGON_Y[point] - BURST_CELL_CENTRE) * scaleY + position.y, position.z, rhw, diffuse,
                      PackSpecular(colour2), HEXAGON_X[point] * BURST_UV_SCALE + uvOrigin.x,
                      HEXAGON_Y[point] * BURST_UV_SCALE + uvOrigin.y);
        }
    }
    else
    {
        count = BURST_SQUARE_VERTICES;
        for (int n = 0; n < count; n++)
        {
            int point = SQUARE_TRIANGLES[n];
            unsigned int diffuse = point < 4 ? PackRgb(colour) : PackArgb(colour, colour.w);
            SetVertex(vertices[n], (SQUARE_X[point] - BURST_CELL_CENTRE) * scaleX + position.x,
                      (SQUARE_Y[point] - BURST_CELL_CENTRE) * scaleY + position.y, position.z, rhw, diffuse,
                      PackSpecular(colour2), SQUARE_X[point] * BURST_UV_SCALE + uvOrigin.x,
                      SQUARE_Y[point] * BURST_UV_SCALE + uvOrigin.y);
        }
    }
    SetBatchBlend(def.blend);
    DAT_1101c804.AddVertices(&DAT_118d0ba8[def.texture], vertices, count);
    return 0;
}

// Kinds 8 and 9: a ribbon of `lightFrames` points from where the effect started to where it is, built in light
// EFFECT_LIGHT_RIBBON and drawn by DrawLight. Returns 1.
static int DrawRibbon(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec, int definition,
                      const Matrix4 &view, const Matrix4 *camera)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 colour;
    Vec4 colour2;
    Vec4 width;
    ReadColour(manager, def, head, rec, colour);
    ReadTrack(def, head, 2, rec, colour2, 0.0f);
    ReadTrack(def, head, 0, rec, width, 1.0f);
    EffectLight &ribbon = DAT_118e99f8[EFFECT_LIGHT_RIBBON];
    ribbon.pointCount = def.lightFrames;
    ribbon.type = definition;
    ribbon.frame = 0;
    ribbon.fade = 1.0f;
    ribbon.lastFrame = -1;
    if (2 < ribbon.pointCount)
    {
        Vec4 from;
        Vec4 to;
        from.Copy(rec.origin);
        to.Copy(rec.partPositions[rec.part]);
        Vec4 along;
        along.Copy(to - from);
        for (int i = 0; i < ribbon.pointCount; i++)
        {
            ribbon.points[i].Copy(from + (along * (float)i) / ribbon.pointCount);
            ribbon.widths[i].Fill(width.x * SPRITE_DEPTH_SCALE);
            ribbon.colours[i].Copy(colour);
            ribbon.specular[i].Copy(colour2);
        }
        manager.DrawLight(EFFECT_LIGHT_RIBBON, view, camera);
    }
    return 1;
}

// Kinds 4, 10 and 0x14: the model of the definition (a node) scaled, turned and placed with the effect's world
// matrix, queued with the effect's colour (and, for kind 0x14, an added tint).
static int DrawModel(EffectManager &manager, EffectDef &def, EffectDef &head, EffectRecord &rec, int definition,
                     const Matrix4 &view, const Matrix4 *camera)
{
    if (manager.m_0050 != 0 && OutsideVisibleArea(rec))
        return 0;
    Vec4 addColour;
    addColour.Fill(0.0f);
    if (def.kind == EFFECT_KIND_MODEL_TINTED && def.trackFlags[2] != 0)
        head.tracks[6].Sample(rec.m_0010, &addColour.x, &addColour.y, &addColour.z, &addColour.w, 0);
    Vec4 colour;
    ReadColour(manager, def, head, rec, colour);
    float turnX = 0.0f;
    float turnY = 0.0f;
    float turnZ = 0.0f;
    if (def.trackFlags[6] != 0)
    {
        float rest;
        head.tracks[10].Sample(rec.m_0010, &turnX, &turnY, &turnZ, &rest, 0);
    }
    Matrix4 placement;
    if (def.trackFlags[0] == 0)
        placement.SetIdentity();
    else
    {
        Vec4 scale;
        head.tracks[4].Sample(rec.m_0010, &scale.x, &scale.y, &scale.z, &scale.w, 0);
        if (def.kind == EFFECT_KIND_MODEL_UNIFORM)
            placement = Scaling(scale.x, scale.x, scale.x);
        else
            placement = Scaling(scale.x, scale.y, scale.z);
    }
    if (turnX != 0.0f)
        placement = RotationX(turnX) * placement;
    if (turnY != 0.0f)
        placement = RotationY(turnY) * placement;
    if (turnZ != 0.0f)
        placement = RotationZ(turnZ) * placement;
    Matrix4 viewCopy = view;
    placement = placement * rec.worldMatrix;
    Node &node = DAT_111be9c8[definition];
    node.StartProximityPass(placement.m, &viewCopy, (Matrix4 *)camera, rec.m_0010, 0);
    SetDrawBlend(def.blend);
    node.QueueDraw(&placement, &viewCopy, 0, 0xffffffff, &colour, &addColour);
    FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
    FUN_1013f070(RENDER_ALPHA_TEST, 1, 0xf);
    return 0;
}

// 0x10189530 (EffectManager::DrawRecord). Draws running effect `record` of draw pass `pass` through `view`
// (`camera` goes on to the light and model draws). A record that has not started (frame below 0) or was removed is
// skipped. With the debug flag set the hit shapes of the effect are drawn first. Each kind then builds its own
// geometry from the definition's tracks (colour, size, turns, texture rectangle) and draws it. Returns 1 for the
// ribbon kinds 8 and 9 that reach their draw, 0 otherwise.
int EffectManager::DrawRecord(int record, const Matrix4 &view, const Matrix4 *camera, int pass)
{
    EffectRecord &rec = DAT_1115cfe8[record];
    int definition = rec.id;
    if (rec.m_0010 < 0 || definition == -2)
        return 0;
    EffectDef &def = DAT_1101dfdc[definition];
    EffectDef &head = DAT_1101dfdc[definition - 1];
    if (DAT_10f654bc != 0)
        DrawDebugShape(def, head, rec, view);
    FUN_1013f070(RENDER_DEPTH_WRITE, def.depthTest != 0, 0);
    switch (def.kind)
    {
    case EFFECT_KIND_LIGHT_23:
        return DrawStreak(*this, def, head, rec, definition, view, camera);
    case EFFECT_KIND_FLARE:
        return DrawFlare(*this, def, head, rec, view);
    case EFFECT_KIND_DISC:
        return DrawDisc(*this, def, head, rec, view, TRAIL_NEAR_DEPTH);
    case EFFECT_KIND_DISC_NEAR:
        return DrawDisc(*this, def, head, rec, view, DISC_NEAR_DEPTH_C);
    case EFFECT_KIND_CONE:
        return DrawCone(*this, def, head, rec, view);
    case EFFECT_KIND_RIBBON_A:
    case EFFECT_KIND_RIBBON_B:
        return DrawRibbon(*this, def, head, rec, definition, view, camera);
    case EFFECT_KIND_QUAD_TEXTURED:
        return DrawQuadTextured(*this, def, head, rec, view);
    case EFFECT_KIND_QUAD_STANDING:
        return DrawQuadStanding(*this, def, head, rec, view);
    case EFFECT_KIND_QUAD:
        return DrawQuadCentred(*this, def, head, rec, view);
    case EFFECT_KIND_SPRITE_RECT:
    case EFFECT_KIND_SPRITE:
        return DrawSpriteKind(*this, def, head, rec, view, pass);
    case EFFECT_KIND_BURST_SQUARE:
    case EFFECT_KIND_BURST_HEXAGON:
        return DrawBurst(*this, def, head, rec, view);
    case EFFECT_KIND_MODEL_UNIFORM:
    case EFFECT_KIND_MODEL:
    case EFFECT_KIND_MODEL_TINTED:
        return DrawModel(*this, def, head, rec, definition, view, camera);
    }
    return 0;
}
