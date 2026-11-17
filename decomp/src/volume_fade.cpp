extern "C" float DAT_104bf8e8;
extern "C" int DAT_104bf8ec;
extern "C" float DAT_104bf8f0;
extern "C" float DAT_1032d4b0;
extern "C" int DAT_104bf8e4;

extern "C" int FUN_1000da40(int id, int value, int frames)
{
    float target = (float)value;
    DAT_104bf8e8 = target;
    DAT_104bf8ec = id;
    if (frames == 0)
    {
        DAT_104bf8f0 = target;
        DAT_104bf8e8 = target;
        DAT_1032d4b0 = 0.0f;
        DAT_104bf8e4 = 0;
        return 0;
    }
    DAT_104bf8e4 = frames;
    DAT_1032d4b0 = (DAT_104bf8e8 - DAT_104bf8f0) / (float)frames;
    return 0;
}
