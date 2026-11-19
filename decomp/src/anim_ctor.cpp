// The constructor of AnimModel (the CharModel of each character record): its vector and matrix members are
// constructed in declaration order, then the scale values set to 1 and the motion fields cleared. Fields are named
// by offset (see anim_reset.cpp for the full list of what AnimModel::ResetState clears).

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
};

// Matrix4 with the engine's constructor and empty destructor (node_bones.cpp).
struct Matrix4
{
    float m[16];
    Matrix4();                                   // 0x101ae770
    ~Matrix4();                                  // 0x1013eda0 (empty)
};

Matrix4 IdentityMatrix();                        // 0x101b0ba0

// A motion channel set (0xa9c bytes, constructor 0x101af420); only the fields set here.
struct MotionChannels
{
    char pad_0000[0xa90];
    int m_0a90;
    int pad_0a94;
    int m_0a98;
    MotionChannels();                            // 0x101af420
};

struct AnimModel
{
    Vec4 m_0000;
    char pad_0010[8];
    Matrix4 m_0018;
    Vec4 m_0058;
    char pad_0068[0x28];
    Vec4 m_0090;
    Vec4 m_00a0;
    char pad_00b0[4];
    Vec4 m_00b4[12];
    Vec4 m_0174[6];
    char pad_01d4[4];
    Matrix4 m_01d8;
    char pad_0218[4];
    Vec4 m_021c;
    Vec4 m_022c;
    char pad_023c[0x10];
    Vec4 m_024c;
    char pad_025c[4];
    float m_0260;
    char pad_0264[4];
    int m_0268;
    char pad_026c[0x2c];
    int m_0298;
    float m_029c;
    char pad_02a0[0xc];
    MotionChannels m_02ac;
    char pad_0d48[8];
    MotionChannels m_0d50;
    MotionChannels m_17ec;
    char pad_2288[4];
    Vec4 m_228c;
    char pad_229c[4];
    int m_22a0;
    char pad_22a4[0x48];
    int m_22ec;
    int m_22f0;
    int m_22f4;
    int m_22f8;
    int m_22fc;
    char pad_2300[8];
    int m_2308;
    char pad_230c[0x10];
    int m_231c;
    char pad_2320[8];
    Matrix4 m_2328;
    int m_2368;
    int m_236c;
    char pad_2370[0x28];
    Vec4 m_2398;
    Vec4 m_23a8;
    Vec4 m_23b8;
    Vec4 m_23c8;
    Vec4 m_23d8;
    Vec4 m_23e8;
    Vec4 m_23f8;
    Vec4 m_2408;
    int m_2418;
    char pad_241c[4];
    Vec4 m_2420;
    char pad_2430[4];
    Matrix4 m_2434;
    char pad_2474[8];
    int m_247c;
    char pad_2480[0x1c];
    AnimModel();
};

// 0x101629f0 (AnimModel::AnimModel). Constructs the members, sets the scale values (0x260, 0x29c) to 1 and clears
// the motion and placement fields, setting the world matrix to the identity twice (two inlined resets).
AnimModel::AnimModel()
{
    m_0260 = 1.0f;
    m_029c = 1.0f;
    m_0268 = 0;
    m_0298 = 0;
    m_17ec.m_0a98 = 0;
    m_231c = 0;
    m_02ac.m_0a90 = 0;
    m_22ec = 0;
    m_247c = 0;
    m_2368 = 0;
    m_236c = 0;
    m_2308 = 0;
    m_2328 = IdentityMatrix();
    m_2418 = 0;
    m_22a0 = 0;
    m_22ec = 0;
    m_22f0 = 0;
    m_247c = 0;
    m_2368 = 0;
    m_236c = 0;
    m_2308 = 0;
    m_2328 = IdentityMatrix();
    m_2418 = 0;
    m_22f4 = 0;
    m_22f8 = 0;
    m_22fc = 0;
}
