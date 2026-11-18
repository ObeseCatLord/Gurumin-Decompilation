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

extern "C" int DAT_10f1bef0[];                   // per game button: what it is mapped to (0 = default)
extern "C" int DAT_10203408;                     // dead zone around the axis centre (in 1/256 units)
extern "C" unsigned char DAT_10f6556c;           // axes present: 1 = z, 2 = rz, 4 = rx, 8 = ry

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
