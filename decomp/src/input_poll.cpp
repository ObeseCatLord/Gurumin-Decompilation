// DirectInput devices (0x10142790, called by the game loop, the loader, the screenshot code and the menus): created
// on first use, then polled every call. Shutdown and force feedback are in input_font.cpp.

#include <windows.h>
#include <objbase.h>
#include <string.h>

struct InputEffect;

// IDirectInputDevice8A (the slots used here).
struct InputDevice
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall GetCapabilities() = 0;
    virtual long __stdcall EnumObjects(int (__stdcall *callback)(const void *, void *), void *context,
                                       unsigned long flags) = 0;
    virtual long __stdcall GetProperty() = 0;
    virtual long __stdcall SetProperty(const GUID &property, const void *header) = 0;
    virtual long __stdcall Acquire() = 0;
    virtual long __stdcall Unacquire() = 0;
    virtual long __stdcall GetDeviceState(unsigned long size, void *state) = 0;
    virtual long __stdcall GetDeviceData() = 0;
    virtual long __stdcall SetDataFormat(const void *format) = 0;
    virtual long __stdcall SetEventNotification() = 0;
    virtual long __stdcall SetCooperativeLevel(HWND window, unsigned long flags) = 0;
    virtual long __stdcall GetObjectInfo() = 0;
    virtual long __stdcall GetDeviceInfo() = 0;
    virtual long __stdcall RunControlPanel() = 0;
    virtual long __stdcall Initialize() = 0;
    virtual long __stdcall CreateEffect(const GUID &effect, const void *description, InputEffect **created,
                                        IUnknown *outer) = 0;
    virtual long __stdcall EnumEffects(int (__stdcall *callback)(const void *, void *), void *context,
                                       unsigned long type) = 0;
    virtual long __stdcall GetEffectInfo() = 0;
    virtual long __stdcall GetForceFeedbackState() = 0;
    virtual long __stdcall SendForceFeedbackCommand() = 0;
    virtual long __stdcall EnumCreatedEffectObjects() = 0;
    virtual long __stdcall Escape() = 0;
    virtual long __stdcall Poll() = 0;
};

// IDirectInput8A (the slots used here).
struct DirectInput
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall CreateDevice(const GUID &device, InputDevice **created, IUnknown *outer) = 0;
    virtual long __stdcall EnumDevices(unsigned long type, int (__stdcall *callback)(const void *, void *),
                                       void *context, unsigned long flags) = 0;
};

// DIEFFECT.
struct EffectDesc
{
    unsigned long size;
    unsigned long flags;
    unsigned long duration;          // microseconds
    unsigned long samplePeriod;
    unsigned long gain;
    unsigned long triggerButton;
    unsigned long triggerRepeatInterval;
    unsigned long axisCount;
    unsigned long *axes;
    long *directions;
    void *envelope;
    unsigned long typeSpecificSize;
    void *typeSpecific;
    unsigned long startDelay;
};

// DIPERIODIC.
struct PeriodicForce
{
    unsigned long magnitude;
    long offset;
    unsigned long phase;
    unsigned long period;            // microseconds
};

// DIPROPDWORD.
struct PropertyDword
{
    unsigned long size;
    unsigned long headerSize;
    unsigned long object;
    unsigned long how;
    unsigned long data;
};

// DIDEVICEINSTANCEA (the start; the rest is names and usage).
struct DeviceInstance
{
    unsigned long size;
    GUID instanceGuid;
    GUID productGuid;
};

// DIDEVICEOBJECTINSTANCEA (the start).
struct DeviceObjectInstance
{
    unsigned long size;
    GUID typeGuid;
    unsigned long offset;
    unsigned long type;
    unsigned long flags;             // DIDOI_FFACTUATOR (1): the object takes force-feedback output
};

// DIEFFECTINFOA (the start).
struct EffectInfo
{
    unsigned long size;
    GUID guid;
};

// DIMOUSESTATE.
struct MouseState
{
    long x;
    long y;
    long z;                          // the wheel
    unsigned char buttons[4];
};

// The input devices (one global object).
struct Input
{
    DirectInput *directInput;
    InputDevice *joystick;
    InputDevice *keyboard;
    InputDevice *mouse;
    GUID effectGuid;                 // 0x10: the periodic effect found on the joystick
    int effectFound;                 // 0x20
    int ready;                       // 0x24: devices set up
    int enumerated;                  // 0x28: DirectInput created and joysticks enumerated
    int forceAxes;                   // 0x2c: force-feedback axes (at most 2 used)
    EffectDesc effect;               // 0x30
    PeriodicForce periodic;          // 0x68
    int Update();
};

extern "C" HINSTANCE DAT_10f140c8;   // the module instance
extern "C" HWND DAT_10f6569c;        // the game window
extern "C" int DAT_10f65570;         // joystick enabled
extern "C" int DAT_10f65574;         // joystick disabled by the settings
extern "C" int DAT_10f65564;         // force feedback enabled
extern "C" int DAT_10f65568;         // the joystick has force feedback
extern "C" int DAT_10f6555c;         // mouse wheel movement accumulated
extern "C" InputEffect *DAT_10f65578;  // the force-feedback effect
extern "C" const GUID DAT_101fccbc;  // IID_IDirectInput8A
extern "C" const GUID DAT_101fcc0c;  // GUID_SysKeyboard
extern "C" const GUID DAT_101fcc1c;  // GUID_SysMouse
extern "C" const char DAT_101fcbf4[];  // c_dfDIJoystick
extern "C" const char DAT_101fc9ec[];  // c_dfDIKeyboard
extern "C" const char DAT_101fc7e4[];  // c_dfDIMouse
extern "C" long __stdcall DirectInput8Create(HINSTANCE instance, unsigned long version, const GUID &iid, void **out,
                                             IUnknown *outer);

// 0x10142670 (Input_CountForceAxes). EnumObjects callback: counts the force-feedback actuator axes. Continues.
extern "C" int __stdcall FUN_10142670(const void *object, void *context)
{
    if ((((const DeviceObjectInstance *)object)->flags & 1) != 0)   // DIDOI_FFACTUATOR
        ((Input *)context)->forceAxes++;
    return 1;
}

// 0x10142690 (Input_TakeEffect). EnumEffects callback: takes the first periodic effect. Stops.
extern "C" int __stdcall FUN_10142690(const void *info, void *context)
{
    Input *input = (Input *)context;
    input->effectGuid = ((const EffectInfo *)info)->guid;
    input->effectFound = 1;
    return 0;
}

// 0x101426d0 (Input_CreateJoystick). EnumDevices callback: creates the joystick from the first game controller that works.
extern "C" int __stdcall FUN_101426d0(const void *instance, void *context)
{
    Input *input = (Input *)context;
    return input->directInput->CreateDevice(((const DeviceInstance *)instance)->instanceGuid, &input->joystick,
                                            0) < 0;
}

// 0x10142790 (Input::Update). The first call creates DirectInput and enumerates the attached game controllers
// (force-feedback ones first when force feedback is on; finding one sets DAT_10f65568) unless the joystick is off.
// Once the game window exists the devices are set up: the joystick exclusive in the foreground (with force
// feedback: auto-centring off, a periodic effect of 10000 over 50 ms periods on up to two axes, 1 s long; a joystick
// where that fails is dropped), the keyboard non-exclusive in the foreground without the Windows key, the mouse
// non-exclusive in the background. After that every call adds the mouse wheel to DAT_10f6555c, acquires the keyboard
// and polls the joystick, reacquiring it while another application has priority. Returns 0 when DirectInput cannot
// be created, else 1.
int Input::Update()
{
    if (ready == 0 && directInput == 0)
    {
        joystick = 0;
        keyboard = 0;
        mouse = 0;
        CoInitialize(0);
        DirectInput8Create(DAT_10f140c8, 0x800, DAT_101fccbc, (void **)&directInput, 0);
        if (directInput == 0)
            return 0;
        if (DAT_10f65570 != 0 && DAT_10f65574 == 0)
        {
            // DI8DEVCLASS_GAMECTRL; DIEDFL_ATTACHEDONLY (| DIEDFL_FORCEFEEDBACK)
            if (DAT_10f65564 != 0 && directInput->EnumDevices(4, FUN_101426d0, this, 0x101) == 0 && joystick != 0)
                DAT_10f65568 = 1;
            else
                directInput->EnumDevices(4, FUN_101426d0, this, 1);
        }
        enumerated = 1;
    }
    if (DAT_10f6569c != 0 && enumerated != 0 && ready == 0)
    {
        if (joystick != 0)
        {
            joystick->SetCooperativeLevel(DAT_10f6569c, 5);         // DISCL_EXCLUSIVE | DISCL_FOREGROUND
            joystick->SetDataFormat(DAT_101fcbf4);
            if (DAT_10f65568 != 0)
            {
                PropertyDword autoCentre;
                autoCentre.size = sizeof(PropertyDword);
                autoCentre.headerSize = 0x10;
                autoCentre.object = 0;
                autoCentre.how = 0;                                  // DIPH_DEVICE
                autoCentre.data = 0;                                 // DIPROPAUTOCENTER_OFF
                if (joystick->SetProperty(*(const GUID *)9, &autoCentre) < 0)   // DIPROP_AUTOCENTER
                {
                    joystick->Release();
                    joystick = 0;
                }
                else
                {
                    unsigned long axes[2];
                    long directions[2];
                    forceAxes = 0;
                    joystick->EnumObjects(FUN_10142670, this, 3);  // DIDFT_AXIS
                    if (forceAxes > 2)
                        forceAxes = 2;
                    memset(&periodic, 0, sizeof(PeriodicForce));
                    memset(&effect, 0, sizeof(EffectDesc));
                    axes[0] = 0;                                     // DIJOFS_X
                    axes[1] = 4;                                     // DIJOFS_Y
                    directions[0] = 0;
                    directions[1] = 1;
                    effect.size = sizeof(EffectDesc);
                    effect.flags = 0x12;                             // DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS
                    effect.duration = 500000;
                    effect.samplePeriod = 0;
                    effect.gain = 10000;
                    effect.triggerButton = 0xffffffff;               // DIEB_NOTRIGGER
                    effect.triggerRepeatInterval = 0;
                    effect.axisCount = forceAxes;
                    effect.axes = axes;
                    effect.directions = directions;
                    effect.typeSpecificSize = sizeof(PeriodicForce);
                    effect.typeSpecific = &periodic;
                    effectFound = 0;
                    if (joystick->EnumEffects(FUN_10142690, this, 3) < 0 || effectFound == 0)   // DIEFT_PERIODIC
                    {
                        joystick->Release();
                        joystick = 0;
                    }
                    else
                    {
                        periodic.period = 50000;
                        periodic.magnitude = 10000;
                        effect.duration = 1000000;
                        if (joystick->CreateEffect(effectGuid, &effect, &DAT_10f65578, 0) < 0)
                        {
                            joystick->Release();
                            joystick = 0;
                        }
                    }
                }
            }
        }
        directInput->CreateDevice(DAT_101fcc0c, &keyboard, 0);
        if (keyboard != 0)
        {
            keyboard->SetCooperativeLevel(DAT_10f6569c, 0x16);       // non-exclusive, foreground, no Windows key
            keyboard->SetDataFormat(DAT_101fc9ec);
        }
        directInput->CreateDevice(DAT_101fcc1c, &mouse, 0);
        if (mouse != 0)
        {
            DAT_10f6555c = 0;
            mouse->SetCooperativeLevel(DAT_10f6569c, 10);            // DISCL_NONEXCLUSIVE | DISCL_BACKGROUND
            mouse->SetDataFormat(DAT_101fc7e4);
        }
        ready = 1;
    }
    if (ready != 0)
    {
        if (mouse != 0)
        {
            MouseState state;
            state.z = 0;
            if (mouse->GetDeviceState(sizeof(MouseState), &state) == 0)
                DAT_10f6555c += (short)state.z;
        }
        if (keyboard != 0)
            keyboard->Acquire();
        if (joystick != 0 && joystick->Poll() != 0)
        {
            long result = joystick->Acquire();
            while (result == (long)0x8007001e)                         // DIERR_OTHERAPPHASPRIO
                result = joystick->Acquire();
        }
    }
    return 1;
}
