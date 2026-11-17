// Background music playback through soundarc.dll (see docs/MODDING_MAP.md "Sound architecture").
// Two streams (0 and 1) are used alternately so a new track can start while the old one fades out.

extern "C" int DAT_102027d8;
extern "C" char DAT_104bfd54;
extern "C" int DAT_104bfd58;
extern "C" int FUN_10001b70(int);
extern "C" int FUN_10001ba0(int stream);
extern "C" int FUN_10001bf0(int stream, int fadeTime);
extern "C" int FUN_10001a90(int stream, const char *path, int a, int b, int c, int d);
extern "C" char *FUN_1013f910(const char *name);

// Plays music file `name`: sets the master music level from the volume option (DAT_102027d8, 0..10),
// stops or fades out (2 s) the stream that is playing, switches to the other stream and starts the new track
// on it. `a`..`d` are passed through to soundarc's Play command (loop and fade parameters).
extern "C" void FUN_10010480(const char *name, int a, int b, int c, int d, int fade)
{
    FUN_10001b70((10 - DAT_102027d8) * 500);
    if (DAT_104bfd54 != 0)
    {
        if (fade != 0)
            FUN_10001bf0(DAT_104bfd58, 2000);
        else
            FUN_10001ba0(DAT_104bfd58);
    }
    DAT_104bfd58 = (DAT_104bfd58 - 1) & 1;
    DAT_104bfd54 = 1;
    FUN_10001a90(DAT_104bfd58, FUN_1013f910(name), c, a, b, d);
}
