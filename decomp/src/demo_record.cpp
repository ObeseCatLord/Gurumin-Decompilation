// Demo recordings: the attract-mode demo ("demo.dat") and recorded play ("<name>.dat") replay the player's input
// frame by frame. The recorder is the global object at 0x1065cb28.
#include <stdio.h>

struct DemoRecording
{
    int pad_0000;
    char buttons_now[0x1c];         // 0x04: the 28 button bytes of the frame being played back
    int values_now[5];              // 0x20: its five values
    char pad_0034[0x40];
    int playFrame;                  // 0x74: next frame to play back
    int frameCount;                 // 0x78: number of recorded frames (at most 36000 = 20 minutes at 30 Hz)
    int pad_007c;
    int values[5][36000];           // 0x80: five 32-bit values per frame (analogue input and the like)
    char buttons[0x1c][36000];      // 0xafd00: 28 button bytes per frame
    int Load(const char *path);
    int PlayFrame();
    int RecordFrame();
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

// 0x1015cf30. Plays back the next recorded frame: its button bytes and values become the current input. Does
// nothing once the recording has ended. Returns 0.
int DemoRecording::PlayFrame()
{
    int frame = playFrame;
    if (frame < frameCount)
    {
        // written out one by one in the original (no loop)
        buttons_now[0] = buttons[0][frame];
        buttons_now[1] = buttons[1][frame];
        buttons_now[2] = buttons[2][frame];
        buttons_now[3] = buttons[3][frame];
        buttons_now[4] = buttons[4][frame];
        buttons_now[5] = buttons[5][frame];
        buttons_now[6] = buttons[6][frame];
        buttons_now[7] = buttons[7][frame];
        buttons_now[8] = buttons[8][frame];
        buttons_now[9] = buttons[9][frame];
        buttons_now[10] = buttons[10][frame];
        buttons_now[11] = buttons[11][frame];
        buttons_now[12] = buttons[12][frame];
        buttons_now[13] = buttons[13][frame];
        buttons_now[14] = buttons[14][frame];
        buttons_now[15] = buttons[15][frame];
        buttons_now[16] = buttons[16][frame];
        buttons_now[17] = buttons[17][frame];
        buttons_now[18] = buttons[18][frame];
        buttons_now[19] = buttons[19][frame];
        buttons_now[20] = buttons[20][frame];
        buttons_now[21] = buttons[21][frame];
        buttons_now[22] = buttons[22][frame];
        buttons_now[23] = buttons[23][frame];
        buttons_now[24] = buttons[24][frame];
        buttons_now[25] = buttons[25][frame];
        buttons_now[26] = buttons[26][frame];
        buttons_now[27] = buttons[27][frame];
        values_now[0] = values[0][frame];
        values_now[1] = values[1][frame];
        values_now[2] = values[2][frame];
        values_now[3] = values[3][frame];
        values_now[4] = values[4][frame];
        playFrame = frame + 1;
    }
    return 0;
}

// 0x1015cd70. Records the current input (28 button bytes and five values) as the next frame and makes it the end
// of the recording. There is no check against the 36000-frame limit. Returns 0.
int DemoRecording::RecordFrame()
{
    // written out one by one in the original (no loop)
    buttons[0][playFrame] = buttons_now[0];
    buttons[1][playFrame] = buttons_now[1];
    buttons[2][playFrame] = buttons_now[2];
    buttons[3][playFrame] = buttons_now[3];
    buttons[4][playFrame] = buttons_now[4];
    buttons[5][playFrame] = buttons_now[5];
    buttons[6][playFrame] = buttons_now[6];
    buttons[7][playFrame] = buttons_now[7];
    buttons[8][playFrame] = buttons_now[8];
    buttons[9][playFrame] = buttons_now[9];
    buttons[10][playFrame] = buttons_now[10];
    buttons[11][playFrame] = buttons_now[11];
    buttons[12][playFrame] = buttons_now[12];
    buttons[13][playFrame] = buttons_now[13];
    buttons[14][playFrame] = buttons_now[14];
    buttons[15][playFrame] = buttons_now[15];
    buttons[16][playFrame] = buttons_now[16];
    buttons[17][playFrame] = buttons_now[17];
    buttons[18][playFrame] = buttons_now[18];
    buttons[19][playFrame] = buttons_now[19];
    buttons[20][playFrame] = buttons_now[20];
    buttons[21][playFrame] = buttons_now[21];
    buttons[22][playFrame] = buttons_now[22];
    buttons[23][playFrame] = buttons_now[23];
    buttons[24][playFrame] = buttons_now[24];
    buttons[25][playFrame] = buttons_now[25];
    buttons[26][playFrame] = buttons_now[26];
    buttons[27][playFrame] = buttons_now[27];
    values[0][playFrame] = values_now[0];
    values[1][playFrame] = values_now[1];
    values[2][playFrame] = values_now[2];
    values[3][playFrame] = values_now[3];
    values[4][playFrame] = values_now[4];
    int next = playFrame + 1;
    playFrame = next;
    frameCount = next;
    return 0;
}
