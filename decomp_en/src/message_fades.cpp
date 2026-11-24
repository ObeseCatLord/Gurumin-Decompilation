// Message box fading and timing (EN build). Ported from the matched JP source decomp/src/message_fades.cpp.
#include "messages.h"
#include "mouse.h"
#include "script_queue.h"

// Button state value of a button that was pressed this frame.
const char BUTTON_PRESSED = 2;

// Messages_Update: advances every message box by one frame. A pending request while a costume unlock bit is set is
// answered (g_scriptFlagByte = 1) on a key or button press. Timed boxes fade in, stay shown for `shownFrames` frames and
// fade out; key boxes fade in and wait for a key before they fade out. Returns 0. Called once per frame.
extern "C" int Messages_Update(int shownFrames)
{
    char left;
    char middle;
    char right;
    g_mouse.GetState(0, &left, &right, &middle, 0, 0, 0);
    if (g_messageRequestPending != 0 && (g_costumeUnlockBits & MESSAGE_ANSWER_BITS) != 0 &&
        (g_keyPressedNow != 0 || left == BUTTON_PRESSED || right == BUTTON_PRESSED || middle == BUTTON_PRESSED))
    {
        g_messageRequestPending = 0;
        g_scriptFlagByte = 1;
    }
    for (int i = 0; i < MESSAGE_COUNT; i++)
    {
        if (g_messageState[i] > MESSAGE_WAIT_KEY_BASE)
        {
            g_messageKeyWait++;
            if (g_messageState[i] == MESSAGE_KEY_SHOWN)
                g_messageTimer[i]++;
            if (g_messageState[i] == MESSAGE_KEY_SHOWN &&
                (g_keyPressedNow != 0 || left == BUTTON_PRESSED || right == BUTTON_PRESSED || middle == BUTTON_PRESSED) &&
                g_messageKeyWait > MESSAGE_KEY_DELAY)
            {
                g_messageState[i] = MESSAGE_KEY_FADE_OUT;
                g_messageTimer[i] = 0;
                g_messageKeyWait = 0;
                g_messageKeyReset = 0;
            }
            if (g_messageState[i] == MESSAGE_KEY_FADE_IN)
                g_messageFade[i] += MESSAGE_FADE_STEP;
            if (g_messageState[i] == MESSAGE_KEY_FADE_OUT)
                g_messageFade[i] -= MESSAGE_FADE_STEP;
            if (g_messageFade[i] == 0)
                g_messageState[i] = MESSAGE_HIDDEN;
            if (g_messageFade[i] >= MESSAGE_FADE_FULL)
                g_messageState[i] = MESSAGE_KEY_SHOWN;
        }
        else
        {
            if (g_messageState[i] == MESSAGE_TIMED_SHOWN)
                g_messageTimer[i]++;
            if (g_messageTimer[i] > shownFrames)
            {
                g_messageState[i] = MESSAGE_TIMED_FADE_OUT;
                g_messageTimer[i] = 0;
            }
            if (g_messageState[i] == MESSAGE_TIMED_FADE_IN)
                g_messageFade[i] += MESSAGE_FADE_STEP;
            if (g_messageState[i] == MESSAGE_TIMED_FADE_OUT)
                g_messageFade[i] -= MESSAGE_FADE_STEP;
            if (g_messageFade[i] == 0)
                g_messageState[i] = MESSAGE_HIDDEN;
            if (g_messageFade[i] >= MESSAGE_FADE_FULL)
                g_messageState[i] = MESSAGE_TIMED_SHOWN;
        }
    }
    return 0;
}
