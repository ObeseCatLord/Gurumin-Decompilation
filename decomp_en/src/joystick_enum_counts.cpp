// DirectInput enumeration callbacks that only count what they see (EN build, EN-only: no JP twin found).
#include "joystick_device.h"

// Counts the pads that are not XInput ones: adds one to `*count` for each. Passed to EnumDevices by the controller setup.
extern "C" int __stdcall Joystick_CountNonXInputCallback(const DeviceInstance *instance, int *count)
{
    if (!Joystick_IsXInputDevice(&instance->productGuid))
        ++*count;
    return ENUM_CONTINUE;
}

// Counts the force feedback actuators of a pad, up to FORCE_ACTUATOR_LIMIT. Passed to EnumObjects by the controller setup.
extern "C" int __stdcall Joystick_CountActuatorCallback(const DeviceObjectInstance *object, unsigned int *count)
{
    if (*count < FORCE_ACTUATOR_LIMIT && (object->flags & OBJECT_FLAG_FORCE_ACTUATOR) != 0)
        ++*count;
    return ENUM_CONTINUE;
}
