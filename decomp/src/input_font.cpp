// DirectInput shutdown and Windows font cleanup.

#include <windows.h>

struct ComObject
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
};

struct InputDevice
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall GetCapabilities() = 0;
    virtual long __stdcall EnumObjects() = 0;
    virtual long __stdcall GetProperty() = 0;
    virtual long __stdcall SetProperty() = 0;
    virtual long __stdcall Acquire() = 0;
    virtual long __stdcall Unacquire() = 0;
};

struct Input
{
    ComObject *directInput;
    InputDevice *keyboard;
    InputDevice *mouse;
    InputDevice *joystick;
    char pad_0010[0x14];
    int m_0024;
    int m_0028;
    int Shutdown();
};

struct FontSet
{
    HFONT fonts[3];
    void *glyphs[0x20];
    char pad_008c[0x2300];
    void *buffer;
    void *buffer2;
    int Release();
};

extern "C" ComObject *DAT_10f65578;
extern "C" void (*DAT_11b1ba40)(void *);

// Unacquires and releases the keyboard, mouse and joystick devices and the DirectInput object.
int Input::Shutdown()
{
    if (DAT_10f65578 != 0)
    {
        DAT_10f65578->Release();
        DAT_10f65578 = 0;
    }
    if (keyboard != 0)
    {
        keyboard->Unacquire();
        keyboard->Release();
    }
    if (mouse != 0)
    {
        mouse->Unacquire();
        mouse->Release();
    }
    if (joystick != 0)
    {
        joystick->Unacquire();
        joystick->Release();
    }
    if (directInput != 0)
        directInput->Release();
    directInput = 0;
    keyboard = 0;
    mouse = 0;
    joystick = 0;
    m_0024 = 0;
    m_0028 = 0;
    return 0;
}

// Frees the font buffers, deletes the three GDI fonts and frees the 32 cached glyph bitmaps.
int FontSet::Release()
{
    if (buffer != 0)
        DAT_11b1ba40(buffer);
    buffer = 0;
    if (buffer2 != 0)
        DAT_11b1ba40(buffer2);
    buffer2 = 0;
    if (fonts[0] != 0)
        DeleteObject(fonts[0]);
    if (fonts[1] != 0)
        DeleteObject(fonts[1]);
    if (fonts[2] != 0)
        DeleteObject(fonts[2]);
    for (int i = 0; i < 0x20; i++)
    {
        if (glyphs[i] != 0)
            DAT_11b1ba40(glyphs[i]);
        glyphs[i] = 0;
    }
    return 1;
}
