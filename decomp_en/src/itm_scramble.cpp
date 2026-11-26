// Descrambling of .itm data files (EN build). JP twin: decomp/src/itm_scramble.cpp (ItmDescramble).
#include <string.h>

#include "itm_scramble.h"

// Seed of the xor check, the key period, and where the upper-cased file name goes in the key table.
const unsigned int ITM_CHECK_SEED = 0xde2f;
const int ITM_KEY_PERIOD = 15;
const int ITM_NAME_OFFSET = 8;
const int ITM_NAME_LENGTH = 8;
const int ITM_TABLE_MASK = 0x3f;

// Reverses the scrambling of a .itm file in place: every byte is xored with a key byte (key text + position, times the
// table byte) and inverted. The table holds the first 8 characters of the upper-cased file name at offset 8. Returns 1
// when the byte sum and xor of the plain data match `sum` and `check`, else 0. Called when a .itm file is loaded.
extern "C" int Itm_Descramble(unsigned char *data, int size, char *name, int sum, unsigned int check)
{
    int total = 0;
    unsigned int mix = ITM_CHECK_SEED;
    char table[70] = "!~_Q#xDRgd-d&yfg&'(8)(5(594er2f4asf5f6e5f4s5fvwyjk%fgqTUCFD4568r";
    char key[16] = "GURUguruMinmIN_";
    _strupr(name);
    strncpy(table + ITM_NAME_OFFSET, name, ITM_NAME_LENGTH);
    for (int i = 0; i < size; i++)
    {
        unsigned char c = (key[i % ITM_KEY_PERIOD] + (i & 7)) * table[i & ITM_TABLE_MASK] ^ ~data[i];
        data[i] = c;
        total += c;
        mix ^= c;
    }
    if (sum == total && check == mix)
        return 1;
    return 0;
}

// Scrambles a .itm file in place (the inverse of Itm_Descramble, same key): every plain byte is added to the byte sum and
// xored into the check, then replaced by the inverted xor with its key byte. The sum and check of the plain data go to
// *sum and *check; returns 1. Called by the item data writer.
extern "C" int Itm_Scramble(unsigned char *data, int size, char *name, int *sum, unsigned int *check)
{
    int total = 0;
    unsigned int mix = ITM_CHECK_SEED;
    char table[70] = "!~_Q#xDRgd-d&yfg&'(8)(5(594er2f4asf5f6e5f4s5fvwyjk%fgqTUCFD4568r";
    char key[16] = "GURUguruMinmIN_";
    _strupr(name);
    strncpy(table + ITM_NAME_OFFSET, name, ITM_NAME_LENGTH);
    for (int i = 0; i < size; i++)
    {
        unsigned char plain = data[i];
        mix ^= plain;
        total += plain;
        data[i] = ~((key[i % ITM_KEY_PERIOD] + (i & 7)) * table[i & ITM_TABLE_MASK] ^ plain);
    }
    *sum = total;
    *check = mix;
    return 1;
}
