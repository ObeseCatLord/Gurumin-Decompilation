// Adding screen-space vertices to a triangle batch (EN build; JP twin: screen_vertices.cpp).
#include "engine.h"
#include "render.h"
#include "texture_mgr.h"

// TriangleBatch::AppendVertices: appends `sourceCount` vertices laid out for the 640x480 game screen, scaling their
// positions to the back buffer. Called by AddVertices. Returns 1.
int TriangleBatch::AppendVertices(const ScreenVertex *source, int sourceCount)
{
    float scaleX = (float)g_backBufferWidth / GAME_SCREEN_WIDTH;
    float scaleY = (float)g_backBufferHeight / GAME_SCREEN_HEIGHT;
    for (int i = 0; i < sourceCount; i++)
    {
        vertices[count + i] = source[i];
        vertices[count + i].x = scaleX * vertices[count + i].x;
        vertices[count + i].y = scaleY * vertices[count + i].y;
    }
    count += sourceCount;
    return 1;
}
