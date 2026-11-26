// Key check of the compressed sound files (EN build; JP twin: sound3d.dll 10002e70, not matched).
#include <string.h>

#include "sound_file.h"

// Bytes of the key block built from the file name, and the seed of its xor check.
const int SOUND_KEY_BLOCK_BYTES = 32;
const unsigned int SOUND_KEY_CHECK_SEED = 0x37;

// SoundFile_KeyMatches: builds a 32-byte block from the first eight characters of the (upper-cased) name, every
// byte being one name character xored with a fixed value (the values below belong to the file format and have no
// other meaning), then adds up the even and odd bytes (signed) and xors them pairwise. Returns 1 when the total of the
// sums and the xor equal `sum` and `check`, the values stored in the sound file. Called when a compressed sound file
// is opened.
extern "C" int SoundFile_KeyMatches(char *name, int sum, unsigned int check)
{
    _strupr(name);
    char block[SOUND_KEY_BLOCK_BYTES];
    memset(block, 0, sizeof(block));
    block[0] = name[6] ^ 0x0f;
    block[1] = name[4] ^ 0x0d;
    block[2] = name[5] ^ 0x10;
    block[3] = name[1] ^ 0x42;
    block[4] = name[5] ^ 0x11;
    block[5] = name[3] ^ 0x2a;
    block[6] = name[7] ^ 0x3f;
    block[7] = name[2] ^ 0x25;
    block[8] = name[3] ^ 0x23;
    block[9] = name[1] ^ 0x49;
    block[10] = name[0] ^ 0x2b;
    block[11] = name[1] ^ 0x57;
    block[12] = name[1] ^ 0x4d;
    block[13] = name[4] ^ 0x11;
    block[14] = name[7] ^ 0x2b;
    block[15] = name[2] ^ 0x03;
    block[16] = name[6] ^ 0x3f;
    block[17] = name[7] ^ 0x3f;
    block[18] = name[4] ^ 0x0c;
    block[19] = name[2] ^ 0x53;
    block[20] = name[5] ^ 0x35;
    block[21] = name[6] ^ 0x2c;
    block[22] = 0x0e;
    block[23] = name[3] ^ 0x49;
    block[24] = name[5] ^ 0x18;
    block[25] = name[1] ^ 0x3e;
    block[27] = 0x49;
    block[28] = name[3] ^ 0x01;
    block[29] = 0x2c;
    block[30] = name[0] ^ 0x71;
    block[31] = name[4] ^ 0x17;
    int evenSum = 0;
    int oddSum = 0;
    unsigned int mix = SOUND_KEY_CHECK_SEED;
    for (int i = 0; i < SOUND_KEY_BLOCK_BYTES; i += 2)
    {
        oddSum += block[i + 1];
        evenSum += block[i];
        mix ^= block[i + 1] ^ block[i];
    }
    if (sum == evenSum + oddSum && check == mix)
        return 1;
    return 0;
}
