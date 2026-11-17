// Positional sound effects through soundarc.dll (soundarc.cpp). World coordinates are scaled by 0.15 into the
// sound engine's space; the second mode argument picks the output mode (1 when the DAT_104c01b8 setting is on).

// soundarc.cpp: play a sound file once / looping. The position and the volume travel as raw 32-bit values.
extern "C" int FUN_100016c0(const char *file, const float *x, const float *y, const float *z, int mode, int loop,
                            float volume);
extern "C" int FUN_10001980(const char *file, const float *x, const float *y, const float *z, int mode, int loop,
                            float volume);
extern "C" char *FUN_1013f910(const char *name);  // path of `name` in the sound folder
extern "C" int DAT_104c01b8;                     // sound output setting (non-zero = mode 1)

// Plays sound `name` at world position (x, y, z), looping on channel `loop` (0 = play once), at `volume`
// (halved for the sound engine).
extern "C" void FUN_10010310(const char *name, float x, float y, float z, int loop, float volume)
{
    x = x * 0.15f;
    y = y * 0.15f;
    z = z * 0.15f;
    volume = volume * 0.5f;
    if (DAT_104c01b8 == 0)
    {
        if (loop != 0)
        {
            FUN_10001980(FUN_1013f910(name), &x, &y, &z, 2, loop, volume);
            return;
        }
        FUN_100016c0(FUN_1013f910(name), &x, &y, &z, 2, 0, volume);
        return;
    }
    if (loop != 0)
    {
        FUN_10001980(FUN_1013f910(name), &x, &y, &z, 1, loop, volume);
        return;
    }
    FUN_100016c0(FUN_1013f910(name), &x, &y, &z, 1, 0, volume);
}
