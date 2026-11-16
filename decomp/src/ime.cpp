#include <windows.h>
#include <imm.h>
#include <stdlib.h>
#include <string.h>

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
    CANDIDATELIST *buffer;

    ImeContext();
    int Release();
    int Attach(HWND target);
    int IsOpen();
    int SetOpen(BOOL open);
    int GetConversion(DWORD *conversion, DWORD *sentence);
    int SetConversion(const DWORD *conversion, const DWORD *sentence);
    LONG GetCompositionValue();
    LONG M_1015d400();
    DWORD GetSelection();
    const char *GetCandidate(int index);
    DWORD GetPageStart();
    DWORD GetCount();
    DWORD GetPageSize();
    int GetResultString(char *out);
    int GetCompositionString(char *out);
    DWORD LoadCandidates();
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

LONG ImeContext::M_1015d400()
{
    ImmGetCompositionStringA(context, 0x100, 0, 0);
    return ImmGetCompositionStringA(context, 0x80, 0, 0);
}

DWORD ImeContext::GetSelection()
{
    if (buffer == 0)
        return 0;
    return buffer->dwSelection;
}

const char *ImeContext::GetCandidate(int index)
{
    CANDIDATELIST *list = buffer;
    if (list == 0)
        return 0;
    return (const char *)list + list->dwOffset[index];
}

DWORD ImeContext::GetPageStart()
{
    if (buffer == 0)
        return 0;
    return buffer->dwPageStart;
}

DWORD ImeContext::GetCount()
{
    if (buffer == 0)
        return 0;
    return buffer->dwCount;
}

DWORD ImeContext::GetPageSize()
{
    if (buffer == 0)
        return 0;
    return buffer->dwPageSize;
}

extern "C" int DAT_10f66bbc;

extern "C" int FUN_1015d500(int value)
{
    DAT_10f66bbc = value;
    return 1;
}
extern "C" ImeContext DAT_10f68124;

extern "C" int FUN_1015d510(void)
{
    return DAT_10f68124.IsOpen();
}

int ImeContext::GetResultString(char *out)
{
    *out = 0;
    LONG length = ImmGetCompositionStringA(context, GCS_RESULTSTR, 0, 0);
    if (length == 0)
        return 0;
    char *text = (char *)operator new(length + 1);
    ImmGetCompositionStringA(context, GCS_RESULTSTR, text, length);
    text[length] = 0;
    strcpy(out, text);
    free(text);
    return 1;
}

int ImeContext::GetCompositionString(char *out)
{
    *out = 0;
    LONG length = ImmGetCompositionStringA(context, GCS_COMPSTR, 0, 0);
    if (length == 0)
        return 0;
    char *text = (char *)operator new(length + 1);
    ImmGetCompositionStringA(context, GCS_COMPSTR, text, length);
    text[length] = 0;
    strcpy(out, text);
    free(text);
    return 1;
}

DWORD ImeContext::LoadCandidates()
{
    if (buffer != 0)
    {
        free(buffer);
        buffer = 0;
    }
    DWORD size = ImmGetCandidateListA(context, 0, 0, 0);
    if (size == 0)
        return 0;
    buffer = (CANDIDATELIST *)operator new(size);
    ImmGetCandidateListA(context, 0, buffer, size);
    return buffer->dwCount;
}