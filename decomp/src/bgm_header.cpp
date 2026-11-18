// Music file check: a .de2 header holds a sum and an xor of a 32-byte key made from the file name (see
// tools/de2_header.py, which builds the same key).

#include <string.h>

// 0x10001cb0 (Bgm_CheckHeader). Upper-cases `name` (in place), builds the key from its first eight characters (in the
// original's order; byte 26 stays 0) and
// returns 1 when the key's byte sum equals `sum` and its bytes xored together with 0x37 equal `check`, else 0.
extern "C" int Bgm_CheckHeader(char *name, int sum, unsigned int check)
{
    char key[32];
    _strupr(name);
    memset(key, 0, sizeof(key));
    key[1] = name[4] ^ 0x0d;
    key[0] = name[6] ^ 0x0f;
    key[2] = name[5] ^ 0x10;
    key[4] = name[5] ^ 0x11;
    key[3] = name[1] ^ 0x42;
    key[6] = name[7] ^ 0x3f;
    key[5] = name[3] ^ 0x2a;
    key[8] = name[3] ^ 0x23;
    key[7] = name[2] ^ 0x25;
    key[9] = name[1] ^ 0x49;
    key[10] = name[0] ^ 0x2b;
    key[14] = name[7] ^ 0x2b;
    key[15] = name[2] ^ 0x03;
    key[11] = name[1] ^ 0x57;
    key[12] = name[1] ^ 0x4d;
    key[19] = name[2] ^ 0x53;
    key[17] = name[7] ^ 0x3f;
    key[18] = name[4] ^ 0x0c;
    key[13] = name[4] ^ 0x11;
    key[25] = name[1] ^ 0x3e;
    key[16] = name[6] ^ 0x3f;
    key[24] = name[5] ^ 0x18;
    key[20] = name[5] ^ 0x35;
    key[28] = name[3] ^ 0x01;
    key[31] = name[4] ^ 0x17;
    key[21] = name[6] ^ 0x2c;
    key[22] = 0x0e;
    key[27] = 0x49;
    key[29] = 0x2c;
    key[23] = name[3] ^ 0x49;
    key[30] = name[0] ^ 0x71;
    int keySum = 0;
    unsigned int keyCheck = 0x37;
    for (int i = 0; i < 32; i++)
    {
        keySum += key[i];
        keyCheck ^= key[i];
    }
    if (sum == keySum && check == keyCheck)
        return 1;
    return 0;
}
