// ToggleInterface, PollInterface, ToggleHolder, Record2c: declared here once (first used by src/texcache.cpp).
#ifndef GURUMIN_TEXCACHE_H
#define GURUMIN_TEXCACHE_H

struct ToggleInterface
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall Slot3() = 0;
    virtual long __stdcall Slot4() = 0;
    virtual long __stdcall Slot5() = 0;
    virtual long __stdcall GetState(int *state) = 0;
    virtual long __stdcall SetState(int state) = 0;
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
    virtual long __stdcall Query(int a, int b, int c, int d) = 0;
};
struct ToggleHolder
{
    ToggleInterface *object;
    int m_0004;
    int m_0008;
    PollInterface *poll;      // 0x0c
    void M_10148910();
    void M_10148920();
    int M_10148930();
    void Poll();
};
struct Record2c
{
    int m_0000;
    int m_0004;
    int m_0008;
    int m_000c;
    int m_0010;
    int m_0014;
    int m_0018;
    int m_001c;
    int m_0020;
    int m_0024;
    int m_0028;
    Record2c();
};

#endif
