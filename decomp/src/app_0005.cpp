extern "C" char DAT_1020e0f8[];
extern "C" int DAT_104bf8d4;
extern "C" int DAT_104bf8d8;

struct Tracker
{
    char m_0000;
    char m_0001;
    char m_0002;
    char m_0003;
    int m_0004;
    int m_0008;
    char pad_000c[8];
    int m_0014;
    int m_0018;
    char pad_001c[4];
    int m_0020;
    int m_0024;
    int M_10001110(int *pos, char *a, char *b, char *c, int *d, char *e);
    int M_10001450();
};

int Tracker::M_10001110(int *pos, char *a, char *b, char *c, int *d, char *e)
{
    if (a)
        *a = m_0000;
    if (b)
        *b = m_0001;
    if (c)
        *c = m_0002;
    if (d)
        *d = m_0014;
    if (e)
        *e = m_0003;
    if (pos)
    {
        pos[0] = m_0004;
        pos[1] = m_0008;
    }
    return 1;
}

int Tracker::M_10001450()
{
    int pos[2];
    M_10001110(pos, 0, 0, 0, 0, 0);
    if (pos[0] != m_0020 || pos[1] != m_0024)
        m_0018 = 1;
    m_0020 = pos[0];
    m_0024 = pos[1];
    return m_0018;
}

extern "C" int FUN_1000cb90(const char *text)
{
    if (text != 0)
    {
        const char *src = text;
        char c;
        do
        {
            c = *src;
            DAT_1020e0f8[src - text] = c;
            src++;
        } while (c != 0);
        if (*text != 0)
        {
            DAT_104bf8d4 = 1;
            DAT_104bf8d8 = 0;
            return 0;
        }
    }
    else
        DAT_1020e0f8[0] = 0;
    return 0;
}
