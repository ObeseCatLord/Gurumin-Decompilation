// Reading the player's controls from the joystick (DirectInput, or the winmm joystick when DirectInput has none) and
// the keyboard, merged by the game's button mappings (joystick_buttons.cpp).

#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <string.h>

#include "dinput_min.h"
#include "joy_state.h"
#include "ui_vars.h"

#define AXIS_CENTRE 0x8000
#define KEYBOARD_STATE_SIZE 256
#define AXIS_LIMIT 0x7f

#define CONTROL_BUTTONS 0
#define CONTROL_AXIS_X 1
#define CONTROL_AXIS_Y 2

// Game button numbers: 1/6 are the x direction (low/high), 2/7 the y direction.
#define BUTTON_X_LOW 1
#define BUTTON_X_HIGH 6
#define BUTTON_Y_LOW 2
#define BUTTON_Y_HIGH 7

// 0x10143180 (Input::ReadControls). Reads the joystick (DirectInput; without one, winmm joystick 0 when the
// joystick is on) and the keyboard. `control` 0 returns the button mask (each bit is set when the joystick mapping
// or the key of one game button is down). `control` 1 and 2 return the x or y axis from -128 to 127: a key or
// button mapped to the low or high end gives the extreme; otherwise the stick (inside the dead zone 0) scaled by
// the sensitivity, when the two direction buttons are mapped to axes (else the buttons alone decide).
int Input::ReadControls(int control)
{
    JoyState joy;
    unsigned char keys[KEYBOARD_STATE_SIZE];
    int joystickRead = 0;

    joy.x = AXIS_CENTRE;
    joy.y = AXIS_CENTRE;
    joy.z = AXIS_CENTRE;
    joy.rx = AXIS_CENTRE;
    joy.ry = AXIS_CENTRE;
    joy.rz = AXIS_CENTRE;
    memset(keys, 0, KEYBOARD_STATE_SIZE);
    joy.buttons[0] = 0;
    joy.buttons[1] = 0;
    joy.buttons[2] = 0;
    joy.buttons[3] = 0;
    joy.buttons[4] = 0;
    joy.buttons[5] = 0;
    joy.buttons[6] = 0;
    joy.buttons[7] = 0;
    joy.buttons[8] = 0;
    joy.buttons[9] = 0;
    joy.buttons[10] = 0;
    joy.buttons[11] = 0;
    if (joystick != 0)
    {
        if (joystick->GetDeviceState(sizeof(JoyState), &joy) != 0)
        {
            joy.x = AXIS_CENTRE;
            joy.y = AXIS_CENTRE;
            joy.z = AXIS_CENTRE;
            joy.rx = AXIS_CENTRE;
            joy.ry = AXIS_CENTRE;
            joy.rz = AXIS_CENTRE;
            joy.buttons[0] = 0;
            joy.buttons[1] = 0;
            joy.buttons[2] = 0;
            joy.buttons[3] = 0;
            joy.buttons[4] = 0;
            joy.buttons[5] = 0;
            joy.buttons[6] = 0;
            joy.buttons[7] = 0;
            joy.buttons[8] = 0;
            joy.buttons[9] = 0;
            joy.buttons[10] = 0;
            joy.buttons[11] = 0;
        }
    }
    else if (DAT_10f65570 != 0)
    {
        JOYINFOEX info;
        info.dwFlags = 0xff;
        info.dwSize = sizeof(JOYINFOEX);
        if (joyGetPosEx(0, &info) == 0)
        {
            joy.x = info.dwXpos;
            joy.y = info.dwYpos;
            joy.z = info.dwZpos;
            joy.rx = info.dwRpos;
            joy.ry = info.dwUpos;
            joy.rz = info.dwVpos;
            joy.buttons[0] = (unsigned char)(info.dwButtons >> 0) & 1;
            joy.buttons[1] = (unsigned char)(info.dwButtons >> 1) & 1;
            joy.buttons[2] = (unsigned char)(info.dwButtons >> 2) & 1;
            joy.buttons[3] = (unsigned char)(info.dwButtons >> 3) & 1;
            joy.buttons[4] = (unsigned char)(info.dwButtons >> 4) & 1;
            joy.buttons[5] = (unsigned char)(info.dwButtons >> 5) & 1;
            joy.buttons[6] = (unsigned char)(info.dwButtons >> 6) & 1;
            joy.buttons[7] = (unsigned char)(info.dwButtons >> 7) & 1;
            joy.buttons[8] = (unsigned char)(info.dwButtons >> 8) & 1;
            joystickRead = 1;
            joy.buttons[9] = (unsigned char)(info.dwButtons >> 9) & 1;
            joy.buttons[10] = (unsigned char)(info.dwButtons >> 10) & 1;
            joy.buttons[11] = (unsigned char)(info.dwButtons >> 11) & 1;
        }
    }
    if (keyboard != 0 && keyboard->GetDeviceState(KEYBOARD_STATE_SIZE, keys) != 0)
        memset(keys, 0, KEYBOARD_STATE_SIZE);

    int result = 0;
    if (control == CONTROL_AXIS_X)
    {
        if (IsKeyboardButtonDown(BUTTON_X_LOW, keys) != 0)
            return -AXIS_LIMIT - 1;
        if (IsKeyboardButtonDown(BUTTON_X_HIGH, keys) != 0)
            return AXIS_LIMIT;
        if (((joystickRead == 0 && joystick == 0) || (DAT_10f1bef0[BUTTON_X_LOW] != 3 && DAT_10f1bef0[BUTTON_X_LOW] != 4)) ||
            (DAT_10f1bef0[BUTTON_X_HIGH] != 3 && DAT_10f1bef0[BUTTON_X_HIGH] != 4))
        {
            if (IsJoystickButtonDown(BUTTON_X_LOW, &joy) != 0)
                return -AXIS_LIMIT - 1;
            return IsJoystickButtonDown(BUTTON_X_HIGH, &joy) != 0 ? AXIS_LIMIT : 0;
        }
    }
    else if (control == CONTROL_AXIS_Y)
    {
        if (IsKeyboardButtonDown(BUTTON_Y_LOW, keys) != 0)
            return -AXIS_LIMIT - 1;
        if (IsKeyboardButtonDown(BUTTON_Y_HIGH, keys) != 0)
            return AXIS_LIMIT;
        if (((joystickRead == 0 && joystick == 0) || (DAT_10f1bef0[BUTTON_Y_LOW] != 1 && DAT_10f1bef0[BUTTON_Y_LOW] != 2)) ||
            (DAT_10f1bef0[BUTTON_Y_HIGH] != 1 && DAT_10f1bef0[BUTTON_Y_HIGH] != 2))
        {
            if (IsJoystickButtonDown(BUTTON_Y_LOW, &joy) != 0)
                return -AXIS_LIMIT - 1;
            return IsJoystickButtonDown(BUTTON_Y_HIGH, &joy) != 0 ? AXIS_LIMIT : 0;
        }
        joy.x = joy.y;
    }
    else
    {
        if (control == CONTROL_BUTTONS)
        {
            if (IsJoystickButtonDown(3, &joy) != 0 || IsKeyboardButtonDown(3, keys) != 0)
                result = 1;
            if (IsJoystickButtonDown(8, &joy) != 0 || IsKeyboardButtonDown(8, keys) != 0)
                result |= 8;
            if (IsJoystickButtonDown(4, &joy) != 0 || IsKeyboardButtonDown(4, keys) != 0)
                result |= 4;
            if (IsJoystickButtonDown(9, &joy) != 0 || IsKeyboardButtonDown(9, keys) != 0)
                result |= 2;
            if (IsJoystickButtonDown(0, &joy) != 0 || IsKeyboardButtonDown(0, keys) != 0)
                result |= 0x10;
            if (IsJoystickButtonDown(5, &joy) != 0 || IsKeyboardButtonDown(5, keys) != 0)
                result |= 0x20;
            if (IsJoystickButtonDown(10, &joy) != 0 || IsKeyboardButtonDown(10, keys) != 0)
                result |= 0x40;
            if (IsJoystickButtonDown(11, &joy) != 0 || IsKeyboardButtonDown(11, keys) != 0)
                result |= 0x80;
            if (IsJoystickButtonDown(12, &joy) != 0 || IsKeyboardButtonDown(12, keys) != 0)
                result |= 0x200;
            if (IsJoystickButtonDown(13, &joy) != 0 || IsKeyboardButtonDown(13, keys) != 0)
                result |= 0x100;
            if (IsJoystickButtonDown(14, &joy) != 0 || IsKeyboardButtonDown(14, keys) != 0)
                result |= 0x200;
            if (IsJoystickButtonDown(1, &joy) != 0 || IsKeyboardButtonDown(1, keys) != 0)
                result |= 0x400;
            if (IsJoystickButtonDown(6, &joy) != 0 || IsKeyboardButtonDown(6, keys) != 0)
                result |= 0x2000;
            if (IsJoystickButtonDown(2, &joy) != 0 || IsKeyboardButtonDown(2, keys) != 0)
                result |= 0x1000;
            if (IsJoystickButtonDown(7, &joy) != 0 || IsKeyboardButtonDown(7, keys) != 0)
                result |= 0x800;
        }
        return result;
    }

    int centred = joy.x / 256 - 0x80;
    if (abs(centred) < DAT_10203408)
        joy.x = AXIS_CENTRE;
    result = (joy.x - AXIS_CENTRE) / (0x100 - DAT_10f65560);
    if (result > -AXIS_LIMIT - 1)
    {
        if (result > AXIS_LIMIT)
            return AXIS_LIMIT;
        if (result > -AXIS_LIMIT - 1)
            return result;
    }
    return -AXIS_LIMIT - 1;
}
