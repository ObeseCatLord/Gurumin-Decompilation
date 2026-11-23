// Characters driven by a step script (FUN_10029cf0 runs one step). The character records are viewed from their
// model part at 0x10db6ee8 here (like char_move.cpp), where all the fields used lie inside one record.

#include "vec4.h"

#include "vec3.h"

#include "matrix4.h"

#include "anim_model.h"

// Per-character script state (world.h CharState).
struct ScriptState
{
    int m_0000;
    int running;              // 0x04
    int step;                 // 0x08: current script step
    int registered;           // 0x0c: 1 once the position is in the camera list
    int pad_0010;
    int value;                // 0x14: value carried between steps (the opacity in tenths)
    int keep;                 // 0x18: non-zero = stay in the world after the script ends
    int result;               // 0x1c: result of the last step (5 = run the next step at once, 2 = waiting)
    char pad_0020[0x20];
};

extern "C" ScriptState DAT_1020b430[0x80];
extern "C" Vec3 *DAT_1032d5b8[];      // positions the camera keeps in view
extern "C" char DAT_1021bbc0[];       // a flag per camera position
extern "C" int DAT_1049f0f0;          // number of camera positions
extern "C" int DAT_104c036c;          // 1 once a scripted character was registered
// 0x10029cf0: runs one script step of character `index`.
extern "C" int FUN_10029cf0(int index, int *a, int step, int *nextStep, Vec4 *position, int *result, int *value,
                            int *b);

// 0x1002a340 (Char_RunScript). Runs the script of character `index` for this frame: steps follow each other at once
// while a step returns 5. When the script ends (a result other than 2 and `keep` not set) the character is reset
// and moved out of the world. On the first call its position is added to the camera list. The opacity follows the
// script value (tenths). Returns 0.
extern "C" int FUN_1002a340(int index)
{
    Vec4 position;
    Vec4 unused;
    int a;
    int b;
    int nextStep;
    int result;
    position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    int value = DAT_1020b430[index].value;
    DAT_10db6ee8[index].mode = 0;
    do
    {
        FUN_10029cf0(index, &a, DAT_1020b430[index].step, &nextStep, &position, &result, &value, &b);
        DAT_1020b430[index].value = value;
        DAT_1020b430[index].result = result;
        DAT_1020b430[index].step = nextStep;
    } while (result == 5);
    if (DAT_1020b430[index].keep == 0 && result != 2)
    {
        DAT_1020b430[index].result = 1;
        DAT_1020b430[index].running = 0;
        DAT_1020b430[index].step = 0;
        DAT_1020b430[index].value = 0;
        DAT_10db6ee8[index].alpha = 0;
        DAT_10db6ee8[index].SetPosition(-5000.0f, -5000.0f, -5000.0f);
    }
    if (DAT_1020b430[index].registered == 0)
    {
        DAT_1032d5b8[DAT_1049f0f0] = &DAT_10db6ee8[index].matrix.translation;
        DAT_1021bbc0[DAT_1049f0f0] = 0;
        DAT_1049f0f0++;
        DAT_1020b430[index].registered = 1;
        DAT_104c036c = 1;
    }
    DAT_10db6ee8[index].alpha = (float)value / 10.0f;
    return 0;
}
