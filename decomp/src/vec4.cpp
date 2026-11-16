struct Vec4
{
    float x;
    float y;
    float z;
    float w;

    Vec4();
    Vec4(float x, float y, float z, float w);
    void Set(float x, float y, float z, float w);
    void Fill(const float &value);
};

Vec4::Vec4()
{
    x = 0;
    y = 0;
    z = 0;
    w = 0;
}

Vec4::Vec4(float nx, float ny, float nz, float nw)
{
    x = nx;
    y = ny;
    z = nz;
    w = nw;
}

void Vec4::Set(float nx, float ny, float nz, float nw)
{
    x = nx;
    y = ny;
    z = nz;
    w = nw;
}

void Vec4::Fill(const float &value)
{
    x = value;
    y = value;
    z = value;
    w = value;
}
