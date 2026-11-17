#include <windows.h>
#include <mmsystem.h>

extern "C" int DAT_10203344;
extern "C" int DAT_10f65498;
extern "C" int DAT_10f65490;
extern "C" int DAT_10f65494;
extern "C" int DAT_10f654c0;
extern "C" int DAT_10f654c4;
extern "C" char DAT_10f65500;
extern "C" DWORD DAT_10f654d8;
extern "C" int DAT_10f65484;
extern "C" int FUN_1013ed20(void);
extern "C" int FUN_1013ed60(void);
extern "C" int FUN_101439e0(void);
extern "C" int FUN_1014b480(void);

extern "C" int FUN_1015c9e0(int mode)
{
    if (DAT_10203344 == 0)
        return 0;
    if (DAT_10f65498 == 0)
    {
        DWORD now;
        if (mode != 0)
        {
            if (DAT_10f654c0 != 0)
                return 0;
            DAT_10f65494 = DAT_10f65490;
            DAT_10f65490 = (DAT_10f65490 - 1) & 1;
            now = timeGetTime();
            DAT_10f654c0 = mode;
            DAT_10f654c4 = mode;
            DAT_10f65500 = 1;
        }
        else
        {
            DAT_10f65490 = DAT_10f65494;
            now = timeGetTime();
            DAT_10f654c0 = 0;
            DAT_10f654c4 = 0;
            DAT_10f65500 = 0;
        }
        DAT_10f654d8 = now + 16;
        FUN_1013ed60();
        if (DAT_10f65484 != 0)
        {
            FUN_101439e0();
            FUN_1013ed20();
            return 0;
        }
        FUN_1014b480();
        FUN_1013ed20();
    }
    return 0;
}
