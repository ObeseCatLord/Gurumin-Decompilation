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

// A GUID (16 bytes) as DirectInput names devices and products.
struct DeviceGuid
{
    unsigned long data1;        // 0x00
    unsigned short data2;       // 0x04
    unsigned short data3;       // 0x06
    unsigned char data4[8];     // 0x08
};

// DIDEVICEINSTANCE: what DirectInput reports for each device it enumerates (only the fields used are named).
struct DeviceInstance
{
    unsigned long size;         // 0x00: size of this record
    DeviceGuid instanceGuid;    // 0x04: identifies this device, used to create it
    DeviceGuid productGuid;     // 0x14: identifies the product (tells XInput pads apart)
};

// IDirectInput8; only the slots used so far are named.
struct DirectInputHost
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    // Slot 3: creates the device with the instance GUID `guid` and stores it in `*device`.
    virtual long __stdcall CreateDevice(const DeviceGuid *guid, JoystickDevice **device, void *outer) = 0;
};

// Argument of Joystick_PickInstanceCallback.
struct JoystickPickContext
{
    int *remaining;             // 0x00: pads still to skip before the wanted one (counts down)
    DeviceGuid *chosen;         // 0x04: receives the instance GUID of the pad the enumeration stopped at
};

// The joystick device currently in use (0 = none). EN 0x022280e8.
extern "C" JoystickDevice *g_activeJoystick;

// Tells whether the product `productGuid` is an XInput pad (asked through WMI, EN 0x7aeb00; not recovered).
extern "C" bool Joystick_IsXInputDevice(const DeviceGuid *productGuid);

// Device enumeration callback that stops at the pad whose number is in `context->remaining`: XInput pads are skipped; for
// every other pad the active joystick is released and its instance GUID stored in `context->chosen`, then the count is
// used up. Returns 1 to go on and 0 to stop. EN 0x7ae040 (joystick_enum.cpp).
extern "C" int __stdcall Joystick_PickInstanceCallback(const DeviceInstance *instance, JoystickPickContext *context);

// Device enumeration callback that creates a device for every pad that is not an XInput one and stores it in the next free
// slot of g_inputPad->devices; `index` counts the slots used. Returns 1 to go on and 0 when the slots are full.
// EN 0x7adf90 (joystick_enum.cpp).
extern "C" int __stdcall Joystick_CreateDeviceCallback(const DeviceInstance *instance, int *index);

// DIDEVICEOBJECTINSTANCE: what DirectInput reports for each object (axis or button) of a joystick (only the fields used are named).
struct DeviceObjectInstance
{
    unsigned long size;         // 0x00: size of this record
    DeviceGuid typeGuid;        // 0x04: kind of object
    unsigned long offset;       // 0x14: offset of the object in the data format
    unsigned long type;         // 0x18: object type and instance number
    unsigned long flags;        // 0x1c: DIDOI_* flags
};

// DIDOI_FFACTUATOR: the object can be driven by a force feedback effect.
const unsigned long OBJECT_FLAG_FORCE_ACTUATOR = 1;
// Force feedback axes the game drives at most.
const unsigned int FORCE_ACTUATOR_LIMIT = 2;

// Value a DirectInput enumeration callback returns to go on / to stop.
const int ENUM_CONTINUE = 1;
const int ENUM_STOP = 0;

// Device enumeration callback that counts the pads that are not XInput ones in `*count`. Returns 1. EN 0x7ae010
// (joystick_enum_counts.cpp).
extern "C" int __stdcall Joystick_CountNonXInputCallback(const DeviceInstance *instance, int *count);

// Object enumeration callback that counts force feedback actuators in `*count`, up to FORCE_ACTUATOR_LIMIT. Returns 1.
// EN 0x7ae300 (joystick_enum_counts.cpp).
extern "C" int __stdcall Joystick_CountActuatorCallback(const DeviceObjectInstance *object, unsigned int *count);

// Number of buttons of the game pad the key state is kept for.
const int PAD_BUTTON_COUNT = 12;
// Non-zero when game pad input is on (the config entry USEJP). EN 0x021f37c8.
extern "C" int g_padEnabled;
// Whether each pad button is held, polled every frame (1 = held). EN 0x02200fd8.
extern "C" unsigned char g_padButtons[PAD_BUTTON_COUNT];
// The held buttons as a bit mask, bit n for button n (0 when game pad input is off). EN 0x79fd90 (joystick_buttons.cpp).
extern "C" unsigned int Pad_GetButtonMask(void);

#endif
