#include <stdlib.h>
#include "game.h"

extern "C" const char *FUN_1013f8a0(const char *name, int unused);
extern "C" int FUN_10173550(const char *path, const char *alphaPath, int flags);

#define DATA(name) FUN_1013f8a0(name, 0)

int Game::LoadMenuTextures(int value)
{
    m_0690 = 0;
    M_10007f20();
    m_7664 = value;
    texMenu1 = FUN_10173550(DATA("menu1.bmp"), DATA("menu1a.bmp"), 0);
    texMenu4 = FUN_10173550(DATA("menu4.bmp"), 0, 0);
    texMenu2 = FUN_10173550(DATA("menu2.bmp"), 0, 0);
    texMenu5 = FUN_10173550(DATA("menu5.bmp"), 0, 0);
    m_0010 = -1;
    texBalloon5 = FUN_10173550(DATA("fukida5p.bmp"), DATA("fukida5a.bmp"), 0);
    texBalloon6 = FUN_10173550(DATA("fukida6p.bmp"), DATA("fukida6a.bmp"), 0);
    texFrame = FUN_10173550(DATA("futi.bmp"), 0, 0);
    texMap01 = FUN_10173550(DATA("map01.bmp"), 0, 0);
    texMap02 = FUN_10173550(DATA("map02.bmp"), 0, 0);
    texUdChip = FUN_10173550(DATA("UD_chip.bmp"), 0, 0);
    texWaku01 = FUN_10173550(DATA("waku01.bmp"), DATA("WAKU01A.bmp"), 0);
    texWaku02 = FUN_10173550(DATA("waku02.bmp"), DATA("WAKU02A.bmp"), 0);
    texWaku03 = FUN_10173550(DATA("waku03.bmp"), DATA("WAKU03A.bmp"), 0);
    texWaku04 = FUN_10173550(DATA("waku04.bmp"), 0, 0);
    texIcon01 = FUN_10173550(DATA("ico01.bmp"), DATA("ico01a.bmp"), 0);
    texWaku05 = FUN_10173550(DATA("waku05.bmp"), 0, 0);
    texBalloon7 = FUN_10173550(DATA("fukida7p.bmp"), DATA("fukida7a.bmp"), 0);
    texMenu14 = FUN_10173550(DATA("menu14.bmp"), 0, 0);
    texMenu15 = FUN_10173550(DATA("menu15.bmp"), 0, 0);
    free(operator new(0x60000));
    m_6ffc = 1;
    return 1;
}
