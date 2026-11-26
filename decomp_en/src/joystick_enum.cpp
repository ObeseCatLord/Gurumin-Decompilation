// DirectInput device enumeration callbacks for the game pads (EN build, EN-only: no JP twin found).
#include "joystick_device.h"
#include "small_b11.h"

// Value a DirectInput enumeration callback returns to go on / to stop.
const int ENUM_CONTINUE = 1;
const int ENUM_STOP = 0;

// Stops at the pad that `context->remaining` counts down to: every XInput pad is skipped; for any other one the joystick
// in use is released, its instance GUID is stored in `context->chosen` and the count is used up one. Passed to
// EnumDevices by the controller polling.
extern "C" int __stdcall Joystick_PickInstanceCallback(const DeviceInstance *instance, JoystickPickContext *context)
{
    if (Joystick_IsXInputDevice(&instance->productGuid))
        return ENUM_CONTINUE;
    if (g_activeJoystick != 0)
    {
        g_activeJoystick->Release();
        g_activeJoystick = 0;
    }
    *context->chosen = instance->instanceGuid;
    if (*context->remaining == 0)
        return ENUM_STOP;
    --*context->remaining;
    return ENUM_CONTINUE;
}

// Creates a device for every pad that is not an XInput one and stores it in slot `*index` of the controller record; a
// device that cannot be created leaves its slot 0. Stops once every slot is used. Passed to EnumDevices by the
// controller polling.
extern "C" int __stdcall Joystick_CreateDeviceCallback(const DeviceInstance *instance, int *index)
{
    if (Joystick_IsXInputDevice(&instance->productGuid))
        return ENUM_CONTINUE;
    if (*index == g_inputPad->deviceCount)
        return ENUM_STOP;
    if (g_inputPad->directInput->CreateDevice(&instance->instanceGuid, &g_inputPad->devices[*index], 0) < 0)
        g_inputPad->devices[*index] = 0;
    ++*index;
    return ENUM_CONTINUE;
}
