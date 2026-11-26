// DirectInput joystick device of the EN build (the controller polling at EN 0x7ae5c0 and the setup helpers around it).
// Only the interface slots used so far are named. This is the one declaration of these types; include it, never
// redeclare them.
#ifndef GURUMIN_EN_JOYSTICK_DEVICE_H
#define GURUMIN_EN_JOYSTICK_DEVICE_H

// Axes a joystick can report (X, Y, Z, rotation X, Y, Z): the first six flags of JoystickAxes::present.
const int JOYSTICK_AXIS_COUNT = 6;
// DISCL_EXCLUSIVE | DISCL_FOREGROUND: the game owns the device while its window is in front.
const unsigned long JOYSTICK_COOPERATIVE_FLAGS = 5;
// DIDFT_AXIS | DIDFT_BUTTON: the objects the enumeration visits.
const unsigned long JOYSTICK_OBJECT_TYPES = 3;

// Which axes a device has, set by the enumeration callback (EN 0x7ae0a0, not recovered).
struct JoystickAxes
{
    unsigned char present[JOYSTICK_AXIS_COUNT];   // 0x00: 1 when the device has the axis
};

// IDirectInputDevice8 (a joystick); only the slots used so far are named.
struct JoystickDevice
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall GetCapabilities() = 0;
    // Slot 4: calls `callback` for every object of the types in `flags`.
    virtual long __stdcall EnumObjects(int (__stdcall *callback)(const void *object, void *context), void *context,
                                       unsigned long flags) = 0;
    virtual long __stdcall GetProperty() = 0;
    virtual long __stdcall SetProperty() = 0;
    virtual long __stdcall Acquire() = 0;
    virtual long __stdcall Unacquire() = 0;
    virtual long __stdcall GetDeviceState() = 0;
    virtual long __stdcall GetDeviceData() = 0;
    // Slot 11: selects the data format the device state is read in.
    virtual long __stdcall SetDataFormat(const void *format) = 0;
    virtual long __stdcall SetEventNotification() = 0;
    // Slot 13: sets how the device is shared with other programs.
    virtual long __stdcall SetCooperativeLevel(void *window, unsigned long flags) = 0;
};

// Argument of the enumeration callback.
struct JoystickEnumContext
{
    JoystickDevice *device;   // 0x00: the device being enumerated
    JoystickAxes *axes;       // 0x04: receives the axes found
};

// The DIDATAFORMAT the joystick state is read in. EN 0x008c2f1c.
extern "C" const unsigned char g_joystickDataFormat[];
// Enumeration callback that records which axes the device has. EN 0x7ae0a0 (not recovered).
extern "C" int __stdcall Joystick_EnumObjectCallback(const void *object, void *context);

// Selects the data format and sharing of `device`, clears `axes` and enumerates the device's axes and buttons into it.
// EN 0x7adf20 (joystick_setup.cpp). Called by the controller polling at EN 0x7ae5c0.
extern "C" void Joystick_Configure(JoystickDevice *device, JoystickAxes *axes, void *window);

#endif
