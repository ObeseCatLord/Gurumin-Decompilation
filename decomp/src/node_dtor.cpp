struct Member
{
    float m[16];
    ~Member();
};

struct Element
{
    float v[4];
    ~Element();
};

struct NodeBody
{
    char pad_0000[0x1cc];
    Member m_01cc;
    Member m_020c;
    Member m_024c;
    char pad_028c[0x48];
    Member m_02d4[0x10];
    ~NodeBody();
};

NodeBody::~NodeBody()
{
}

