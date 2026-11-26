// The game's input devices (EN build; JP: Input in decomp/include/dinput_min.h). Only the methods EN code has used so
// far are declared; the data layout is not recovered, so sizeof(Input) is not the real size. Declared once here.
#ifndef GURUMIN_EN_INPUT_DEVICE_H
#define GURUMIN_EN_INPUT_DEVICE_H

struct Input
{
    int Update();                   // EN 0x7a3260 (JP 0x10142790): creates or polls the devices
    int ReadControls(int control);  // EN 0x7a02a0 (JP 0x10143180): stick axes and button mask of control `control`
    int Rumble(float divisor, int milliseconds);   // EN 0x7a8830: vibrates at 1 / `divisor` of full strength when USEVB is on
};

// The game's input devices. EN 0x015e9aa0 (JP DAT_10f08008).
extern "C" Input g_input;

#endif
