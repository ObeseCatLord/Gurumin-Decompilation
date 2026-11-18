// Demo recordings: the attract-mode demo ("demo.dat") and recorded play ("<name>.dat") replay the player's input
// frame by frame. The recorder is the global object at 0x1065cb28.
#include <stdio.h>

struct DemoRecording
{
    char pad_0000[0x78];
    int frameCount;                 // 0x78: number of recorded frames (at most 36000 = 20 minutes at 30 Hz)
    int pad_007c;
    int values[5][36000];           // 0x80: five 32-bit values per frame (analogue input and the like)
    char buttons[0x1c][36000];      // 0xafd00: 28 button bytes per frame
    int Load(const char *path);
};

// 0x1015cc90. Loads a recording from `path`: the frame count, then per frame the five values and the 28 button
// bytes. A missing file leaves the recording as it is. Returns 0. Called by Game_Startup.
int DemoRecording::Load(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file != 0)
    {
        fread(&frameCount, 1, 4, file);
        for (int frame = 0; frame < frameCount; frame++)
        {
            int k;
            for (k = 0; k < 5; k++)
                fread(&values[k][frame], 1, 4, file);
            for (k = 0; k < 0x1c; k++)
                fread(&buttons[k][frame], 1, 1, file);
        }
        fclose(file);
    }
    return 0;
}
