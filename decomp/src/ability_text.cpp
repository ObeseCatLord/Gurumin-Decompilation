// The effect text of an equipment ability (the costume and helmet abilities shown in the equipment menu): the
// ability bits are tested in a fixed priority order (pairs of bits give the stronger effect when both are set) and
// the text of the first one that is set is copied to `text` (Shift-JIS; the decoded text is in each comment).

#include <string.h>

// 0x100144c0 (Ability_GetDescription). Copies the text of the highest-priority ability set in `flags` to `text`
// and returns that ability's bits, or returns 0 (leaving `text` alone) when none is set.
extern "C" int FUN_100144c0(int flags, char *text)
{
    if ((flags & 0x3) == 0x3)
    {
        strcpy(text, "\x90\x85\x83_\x83\x81\x81[\x83W\x89\xf1\x95\x9c");
        return 0x3;
    }
    if ((flags & 0x2) != 0)
    {
        strcpy(text, "\x90\x85\x83_\x83\x81\x81[\x83W\x96\xb3\x8c\xf8");
        return 0x2;
    }
    if ((flags & 0x1) != 0)
    {
        strcpy(text, "\x90\x85\x83_\x83\x81\x81[\x83W\x94\xbc\x8c\xb8");
        return 0x1;
    }
    if ((flags & 0x3000) == 0x3000)
    {
        strcpy(text, "\x83K\x83X\x83_\x83\x81\x81[\x83W\x89\xf1\x95\x9c");
        return 0x3000;
    }
    if ((flags & 0x2000) != 0)
    {
        strcpy(text, "\x83K\x83X\x83_\x83\x81\x81[\x83W\x96\xb3\x8c\xf8");
        return 0x2000;
    }
    if ((flags & 0x1000) != 0)
    {
        strcpy(text, "\x83K\x83X\x83_\x83\x81\x81[\x83W\x94\xbc\x8c\xb8");
        return 0x1000;
    }
    if ((flags & 0x30000) == 0x30000)
    {
        strcpy(text, "\x83g\x83\x89\x83" "b\x83v\x83_\x83\x81\x81[\x83W\x89\xf1\x95\x9c");
        return 0x30000;
    }
    if ((flags & 0x20000) != 0)
    {
        strcpy(text, "\x83g\x83\x89\x83" "b\x83v\x83_\x83\x81\x81[\x83W\x96\xb3\x8c\xf8");
        return 0x20000;
    }
    if ((flags & 0x10000) != 0)
    {
        strcpy(text, "\x83g\x83\x89\x83" "b\x83v\x83_\x83\x81\x81[\x83W\x94\xbc\x8c\xb8");
        return 0x10000;
    }
    if ((flags & 0x8000000) != 0)
    {
        strcpy(text, "\x8a\x8a\x97\x8e\x83_\x83\x81\x81[\x83W\x96\xb3\x8c\xf8");
        return 0x8000000;
    }
    if ((flags & 0x4000000) != 0)
    {
        strcpy(text, "\x8a\x8a\x97\x8e\x83_\x83\x81\x81[\x83W\x94\xbc\x8c\xb8");
        return 0x4000000;
    }
    if ((flags & 0x8) != 0)
    {
        strcpy(text, "\x83~\x83j\x83}\x83" "b\x83v\x82\xc9\x92\xd9\x95\x5c\x8e\xa6");
        return 0x8;
    }
    if ((flags & 0x4) != 0)
    {
        strcpy(text, "\x83~\x83j\x83}\x83" "b\x83v\x82\xc9\x95\xf3\x94\xa0\x95\x5c\x8e\xa6");
        return 0x4;
    }
    if ((flags & 0x800) != 0)
    {
        strcpy(text, "\x82\xc7\x82\xf1\x82\xc8\x8dU\x8c\x82\x82\xc5\x82\xe0\x82g\x82o\x89\xf1\x95\x9c");
        return 0x800;
    }
    if ((flags & 0x80) == 0x80)
    {
        strcpy(text, "\x83N\x83\x8a\x83" "e\x83" "B\x83J\x83\x8b\x8dU\x8c\x82\x82\xc5\x82g\x82o\x89\xf1\x95\x9c");
        return 0x80;
    }
    if ((flags & 0x8000) != 0)
    {
        strcpy(text, "\x8f\xed\x82\xc9\x82g\x82o\x89\xf1\x95\x9c");
        return 0x8000;
    }
    if ((flags & 0x4000) != 0)
    {
        strcpy(text, "\x91\xd2\x8b@\x92\x86\x82\xc9\x82g\x82o\x89\xf1\x95\x9c");
        return 0x4000;
    }
    if ((flags & 0x80000) != 0)
    {
        strcpy(text, "\x8f\xed\x82\xc9\x82" "c\x82o\x89\xf1\x95\x9c");
        return 0x80000;
    }
    if ((flags & 0x40000) != 0)
    {
        strcpy(text, "\x91\xd2\x8b@\x92\x86\x82\xc9\x82" "c\x82o\x89\xf1\x95\x9c");
        return 0x40000;
    }
    if ((flags & 0x800000) != 0)
    {
        strcpy(text, "\x83N\x83\x8a\x83" "e\x83" "B\x83J\x83\x8b\x93\xfc\x8e\xe8\x82R\x94{");
        return 0x800000;
    }
    if ((flags & 0x400000) != 0)
    {
        strcpy(text, "\x83N\x83\x8a\x83" "e\x83" "B\x83J\x83\x8b\x93\xfc\x8e\xe8\x82Q\x94{");
        return 0x400000;
    }
    if ((flags & 0x3000000) == 0x3000000)
    {
        strcpy(text, "\x91\xae\x90\xab\x8dU\x8c\x82\x82R\x94{");
        return 0x3000000;
    }
    if ((flags & 0x2000000) != 0)
    {
        strcpy(text, "\x91\xae\x90\xab\x8dU\x8c\x82\x82Q\x94{");
        return 0x2000000;
    }
    if ((flags & 0x1000000) != 0)
    {
        strcpy(text, "\x91\xae\x90\xab\x8dU\x8c\x82\x82P\x81" "D\x82T\x94{");
        return 0x1000000;
    }
    if ((flags & 0x30000000) == 0x30000000)
    {
        strcpy(text, "\x82\xa8\x8b\xe0\x93\xfc\x8e\xe8\x82R\x94{");
        return 0x30000000;
    }
    if ((flags & 0x20000000) != 0)
    {
        strcpy(text, "\x82\xa8\x8b\xe0\x93\xfc\x8e\xe8\x82Q\x94{");
        return 0x20000000;
    }
    if ((flags & 0x10000000) != 0)
    {
        strcpy(text, "\x82\xa8\x8b\xe0\x93\xfc\x8e\xe8\x82P\x81" "D\x82T\x94{");
        return 0x10000000;
    }
    if ((flags & 0x300000) == 0x300000)
    {
        strcpy(text, "\x82\xa8\x93X\x82\xc5\x82T\x82O\x81\x93\x8a\x84\x88\xf8");
        return 0x300000;
    }
    if ((flags & 0x200000) != 0)
    {
        strcpy(text, "\x82\xa8\x93X\x82\xc5\x82P\x82O\x81\x93\x8a\x84\x88\xf8");
        return 0x200000;
    }
    if ((flags & 0x100000) != 0)
    {
        strcpy(text, "\x82\xa8\x93X\x82\xc5\x82T\x81\x93\x8a\x84\x88\xf8");
        return 0x100000;
    }
    if ((flags & 0x40) != 0)
    {
        strcpy(text, "\x83_\x83\x81\x81[\x83W\x82V\x82T\x81\x93\x8cy\x8c\xb8");
        return 0x40;
    }
    if ((flags & 0x30) == 0x30)
    {
        strcpy(text, "\x83_\x83\x81\x81[\x83W\x82U\x82T\x81\x93\x8cy\x8c\xb8");
        return 0x30;
    }
    if ((flags & 0x20) != 0)
    {
        strcpy(text, "\x83_\x83\x81\x81[\x83W\x82T\x82O\x81\x93\x8cy\x8c\xb8");
        return 0x20;
    }
    if ((flags & 0x10) != 0)
    {
        strcpy(text, "\x83_\x83\x81\x81[\x83W\x82Q\x82T\x81\x93\x8cy\x8c\xb8");
        return 0x10;
    }
    if ((flags & 0x400) != 0)
    {
        strcpy(text, "\x8dU\x8c\x82\x97\xcd\x82Q\x94{");
        return 0x400;
    }
    if ((flags & 0x300) == 0x300)
    {
        strcpy(text, "\x8dU\x8c\x82\x97\xcd\x82P\x81" "D\x82V\x94{");
        return 0x300;
    }
    if ((flags & 0x200) != 0)
    {
        strcpy(text, "\x8dU\x8c\x82\x97\xcd\x82P\x81" "D\x82T\x94{");
        return 0x200;
    }
    if ((flags & 0x100) != 0)
    {
        strcpy(text, "\x8dU\x8c\x82\x97\xcd\x82P\x81" "D\x82Q\x94{");
        return 0x100;
    }
    return 0;
}
