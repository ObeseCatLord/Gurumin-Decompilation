// InputState: declared here once (first used by src/input_state.cpp).
#ifndef GURUMIN_INPUT_STATE_H
#define GURUMIN_INPUT_STATE_H

// Button bytes in `pressed` and `held`: bits 10-13 of the raw state come first, then bits 0, 3, 2, 1, 9, 8 and 4-7.
struct InputState
{
    unsigned int buttons;        // raw button bits of the current frame
    union
    {
        struct
        {
            char pressed[14];    // 0x04: 1 = went down this frame
            char held[14];       // 0x12: 1 = down
        };
        char buttons_now[0x1c];  // 0x04: the same 28 bytes (the frame being played back)
    };
    int SetButtons(unsigned int newButtons);
    void M_1013edb0();           // 0x1013edb0 (an empty method)
};

#endif
