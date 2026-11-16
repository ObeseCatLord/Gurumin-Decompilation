#include <windows.h>
#include <imm.h>
#include <stdlib.h>

struct StateArrays
{
    char pad_0000[0x20];
    int m_0020[8];
    int m_0040[8];
    int M_1015caa0(int value, int index);
    int M_1015cac0(int value, int index);
};

struct ImeContext
{
    HWND window;
    HIMC context;
    int pad_0008;
    void *buffer;

    ImeContext();
    int Release();
    int Attach(HWND target);
    int IsOpen();
    int SetOpen(BOOL open);
    int GetConversion(DWORD *conversion, DWORD *sentence);
    int SetConversion(const DWORD *conversion, const DWORD *sentence);
    LONG GetCompositionValue();
};

int StateArrays::M_1015caa0(int value, int index)
{
    m_0020[index] = value;
    return 1;
}

int StateArrays::M_1015cac0(int value, int index)
{
    m_0040[index] = value;
    return 1;
}

ImeContext::ImeContext()
{
    window = 0;
    context = 0;
    buffer = 0;
}

int ImeContext::Release()
{
    if (buffer != 0)
    {
        free(buffer);
        buffer = 0;
    }
    if (context != 0)
    {
        ImmReleaseContext(window, context);
        context = 0;
        window = 0;
    }
    return 1;
}

int ImeContext::Attach(HWND target)
{
    Release();
    if (target != 0)
    {
        window = target;
        context = ImmGetContext(target);
    }
    return 1;
}

int ImeContext::IsOpen()
{
    return ImmGetOpenStatus(context) != 0;
}

int ImeContext::SetOpen(BOOL open)
{
    return ImmSetOpenStatus(context, open) != 0;
}

int ImeContext::GetConversion(DWORD *conversion, DWORD *sentence)
{
    return ImmGetConversionStatus(context, conversion, sentence) != 0;
}

int ImeContext::SetConversion(const DWORD *conversion, const DWORD *sentence)
{
    return ImmSetConversionStatus(context, *conversion, *sentence) != 0;
}

LONG ImeContext::GetCompositionValue()
{
    LONG value[2];
    if (ImmGetCompositionStringA(context, 0x20, value, sizeof(value)) == 0)
        return 0;
    return value[1];
}
