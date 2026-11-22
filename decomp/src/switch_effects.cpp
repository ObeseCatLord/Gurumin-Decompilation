// Effects on the switch hold slots (switches.cpp): every 12 frames each used slot whose object is within 3000
// units of the camera target and visible starts its effect again (attached to the object, or at its position),
// or, for objects with flag 0x20, releases its holder.

// Effect start parameters (model_state.cpp ModelState), only the field set here.
struct ModelState
{
    char pad_0000[0xd];
    char m_000d;                  // 0x0d: 1 = the effect follows its attachment
    char pad_000e[0x36];
    ModelState();                 // 0x101668b0
};

#include "vec4.h"

#include "matrix4.h"

// A map object that can be held as a switch.
struct SwitchObject
{
    char pad_0000[0xac];
    unsigned char flags;          // 0xac: 0x20 = no effect, the holder is released instead
    char pad_00ad[0x19f];
    Matrix4 matrix;               // 0x24c: world matrix
};

struct EffectManager
{
    // 0x10172350 (char_effects.cpp): starts effect `effect` at `position`.
    int M_10172350(int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g, int *handle);
};

extern "C" SwitchObject *DAT_1030bbf8[0x80];     // object of each used hold slot (0 = unused)
extern "C" int DAT_10255b08[0x80];               // holder of each slot
extern "C" int DAT_10219fc0[0x80];               // effect kind of each slot (0, 1 or 2)
extern "C" int DAT_1042e0a8[0x80];               // 1 once the slot's effect was started
extern "C" unsigned int DAT_104bf890;            // frame counter
extern "C" Vec4 DAT_118e6cb8;                    // camera target
extern "C" EffectManager DAT_10daaa80;
extern "C" int FUN_10015860(Vec4 position);      // Vis_IsVisible
extern "C" int FUN_10057b90(int holder);         // Switch_Release

// 0x10058500 (Switch_UpdateEffects). For each hold slot in use whose object is near (under 3000 units from the
// camera target) and visible: the first time and then every 12 frames, objects with flag 0x20 release their
// holder; the others start effect 0x65 attached to the object (kind 0), or effect 0x6a (kind 1) or 0x86b
// (kind 2) at the object's position. Returns 1.
extern "C" int FUN_10058500(void)
{
    for (int slot = 0; slot < 0x80; slot++)
    {
        if (DAT_1030bbf8[slot] != 0)
        {
            Vec4 at = DAT_1030bbf8[slot]->matrix.GetTranslation();
            if (at.Distance(DAT_118e6cb8) < 3000.0f && FUN_10015860(at))
            {
                Vec4 position;
                if (DAT_1042e0a8[slot] == 0 || DAT_104bf890 % 12 == 0)
                {
                    DAT_1042e0a8[slot] = 1;
                    if (DAT_1030bbf8[slot]->flags & 0x20)
                        FUN_10057b90(DAT_10255b08[slot]);
                    else
                    {
                        if (DAT_10219fc0[slot] == 0)
                        {
                            position.Fill(0.0f);
                            ModelState state;
                            state.m_000d = 1;
                            DAT_10daaa80.M_10172350(0x65, position, -1, -1, (int)&DAT_1030bbf8[slot]->matrix, 0, 0,
                                                    0, (int)&state, 0);
                        }
                        if (DAT_10219fc0[slot] == 1)
                        {
                            ModelState state;
                            state.m_000d = 1;
                            position.Copy(DAT_1030bbf8[slot]->matrix.GetTranslation());
                            DAT_10daaa80.M_10172350(0x6a, position, -1, -1, 0, 0, 0, 0, (int)&state, 0);
                        }
                        if (DAT_10219fc0[slot] == 2)
                        {
                            ModelState state;
                            state.m_000d = 1;
                            position.Copy(DAT_1030bbf8[slot]->matrix.GetTranslation());
                            DAT_10daaa80.M_10172350(0x86b, position, -1, -1, 0, 0, 0, 0, (int)&state, 0);
                        }
                    }
                }
            }
        }
    }
    return 1;
}
