// Following the walking routes of the map: each route has 16 points (x, y, z and a code). The code holds an action
// (code / 10000) and a value (code % 10000), each wrapped to -4999..5000; the action's low two bits say what happens
// when the walker reaches the point and the rest is a group number.

struct RoutePoint
{
    int x;
    int y;
    int z;
    int code;
};

extern "C" RoutePoint DAT_10210fc0[][16];        // the routes of the map
extern "C" int DAT_1020b444[][16];               // per walker: [0] = 1 while it walks its route backwards

// 0x1002f320 (Route_Follow). Reads point `point` of route `route` for walker `walker`: stores its action (low two
// bits), value and group in *outAction, *outValue and *outGroup. When `position` lies within `radius` of the point
// on all three axes the walker moves on: action 0 goes to the next point in its direction, 1 forwards, 2 back to
// point 1 and 3 turns it round (1 and 2 also set it walking forwards). Stores the point it heads for in *outPoint
// and that point's coordinates in `position`. Returns 1-3 for the actions 1-3 when the point was reached, else 0.
extern "C" int FUN_1002f320(int walker, int route, int point, int *outPoint, float *position, int *outAction,
                            int *outValue, int *outGroup, int radius)
{
    float x = (float)DAT_10210fc0[route][point].x;
    float y = (float)DAT_10210fc0[route][point].y;
    float z = (float)DAT_10210fc0[route][point].z;
    int action = DAT_10210fc0[route][point].code / 10000;
    int result = 0;
    if (action > 5000)
        action -= 10000;
    *outAction = action;
    int value = DAT_10210fc0[route][point].code % 10000;
    if (value > 5000)
        value -= 10000;
    *outValue = value;
    *outGroup = *outAction / 4;
    *outAction = *outAction % 4;
    float range = (float)radius;
    int next = point;
    if (x - range < position[0] && range + x > position[0] && y - range < position[1] && range + y > position[1] &&
        z - range < position[2] && range + z > position[2])
    {
        switch (*outAction)
        {
        case 0:
            if (DAT_1020b444[walker][0] != 0)
                next = point - 1;
            else
                next = point + 1;
            break;
        case 1:
            result = 1;
            DAT_1020b444[walker][0] = 0;
            next = point + 1;
            break;
        case 2:
            result = 2;
            DAT_1020b444[walker][0] = 0;
            next = 1;
            break;
        case 3:
            result = 3;
            DAT_1020b444[walker][0] = 1;
            next = point - 1;
            break;
        }
        x = (float)DAT_10210fc0[route][next].x;
        y = (float)DAT_10210fc0[route][next].y;
        z = (float)DAT_10210fc0[route][next].z;
    }
    *outPoint = next;
    position[0] = x;
    position[1] = y;
    position[2] = z;
    return result;
}
