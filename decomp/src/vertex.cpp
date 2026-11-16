struct Vertex
{
    float v[28];
    void SetPosition(float x, float y, float z);
    void Set(float x, float y, float z, float nx, float ny, float nz, float u, float t, float r, float g, float b, float a);
};

void Vertex::SetPosition(float x, float y, float z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = 1.0f;
    v[4] = 0;
    v[5] = 0;
    v[6] = 1.0f;
    v[7] = 1.0f;
    v[8] = 1.0f;
    v[9] = 1.0f;
    v[10] = 1.0f;
    v[11] = 1.0f;
    v[12] = 0;
    v[13] = 0;
    v[14] = 0;
    v[15] = 0;
    v[16] = 0;
    v[17] = 0;
    v[18] = 0;
    v[19] = 0;
    v[20] = 0;
    v[21] = 0;
    v[22] = 0;
    v[23] = 0;
    v[24] = 1.0f;
    v[25] = 0;
    v[26] = 0;
    v[27] = 0;
}

void Vertex::Set(float x, float y, float z, float nx, float ny, float nz, float u, float t, float r, float g, float b, float a)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
    v[3] = 1.0f;
    v[4] = nx;
    v[5] = ny;
    v[6] = nz;
    v[7] = 1.0f;
    v[8] = r;
    v[9] = g;
    v[10] = b;
    v[11] = a;
    v[12] = u;
    v[13] = t;
    v[14] = 0;
    v[15] = 0;
    v[16] = 0;
    v[17] = 0;
    v[18] = 0;
    v[19] = 0;
    v[20] = 0;
    v[21] = 0;
    v[22] = 0;
    v[23] = 0;
    v[24] = 1.0f;
    v[25] = 0;
    v[26] = 0;
    v[27] = 0;
}
