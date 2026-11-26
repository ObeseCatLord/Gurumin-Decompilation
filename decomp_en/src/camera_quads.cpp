// Camera quad table (EN build, EN-only: no JP twin found).
#include "camera.h"

// Camera_AddQuad: appends the next camera quad with the corners (x0, y0), (x1, y1), (x2, y2), (x3, y3) given in whole
// numbers, and stores the half diagonal from corner 0 to corner 2 and its negation. Returns 0. No direct caller
// recovered.
extern "C" int Camera_AddQuad(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3)
{
    int index = g_cameraState1664fe4++;
    g_cameraQuads[index].corner[0].x = (float)x0;
    g_cameraQuads[index].corner[0].y = (float)y0;
    g_cameraQuads[index].corner[1].x = (float)x1;
    g_cameraQuads[index].corner[1].y = (float)y1;
    g_cameraQuads[index].corner[2].x = (float)x2;
    g_cameraQuads[index].corner[2].y = (float)y2;
    float halfX = (float)(x2 - x0) * CAMERA_QUAD_HALF;
    g_cameraQuads[index].corner[3].x = (float)x3;
    float halfY = (float)(y2 - y0) * CAMERA_QUAD_HALF;
    g_cameraQuads[index].corner[3].y = (float)y3;
    g_cameraQuads[index].halfDiagonalX = halfX;
    g_cameraQuads[index].halfDiagonalY = halfY;
    g_cameraQuads[index].negHalfDiagonalX = -halfX;
    g_cameraQuads[index].negHalfDiagonalY = -halfY;
    return 0;
}
