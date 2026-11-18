// Route lengths: each entry of the map table at 0x10210fc0 (see map_icons.cpp) holds a route of up to 15 points.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    float Length();                              // 0x101ac4e0: length of x/y/z
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
};

// A route point: a position and a code. The code's low four digits (as a signed value, 5001..9999 meaning
// -4999..-1) are an extra cost of the point; the digits above them (signed the same way) modulo 4 above 1 end the
// route.
struct RoutePoint
{
    int x;
    int y;
    int z;
    int code;
};

// Entry of the map table, 0x100 bytes.
struct MapEntry
{
    int id;
    int pad_0004;
    int icon;                 // 0x08: index into the icon name table
    int pad_000c;
    RoutePoint route[15];     // 0x10
};

// A signed four-digit field: values above 5000 count down from 10000.
inline int SignedField(int value)
{
    if (value > 5000)
        value -= 10000;
    return value;
}

extern "C" MapEntry DAT_10210fc0[];

// 0x10049990 (Map_RouteLength). Returns the length of the route of map table entry `map`, walked until a point ends
// it and back again, plus each point's extra cost times `costScale`. Point i is stored at entries i and 2 * i of one
// array and the segments are measured between the 2 * i entries. Because entry 2 is overwritten by the second point
// in the same step, the first segment measures 0 and the route closes at the second point, not the first.
extern "C" float FUN_10049990(int map, int costScale)
{
    float total = 0;
    Vec4 points[32];
    int count = 0;
    for (int i = 1; i < 16; i++)
    {
        points[i].x = points[2 * i].x = (float)DAT_10210fc0[map].route[i - 1].x;
        points[i].y = points[2 * i].y = (float)DAT_10210fc0[map].route[i - 1].y;
        points[i].z = points[2 * i].z = (float)DAT_10210fc0[map].route[i - 1].z;
        if (i > 1)
            total += (points[2 * i] - points[2 * (i - 1)]).Length();
        if (SignedField(DAT_10210fc0[map].route[i - 1].code % 10000) != 0)
            total += (float)(SignedField(DAT_10210fc0[map].route[i - 1].code % 10000) * costScale);
        count++;
        if (SignedField(DAT_10210fc0[map].route[i - 1].code / 10000) % 4 > 1)
            break;
    }
    total += (points[2 * count] - points[2]).Length();
    return total;
}
