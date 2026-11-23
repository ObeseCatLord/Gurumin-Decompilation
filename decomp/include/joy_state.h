// JoyState: declared here once (first used by src/joystick_buttons.cpp).
#ifndef GURUMIN_JOY_STATE_H
#define GURUMIN_JOY_STATE_H

// The DirectInput joystick state (DIJOYSTATE): axes from 0 to 65535, centre 32768.
struct JoyState
{
    int x;                        // lX
    int y;                        // lY
    int z;                        // lZ
    int rx;                       // lRx
    int ry;                       // lRy
    int rz;                       // lRz
    int sliders[2];
    unsigned int pov[4];
    unsigned char buttons[32];    // 0x30
};

#endif
