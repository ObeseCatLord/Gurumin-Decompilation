// The button state of the input object (the recorder at 0x1065cb28, demo_record.cpp): the raw button bits and,
// for 14 buttons, one byte that is 1 on the frame the button goes down and one byte that is 1 while it is held.

#include "input_state.h"

// 0x1015cae0 (InputState::SetButtons). Takes `newButtons` as the button bits of the new frame: a button is pressed
// when its bit is set now and was clear before, held while its bit is set (the first eight buttons are read from the
// low byte). Returns 1.
int InputState::SetButtons(unsigned int newButtons)
{
    unsigned int previous = buttons;
    if (!(previous & 0x1) && (newButtons & 0x1))
        pressed[4] = 1;
    else
        pressed[4] = 0;
    if (!(previous & 0x2) && (newButtons & 0x2))
        pressed[7] = 1;
    else
        pressed[7] = 0;
    if (!(previous & 0x4) && (newButtons & 0x4))
        pressed[6] = 1;
    else
        pressed[6] = 0;
    if (!(previous & 0x8) && (newButtons & 0x8))
        pressed[5] = 1;
    else
        pressed[5] = 0;
    if (!(previous & 0x400) && (newButtons & 0x400))
        pressed[0] = 1;
    else
        pressed[0] = 0;
    if (!(previous & 0x800) && (newButtons & 0x800))
        pressed[3] = 1;
    else
        pressed[3] = 0;
    if (!(previous & 0x1000) && (newButtons & 0x1000))
        pressed[2] = 1;
    else
        pressed[2] = 0;
    if (!(previous & 0x2000) && (newButtons & 0x2000))
        pressed[1] = 1;
    else
        pressed[1] = 0;
    if (!(previous & 0x10) && (newButtons & 0x10))
        pressed[10] = 1;
    else
        pressed[10] = 0;
    if (!(previous & 0x20) && (newButtons & 0x20))
        pressed[11] = 1;
    else
        pressed[11] = 0;
    if (!(previous & 0x40) && (newButtons & 0x40))
        pressed[12] = 1;
    else
        pressed[12] = 0;
    if (!(previous & 0x80) && (newButtons & 0x80))
        pressed[13] = 1;
    else
        pressed[13] = 0;
    if (!(previous & 0x100) && (newButtons & 0x100))
        pressed[9] = 1;
    else
        pressed[9] = 0;
    if (!(previous & 0x200) && (newButtons & 0x200))
        pressed[8] = 1;
    else
        pressed[8] = 0;
    held[4] = ((unsigned char)newButtons >> 0) & 1;
    held[7] = ((unsigned char)newButtons >> 1) & 1;
    held[6] = ((unsigned char)newButtons >> 2) & 1;
    held[5] = ((unsigned char)newButtons >> 3) & 1;
    held[0] = (newButtons >> 10) & 1;
    held[3] = (newButtons >> 11) & 1;
    held[2] = (newButtons >> 12) & 1;
    held[1] = (newButtons >> 13) & 1;
    held[10] = ((unsigned char)newButtons >> 4) & 1;
    held[11] = ((unsigned char)newButtons >> 5) & 1;
    held[12] = ((unsigned char)newButtons >> 6) & 1;
    held[13] = ((unsigned char)newButtons >> 7) & 1;
    held[9] = (newButtons >> 8) & 1;
    held[8] = (newButtons >> 9) & 1;
    buttons = newButtons;
    return 1;
}
