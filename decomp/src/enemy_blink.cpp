// An enemy that appears out of nowhere (0x10046f10, called from the behaviour dispatch 0x100dfcf0): it waits
// hidden far away until the player comes near its home, picks a random spot around the home that it can stand on
// and slides into place while fading in.

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = start, 1 = hidden, 2 = appearing
    int wait;                 // 0x04: frames since appearing
    int talk;                 // 0x08: distance from its home at which the player makes it appear
    char pad_000c[0x34];
};

// The last position and step of each character (route_walk.cpp StepRecord); here the position holds the home.
struct LastPosition
{
    Vec4 position;
    Vec4 step;
    char pad_0020[0x20];
};

struct VisData;

// The map's collision node (world.h), only what is used here.
struct CollisionWorld
{
    int Collide(int name, int mask, Vec4 *from, Vec4 *to, Vec4 *hit);  // Node::Collide 0x101a8e60
    int RayCast(const char *name, unsigned kind, unsigned mask, const Vec4 *to, const Vec4 *from, Vec4 *hit,
                float *t);                                              // 0x101a9670
    char pad_0000[0x1864];
    VisData *vis;             // 0x1864: visibility cells (0 = none)
};

extern "C" NpcState DAT_1020b430[0x80];
extern "C" LastPosition DAT_10802bf8[0x80];
extern "C" char DAT_1020d540[0x80];           // 1 = the character is in play
extern "C" CollisionWorld *DAT_10255f08;
extern "C" int FUN_1013f880(void);            // the game's random number
extern "C" int FUN_10015860(Vec4 pos);        // Vis_IsVisible
extern "C" int FUN_10043cc0(int index);       // Char_ResetState

// 0x10046f10 (Enemy_AppearNearby). Remembers where it was placed (its home) and hides at x = -200000. Once the
// player is within the talk id's units of the home it tries up to ten random directions from the home (1000 units
// at a random pitch and heading, pulled back to 75 units short of what the ray hits when that is still 500 units
// away, and in a visible cell): where it can stand and finds ground below, it moves there and slides back a tenth of
// the way home each frame for 10 frames, half transparent, before its state is reset. Returns 0.
extern "C" int FUN_10046f10(int index)
{
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].state = 1;
        DAT_10802bf8[index].position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        DAT_1020d540[index] = 0;
        DAT_10db6ee8[index].matrix.m[12] = -200000.0f;
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        return 0;
    }
    if (DAT_1020b430[index].state == 1)
    {
        int reach = DAT_1020b430[index].talk;
        if ((DAT_10802bf8[index].position - DAT_10db6ee8[0].matrix.GetTranslation()).LengthSquared3() <
            (float)(reach * reach))
        {
            Vec4 spot;
            Vec4 ground;
            Vec4 probe;
            Vec4 hit;
            probe.Copy(DAT_10802bf8[index].position);
            probe.z += 50.0f;
            int tries;
            for (tries = 0; tries < 10; tries++)
            {
                spot.Fill(0.0f);
                spot.y = -1000.0f;
                spot.RotateX((float)(FUN_1013f880() % 180 - 90) * 3.1415f / 180.0f);
                spot.RotateZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
                spot.Copy(spot + DAT_10802bf8[index].position);
                if (DAT_10255f08 != 0)
                {
                    if (DAT_10255f08->RayCast(0, 4, 0, &spot, &probe, &hit, 0) != 0)
                    {
                        spot.Copy(probe - hit);
                        spot.Normalized3();
                        spot.Copy(spot * 75.0f);
                        spot.Copy(hit + spot);
                        if ((spot - probe).LengthSquared3() < 250000.0f)
                            continue;
                    }
                    if (DAT_10255f08 != 0 && DAT_10255f08->vis != 0 && FUN_10015860(spot) == 0)
                        continue;
                }
                DAT_10db6ee8[index].SetPosition(probe.x, probe.y, probe.z);
                if (DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
                    continue;
                DAT_10db6ee8[index].SetPosition(-200000.0f, 0, 0);
                ground.Copy(probe);
                probe.z -= 1000.0f;
                if (DAT_10255f08 != 0 && DAT_10255f08->Collide(0, 4, &ground, &probe, &hit) != 0)
                    break;
            }
            if (tries < 10)
            {
                DAT_1020b430[index].state = 2;
                DAT_1020b430[index].wait = 0;
                DAT_10db6ee8[index].SetPosition(spot.x, spot.y, spot.z);
                DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position - spot);
                DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * 0.1f);
                DAT_10db6ee8[index].mode = 0;
                DAT_10db6ee8[index].alpha = 0.5f;
                return 0;
            }
        }
    }
    else if (DAT_1020b430[index].state == 2)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        DAT_1020b430[index].wait++;
        Vec4 position;
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        position.Copy(position + DAT_10802bf8[index].position);
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
        DAT_10db6ee8[index].alpha = 0.5f;
        if (DAT_1020b430[index].wait == 10)
        {
            DAT_10db6ee8[index].alpha = 1.0f;
            DAT_10db6ee8[index].mode = 1;
            FUN_10043cc0(index);
        }
    }
    return 0;
}
