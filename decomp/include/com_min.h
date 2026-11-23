// The COM base interface (IUnknown) for the Direct3D, DirectDraw and DirectShow views in d3d9_min.h, ddraw_min.h and
// dshow_stream.cpp (this compiler's SDK has no d3d9.h, ddraw.h or amstream.h). This is the one declaration of
// IUnknownLike: include this header, never redeclare it.
#ifndef GURUMIN_COM_MIN_H
#define GURUMIN_COM_MIN_H

struct _GUID;  // GUID in windows.h

struct IUnknownLike
{
    virtual long __stdcall QueryInterface(const struct _GUID &iid, void **object) = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
};

#endif
