struct Matrix4;

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
    Vec4 Transformed(const Matrix4 &matrix) const;
};

Vec4 Cross(const Vec4 &a, const Vec4 &b);

Vec4 CrossTransformed(const Vec4 &a, const Matrix4 &matrix, const Vec4 &b)
{
    Vec4 tb;
    Vec4 ta;
    Vec4 result;
    ta.Copy(a.Transformed(matrix));
    tb.Copy(b.Transformed(matrix));
    result.Copy(Cross(ta, tb));
    return result;
}
