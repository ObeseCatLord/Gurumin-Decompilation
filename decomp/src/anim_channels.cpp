// Constructors of the animation containers. The bodies are empty: all the work is done by the member
// constructors (Vec4, KeyTrack, Matrix4, ModelState), which the compiler calls in declaration order.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
};

// One animated channel: `count` key frames with frame numbers, values and per-key flags (containers.cpp).
struct KeyTrack
{
    int m_0000;
    int *frames;
    Vec4 *values;
    char *flags;
    int count;
    KeyTrack();
};

// Eight base values plus twelve key tracks (position, rotation, scale, colour, ... per node).
struct AnimChannels
{
    Vec4 m_0000;
    Vec4 m_0010;
    Vec4 m_0020;
    Vec4 m_0030;
    Vec4 m_0040;
    Vec4 m_0050;
    Vec4 m_0060;
    Vec4 m_0070;
    KeyTrack m_0080;
    KeyTrack m_0094;
    KeyTrack m_00a8;
    KeyTrack m_00bc;
    KeyTrack m_00d0;
    KeyTrack m_00e4;
    KeyTrack m_00f8;
    KeyTrack m_010c;
    KeyTrack m_0120;
    KeyTrack m_0134;
    KeyTrack m_0148;
    KeyTrack m_015c;
    AnimChannels();
};

// 0x101727f0.
AnimChannels::AnimChannels()
{
}

struct Matrix4
{
    float m[16];
    Matrix4();
    ~Matrix4();
};

// Declared throw(): the original leaves out the EH state update before this constructor, which only fits a
// compiler that knew it cannot throw (whole-program optimisation).
struct ModelState
{
    char pad_0000[0x30];
    ModelState() throw();
};

// A placed model: bounding vectors, three matrices (world, local, previous) and its animation state.
struct ModelInstance
{
    char pad_0000[0x24];
    Vec4 m_0024[2];
    Vec4 m_0044;
    Vec4 m_0054;
    Vec4 m_0064;
    Vec4 m_0074;
    Vec4 m_0084;
    Vec4 m_0094;
    Vec4 m_00a4;
    Vec4 m_00b4;
    Matrix4 m_00c4;
    Matrix4 m_0104;
    Matrix4 m_0144;
    char pad_0184[0x10];
    ModelState state;
    ModelInstance();
};

// 0x101728c0.
ModelInstance::ModelInstance()
{
}
