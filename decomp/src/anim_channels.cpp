struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
};

struct KeyTrack
{
    int m_0000;
    int *frames;
    Vec4 *values;
    char *flags;
    int count;
    KeyTrack();
};

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

AnimChannels::AnimChannels()
{
}

struct Matrix4
{
    float m[16];
    Matrix4();
    ~Matrix4();
};

struct ModelState
{
    char pad_0000[0x30];
    ModelState() throw();
};

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

ModelInstance::ModelInstance()
{
}
