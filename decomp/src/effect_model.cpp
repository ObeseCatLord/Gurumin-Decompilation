struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Set(float x, float y, float z, float w);
};

struct Matrix4
{
    float m[16];
};

struct KeyBank
{
    void Clear();
};

struct Node
{
    void Init();
    void Release();
    int Load(const char *path, int a, int b, int c);
    int Rebuild();
    int M_10199340();
    int M_1019afa0(int a, int b);
};

struct EffectModel
{
    int pad_0000;
    Node *node;
    int start;
    int end;
    Vec4 color;
    Vec4 offset;
    int m_0030;
    int reverseStart;
    int m_0038;
    int m_003c;
    Matrix4 matrix;
    KeyBank keys;
    char pad_0081[0xa83];
    int m_0b04;
    int M_1000b9f0(const char *path, int value, int start, int end);
};

extern "C" Matrix4 DAT_10db9210;
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
extern "C" void (*DAT_11b1ba40)(void *);
extern "C" const char *FUN_1013f8a0(const char *format, ...);

int EffectModel::M_1000b9f0(const char *path, int value, int start, int end)
{
    color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    this->end = end;
    m_0030 = 0;
    this->start = start;
    reverseStart = 0;
    matrix = DAT_10db9210;
    offset.Set(0.0f, 0.0f, 0.0f, 0.0f);
    m_003c = 0;
    if (start < 0)
    {
        reverseStart = -start;
        this->start = 0;
    }
    m_0038 = value;
    if (node != 0)
    {
        node->Release();
        DAT_11b1ba40(node);
    }
    keys.Clear();
    node = (Node *)DAT_11b0b938(0x18a0, "m_frm");
    node->Init();
    node->Load(FUN_1013f8a0(path, 0), 0, 0, 0);
    if (end < 0)
        this->end = node->M_10199340();
    node->Rebuild();
    node->M_1019afa0(1, 0);
    m_0b04 = 0;
    return 1;
}
