// Point, Obj: declared here once (first used by src/app_0001.cpp).
#ifndef GURUMIN_APP_0001_H
#define GURUMIN_APP_0001_H

#include <windows.h>

struct Point
{
    int x;
    int y;
};
struct Obj
{
    char m_0000;
    char m_0001;
    char m_0002;
    char m_0003;
    int m_0004;
    int m_0008;
    int clickX;               // 0x0c: where the left button was last pressed
    int clickY;               // 0x10
    int m_0014;
    int changed;
    int m_001c;
    int lastX;
    int lastY;
    int leftHeld;             // 0x28: 1 while the left button stays down
    int rightHeld;            // 0x2c
    int middleHeld;           // 0x30
    DWORD clickTime;          // 0x34: tick count of the last left press (0 after a double click)
    int m_0038;
    int m_003c;
    int m_0040;
    int M_10001010(int p, int q, int r);
    int GetState(Point *pos, char *a, char *b, char *c, int *d, char *e);
    int UpdateChanged();
    int ReportCursor();
    int Update(Point *pos, char *left, char *middle, char *right, int *wheel, char *doubleClick);
};

#endif
