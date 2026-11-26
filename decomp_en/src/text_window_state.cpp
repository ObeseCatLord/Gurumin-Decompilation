// Text window makers and the enable switch (EN build, EN-only: no JP twin found).
#include <string.h>
#include "font_renderer.h"
#include "ime.h"
#include "text_window.h"

// TextWindow::InitStacked: makes a 256 x 256 window at (x, y) that joins the window stack when it is enabled, points
// its scratch view at the shared scratch block and clears its item pointers. The last two arguments are not read.
// Returns 1. No direct caller recovered.
int TextWindow::InitStacked(Mouse *windowMouse, int valueA, int valueB, int valueC, int valueD, int x, int y,
                            int unusedA, int unusedB)
{
    scratch.Set(g_textWindowScratch, TEXT_WINDOW_SCRATCH_SIZE);
    left = x;
    top = y;
    m_0010 = valueA;
    m_0014 = valueB;
    m_13ac = valueC;
    m_13b0 = valueD;
    mouse = windowMouse;
    width = TEXT_POPUP_SIDE;
    height = TEXT_POPUP_SIDE;
    stacked = 1;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_0038 = 1;
    m_0034 = 0;
    m_003c = 0;
    m_0040 = 0;
    enabled = 0;
    m_0048 = -1;
    textInputPending = 0;
    m_1398 = 0;
    m_004c = -1;
    m_0074 = 0;
    m_139c = 0;
    m_0068 = 0;
    memset(choices, 0, sizeof(choices));
    return 1;
}

// TextWindow::InitPopup: the same window as InitStacked but standing alone (it never joins the window stack).
// Returns 1. No direct caller recovered.
int TextWindow::InitPopup(Mouse *windowMouse, int valueA, int valueB, int valueC, int valueD, int x, int y,
                          int unusedA, int unusedB)
{
    scratch.Set(g_textWindowScratch, TEXT_WINDOW_SCRATCH_SIZE);
    left = x;
    top = y;
    m_0010 = valueA;
    m_0014 = valueB;
    m_13ac = valueC;
    m_13b0 = valueD;
    mouse = windowMouse;
    width = TEXT_POPUP_SIDE;
    height = TEXT_POPUP_SIDE;
    stacked = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_0038 = 1;
    m_0034 = 0;
    m_003c = 0;
    m_0040 = 0;
    enabled = 0;
    m_0048 = -1;
    textInputPending = 0;
    m_1398 = 0;
    m_004c = -1;
    m_0074 = 0;
    m_139c = 0;
    m_0068 = 0;
    memset(choices, 0, sizeof(choices));
    return 1;
}

// TextWindow::InitFullScreen: makes a stand-alone window over the whole 640 x 480 screen at (0, 0) with the fourth
// state flag set. Returns 1. No direct caller recovered.
int TextWindow::InitFullScreen(Mouse *windowMouse, int valueA, int valueB)
{
    scratch.Set(g_textWindowScratch, TEXT_WINDOW_SCRATCH_SIZE);
    m_0010 = valueA;
    m_0014 = valueB;
    mouse = windowMouse;
    left = 0;
    top = 0;
    width = TEXT_FULL_SCREEN_WIDTH;
    height = TEXT_FULL_SCREEN_HEIGHT;
    m_13ac = 0;
    m_13b0 = 0;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_0038 = 0;
    m_0034 = 0;
    m_003c = 0;
    m_0040 = 1;
    enabled = 0;
    m_0048 = -1;
    stacked = 0;
    textInputPending = 0;
    m_1398 = 0;
    m_004c = -1;
    m_0074 = 0;
    m_139c = 0;
    m_0068 = 0;
    memset(choices, 0, sizeof(choices));
    return 1;
}

// TextWindow::SetEnabled: turns the window on or off. A stacked window is pushed on (or popped off) the window stack
// g_textWindowTop. Resets the selection state, ends text input that the window on top before the change had
// started, and flags g_textWindowChanged when the top window changed. Returns `enable`. No direct caller recovered.
int TextWindow::SetEnabled(int enable)
{
    TextWindow *previousTop = g_textWindowTop;
    if (enabled != enable)
    {
        if (enable)
        {
            if (!stacked)
            {
                g_textWindowTop = this;
                m_1398 = 0;
            }
            else
            {
                m_1398 = 0;
                below = g_textWindowTop;
                g_textWindowDepth++;
                g_textWindowTop = this;
            }
        }
        else if (stacked)
        {
            TextWindow *under = below;
            g_textWindowDepth--;
            g_textWindowTop = under;
        }
    }
    enabled = enable;
    m_0018 = 0;
    m_001c = 0;
    m_0020 = 0;
    m_004c = -1;
    m_0048 = -1;
    textInputPending = 0;
    if (previousTop != 0)
    {
        if (previousTop->textInputPending)
        {
            TextInput_End();
            previousTop->textInputPending = 0;
        }
        if (Ime_IsOpen())
        {
            TextInput_End();
            previousTop->textInputPending = 0;
        }
    }
    if (g_textWindowTop != previousTop)
        g_textWindowChanged = 1;
    m_1374 = 0;
    m_1384 = 0;
    m_1388 = 0;
    m_138c = 0;
    m_0070 = 0;
    return enable;
}

// TextWindow::AddCaptionItem: appends an item of kind TEXT_ITEM_KIND_CAPTION at (x, y) of size itemW x itemH whose
// label is a copy of `text`; a width of 0 takes the size from `text` measured at scale 0.8 (height one line)
// instead; clears the variable `result` when one is given. Returns 0. No direct caller recovered.
int TextWindow::AddCaptionItem(int *result, int x, int y, int itemW, int itemH, const char *text)
{
    if (result != 0)
        *result = 0;
    itemSource[itemCount].value = result;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    itemWidth[itemCount] = itemW;
    itemHeight[itemCount] = itemH;
    if (itemW == 0)
    {
        g_textWindowFont->MeasureWindowsText(text, TEXT_ITEM_MEASURE_SCALE, &itemW, &itemH);
        itemWidth[itemCount] = itemW;
        itemHeight[itemCount] = TEXT_ITEM_LINE_HEIGHT;
    }
    itemKind[itemCount] = TEXT_ITEM_KIND_CAPTION;
    strcpy(itemLabel[itemCount], text);
    itemCount++;
    return 0;
}
