// Destructor of the scene node body (0x18a0 bytes per node).

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

// 0x10191480. Empty body: the compiler destroys the matrix members and the array of sixteen matrices.
NodeBody::~NodeBody()
{
}

