// Destructor of a menu panel object that owns four resources (each with its own destructor).

struct Resource
{
    char data[0x10];
    ~Resource();
};

struct Panel
{
    char pad_0000[0x18];
    Resource m_0018;
    char pad_0028[0x1b0];
    Resource m_01d8;
    char pad_01e8[0x2140];
    Resource m_2328;
    char pad_2338[0xfc];
    Resource m_2434;
    ~Panel();
};

// Empty body: the compiler destroys the four Resource members in reverse order.
Panel::~Panel()
{
}
