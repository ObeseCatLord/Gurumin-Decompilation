// Attached mesh slots (see ai_pick.cpp for the slot table).

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    void Set(float x, float y, float z, float w);   // 0x101ac9c0
};

struct Matrix
{
    float m[16];
    Matrix();
    ~Matrix();
    void SetIdentity();
};

Matrix LookAt(Vec4 eye, Vec4 target, const Vec4 *up, int a);                   // 0x101b1c20
Matrix Perspective(float nearPlane, float farPlane, float fieldOfView);          // 0x101b0c10
Matrix Scaling(float x, float y, float z);                                       // 0x101b13c0
Matrix Multiply(const Matrix &left, const Matrix &right);                        // 0x101b10f0

// A model node, only the call used here.
struct Node
{
    void QueueDraw(const Matrix *parent, const Matrix *view, const Vec4 *direction, int a, const Vec4 *colour,
                   int b);                       // 0x10199e20
};

struct MeshSlot
{
    int pad_0000;
    Node *mesh;               // 0x04: the attached model (0 = slot free)
    int delay;                // 0x08: frames before it is drawn
    int pad_000c;
    Vec4 colour;              // 0x10
    char pad_0020[0x10];
    int visible;              // 0x30
    int frame;                // 0x34: frames since it was attached
    int pad_0038;
    int ownCamera;            // 0x3c: non-zero = drawn with its own fixed camera
    Matrix matrix;            // 0x40: placement of the model
    char pad_0080[0xa88];
    int Draw(const Matrix &view);                // 0x1000baf0
};

extern "C" MeshSlot DAT_10edbe08[0x40];
extern "C" Matrix DAT_108392d8;

// 0x1000baf0 (MeshSlot::Draw). Queues the slot's model for drawing once it is visible and its delay has passed:
// with `view`, or with its own camera (480 units back and 120 up, looking at the origin, a 45 degree perspective from
// 20 to 5000 units squeezed to 4:3) and an identity placement. Returns 0 for a free slot, else 1.
int MeshSlot::Draw(const Matrix &view)
{
    if (mesh == 0)
        return 0;
    if (visible != 0 && frame >= delay)
    {
        if (ownCamera != 0)
        {
            Matrix result;
            Matrix unused1;
            Matrix camera;
            Matrix projection;
            Matrix unused2;
            Vec4 up;
            Vec4 target;
            Vec4 eye;
            up.x = 0;
            up.y = 1.0f;
            up.z = 0;
            target.Set(0, 0, 0, 0);
            eye.Copy(target);
            eye.z += 120.0;
            eye.y -= 480.0;
            camera = LookAt(eye, target, &up, 0);
            projection = Multiply(Scaling(1.0f, 1.333f, 1.0f), Perspective(20.0f, 5000.0f, 0.785375f));
            result = Multiply(projection, camera);
            matrix.SetIdentity();
            mesh->QueueDraw(&matrix, &result, 0, -1, &colour, 0);
        }
        else
            mesh->QueueDraw(&matrix, &view, 0, -1, &colour, 0);
    }
    return 1;
}

// Updates every used mesh slot with the world matrix DAT_108392d8. The identity matrix is unused.
int ResetMeshSlots()
{
    Matrix identity;
    identity.SetIdentity();
    for (int i = 0; i < 0x40; i++)
    {
        if (DAT_10edbe08[i].mesh != 0)
            DAT_10edbe08[i].Draw(DAT_108392d8);
    }
    return 0;
}

