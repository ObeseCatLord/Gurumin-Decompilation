struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Copy(const Vec4 &source);
};

struct Matrix4
{
    float m[16];
};

struct Block80
{
    char data[0x80];
};

struct Node;

extern "C" int DAT_11b044d0;
extern "C" int DAT_11b044dc;
extern "C" int DAT_11b002b8;
extern "C" Node *DAT_11b00070[0x40];
extern "C" char DAT_11b00270[0x40];
extern "C" char DAT_11b00030[0x40];
extern "C" char DAT_11af3f68[0x40];
extern "C" char DAT_11afffa8[0x40];
extern "C" int DAT_11b00170[0x40];
extern "C" Vec4 DAT_11b04518[0x40];
extern "C" Vec4 DAT_11b04918[0x40];
extern "C" Vec4 DAT_11b07128[0x40];
extern "C" Block80 DAT_11b04d18[0x30];
extern "C" Matrix4 DAT_11b06518[0x30];
extern "C" int DAT_10f65520;
extern "C" int DAT_10f655c4;
extern "C" Vec4 DAT_10f66948;
extern "C" void *(*DAT_11b07530)(void *dst, const void *src, int size);

extern "C" int FUN_10199880(Node *node, const void *extra, const Matrix4 *matrix, const Vec4 *a, const Vec4 *b, char flags)
{
    DAT_11b00070[DAT_11b044d0] = node;
    char option = (char)DAT_10f65520;
    DAT_11b00270[DAT_11b044d0] = option;
    DAT_11b00030[DAT_11b044d0] = flags;
    DAT_11af3f68[DAT_11b044d0] = (char)0xff;
    DAT_11afffa8[DAT_11b044d0] = (char)0xff;
    DAT_11b04518[DAT_11b044d0].Copy(*a);
    DAT_11b04918[DAT_11b044d0].Copy(*b);
    DAT_11b07128[DAT_11b044d0].Copy(DAT_10f66948);
    DAT_11b00170[DAT_11b044d0] = DAT_10f655c4;
    if (extra != 0)
    {
        DAT_11b07530(&DAT_11b04d18[DAT_11b044dc], extra, 0x80);
        DAT_11afffa8[DAT_11b044d0] = (char)DAT_11b044dc;
        if (DAT_11b044dc < 0x2f)
            DAT_11b044dc++;
    }
    if (matrix != 0)
    {
        DAT_11b06518[DAT_11b002b8] = *matrix;
        DAT_11af3f68[DAT_11b044d0] = (char)DAT_11b002b8;
        if (DAT_11b002b8 < 0x2f)
            DAT_11b002b8++;
    }
    if (DAT_11b044d0 < 0x3f)
        DAT_11b044d0++;
    return 0;
}
