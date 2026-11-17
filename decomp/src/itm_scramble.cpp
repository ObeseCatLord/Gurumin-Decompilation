#include <string.h>

extern "C" int FUN_101614c0(unsigned char *data, int size, char *name, int *sum, unsigned int *check)
{
    char table[70] = "!~_Q#xDRgd-d&yfg&'(8)(5(594er2f4asf5f6e5f4s5fvwyjk%fgqTUCFD4568r";
    char key[16] = "GURUguruMinmIN_";
    int total = 0;
    unsigned int mix = 0xde2f;
    _strupr(name);
    strncpy(table + 8, name, 8);
    for (int i = 0; i < size; i++)
    {
        unsigned char c = data[i];
        mix ^= c;
        total += c;
        data[i] = ~((key[i % 15] + (i & 7)) * table[i & 0x3f] ^ c);
    }
    *sum = total;
    *check = mix;
    return 1;
}

extern "C" int FUN_101615d0(unsigned char *data, int size, char *name, int sum, unsigned int check)
{
    char table[70] = "!~_Q#xDRgd-d&yfg&'(8)(5(594er2f4asf5f6e5f4s5fvwyjk%fgqTUCFD4568r";
    char key[16] = "GURUguruMinmIN_";
    int total = 0;
    unsigned int mix = 0xde2f;
    _strupr(name);
    strncpy(table + 8, name, 8);
    for (int i = 0; i < size; i++)
    {
        unsigned char c = (key[i % 15] + (i & 7)) * table[i & 0x3f] ^ ~data[i];
        data[i] = c;
        total += c;
        mix ^= c;
    }
    if (sum == total && check == mix)
        return 1;
    return 0;
}
