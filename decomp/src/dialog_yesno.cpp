// Yes/no dialog (menu id 0x216): a message box with the choices "hai" (yes) and "iie" (no) in Shift-JIS.
// The choice's value is reported through the menu system; the cancel button returns `noValue`.

#include "game.h"

extern "C" Game DAT_104b7200;
extern "C" int FUN_1000cb30(const char *text);
extern "C" int FUN_1000cb90(const char *text);

#define TEXT_YES "\x82\xcd\x82\xa2"
#define TEXT_NO "\x82\xa2\x82\xa2\x82\xa6"

// Shows `message` with "yes" as the first choice.
extern "C" int FUN_10010520(const char *message, int yesValue, int noValue)
{
    DAT_104b7200.OpenWindow(0x216, 0x100, 0x90, 0x7c, 0x60);
    DAT_104b7200.M_10008330(0x216, (int *)noValue);  // the original passes the same value as the result pointer
    FUN_1000cb30("");
    FUN_1000cb90(message);
    int slot = DAT_104b7200.FindSlot(0x216);
    if (slot != 3)
        DAT_104b7200.m_0128[slot] = 0x20;
    DAT_104b7200.M_1000b2c0(0x216, TEXT_YES, yesValue);
    DAT_104b7200.M_1000b2c0(0x216, TEXT_NO, noValue);
    return 1;
}

// Shows `message` with "no" as the first choice (for dangerous questions such as overwriting a save).
extern "C" int FUN_100105e0(const char *message, int yesValue, int noValue)
{
    DAT_104b7200.OpenWindow(0x216, 0x100, 0x90, 0x7c, 0x60);
    DAT_104b7200.M_10008330(0x216, (int *)noValue);  // the original passes the same value as the result pointer
    FUN_1000cb30("");
    FUN_1000cb90(message);
    int slot = DAT_104b7200.FindSlot(0x216);
    if (slot != 3)
        DAT_104b7200.m_0128[slot] = 0x20;
    DAT_104b7200.M_1000b2c0(0x216, TEXT_NO, noValue);
    DAT_104b7200.M_1000b2c0(0x216, TEXT_YES, yesValue);
    return 1;
}
