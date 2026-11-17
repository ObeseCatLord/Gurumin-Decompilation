// Projection parameters.

extern "C" int DAT_10f668f8;
extern "C" int DAT_10f668fc;
extern "C" int DAT_10f66900;
extern "C" float DAT_10f66968;
extern "C" float DAT_10f6696c;
extern "C" int DAT_10f66970;
extern "C" int DAT_10f66974;

// Stores the viewport (a, b, c, d) and the near plane, and precomputes 1 / (far - near) for depth scaling.
extern "C" int FUN_10199dc0(int a, int b, int c, int d, float nearZ, float farZ)
{
    DAT_10f668f8 = a;
    DAT_10f668fc = b;
    DAT_10f66900 = c;
    DAT_10f66968 = nearZ;
    DAT_10f6696c = 1.0f / (farZ - nearZ);
    DAT_10f66970 = 0;
    DAT_10f66974 = d;
    return 1;
}
