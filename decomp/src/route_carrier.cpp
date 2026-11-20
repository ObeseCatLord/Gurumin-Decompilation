#include <math.h>

// A character that walks one of the map's routes (0x100451c0, behaviour 9 of Char_RunBehaviour 0x100dfcf0),
// optionally carrying another character on its back.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    void Fill(const float &value);               // 0x101acc40: sets all four components
    Vec4 Normalize3();                           // 0x101af080: normalises x/y/z in place, returns a copy
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
    Vec4 operator+(const Vec4 &other) const;     // 0x101ac9e0
    Vec4 operator*(const float &scale) const;    // 0x101acb00 (x/y/z only)
};

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

// A character record seen from its model part (char_move.cpp CharRecord), only the fields used here.
struct WalkerRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    void SetPosition(float x, float y, float z);             // 0x10162610: moves the model
    char pad_0000[0xd4c];
    float alpha;              // 0xd4c: model opacity (1.0 = solid)
    char pad_0d50[0x159c];
    void *root;               // 0x22ec: root node of the model (0 = no model loaded)
    char pad_22f0[0x38];
    Matrix4 matrix;           // 0x2328: world matrix
    char pad_2368[0x118];
    float targetHeading;      // 0x2480: angle the character turns towards
    char pad_2484[0x14];
    int m_2498;               // 0x2498: set to 1 by the scene start for the player
};

typedef char check_walker[sizeof(WalkerRecord) == 0x249c ? 1 : -1];

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = start, 1 = pausing, 2 = walking, 4 = dropping its rider, 5 = fading out
    int wait;                 // 0x04: frames in the current state
    int talk;                 // 0x08: route + rider offset * 100
    int route;                // 0x0c: the route walked
    int point;                // 0x10: the route point headed for
    int backwards;            // 0x14: 1 while walking the route backwards (Route_Follow)
    int speed;                // 0x18: units per frame (from the route's first point)
    int rider;                // 0x1c: character carried (0 = none)
    int pause;                // 0x20: frames to wait at a point
    int pad_0024;
    int fadeActions;          // 0x28: bit n set = fade out when a point with action n is reached
    char pad_002c[0x14];
};

// The last position and step of each character (route_walk.cpp StepRecord); here the position holds the velocity.
struct LastPosition
{
    Vec4 position;
    Vec4 step;
    char pad_0020[0x20];
};

// Per-slot state table at 0x10daaaec (actor_scan.cpp ActorSlot).
struct ActorSlot
{
    int health;               // 0x00
    int maxHealth;            // 0x04
    char pad_0008[0x180];
};

struct RoutePoint
{
    int x;
    int y;
    int z;
    int code;
};

extern "C" WalkerRecord DAT_10db6ee8[0x80];
extern "C" NpcState DAT_1020b430[0x80];
extern "C" LastPosition DAT_10802bf8[0x80];
extern "C" ActorSlot DAT_10daaaec[0x80];
extern "C" RoutePoint DAT_10210fc0[][16];     // the routes of the map
extern "C" int DAT_10210b30[0x80];            // non-zero = told to react
extern "C" int DAT_10210d30[0x80];            // non-zero = the character is being defeated
extern "C" int DAT_10254598[0x80];
extern "C" char DAT_1020d540[0x80];           // 1 = the character is in play
extern "C" char DAT_1021b4c8[0x80];
extern "C" char DAT_10256420[0x80];
extern "C" Vec4 DAT_10867e18[0x80];
extern "C" int FUN_10043cc0(int index);       // Char_ResetState
extern "C" int FUN_1002f320(int walker, int route, int point, int *outPoint, float *position, int *outAction,
                            int *outValue, int *outGroup, int radius);  // Route_Follow

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x100451c0 (Npc_WalkRoute). Walks route talk % 100 from point 1 at the speed in the route's first point (x),
// easing its velocity (0.9 old + 0.1 new), turning to the point ahead and pausing where a point's value says so.
// A point whose action is in the route's fade mask (first point's code) fades it out over 15 frames and takes it
// out of play; a point with value 1 (or being told to react) drops the rider (talk / 100 characters later), which
// is reset. While it walks, the rider sits 50 units above it. Returns 0.
extern "C" int FUN_100451c0(int index)
{
    Vec4 player;
    Vec4 self;
    Vec4 target;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    target.Copy(self);
    int point;
    int action;
    int value;
    int group;
    DAT_10db6ee8[index].m_2498 = 0;
    if (DAT_10210b30[index] != 0)
    {
        DAT_1020b430[index].state = 4;
        DAT_10210b30[index] = 0;
        DAT_10db6ee8[index].alpha = 1.0f;
    }
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].point = 1;
        DAT_1020b430[index].backwards = 0;
        DAT_1020b430[index].route = DAT_1020b430[index].talk % 100;
        FUN_1002f320(index, DAT_1020b430[index].talk % 100, 1, &point, &target.x, &action, &value, &group, 20);
        DAT_1020b430[index].point = point;
        DAT_1020b430[index].state = 2;
        DAT_1020b430[index].wait = 0;
        DAT_1020b430[index].rider = DAT_1020b430[index].talk / 100 + index;
        DAT_1020b430[index].speed = DAT_10210fc0[DAT_1020b430[index].talk % 100][0].x;
        DAT_1020b430[index].fadeActions = DAT_10210fc0[DAT_1020b430[index].talk % 100][0].code;
        if (value > 1)
        {
            DAT_1020b430[index].pause = value;
            DAT_1020b430[index].state = 1;
            DAT_1020b430[index].wait = 0;
        }
    }
    else if (DAT_1020b430[index].state == 1)
    {
        if (++DAT_1020b430[index].wait > DAT_1020b430[index].pause)
        {
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].state = 2;
        }
    }
    else if (DAT_1020b430[index].state == 2)
    {
        Vec4 step;
        Vec4 velocity;
        int reached = FUN_1002f320(index, DAT_1020b430[index].route, DAT_1020b430[index].point, &point, &target.x,
                                   &action, &value, &group, DAT_1020b430[index].speed / 2 + 20);
        DAT_10db6ee8[index].targetHeading = (float)atan2(target.x - self.x, target.y - self.y) + 3.1415f;
        DAT_10db6ee8[index].PlayMotion(0, 0, 5, 1.0f);
        if (DAT_1020b430[index].point != point)
        {
            DAT_1020b430[index].state = 2;
            DAT_1020b430[index].wait = 0;
            if (((1 << reached) & DAT_1020b430[index].fadeActions) != 0)
            {
                DAT_1020b430[index].state = 5;
                DAT_1020b430[index].wait = 0;
                if (value == 2)
                {
                    DAT_1020b430[index].state = 2;
                    DAT_1020b430[index].wait = 0;
                }
                else if (value == 1)
                {
                    if (DAT_1020b430[index].rider != 0)
                        DAT_1020b430[index].state = 4;
                }
                else if (value > 1)
                {
                    DAT_1020b430[index].pause = value;
                    DAT_1020b430[index].state = 1;
                    DAT_1020b430[index].wait = 0;
                }
            }
            else if (value == 1)
            {
                if (DAT_1020b430[index].rider != 0)
                    DAT_1020b430[index].state = 4;
            }
            else if (value > 1)
            {
                DAT_1020b430[index].pause = value;
                DAT_1020b430[index].state = 1;
                DAT_1020b430[index].wait = 0;
            }
        }
        step.Copy(target - self);
        step.Normalize3();
        step.Copy(step * (float)DAT_1020b430[index].speed);
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.9f + step * 0.1f);
        Vec4 position;
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + DAT_10802bf8[index].position);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
        DAT_1020b430[index].point = point;
    }
    else if (DAT_1020b430[index].state == 4)
    {
        int rider = DAT_1020b430[index].rider;
        if (rider != 0 && DAT_10db6ee8[rider].root != 0 && DAT_10210d30[rider] == 0 && DAT_10daaaec[rider].health > 0)
            FUN_10043cc0(rider);
        DAT_1020b430[index].rider = 0;
        DAT_1020b430[index].state = 2;
        Vec4 position;
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.9f);
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + DAT_10802bf8[index].position);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
    }
    else if (DAT_1020b430[index].state == 5)
    {
        DAT_10db6ee8[index].alpha = (float)(15 - DAT_1020b430[index].wait++) / 15.0f;
        if (DAT_1020b430[index].wait == 15)
        {
            DAT_10db6ee8[index].alpha = 1.0f;
            DAT_10db6ee8[index].SetPosition(-200000.0f, 0, 0);
            DAT_1020d540[index] = 0;
            DAT_1021b4c8[index] = 0;
            DAT_10867e18[index].Fill(0.0f);
            DAT_10256420[index] = 0;
        }
        Vec4 position;
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.9f);
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation() + DAT_10802bf8[index].position);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
    }
    if (DAT_10210d30[index] == 0)
    {
        int rider = DAT_1020b430[index].rider;
        if (rider != 0 && DAT_10db6ee8[rider].root != 0 && DAT_10210d30[rider] == 0 && DAT_10daaaec[rider].health > 0)
        {
            Vec4 position;
            position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            DAT_10db6ee8[DAT_1020b430[index].rider].SetPosition(position.x, position.y, position.z + 50.0f);
            DAT_10254598[DAT_1020b430[index].rider] = 0;
        }
    }
    return 0;
}
