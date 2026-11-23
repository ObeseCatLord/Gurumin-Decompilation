// A character held in place by a spell (0x100469d0, behaviour 0x18 of Char_RunBehaviour 0x100dfcf0): it sparkles
// until the scripts release it, then it is freed with a flash and its state is reset.

#include "vec4.h"

#include "matrix4.h"

Matrix4 RotationZ(float angle);                  // 0x101b1b40

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_slot.h"

#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"

extern "C" int FUN_1013f880(void);            // the game's random number
extern "C" int FUN_10043cc0(int index);       // Char_ResetState

// 0x100469d0 (Npc_Held). On its first frame it saves the slot's m_0140 and starts effect 0x36c6 (turned at random,
// following the model's "Bip01 Spine" node or else the character); every eighth frame after that it adds sparkle
// 0x36c7. While the scripts run it stays held (m_0267, m_0294, slot m_013e and m_0094 0x1d) until DAT_103153b0 is
// set or its group releases it (slot m_0080 1, with flash 0x2265 50 units above it): then its effects stop, effect
// 0x1390 plays on it, the slot gets its m_0140 back and its state is reset. Returns 0.
int FUN_100469d0(int index)
{
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].saved = DAT_10daaae8[index].m_0144;
        DAT_1020b430[index].state = 1;
        EffectOptions options;
        Matrix4 matrix;
        Vec4 position;
        options.owner = index + 100;
        options.followOwner = 1;
        matrix = RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
        matrix.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
        if (DAT_10db6ee8[index].root->Find("Bip01 Spine", 0) != 0)
            DAT_10daaa80.Start(0x36c6, position, -1, -1, &DAT_10db6ee8[index].root->Find("Bip01 Spine", 0)->worldMatrix,
                               0, 0, &matrix, &options, 0);
        else
            DAT_10daaa80.Start(0x36c6, position, -1, -1, &DAT_10db6ee8[index].matrix, 0, 0, &matrix, &options, 0);
        DAT_10db6ee8[index].mode = 0;
        DAT_10db6ee8[index].m_0267 = 1;
        DAT_10db6ee8[index].m_0294 = 1;
        DAT_10daaae8[index].held = 1;
        DAT_10daaae8[index].m_0098 = 0x1d;
        DAT_10255a88[index] = 0;
    }
    else if (++DAT_1020b430[index].wait % 8 == 0)
    {
        Matrix4 matrix;
        Vec4 position;
        EffectOptions options;
        options.followOwner = 1;
        matrix = RotationZ((float)(FUN_1013f880() % 360) * 3.1415f / 180.0f);
        matrix.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
        DAT_10daaa80.Start(0x36c7, position, -1, -1, 0, 0, 0, &matrix, &options, 0);
    }
    if (DAT_1032d69c != 0)
    {
        DAT_10db6ee8[index].mode = 0;
        DAT_10db6ee8[index].m_0267 = 1;
        DAT_10db6ee8[index].m_0294 = 1;
        DAT_10daaae8[index].held = 1;
        DAT_10daaae8[index].m_0098 = 0x1d;
        DAT_10255a88[index] = 0;
        if (DAT_103153b0[index] == 0 && DAT_10daaae8[index].m_0084 != 1)
            return 0;
        if (DAT_103153b0[index] == 0)
        {
            Vec4 position;
            position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            position.z += 50.0f;
            DAT_10daaa80.Start(0x2265, position, -1, -1, 0, 0, 0, 0, 0, 0);
        }
        DAT_10daaae8[index].m_0144 = (char)DAT_1020b430[index].saved;
        DAT_10db6ee8[index].mode = 1;
        DAT_10db6ee8[index].m_0267 = 0;
        DAT_10daaae8[index].held = 0;
        DAT_10daaae8[index].m_0098 = 0;
        DAT_10daaa80.RemoveOwnedBy(index + 100);
        Vec4 position;
        DAT_10daaa80.Start(0x1390, position, -1, -1, 0, 0, 0, &DAT_10db6ee8[index].matrix, 0, 0);
        DAT_103153b0[index] = 0;
        DAT_10db6ee8[index].m_0294 = 0;
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
        FUN_10043cc0(index);
        DAT_10255a88[index] = 1;
    }
    return 0;
}
