// Stacking the carried object (run from 0x100f5a80): while the player carries an object (DAT_104bfdd8, a slot of
// 0x5c and above), the nearest object of kind 5 or '_' right below it (a crate or a box) is found and the carried
// one is placed on top of it.

#include "world.h"

#include "vec4.h"

struct RootNode
{
    void *Find(const char *name, int index);         // 0x1019ece0 (Node::Find)
};

extern "C" Character DAT_10db6ed8[0x80];
extern "C" char DAT_1021f3c0[0x80];                  // kind of each character slot
extern "C" int DAT_104bfdd8;                         // the carried object's slot (below 0x5c: nothing carried)
extern "C" int DAT_104c0384;                         // the object it rests on (0 = none yet)
extern "C" int DAT_104c0388;                         // the carried slot DAT_104c0384 was found for
extern "C" Vec4 DAT_10867df8;                        // top of the object below; z becomes the height above it

// 0x1001d920 (Carry_StackOnObject). Finds the object of kind 5 or 0x5f (slots 0x5c..0x7f) whose top lies 135..165
// units above the carried object (285..315 for a tall one, which has the node "@O_019b") within 25 units in x and
// y, keeping the highest, and stores its position in DAT_10867df8 with z made relative. Once found, the carried
// object follows it: its model is placed at the remembered x/y on top. Slots moved below z -20000 (removed)
// reset the search. Returns 0.
extern "C" int FUN_1001d920(void)
{
    if (DAT_104bfdd8 >= 0x5c)
    {
        if (DAT_104c0388 != DAT_104bfdd8 ||
            (DAT_104c0384 >= 0x5c && DAT_10db6ed8[DAT_104c0384].matrix._41 < -20000.0f))
        {
            DAT_104c0384 = 0;
            DAT_104c0388 = DAT_104bfdd8;
        }
        Vec4 position = DAT_10db6ed8[DAT_104bfdd8].matrix.GetTranslation();
        Vec4 other;
        if (DAT_104c0384 < 0x5c)
        {
            int tall = 0;
            if (DAT_10db6ed8[DAT_104bfdd8].m_22fc->Find("@O_019b", 0) != 0)
                tall = 1;
            DAT_10867df8.Fill(0.0f);
            DAT_10867df8.z = 999999.0f;
            for (int i = 0x5c; i < 0x80; i++)
            {
                if (DAT_104c0384 == DAT_104bfdd8 || (DAT_1021f3c0[i] != 5 && DAT_1021f3c0[i] != '_'))
                    continue;
                other.Copy(DAT_10db6ed8[i].matrix.GetTranslation());
                if (DAT_104c0384 != 0 && (DAT_104c0384 < 0x5c || !(DAT_10867df8.z > other.z)))
                    continue;
                if (tall != 0)
                {
                    if (!(position.z + 285.0f < other.z) || !(position.z + 315.0f > other.z))
                        continue;
                }
                else
                {
                    if (!(position.z + 135.0f < other.z) || !(position.z + 165.0f > other.z))
                        continue;
                }
                if (position.x - 25.0f < other.x && position.x + 25.0f > other.x && position.y - 25.0f < other.y &&
                    position.y + 25.0f > other.y)
                {
                    DAT_104c0384 = i;
                    DAT_10867df8.Copy(other);
                }
            }
            DAT_10867df8.z -= position.z;
            return 0;
        }
        if (DAT_10db6ed8[DAT_104bfdd8].matrix._41 > -20000.0f)
        {
            DAT_10867df8.x = position.x;
            DAT_10867df8.y = position.y;
            DAT_10db6ed8[DAT_104c0384].model.M_10162610(position.x, position.y, DAT_10867df8.z + position.z);
            return 0;
        }
    }
    DAT_104c0384 = 0;
    return 0;
}
