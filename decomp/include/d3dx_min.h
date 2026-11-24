// The two D3DX pieces the renderer uses (the VS2003 SDK has no d3dx9.h): ID3DXBuffer and the shader assembler, which
// is linked into the game as a plain function. This is the one declaration of both: include this header.
#ifndef GURUMIN_D3DX_MIN_H
#define GURUMIN_D3DX_MIN_H

#include "com_min.h"

// ID3DXBuffer: the compiled shader or the error text D3DXAssembleShader returns.
struct ID3DXBuffer : IUnknownLike
{
    virtual void *__stdcall GetBufferPointer() = 0;       // slot 3
    virtual unsigned long __stdcall GetBufferSize() = 0;  // slot 4
};

// 0x101c37c6 (D3DXAssembleShader): assembles shader source text into byte code; `errors` receives the messages.
extern "C" long __stdcall FUN_101c37c6(const char *source, unsigned int length, const void *defines,
                                       const void *include, unsigned long flags, ID3DXBuffer **shader,
                                       ID3DXBuffer **errors);

#endif
