// ToggleHolder and its COM-style interfaces in the EN build (JP: decomp/include/texcache.h).
// This is the one declaration of these types in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_TEXCACHE_H
#define GURUMIN_EN_TEXCACHE_H

// First COM object of a ToggleHolder (slots after IUnknown; only the state accessors are used).
struct ToggleInterface
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall Slot3() = 0;
    virtual long __stdcall Slot4() = 0;
    virtual long __stdcall Slot5() = 0;
    virtual long __stdcall GetState(int *state) = 0;   // vtable +0x18
    virtual long __stdcall SetState(int state) = 0;    // vtable +0x1c
};

// Second COM object of a ToggleHolder; only its slot 6 (a four-argument query) is used.
struct PollInterface
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall Slot3() = 0;
    virtual long __stdcall Slot4() = 0;
    virtual long __stdcall Slot5() = 0;
    virtual long __stdcall Query(int a, int b, int c, int d) = 0;   // vtable +0x18
};

// Status returned by PollInterface::Query when playback has finished.
const long QUERY_FINISHED = 0x40003;

struct ToggleHolder
{
    ToggleInterface *object;   // 0x00: the object that is switched on or off
    int m_0004;                // 0x04: not yet named
    int m_0008;                // 0x08: not yet named
    PollInterface *poll;       // 0x0c: object asked whether playback has finished

    void SwitchOff();          // sets the held object's state to 0 (inline in Poll)
    int IsOff();               // EN 0x7a1f50 (JP 0x10148930)
    void Poll();               // EN 0x7a82f0 (JP 0x10148960)
};

#endif
