// Setting up a DirectInput joystick (EN build, EN-only: no JP twin found).
#include <string.h>
#include "joystick_device.h"

// Clears the axis flags, selects the joystick data format, takes the device exclusively while `window` is in front and
// runs the enumeration that fills `axes`. Called when a controller is opened by the polling at EN 0x7ae5c0.
extern "C" void Joystick_Configure(JoystickDevice *device, JoystickAxes *axes, void *window)
{
    JoystickEnumContext context;
    context.axes = axes;
    memset(axes->present, 0, sizeof(axes->present));
    context.device = device;
    device->SetDataFormat(g_joystickDataFormat);
    device->SetCooperativeLevel(window, JOYSTICK_COOPERATIVE_FLAGS);
    device->EnumObjects(Joystick_EnumObjectCallback, &context, JOYSTICK_OBJECT_TYPES);
}
