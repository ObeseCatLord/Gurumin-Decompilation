// The default enemy step (0x100e21a0, called from the enemy and scene updates): turn to the player, idle, then
// pick an action. Characters 11, 12, 14, 20 and 25 and those above 0x39 have small special cases.

#include "vec4.h"

#include "matrix4.h"

// A model's motion playback state (model_state.cpp Playback), only the fields used here.
struct Playback
{
    char pad_0000[0x7c74];
    int blending;             // 0x7c74: non-zero while blending into another layer
    int pad_7c78;
    int nextLayer;            // 0x7c7c: layer being blended into
};

// A character record seen from its model part (char_move.cpp CharRecord), only the fields used here.
struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    void SetPosition(float x, float y, float z);             // 0x10162610: moves the model
    char pad_0000[0x265];
    char m_0265;              // 0x265
    char pad_0266[0x32];
    Playback *motion;         // 0x298: the model's motion playback state
    char pad_029c[0xa90];
    int m_0d2c;               // 0xd2c
    char pad_0d30[0x15f8];
    Matrix4 matrix;           // 0x2328: world matrix
    char pad_2368[0x114];
    float heading;            // 0x247c: facing angle in radians
    char pad_2480[0x1c];
};

typedef char check_charrecord[sizeof(CharRecord) == 0x249c ? 1 : -1];

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = turning to the player, 1 = idling, 2 = done, else = counting to an action
    int wait;
    int talk;
    int count;                // 0x0c
    char pad_0010[0x30];
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" NpcState DAT_1020b430[0x80];
extern "C" int DAT_10c62ed0;                  // current map
extern "C" int DAT_10c6351c;                  // the character being talked to
extern "C" int DAT_10de6e80;                  // character 20's frame counter (CharState of character 0x5d)
extern "C" Vec4 DAT_10803118;                 // character 20's resting place
extern "C" int DAT_10c6564c;
extern "C" int DAT_10c65648;
extern "C" int FUN_100e1c40(int index);                // Enemy_IsBusy
extern "C" int FUN_10028360(int index, int a);         // Enemy_TurnToPlayer
extern "C" int FUN_10027560(int index);                // Enemy_IdleMotion
extern "C" int FUN_10026bf0(int index);                // Enemy_PickAction_65_35

// 0x100e21a0 (Enemy_DefaultStep). Character 14 clears its m_0d2c on maps below 20; character 20 being talked to
// (0xdc) slides 2 units a frame ahead of its resting place during frames 0x82..0x8c, else follows its motion back
// to it. Busy characters do nothing; characters 12, 14, 20, 25 and above 0x39 restart (20 also sets m_0265);
// character 11 plays motion 10 while DAT_10c6564c is set without DAT_10c65648. Otherwise it turns to the player,
// idles, and picks an action (65/35) once its count passes 0. Returns 0.
extern "C" int FUN_100e21a0(int index)
{
    if (index == 14)
    {
        if (DAT_10c62ed0 < 20)
            DAT_10db6ee8[index].m_0d2c = 0;
    }
    else if (index == 20 && DAT_10c6351c == 0xdc)
    {
        if (DAT_10de6e80 < 0x8d && DAT_10de6e80 > 0x81)
        {
            Vec4 position;
            position.y = (float)(DAT_10de6e80 - 0x82);
            position.y += position.y;
            position.RotateZ(-DAT_10db6ee8[index].heading);
            position.Copy(position + DAT_10803118);
            DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
            return 0;
        }
        if (DAT_10db6ee8[index].motion->blending == 0)
            return 0;
        if (DAT_10db6ee8[index].motion->nextLayer == 4)
        {
            DAT_10803118.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            return 0;
        }
        if (DAT_10db6ee8[index].motion->blending == 0 || DAT_10db6ee8[index].motion->nextLayer != 2)
            return 0;
        Vec4 position;
        position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        float z = position.z;
        position.Copy(position * 0.6f + DAT_10803118 * 0.4f);
        position.z = z;
        DAT_10db6ee8[index].SetPosition(position.x, position.y, position.z);
        return 0;
    }
    if (FUN_100e1c40(index) != 0)
        return 0;
    if (index == 0x19 || index == 0xc || index == 0x14 || index == 0xe || index > 0x39)
    {
        if (index == 0x14)
            DAT_10db6ee8[index].m_0265 = 1;
        DAT_1020b430[index].state = 0;
    }
    if (index == 0xb && DAT_10c6564c != 0 && DAT_10c65648 == 0)
    {
        DAT_10db6ee8[index].PlayMotion(10, 0, 5, 1.0f);
        return 0;
    }
    switch (DAT_1020b430[index].state)
    {
    case 0:
        if (FUN_10028360(index, 2) == 0)
            return 0;
        break;
    case 1:
        if (FUN_10027560(index) == 0)
            return 0;
        break;
    case 2:
        break;
    default:
        if (++DAT_1020b430[index].count > 0)
        {
            DAT_1020b430[index].count = 0;
            FUN_10026bf0(index);
        }
        return 0;
    }
    DAT_1020b430[index].count = 0;
    DAT_1020b430[index].state = -1;
    return 0;
}
