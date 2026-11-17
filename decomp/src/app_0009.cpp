// Screen effects, message timers and sound timers.

#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Set(float x, float y, float z, float w);
    void Copy(const Vec4 &source);
    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
    Vec4 operator*(const float &scale) const;   // 0x101acb00 (x/y/z only)
};

extern "C" void FUN_1000f360(int, int);
extern "C" void FUN_10001c50(int);
extern "C" int DAT_104bfd28;
extern "C" int DAT_104bfd2c;
extern "C" int DAT_104bfd30;
extern "C" int DAT_104bfd5c;
extern "C" int DAT_1030c568;
// Screen colour fade: the colour moves from DAT_10da90d0 towards DAT_10c67f60 by DAT_10daa800 per frame.
extern "C" int DAT_1042da68;   // frames left
extern "C" Vec4 DAT_10da90d0;  // current colour
extern "C" Vec4 DAT_10daa800;  // step per frame
extern "C" Vec4 DAT_10c67f60;  // target colour
extern "C" int DAT_104bfd4c;
extern "C" char DAT_104c01fc;
extern "C" int DAT_104bf8c8;
extern "C" int DAT_104bf8cc;
extern "C" int DAT_104bf8d0;
extern "C" char DAT_10219bc0[];
extern "C" char DAT_10467ea0[];
extern "C" int DAT_10255040[0x20];
extern "C" int DAT_1020d5c8[0x20];
extern "C" int DAT_1030c278[0x80];
extern "C" int DAT_10447dd8[0x80];
extern "C" int DAT_104c01b0;
extern "C" short DAT_10447338[];

// Starts the screen fade-in (state 1).
extern "C" int FUN_1000ebf0(void)
{
    DAT_104bfd28 = 1;
    DAT_104bfd2c = 0;
    DAT_104bfd30 = 0;
    DAT_104bfd5c = 0;
    return 0;
}

// Advances the screen fade: fading in (state 1) for 15 frames then fully shown (state 2, DAT_1030c568 = 1);
// fading out (state 3) back to 0.
extern "C" int FUN_1000ec10(void)
{
    if (DAT_104bfd28 == 1)
    {
        if (++DAT_104bfd2c > 15)
        {
            DAT_104bfd28 = 2;
            DAT_1030c568 = 1;
        }
    }
    else if (DAT_104bfd28 == 3)
    {
        if (--DAT_104bfd2c == 0)
            DAT_104bfd28 = 0;
    }
    return 1;
}

// Starts a colour fade to (r, g, b, a) over `frames` frames; one frame or less sets the colour at once.
extern "C" void FUN_1000ec70(int frames, float r, float g, float b, float a)
{
    if (frames <= 1)
    {
        DAT_10da90d0.Set(r, g, b, a);
        DAT_1042da68 = 0;
        return;
    }
    DAT_10c67f60.Set(r, g, b, a);
    DAT_10daa800.Copy((DAT_10c67f60 - DAT_10da90d0) * (1.0f / (float)frames));
    DAT_1042da68 = frames;
}

// Advances the colour blend of DAT_10da90d0 by one step, snapping to the target on the last frame.
// Not byte-identical yet (see open.tsv).
extern "C" void FUN_1000ed20(void)
{
    if (DAT_1042da68 != 0)
    {
        DAT_1042da68--;
        DAT_10da90d0.Copy(DAT_10da90d0 + DAT_10daa800);
        if (DAT_1042da68 == 0)
            DAT_10da90d0.Copy(DAT_10c67f60);
    }
}

// Like FUN_1000ed70 (app_0013.cpp) but shows the message in the alternative style (DAT_104c01fc = 1).
extern "C" int FUN_1000edf0(const char *text, const char *subtext, int frames)
{
    DAT_104bfd4c = frames;
    DAT_104c01fc = 1;
    DAT_104bf8c8 = 1;
    DAT_104bf8cc = 0;
    DAT_104bf8d0 = 0;
    DAT_10467ea0[0] = 0;
    strcpy(DAT_10219bc0, text);
    if (subtext != 0)
        strcpy(DAT_10467ea0, subtext);
    return 1;
}

// Advances the message window: closes after its display time, animating the fade in/out over 10 frames.
extern "C" int FUN_1000ee60(void)
{
    if (++DAT_104bf8d0 > DAT_104bfd4c)
        DAT_104bf8c8 = 2;
    if (DAT_104bf8c8 == 1)
    {
        if (DAT_104bf8cc < 10)
            DAT_104bf8cc++;
    }
    else if (DAT_104bf8c8 == 2)
    {
        if (DAT_104bf8cc > 0)
            DAT_104bf8cc--;
    }
    if (DAT_104bf8cc == 0)
        DAT_104bf8c8 = 0;
    return 1;
}

// Clears each of the 32 short-lived flags 30 frames after it was set.
extern "C" int FUN_1000f320(void)
{
    for (int i = 0; i < 0x20; i++)
    {
        if (DAT_10255040[i] != 0)
        {
            if (++DAT_1020d5c8[i] > 30)
            {
                DAT_1020d5c8[i] = 0;
                DAT_10255040[i] = 0;
            }
        }
    }
    return 0;
}

// Counts down the 128 delayed actions and runs each one (FUN_1000f360 with its stored value) when it expires.
extern "C" int FUN_1000f490(void)
{
    for (int i = 0; i < 0x80; i++)
    {
        if (DAT_1030c278[i] != 0)
        {
            if (--DAT_1030c278[i] == 0)
                FUN_1000f360(i, DAT_10447dd8[i]);
        }
    }
    return 0;
}

// Stops the looping sound of slot `index` (soundarc command 0x224) unless sound is muted; -11 marks a free slot.
extern "C" void FUN_100102a0(int index)
{
    if (DAT_104c01b0 == 0)
    {
        short sound = DAT_10447338[index];
        if (sound != -11)
        {
            FUN_10001c50(sound);
            DAT_10447338[index] = -11;
        }
    }
}
