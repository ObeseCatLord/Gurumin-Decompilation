struct Point
{
    int x;
    int y;
};

extern "C" int __stdcall FUN_10001000(int, int, int)
{
    return 1;
}

struct Obj
{
    int pad0;
    int m_0004;
    int m_0008;
    int pad_000c[4];
    int m_001c;
    int pad_0020[6];
    int m_0038;
    int m_003c;
    int m_0040;
    int M_10001010(int p, int q, int r);
};

int Obj::M_10001010(int p, int q, int r)
{
    if (p == 0)
        return 0;
    if (q == 0)
        return 0;
    m_0038 = q;
    m_003c = r;
    m_001c = p;
    m_0004 = 0;
    m_0008 = 0;
    m_0040 = 0;
    return 1;
}

extern "C" int __stdcall FUN_10001050(const Point *pt, int x, int y, int width, int height)
{
    if (pt->x < x || pt->y < y)
        return 0;
    if (pt->x > x + width || pt->y > y + height)
        return 0;
    return 1;
}
