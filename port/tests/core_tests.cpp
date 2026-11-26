// Native tests for gurumin_core: they call decomp functions compiled unchanged from decomp/src and compare the
// results with values worked out by hand. Every function asserted in the "matched" sections has a 100% entry in
// progress/jp/report.json (byte-identical to APP.dll when built with the original compiler). Results that go
// through a port stand-in (compat/standins.cpp) are in their own section, because the stand-ins are not decompiled.
//
// Several free functions are declared per file in the decomp instead of in a shared header; their prototypes are
// repeated below exactly as the decomp defines them, so the linker binds them to the decomp's definitions.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "vec4.h"
#include "vec3.h"
#include "matrix4.h"
#include "plane.h"
#include "mesh.h"
#include "key_track.h"
#include "motion_list.h"
#include "node.h"
#include "input_state.h"
#include "flag_list.h"

// Declared in decomp headers that pull in large parts of the engine; repeated here.
extern "C" int FUN_101612b0(unsigned char *out, int size, const unsigned char *packed, int packedSize);  // Rle_Decode
extern "C" int FUN_1015d520(char *text);                                                               // Text_ToFullWidth
extern "C" int FUN_101602d0(const char *path, int *sum, unsigned int *check);                         // File_Checksum
extern "C" int FUN_10160360(const char *path, int base, int index, void *first, void *second);       // File_ReadRecord
extern "C" int FUN_100144c0(int flags, char *text);                                                   // Ability_GetDescription
// Defined in vec_math.cpp / vec_ops.cpp / containers.cpp / point_transform.cpp without a shared declaration.
Vec4 Cross(const Vec4 &a, const Vec4 &b);
Vec4 Nlerp(const Vec4 &a, const Vec4 &b, float t);
Vec4 Slerp(const Vec4 &a, const Vec4 &b, float t);
Vec4 TransformNormal(const Vec4 &v, const Matrix4 &matrix);
Vec4 TriangleNormal(const Vec4 &a, const Vec4 &origin, const Vec4 &b);
Plane PlaneFromTriangle(const Vec4 &a, const Vec4 &origin, const Vec4 &b);
Vec4 TransformPoint(const Vec4 &point, const Matrix4 &matrix);
int Point_Transform(Vec4 *point, Matrix4 *matrix);

// The decomp hard-codes 0x70 as the vertex stride (containers.cpp); it must hold on 64-bit hosts too.
static_assert(sizeof(Vertex) == 0x70, "Vertex must stay 112 bytes");
static_assert(sizeof(Vec4) == 16, "Vec4 must stay 16 bytes");
static_assert(sizeof(Matrix4) == 64, "Matrix4 must stay 64 bytes");

namespace
{
int g_checks = 0;
int g_failures = 0;
const char *g_section = "";

void Section(const char *name)
{
    g_section = name;
    std::printf("[%s]\n", name);
}

void Check(bool ok, const char *what, int line)
{
    g_checks++;
    if (!ok)
    {
        g_failures++;
        std::printf("  FAIL %s:%d: %s\n", g_section, line, what);
    }
}

bool Near(float a, float b, float tolerance = 1e-5f)
{
    return std::fabs(a - b) <= tolerance;
}

bool VecNear(const Vec4 &v, float x, float y, float z, float w, float tolerance = 1e-5f)
{
    return Near(v.x, x, tolerance) && Near(v.y, y, tolerance) && Near(v.z, z, tolerance) && Near(v.w, w, tolerance);
}

#define CHECK(cond) Check((cond), #cond, __LINE__)

const float kHalfPi = 1.57079632679f;
const float kInvSqrt2 = 0.70710678f;

// Zeroed storage for engine objects that have no constructor in the compiled set (the engine allocates them raw and
// calls Init, as these tests do).
template <typename T> T *AllocateRaw()
{
    return static_cast<T *>(std::calloc(1, sizeof(T)));
}

std::string TempPath(const char *name)
{
    const char *dir = std::getenv("TMPDIR");
    std::string path = dir && *dir ? dir : "/tmp";
    return path + "/gurumin_core_tests_" + name;
}

void TestVectors()
{
    Section("vec4/vec_math: vectors (matched)");
    Vec4 zero;
    CHECK(VecNear(zero, 0, 0, 0, 0));
    Vec4 a(1, 2, 3, 9);
    Vec4 b(4, 5, 6, 9);
    CHECK(Near(Vec3_Dot(&a, &b), 32.0f));
    CHECK(Near(Vec3_LengthSquared(&a), 14.0f));
    Vec4 c(3, 4, 12, 0);
    CHECK(Near(Vec3_Length(&c), 13.0f));
    CHECK(Near(c.Length2D(), 5.0f));
    Vec4 d(3, 4, 0, 7);
    CHECK(Near(d.Length(), 5.0f));
    Vec4 e(1, 2, 2, 5);
    CHECK(Near(e.LengthSquared3(), 9.0f));

    Vec4 p(1, 1, 1, 0);
    Vec4 q(4, 5, 1, 0);
    CHECK(Near(p.Distance(q), 5.0f));
    CHECK(Near(p.DistanceSquared(q), 25.0f));
    Vec4 r(0, 0, 9, 0);
    CHECK(Near(r.Distance2D(Vec4(3, 4, -9, 0)), 5.0f));

    Vec4 s(1, 2, 3, 4);
    Vec4 t(10, 20, 30, 40);
    CHECK(VecNear(s + t, 11, 22, 33, 44));
    CHECK(VecNear(t - s, 9, 18, 27, 36));
    CHECK(VecNear(s * t, 10, 40, 90, 160));
    float two = 2.0f;
    CHECK(VecNear(s * two, 2, 4, 6, 0));       // x/y/z scaled; the result's w is the constructor's 0
    CHECK(VecNear(s.Divided(two), 0.5f, 1, 1.5f, 0));
    int four = 4;
    CHECK(VecNear(Vec4(4, 8, 12, 99) / four, 1, 2, 3, 0));
    Vec4 filled;
    float seven = 7.0f;
    filled.Fill(seven);
    CHECK(VecNear(filled, 7, 7, 7, 7));
    filled.Set(1, 2, 3, 4);
    CHECK(VecNear(filled, 1, 2, 3, 4));

    CHECK(VecNear(Cross(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0)), 0, 0, 1, 0));
    CHECK(VecNear(Cross(Vec4(0, 1, 0, 0), Vec4(1, 0, 0, 0)), 0, 0, -1, 0));

    Vec4 n(0, 3, 0, 4);
    n.Normalize();  // all four components
    CHECK(VecNear(n, 0, 0.6f, 0, 0.8f));
    Vec4 m(0, 0, 5, 7);
    Vec4 unit = m.Normalized3();  // x/y/z only; this keeps its w, the returned copy has w = 0
    CHECK(VecNear(unit, 0, 0, 1, 0));
    CHECK(VecNear(m, 0, 0, 1, 7));

    Vec4 rz(1, 0, 5, 1);
    rz.RotateZ(kHalfPi);
    CHECK(VecNear(rz, 0, 1, 5, 1));

    Box2 box;
    box.min.Set(0, 0, 0, 0);
    box.max.Set(1, 1, 1, 0);
    Vec4 inside(0.5f, 0.5f, 0.5f, 0);
    Vec4 corner(1, 1, 1, 0);
    Vec4 above(0.5f, 0.5f, 9, 0);
    Vec4 outside(2, 0.5f, 0.5f, 0);
    CHECK(inside.InBox(&box) == 1);
    CHECK(corner.InBox(&box) == 1);  // edges included
    CHECK(above.InBox(&box) == 0);
    CHECK(above.InRect(&box) == 1);  // x/y only
    CHECK(outside.InRect(&box) == 0);
    CHECK(outside.InBoxMargin(&box, 1.5f) == 1);

    Vec4 segA(-1, 0, 0, 0);
    Vec4 segB(1, 0, 0, 0);
    Vec4 closest;
    Vec4 overMiddle(0, 1, 0, 0);
    CHECK(Near(overMiddle.DistanceToSegment(segA, segB, closest), 1.0f));
    CHECK(VecNear(closest, 0, 0, 0, 0));
    Vec4 pastEnd(3, 0, 0, 0);
    CHECK(Near(pastEnd.DistanceToSegment(segA, segB, closest), 2.0f));
    CHECK(VecNear(closest, 1, 0, 0, 0));
    CHECK(Near(pastEnd.DistanceSquaredToSegment(segA, segB), 4.0f));
    Vec4 offLine(3, 2, 0, 0);
    CHECK(Near(offLine.DistanceSquaredToLine(segA, segB), 4.0f));
    CHECK(Near(offLine.DistanceToLine(segA, segB), 2.0f));
}

void TestMatrices()
{
    Section("vec_math/containers/vec4: matrices (matched)");
    Matrix4 matrix;
    matrix.SetIdentity();
    for (int i = 0; i < 16; i++)
        CHECK(matrix.m[i] == ((i % 5 == 0) ? 1.0f : 0.0f));
    matrix.Translate(1, 2, 3);
    CHECK(VecNear(matrix.GetTranslation(), 1, 2, 3, 0));
    CHECK(VecNear(TransformCoord(Vec4(1, 1, 1, 1), matrix), 2, 3, 4, 1));
    matrix.SetTranslation(Vec4(5, 6, 7, 99));
    CHECK(matrix.m[12] == 5 && matrix.m[13] == 6 && matrix.m[14] == 7 && matrix.m[15] == 1);

    Matrix4 turn;
    turn.SetIdentity();
    turn.SetRotationZ(kHalfPi);
    CHECK(Near(turn.m[0], 0) && Near(turn.m[1], -1) && Near(turn.m[4], 1) && Near(turn.m[5], 0));
    CHECK(VecNear(TransformNormal(Vec4(1, 0, 0, 0), turn), 0, -1, 0, 0));

    Matrix4 transposed;
    transposed.SetIdentity();
    transposed.SetRotationTransposed(turn);
    CHECK(Near(transposed.m[1], 1) && Near(transposed.m[4], -1));
    turn.ResetRotation();
    CHECK(Near(turn.m[0], 1) && Near(turn.m[1], 0) && Near(turn.m[4], 0) && Near(turn.m[5], 1));

    Section("vec4/vec_math: quaternions (matched)");
    Matrix4 identity;
    identity.SetIdentity();
    CHECK(VecNear(QuatFromMatrix(identity), 0, 0, 0, 1));
    Matrix4 quarter;
    quarter.SetIdentity();
    quarter.SetRotationZ(kHalfPi);
    CHECK(VecNear(QuatFromMatrix(quarter), 0, 0, kInvSqrt2, kInvSqrt2));
    CHECK(VecNear(QuatMultiply(Vec4(1, 0, 0, 0), Vec4(0, 1, 0, 0)), 0, 0, 1, 0));  // i * j = k
    CHECK(VecNear(QuatMultiply(Vec4(0, 0, 0, 1), Vec4(0.5f, 0.5f, 0.5f, 0.5f)), 0.5f, 0.5f, 0.5f, 0.5f));
    Vec4 none(0, 0, 0, 1);
    Vec4 half(0, 0, 1, 0);
    CHECK(VecNear(Nlerp(none, half, 0.5f), 0, 0, kInvSqrt2, kInvSqrt2));
    CHECK(VecNear(Slerp(none, half, 0.5f), 0, 0, kInvSqrt2, kInvSqrt2));
    CHECK(VecNear(Slerp(none, half, 0.0f), 0, 0, 0, 1));

    Section("vec_ops: triangles (matched)");
    CHECK(VecNear(TriangleNormal(Vec4(1, 0, 0, 0), Vec4(0, 0, 0, 0), Vec4(0, 1, 0, 0)), 0, 0, 1, 0));
    Plane plane = PlaneFromTriangle(Vec4(1, 0, 2, 0), Vec4(0, 0, 2, 0), Vec4(0, 1, 2, 0));
    CHECK(Near(plane.a, 0) && Near(plane.b, 0) && Near(plane.c, 1) && Near(plane.d, -2));
}

void TestBoxes()
{
    Section("box_overlap: box tests (matched)");
    // The overlap tests are Node methods that do not use `this`; zeroed storage stands in for a node.
    Node *node = AllocateRaw<Node>();
    Vec4 first[2];
    first[NODE_BOX_MIN].Set(0, 0, 0, 0);
    first[NODE_BOX_MAX].Set(2, 2, 2, 0);
    Vec4 second[2];
    second[NODE_BOX_MIN].Set(0, 0, 0, 0);
    second[NODE_BOX_MAX].Set(2, 2, 2, 0);
    Vec4 origin(0, 0, 0, 0);
    Vec4 diagonal(1, 1, 1, 0);
    Vec4 delta;
    Vec4 negated;
    CHECK(node->BoxesOverlap(first, &origin, second, &diagonal, &delta, &negated) == 1);
    CHECK(VecNear(delta, -1, -1, -1, 0));
    CHECK(VecNear(negated, 1, 1, 1, 0));
    Vec4 touching(2, 0, 0, 0);
    CHECK(node->BoxesOverlap(first, &origin, second, &touching, nullptr, nullptr) == 0);  // faces touching
    Vec4 apart(0, 0, 5, 0);
    CHECK(node->BoxesOverlap(first, &origin, second, &apart, nullptr, nullptr) == 0);
    std::free(node);
}

void TestContainers()
{
    Section("containers: index lists and vertex packs (matched)");
    IndexList *list = AllocateRaw<IndexList>();
    list->Init();
    CHECK(list->Resize(6) == 1);
    CHECK(list->count == 6 && list->indices != nullptr);
    for (int i = 0; i < 6; i++)
        list->indices[i] = i * 3;
    IndexList *copy = AllocateRaw<IndexList>();
    copy->Init();
    CHECK(copy->CopyFrom(list) == 1);
    CHECK(copy->count == 6 && copy->indices != list->indices && copy->indices[5] == 15);
    CHECK(list->Free() == 1 && list->count == 0 && list->indices == nullptr);
    copy->Free();
    std::free(list);
    std::free(copy);

    VertexPack *pack = AllocateRaw<VertexPack>();
    pack->Init();
    CHECK(pack->SetVertexCount(3) == 1);
    CHECK(pack->vertexCount == 3 && pack->vertices != nullptr);
    for (int i = 0; i < 3; i++)
        pack->vertices[i].SetPosition((float)i, (float)(i * 2), (float)(i * 3));
    pack->vertices[2].Set(1, 2, 3, 0, 1, 0, 0.25f, 0.75f, 0.1f, 0.2f, 0.3f, 0.4f);
    pack->radius = 4.5f;
    pack->material = 11;
    VertexPack *packCopy = AllocateRaw<VertexPack>();
    packCopy->Init();
    CHECK(packCopy->CopyFrom(pack) == 1);
    CHECK(packCopy->vertexCount == 3 && packCopy->material == 11 && packCopy->radius == 4.5f);
    CHECK(packCopy->vertices != pack->vertices);
    CHECK(std::memcmp(packCopy->vertices, pack->vertices, 3 * sizeof(Vertex)) == 0);
    CHECK(pack->FreeVertices() == 1 && pack->vertices == nullptr);
    packCopy->FreeVertices();
    std::free(pack);
    std::free(packCopy);

    Section("vertex: vertex records (matched)");
    Vertex vertex;
    vertex.SetPosition(1, 2, 3);
    CHECK(vertex.position[0] == 1 && vertex.position[1] == 2 && vertex.position[2] == 3 && vertex.position[3] == 1);
    CHECK(vertex.normal[2] == 1 && vertex.colour[0] == 1 && vertex.colour[3] == 1 && vertex.uv[0] == 0);
    CHECK(vertex.m_0060 == 1);
    vertex.Set(4, 5, 6, 0, 0, -1, 0.5f, 0.25f, 0.1f, 0.2f, 0.3f, 0.4f);
    CHECK(vertex.position[0] == 4 && vertex.normal[2] == -1 && vertex.uv[0] == 0.5f && vertex.uv[1] == 0.25f);
    CHECK(Near(vertex.colour[0], 0.1f) && Near(vertex.colour[3], 0.4f));

    Section("containers/vec4: key-frame tracks and banks (matched)");
    KeyTrack *track = AllocateRaw<KeyTrack>();
    track->Init();
    CHECK(track->count == 0 && track->capacity == 16);
    CHECK(track->Insert(10, 1, 2, 3, 4) == 0);
    CHECK(track->Insert(5, 5, 6, 7, 8) == 0);  // sorted by frame: goes in front
    CHECK(track->count == 2 && track->frames[0] == 5 && track->frames[1] == 10);
    CHECK(VecNear(track->values[1], 1, 2, 3, 4));
    CHECK(track->Set(10, 9, 9, 9, 9) == 1);  // existing key: value replaced
    CHECK(VecNear(track->values[1], 9, 9, 9, 9));
    CHECK(track->Set(20, 1, 1, 1, 1) == 2);  // new key: inserted at its index
    CHECK(track->LastFrame() == 20);
    CHECK(track->HasFrame(5) == 1 && track->HasFrame(6) == 0 && track->HasFrame(20) == 1);
    for (int frame = 100; frame < 140; frame++)  // grows the arrays through Reserve
        track->Insert(frame, (float)frame, 0, 0, 0);
    CHECK(track->count == 43 && track->capacity >= track->count + 16);
    CHECK(track->frames[42] == 139 && track->values[42].x == 139.0f);
    CHECK(track->MarkAll() == 0);
    CHECK(track->flags[0] == KEY_FLAG_MOTION && track->flags[42] == KEY_FLAG_MOTION);

    KeyBank *bank = new KeyBank();
    CHECK(bank->count == 0 && bank->frames[0] == 0);
    CHECK(bank->Insert(30, 3, 0, 0, 0) == 0);
    CHECK(bank->Insert(10, 1, 0, 0, 0) == 0);
    CHECK(bank->Set(30, 4, 0, 0, 0) == 1);
    CHECK(bank->count == 2 && bank->frames[0] == 10 && bank->frames[1] == 30 && bank->values[1].x == 4.0f);
    bank->Clear();
    CHECK(bank->count == 0 && VecNear(bank->values[0], 0, 0, 0, 0));

    Section("containers: motion list (matched)");
    MotionList *motions = AllocateRaw<MotionList>();
    CHECK(motions->AddEntry(7, 70, "walk", 1) == 1);
    CHECK(motions->entryCount == 1 && std::strcmp(motions->names[0], "walk") == 0 && motions->values[0] == 70);
    CHECK(motions->Insert(-1, 5, 9) == 1);  // append
    CHECK(motions->Insert(0, 3, 9) == 1);   // insert in front
    CHECK(motions->count == 2 && motions->items[0] == 3 && motions->items[1] == 5 && motions->m_0208 == 9);
    CHECK(motions->Insert(5, 1, 9) == 0);   // past the end
    CHECK(motions->Clear() == 1 && motions->count == 0);
    std::free(motions);

    Section("stand-in dependent (compat/standins.cpp; not verified against APP.dll)");
    // KeyTrack::CopyFrom and KeyBank::CopyTo are matched, but they append through KeyTrack::AddKey, a stand-in.
    KeyTrack *trackCopy = AllocateRaw<KeyTrack>();
    trackCopy->Init();
    CHECK(trackCopy->CopyFrom(track) == 1);
    CHECK(trackCopy->count == track->count && trackCopy->frames[42] == 139 && trackCopy->flags[0] == KEY_FLAG_MOTION);
    bank->Insert(4, 1, 2, 3, 4);
    KeyTrack *fromBank = AllocateRaw<KeyTrack>();
    CHECK(bank->CopyTo(fromBank) == 0);
    CHECK(fromBank->count == 1 && fromBank->frames[0] == 4 && VecNear(fromBank->values[0], 1, 2, 3, 4));
    // TransformPoint (matched) calls Vec4::Transform; Point_Transform (matched) uses Matrix4::operator*.
    Matrix4 move;
    move.SetIdentity();
    move.Translate(10, 20, 30);
    CHECK(VecNear(TransformPoint(Vec4(1, 2, 3, 1), move), 11, 22, 33, 1));
    Matrix4 turnAndMove;
    turnAndMove.SetIdentity();
    turnAndMove.SetRotationZ(kHalfPi);
    turnAndMove.Translate(10, 0, 0);
    Vec4 point(1, 0, 0, 0);
    CHECK(Point_Transform(&point, &turnAndMove) == 0);
    CHECK(Near(point.x, 10) && Near(point.y, -1) && Near(point.z, 0));

    // Note: KeyTrack::Reserve allocates with new[] and KeyTrack::Free releases with free(), as the original does.
    // That is harmless with glibc (and with these trivially destructible element types) but trips ASan.
    track->Free();
    CHECK(track->count == 0 && track->capacity == 0 && track->frames == nullptr);
    trackCopy->Free();
    fromBank->Free();
    std::free(track);
    std::free(trackCopy);
    std::free(fromBank);
    delete bank;
}

void TestFormats()
{
    Section("rle_decode: bitmap run-length packing (matched)");
    const unsigned char packed[] = {
        0x02, 'A', 'B', 'C',  // 3 literal bytes
        0x23, 'x',            // 4 copies of 'x'
        0x41,                 // 2 zero bytes
        0x60,                 // 1 0xff byte
        0x50, 0x00,           // long zero run: 0 + 33 bytes
    };
    unsigned char out[3 + 4 + 2 + 1 + 33];
    std::memset(out, 0x55, sizeof(out));
    CHECK(FUN_101612b0(out, (int)sizeof(out), packed, (int)sizeof(packed)) == 1);
    CHECK(std::memcmp(out, "ABCxxxx", 7) == 0 && out[7] == 0 && out[8] == 0 && out[9] == 0xff);
    bool zeros = true;
    for (int i = 10; i < (int)sizeof(out); i++)
        zeros = zeros && out[i] == 0;
    CHECK(zeros);
    unsigned char small[8];
    CHECK(FUN_101612b0(small, 7, packed, 6) == 1);  // the first two runs alone: 7 bytes from 6
    CHECK(FUN_101612b0(small, 7, packed, 7) == 0);  // a packed byte left over
    const unsigned char bad[] = {0x80};
    CHECK(FUN_101612b0(small, 1, bad, 1) == 0);     // unknown run kind

    Section("text_fullwidth: Shift-JIS full-width conversion (matched)");
    char text[32] = "A1";
    CHECK(FUN_1015d520(text) == 0);
    CHECK(std::memcmp(text, "\x82\x60\x82\x50", 5) == 0);  // full-width A and 1
    char space[8] = "\x81\x40";                             // full-width space
    FUN_1015d520(space);
    CHECK(std::memcmp(space, "\x81\x51", 3) == 0);          // becomes the full-width underscore

    Section("ability_text: ability descriptions (matched)");
    char description[64] = "unchanged";
    CHECK(FUN_100144c0(0, description) == 0);
    CHECK(std::strcmp(description, "unchanged") == 0);
    CHECK(FUN_100144c0(0x3, description) == 0x3);
    CHECK(std::strcmp(description, "\x90\x85\x83_\x83\x81\x81[\x83W\x89\xf1\x95\x9c") == 0);
    CHECK(FUN_100144c0(0x3001, description) == 0x1);   // the first bit pair wins over 0x3000
    CHECK(FUN_100144c0(0x2000, description) == 0x2000);

    Section("file_check: data file checks (matched)");
    std::string path = TempPath("check.bin");
    FILE *file = std::fopen(path.c_str(), "wb");
    CHECK(file != nullptr);
    if (file != nullptr)
    {
        const unsigned char words[] = {0x10, 0, 0, 0, 0x20, 0, 0, 0, 0xaa, 0xbb, 0, 0, 0x01, 0x02, 0x03, 0x04};
        std::fwrite(words, 1, sizeof(words), file);
        std::fclose(file);
        int sum = 0;
        unsigned int check = 0;
        CHECK(FUN_101602d0(path.c_str(), &sum, &check) == 1);
        CHECK(sum == (int)(0x10u + 0x20u + 0xbbaau + 0x04030201u));
        CHECK(check == (0xcc82u ^ 0x10u ^ 0x20u ^ 0xbbaau ^ 0x04030201u));
        unsigned int first = 0;
        unsigned int second = 0;
        CHECK(FUN_10160360(path.c_str(), 0, 1, &first, &second) == 1);  // record 1 = bytes 8..15
        CHECK(first == 0xbbaau && second == 0x04030201u);
        std::remove(path.c_str());
    }
    int sum = 0;
    unsigned int check = 0;
    CHECK(FUN_101602d0(TempPath("missing.bin").c_str(), &sum, &check) == 0);
}

void TestState()
{
    Section("input_state: button edges (matched)");
    InputState input;
    std::memset(&input, 0, sizeof(input));
    CHECK(input.SetButtons(0x401) == 1);
    CHECK(input.pressed[4] == 1 && input.pressed[0] == 1 && input.pressed[7] == 0);
    CHECK(input.held[4] == 1 && input.held[0] == 1);
    input.SetButtons(0x401);  // still down: no new press
    CHECK(input.pressed[4] == 0 && input.pressed[0] == 0 && input.held[4] == 1);
    input.SetButtons(0x200);
    CHECK(input.held[4] == 0 && input.held[0] == 0 && input.pressed[8] == 1 && input.held[8] == 1);
    CHECK(input.buttons == 0x200);

    Section("flag_list: id registry (matched)");
    FlagOwner *owner = AllocateRaw<FlagOwner>();  // about 4 MB of which only the tail is used
    owner->marked[0] = 42;
    CHECK(owner->M_100020d0(42) == 1);
    CHECK(owner->pairs[0].id == 42 && owner->pairs[0].flag == 1);
    CHECK(owner->M_100020d0(7) == 1);
    CHECK(owner->pairs[1].id == 7 && owner->pairs[1].flag == 0);
    CHECK(owner->M_100020d0(42) == 1 && owner->pairs[2].id == 0);  // already registered
    for (int i = 0; i < 0x100; i++)
        owner->pairs[i].id = 1000 + i;
    CHECK(owner->M_100020d0(5) == 0);  // table full
    std::free(owner);
}
}

int main()
{
    std::printf("gurumin_core tests (%zu-bit, %s)\n", sizeof(void *) * 8,
#if defined(__aarch64__)
                "aarch64"
#elif defined(__x86_64__)
                "x86_64"
#else
                "other"
#endif
    );
    TestVectors();
    TestMatrices();
    TestBoxes();
    TestContainers();
    TestFormats();
    TestState();
    std::printf("%d checks, %d failed\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
