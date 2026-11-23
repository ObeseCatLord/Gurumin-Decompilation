// Buffer pool maintenance (see buffer_pool.cpp), a toggle wrapper and frame presentation.

#include "d3d9_min.h"

#include "texcache.h"




#include "render_vars.h"

extern "C" void FUN_101493f0(void);
extern "C" int FUN_1013ed60(void);
extern "C" void FUN_101439e0(void);
extern "C" void FUN_1014b480(void);

// Clears the buffer pool tables and its VRAM counters without releasing anything (start-up).
extern "C" void FUN_10148310(void)
{
    int i;
    for (i = 0; i < 0x200; i++)
        DAT_10f0cd40[i] = 0;
    for (i = 0; i < 0x200; i++)
        DAT_10f1bff0[i] = 0;
    for (i = 0; i < 0x200; i++)
        DAT_10f1fd90[i] = 0;
    DAT_10f140c0 = 0;
    DAT_10f0ff80 = 0;
}

// Releases every pooled surface but keeps the entries reserved (before a device reset).
extern "C" int FUN_10148350(void)
{
    for (int i = 0; i < 0x200; i++)
    {
        if (DAT_10f1bff0[i] != 0)
        {
            DAT_10f1bff0[i]->Release();
            DAT_10f0ff80 -= DAT_10f1de70[i];
        }
        DAT_10f1de70[i] = 0;
        DAT_10f1bff0[i] = 0;
        DAT_10f0cd40[i] = 0;
    }
    return 0;
}

// Releases every pooled surface and frees all entries.
extern "C" int FUN_101483a0(void)
{
    DAT_10f0ff80 = 0;
    for (int i = 0; i < 0x200; i++)
    {
        if (DAT_10f1bff0[i] != 0)
            DAT_10f1bff0[i]->Release();
        DAT_10f1fd90[i] = 0;
        DAT_10f1bff0[i] = 0;
        DAT_10f0cd40[i] = 0;
    }
    return 0;
}

// Ages every pool entry by one frame (for eviction of unused buffers).
extern "C" int FUN_101483f0(void)
{
    for (int i = 0; i < 0x200; i++)
        DAT_10f0cd40[i]++;
    return 0;
}

// Constructor: all fields 0.
Record2c::Record2c()
{
    m_0000 = 0;
    m_0004 = 0;
    m_0008 = 0;
    m_000c = 0;
    m_0010 = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_0024 = 0;
    m_0028 = 0;
}

// Switches the held COM object on (SetState(1)).
void ToggleHolder::M_10148910()
{
    if (object != 0)
        object->SetState(1);
}

// Switches the held COM object off (SetState(0)).
void ToggleHolder::M_10148920()
{
    if (object != 0)
        object->SetState(0);
}

// 1 when the held object reports state 0 (or there is none).
int ToggleHolder::M_10148930()
{
    int state = 0;
    if (object != 0)
        object->GetState(&state);
    return state == 0;
}

// Ends the frame: finishes the 2D overlay (FUN_101493f0), ends the Direct3D scene, presents the back buffer and
// starts the next frame. Returns 0 once the game is quitting.
extern "C" int FUN_1015c960(void)
{
    if (DAT_10f6552c != 0)
        return 0;
    FUN_101493f0();
    FUN_1013ed60();
    DAT_10f1fcf0->Present(0, 0, 0, 0);
    if (DAT_10f65484 != 0)
        FUN_101439e0();
    else
        FUN_1014b480();
    return 1;
}

// Same as FUN_1015c960 without the overlay pass (used for loading screens).
extern "C" int FUN_1015c9b0(void)
{
    if (DAT_10f6552c != 0)
        return 0;
    FUN_1013ed60();
    DAT_10f1fcf0->Present(0, 0, 0, 0);
    FUN_1014b480();
    return 1;
}

// While the held object is on, asks the second object for its status and switches the holder off when that fails
// or returns 0x40003 (finished).
void ToggleHolder::Poll()
{
    if (poll != 0)
    {
        int state = 0;
        if (object != 0)
        {
            object->GetState(&state);
            if (state != 0)
            {
                long result = poll->Query(0, 0, 0, 0);
                if (result < 0 || result == 0x40003)
                    M_10148920();
            }
        }
    }
}
