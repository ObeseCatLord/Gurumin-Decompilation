// Reading the game's buttons from a DirectInput joystick state: each game button is mapped (DAT_10f1bef0) to an
// axis direction or a joystick button, with a default layout for map 0.

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

#include "ui_vars.h"

// An axis counts as pushed towards its low or high end past the dead zone around the centre (128 in 1/256 units).
// 0x10142b20 (Joystick_IsButtonDown). Whether game button `button` is down in `state`: maps 1-4 are the y and x
// directions, 5-8 and 21-24 the z, rz, rx and ry directions (when the stick has that axis; otherwise the next
// mapping is tried), 9-20 joystick buttons 0-11. Map 0 uses the default layout of each game button.
extern "C" int __stdcall FUN_10142b20(int button, const JoyState *state)
{
    switch (DAT_10f1bef0[button])
    {
    case 0:
        switch (button)
        {
        case 0:
            return state->buttons[6];
        case 1:
            return state->x / 256 < 0x80 - DAT_10203408;
        case 2:
            return state->y / 256 < 0x80 - DAT_10203408;
        case 3:
            return state->buttons[0];
        case 4:
            return state->buttons[2];
        case 5:
            return state->buttons[7];
        case 6:
            return state->x / 256 > DAT_10203408 + 0x80;
        case 7:
            return state->y / 256 > DAT_10203408 + 0x80;
        case 8:
            return state->buttons[1];
        case 9:
            return state->buttons[3];
        case 10:
            return state->buttons[8];
        case 11:
            return state->buttons[9];
        case 12:
            return state->buttons[10];
        case 13:
            return state->buttons[11];
        case 14:
            return state->buttons[10];
        }
        break;
    case 1:
        return state->y / 256 < 0x80 - DAT_10203408;
    case 2:
        return state->y / 256 > DAT_10203408 + 0x80;
    case 3:
        return state->x / 256 < 0x80 - DAT_10203408;
    case 4:
        return state->x / 256 > DAT_10203408 + 0x80;
    case 5:
        if (DAT_10f6556c & 1)
            return state->z / 256 < 0x80 - DAT_10203408;
    case 6:
        if (DAT_10f6556c & 1)
            return state->z / 256 > DAT_10203408 + 0x80;
    case 7:
        if (DAT_10f6556c & 2)
            return state->rz / 256 < 0x80 - DAT_10203408;
    case 8:
        if (DAT_10f6556c & 2)
            return state->rz / 256 > DAT_10203408 + 0x80;
    case 9:
        return state->buttons[0];
    case 10:
        return state->buttons[1];
    case 11:
        return state->buttons[2];
    case 12:
        return state->buttons[3];
    case 13:
        return state->buttons[4];
    case 14:
        return state->buttons[5];
    case 15:
        return state->buttons[6];
    case 16:
        return state->buttons[7];
    case 17:
        return state->buttons[8];
    case 18:
        return state->buttons[9];
    case 19:
        return state->buttons[10];
    case 20:
        return state->buttons[11];
    case 21:
        if (DAT_10f6556c & 4)
            return state->rx / 256 < 0x80 - DAT_10203408;
    case 22:
        if (DAT_10f6556c & 4)
            return state->rx / 256 > DAT_10203408 + 0x80;
    case 23:
        if (DAT_10f6556c & 8)
            return state->ry / 256 < 0x80 - DAT_10203408;
    case 24:
        if (DAT_10f6556c & 8)
            return state->ry / 256 > DAT_10203408 + 0x80;
    }
    return 0;
}


// 0x10142dd0 (Keyboard_IsButtonDown). Whether game button `button` is down in the DirectInput key array `keys`
// (non-zero = down, bit 7 of the key). Map 0 uses the default layout of each game button; maps 1-0x21 are fixed
// keys (arrows, Enter, Space, letters, F1-F6, the numeric keypad), 5 and 6 being left Shift or left Ctrl, each also
// accepting right Ctrl.
extern "C" int __stdcall FUN_10142dd0(int button, const unsigned char *keys)
{
    switch (DAT_10f60c30[button])
    {
    case 0:
        switch (button)
        {
        case 0:
            return keys[0x3b] & 0x80;
        case 1:
            return keys[0xcb] & 0x80;
        case 2:
            return keys[0xc8] & 0x80;
        case 3:
            return keys[0x1e] & 0x80;
        case 4:
            return keys[0x2e] & 0x80;
        case 5:
            return keys[0x3c] & 0x80;
        case 6:
            return keys[0xcd] & 0x80;
        case 7:
            return keys[0xd0] & 0x80;
        case 8:
            return keys[0x30] & 0x80;
        case 9:
            return keys[0x20] & 0x80;
        case 10:
            return keys[0x3d] & 0x80;
        case 11:
            return keys[0x3e] & 0x80;
        case 12:
            return keys[0x3f] & 0x80;
        case 13:
            return keys[0x40] & 0x80;
        case 14:
            return keys[0x3f] & 0x80;
        }
        break;
    case 1:
        return keys[0xc8] & 0x80;
    case 2:
        return keys[0xd0] & 0x80;
    case 3:
        return keys[0xcb] & 0x80;
    case 4:
        return keys[0xcd] & 0x80;
    case 5:
        return (keys[0x2a] & 0x80) || (keys[0x9d] & 0x80);
    case 6:
        return (keys[0x1d] & 0x80) || (keys[0x9d] & 0x80);
    case 7:
        return keys[0x1c] & 0x80;
    case 8:
        return keys[0x39] & 0x80;
    case 9:
        return keys[0x01] & 0x80;
    case 10:
        return keys[0x1e] & 0x80;
    case 11:
        return keys[0x30] & 0x80;
    case 12:
        return keys[0x2e] & 0x80;
    case 13:
        return keys[0x20] & 0x80;
    case 14:
        return keys[0x26] & 0x80;
    case 15:
        return keys[0x13] & 0x80;
    case 16:
        return keys[0x2d] & 0x80;
    case 17:
        return keys[0x2c] & 0x80;
    case 18:
        return keys[0x3b] & 0x80;
    case 19:
        return keys[0x3c] & 0x80;
    case 20:
        return keys[0x3d] & 0x80;
    case 21:
        return keys[0x3e] & 0x80;
    case 22:
        return keys[0x3f] & 0x80;
    case 23:
        return keys[0x40] & 0x80;
    case 24:
        return keys[0x52] & 0x80;
    case 25:
        return keys[0x4f] & 0x80;
    case 26:
        return keys[0x50] & 0x80;
    case 27:
        return keys[0x51] & 0x80;
    case 28:
        return keys[0x4b] & 0x80;
    case 29:
        return keys[0x4c] & 0x80;
    case 30:
        return keys[0x4d] & 0x80;
    case 31:
        return keys[0x47] & 0x80;
    case 32:
        return keys[0x48] & 0x80;
    case 33:
        return keys[0x49] & 0x80;
    }
    return 0;
}
