// Game (DAT_104b7200) menu window update: the per-frame logic of the three window slots (fade, entry highlight,
// mouse and key handling of list and grid windows). Coordinates are 640x480 screen pixels.

#include "game.h"
#include "depth_sort.h"
#include "tracker.h"
#include "ui_vars.h"

#define WINDOW_SLOTS 3
#define OPACITY_FULL 0x20               // m_0098 counts up to this for the active window
#define OPACITY_BACKGROUND 0x10         // inactive open windows fade down to this
#define OPACITY_STEP 2
#define OPACITY_KEYS_ABOVE 5            // keys act only while m_0098 is above this

#define HIGHLIGHT_FULL 255
#define HIGHLIGHT_RISE 0.65f
#define HIGHLIGHT_FALL 0.2f

#define ENTRY_STATE_MASK 0xf00          // bits 8-11 of m_5db4: non-zero for an entry that cannot be chosen
#define ENTRY_KIND_TOGGLE 1             // the whole m_5db4 value is 1 for a toggle entry
#define MAX_ENTRIES 0x80
#define LABEL_SEARCH_LIMIT 0x100

#define ROW_TOP 0x14                    // first entry row below the window top
#define TAB_TOP -0x1a                   // page tabs
#define TAB_BOTTOM 4
#define PREVIOUS_TAB_LEFT 0xa0
#define NEXT_TAB_LEFT 0xf0
#define TAB_RIGHT 0x140
#define DRAG_MARGIN_ABOVE 0x6c          // a press inside the list keeps scrolling while the pointer is this far outside
#define DRAG_MARGIN_BELOW 0x94
#define CELL_MARGIN 4
#define POINTER_ROW_LEFT 8              // the pointer is put on the cursor row at this x
#define POINTER_ROW_TOP 0x18

#define KEY_REPEAT_FIRST 5              // m_0104 after a key or click acts
#define KEY_REPEAT_GRID_MOVE 2
#define KEY_REPEAT_GRID_CONFIRM 10
#define KEY_REPEAT_MIN -10
#define REPEAT_WRAP_LIMIT 2             // wrapping around is allowed while m_0108 is below this
#define MOUSE_CLICK 2

#define SOUND_CURSOR 1

// 0x1000a0b0 (Game_UpdateWindows). Advances the open windows by one frame, the one with the smallest fade first in
// the sort and handled last: its opacity (up to 32 while it is the active window, 16 behind it, down to 1 and closed
// when its slot is released), the entry highlights, and for the active window the pointer and keys. List windows
// (m_0158 = 0) move the cursor with up/down, the wheel and the pointer, scroll, turn the pages (page tabs or keys)
// and choose with select, cancel or the alternate key; grid windows move over cells that can be chosen. Choosing
// sets the variable the entry points at. m_0104 is the key repeat counter. Finally the cursor of windows that
// remember it is stored in m_0434 and the wheel state is saved in m_0000/m_0004. Returns 0.
int Game::UpdateWindows()
{
    float fades[WINDOW_SLOTS];
    int order[WINDOW_SLOTS];

    fades[0] = m_7668[0];
    fades[1] = m_7668[1];
    fades[2] = m_7668[2];
    order[0] = 0;
    order[1] = 1;
    order[2] = 2;
    FUN_10165020(fades, 0, 0, order, WINDOW_SLOTS);
    int n = WINDOW_SLOTS - 1;
    do
    {
        int w = order[n];
        if (m_008c[w] != 0)
        {
            if (m_0110[w] != 0)
            {
                if (m_0088 == w)
                {
                    if (m_0098[w] < OPACITY_FULL)
                        m_0098[w] += OPACITY_STEP;
                }
                if (m_0088 != w)
                {
                    if (m_0098[w] > OPACITY_BACKGROUND)
                        m_0098[w] -= OPACITY_STEP;
                }
            }
            else
            {
                if (m_0098[w] > 0)
                    m_0098[w] -= OPACITY_STEP;
                if (m_0098[w] < 1)
                {
                    m_0098[w] = 1;
                    m_008c[w] = 0;
                    m_00a4[w] = 0;
                    for (int entry = 0; entry < MAX_ENTRIES; entry++)
                        m_069c[w][entry][0] = 0;
                }
            }
            for (int row = 0; row < m_011c[w] + 1; row++)
            {
                int entry = m_0134[w] + row;
                int lit = (m_0088 == w && m_0140[w] == entry) ? 1 : 0;
                float *highlight = &m_7000[w][entry];
                float gap = (float)(lit * HIGHLIGHT_FULL) - *highlight;
                if (gap > 0.0f)
                    *highlight = gap * HIGHLIGHT_RISE + *highlight;
                else
                    *highlight = gap * HIGHLIGHT_FALL + *highlight;
            }
            if (m_0158[w] != 0)
            {
                if ((m_7650 == 0 || *m_7650 == 0) && m_0088 == w && m_0110[w] != 0)
                {
                    char leftClick = 0;
                    char middleClick = 0;
                    char rightClick = 0;
                    int pointer[2];
                    int wheel;
                    if (controls->up == 0 && controls->down == 0 && controls->alternate == 0 && controls->confirm == 0
                        && controls->hold == 0)
                        m_0108 = 0;
                    if (m_0104 > KEY_REPEAT_MIN)
                        m_0104--;
                    if (m_0104 - m_0108 < -1)
                    {
                        tracker->M_10001450();
                        if (tracker->M_10001440() != 0
                            && tracker->M_10001110(pointer, &leftClick, &middleClick, &rightClick, &wheel, 0) != 0
                            && pointer[0] >= m_00b0[w] && pointer[1] >= m_00bc[w])
                        {
                            if (pointer[0] <= m_0164[w] * m_0158[w] - CELL_MARGIN + m_00b0[w]
                                && pointer[1] <= m_014c[w] * m_0128[w] - CELL_MARGIN + m_00bc[w])
                            {
                                int column = (pointer[0] - m_00b0[w]) / m_0164[w];
                                int row = (pointer[1] - m_00bc[w]) / m_0128[w];
                                if ((m_5db4[w][column + m_0158[w] * row] & ENTRY_STATE_MASK) == 0)
                                {
                                    m_0170[w] = column;
                                    m_017c[w] = row;
                                }
                            }
                        }
                        int skipRows = 0;
                        int skipColumns = 0;
                        int stepRow;
                        int stepColumn;
                        for (;;)
                        {
                            stepColumn = 0;
                            stepRow = 0;
                            if (controls->up != 0 && controls->hold == 0 && m_017c[w] + skipRows > 0)
                                stepRow = -1;
                            if (controls->down != 0 && controls->hold == 0 && m_017c[w] + skipRows < m_014c[w] - 1)
                                stepRow = 1;
                            if (controls->left != 0 && controls->hold == 0 && m_0170[w] + skipColumns > 0)
                                stepColumn = -1;
                            if (controls->right == 0 || controls->hold != 0 || m_0170[w] + skipColumns >= m_0158[w] - 1)
                            {
                                if (stepColumn == 0 && stepRow == 0)
                                    goto moved;
                            }
                            else
                                stepColumn = 1;
                            m_0104 = KEY_REPEAT_GRID_MOVE;
                            tracker->M_100014a0();
                            if ((m_5db4[w][stepColumn + (m_017c[w] + stepRow + skipRows) * m_0158[w] + m_0170[w]
                                           + skipColumns]
                                 & ENTRY_STATE_MASK)
                                == 0)
                                break;
                            skipColumns += stepColumn;
                            skipRows += stepRow;
                        }
                        m_0170[w] += stepColumn + skipColumns;
                        m_017c[w] += stepRow + skipRows;
                    moved:
                        if (controls->confirm != 0 || rightClick == MOUSE_CLICK)
                        {
                            if (m_579c[w].entries[MENU_RESULT] != 0)
                                *m_579c[w].entries[MENU_RESULT] = 1;
                            m_0104 = KEY_REPEAT_GRID_CONFIRM;
                        }
                        if (controls->hold != 0 || leftClick == MOUSE_CLICK)
                        {
                            int *target = m_579c[w].entries[m_0140[w]];
                            if (target != 0)
                                *target = 1;
                            m_0104 = KEY_REPEAT_FIRST;
                        }
                    }
                }
            }
            else
            {
                m_010c = 0;
                if ((m_7650 == 0 || *m_7650 == 0) && m_0088 == w && m_0110[w] != 0)
                {
                    char leftClick = 0;
                    char middleClick = 0;
                    char rightClick = 0;
                    int pointer[2];
                    int wheel;
                    if (controls->up == 0 && controls->down == 0 && controls->alternate == 0 && controls->confirm == 0
                        && controls->hold == 0)
                    {
                        m_0108 = 0;
                        m_0104 = 0;
                    }
                    if (m_0098[w] > OPACITY_BACKGROUND)
                    {
                        tracker->M_10001450();
                        if (tracker->M_10001440() != 0
                            && tracker->M_10001110(pointer, &leftClick, &middleClick, &rightClick, &wheel, 0) != 0)
                        {
                            int last = m_00e0[w];
                            if (m_011c[w] <= m_00e0[w])
                                last = m_011c[w];
                            if (m_0194[w] != 0 && pointer[0] >= m_00b0[w] + PREVIOUS_TAB_LEFT
                                && pointer[1] >= m_00bc[w] + TAB_TOP && pointer[0] <= m_00b0[w] + NEXT_TAB_LEFT
                                && pointer[1] <= m_00bc[w] + TAB_BOTTOM)
                                m_010c = 1;
                            if (m_0194[w] != 0 && pointer[0] >= m_00b0[w] + NEXT_TAB_LEFT
                                && pointer[1] >= m_00bc[w] + TAB_TOP && pointer[0] <= m_00b0[w] + TAB_RIGHT
                                && pointer[1] <= m_00bc[w] + TAB_BOTTOM)
                                m_010c = 2;
                            if (pointer[0] >= m_00b0[w] && pointer[1] >= m_00bc[w] + ROW_TOP
                                && pointer[0] <= m_00c8[w] + m_00b0[w]
                                && pointer[1] <= m_0128[w] * (last + 1) + ROW_TOP + m_00bc[w])
                                m_068c[w] = 1;
                            if (m_068c[w] != 0 && pointer[0] >= m_00b0[w]
                                && pointer[1] >= m_00bc[w] - DRAG_MARGIN_ABOVE
                                && pointer[0] <= m_00c8[w] + m_00b0[w]
                                && pointer[1] <= (last + 1) * m_0128[w] + DRAG_MARGIN_BELOW + m_00bc[w])
                            {
                                int row = (pointer[1] - m_00bc[w] - ROW_TOP) / m_0128[w];
                                if (row < 0)
                                {
                                    m_0140[w] = m_0134[w];
                                    if (m_011c[w] < MAX_ENTRIES && m_0134[w] > 0)
                                    {
                                        m_0134[w]--;
                                        m_0140[w]--;
                                        if (m_0140[w] - m_0134[w] > m_011c[w])
                                            m_0140[w]--;
                                    }
                                }
                                if (row > last)
                                {
                                    m_0140[w] = last + m_0134[w];
                                    if (m_011c[w] < MAX_ENTRIES && m_069c[w][m_0134[w] + m_011c[w] + 1][0] != 0)
                                    {
                                        m_0134[w]++;
                                        m_0140[w]++;
                                        if (m_0140[w] - m_0134[w] < 0)
                                            m_0140[w]++;
                                    }
                                }
                            }
                            if (pointer[0] >= m_00b0[w] && pointer[1] >= m_00bc[w] + ROW_TOP
                                && pointer[0] <= m_00c8[w] + m_00b0[w]
                                && pointer[1] <= (last + 1) * m_0128[w] + ROW_TOP + m_00bc[w])
                            {
                                int top = m_0134[w];
                                m_0140[w] = (pointer[1] - m_00bc[w] - ROW_TOP) / m_0128[w] + top;
                                if (last < m_0140[w] - top)
                                    m_0140[w] = top + last;
                            }
                            if (wheel > 0)
                            {
                                if (m_011c[w] < MAX_ENTRIES && m_0134[w] > 0)
                                {
                                    m_0134[w]--;
                                    m_0140[w]--;
                                    if (m_0140[w] - m_0134[w] > m_011c[w])
                                        m_0140[w]--;
                                }
                                else if (m_00f8[w] == 0 && m_0140[w] > 0)
                                {
                                    m_0140[w]--;
                                    tracker->M_10001090(m_00b0[w] + POINTER_ROW_LEFT,
                                                        (m_0140[w] - m_0134[w]) * m_0128[w] + POINTER_ROW_TOP
                                                            + m_00bc[w]);
                                }
                            }
                            if (wheel < 0)
                            {
                                if (m_011c[w] < MAX_ENTRIES && m_069c[w][m_0134[w] + m_011c[w] + 1][0] != 0)
                                {
                                    m_0134[w]++;
                                    m_0140[w]++;
                                    if (m_0140[w] - m_0134[w] < 0)
                                        m_0140[w]++;
                                }
                                else if (m_00f8[w] == 0 && m_069c[w][m_0140[w] + 1][0] != 0)
                                {
                                    m_0140[w]++;
                                    tracker->M_10001090(m_00b0[w] + POINTER_ROW_LEFT,
                                                        (m_0140[w] - m_0134[w]) * m_0128[w] + POINTER_ROW_TOP
                                                            + m_00bc[w]);
                                }
                            }
                        }
                    }
                    if (m_0104 > KEY_REPEAT_MIN)
                        m_0104--;
                    if (m_0104 < 0 && m_0098[w] > OPACITY_BACKGROUND)
                    {
                        if (m_0098[w] > OPACITY_KEYS_ABOVE && controls->up != 0 && controls->hold == 0)
                        {
                            if (m_0140[w] > 0)
                            {
                                (*m_7660)(SOUND_CURSOR);
                                m_0104 = KEY_REPEAT_FIRST;
                                m_0140[w]--;
                                if (m_0140[w] - m_0134[w] < 0)
                                    m_0134[w]--;
                            }
                            else
                            {
                                if (m_0108 < REPEAT_WRAP_LIMIT)
                                {
                                    int last = 0;
                                    while (last < LABEL_SEARCH_LIMIT && m_069c[w][last + 1][0] != 0)
                                        last++;
                                    (*m_7660)(SOUND_CURSOR);
                                    m_0104 = KEY_REPEAT_FIRST;
                                    m_0140[w] = last;
                                    while (m_0140[w] - m_0134[w] > m_011c[w])
                                        m_0134[w]++;
                                    while (m_0140[w] - m_0134[w] < 0)
                                        m_0134[w]--;
                                }
                            }
                            tracker->M_100014a0();
                        }
                        if (m_0104 < 0)
                        {
                            if (m_0098[w] > OPACITY_KEYS_ABOVE && controls->down != 0 && controls->hold == 0)
                            {
                                if (m_069c[w][m_0140[w] + 1][0] != 0)
                                {
                                    (*m_7660)(SOUND_CURSOR);
                                    m_0104 = KEY_REPEAT_FIRST;
                                    m_0140[w]++;
                                    if (m_0140[w] - m_0134[w] > m_011c[w])
                                        m_0134[w]++;
                                }
                                else
                                {
                                    if (m_0108 < REPEAT_WRAP_LIMIT)
                                    {
                                        (*m_7660)(SOUND_CURSOR);
                                        m_0104 = KEY_REPEAT_FIRST;
                                        m_0140[w] = 0;
                                        while (m_0140[w] - m_0134[w] > m_011c[w])
                                            m_0134[w]++;
                                        while (m_0140[w] - m_0134[w] < 0)
                                            m_0134[w]--;
                                    }
                                }
                                tracker->M_100014a0();
                            }
                            if (m_0104 < 0)
                            {
                                if (m_010c == 0 && m_0098[w] > OPACITY_KEYS_ABOVE
                                    && (controls->select != 0 || leftClick == MOUSE_CLICK))
                                {
                                    if ((m_5db4[w][m_0140[w]] & ENTRY_STATE_MASK) == 0)
                                    {
                                        int *target = m_579c[w].entries[m_0140[w]];
                                        if (m_5db4[w][m_0140[w]] == ENTRY_KIND_TOGGLE)
                                        {
                                            if (target != 0)
                                                *target = (*target == 0);
                                        }
                                        else if (target != 0)
                                            *target = 1;
                                        m_0104 = KEY_REPEAT_FIRST;
                                    }
                                }
                                if (m_0104 < 0)
                                {
                                    int *page = m_0194[w];
                                    if (page != 0 && m_0098[w] > OPACITY_KEYS_ABOVE && controls->previousPage != 0
                                        && *page > m_01ac[w])
                                    {
                                        (*page)--;
                                        if (m_0188[w] != 0)
                                            *m_0188[w] = 1;
                                        m_0104 = KEY_REPEAT_FIRST;
                                    }
                                    if (m_0104 < 0)
                                    {
                                        page = m_0194[w];
                                        if (page != 0 && m_0098[w] > OPACITY_KEYS_ABOVE && controls->nextPage != 0
                                            && *page < m_01a0[w])
                                        {
                                            (*page)++;
                                            if (m_0188[w] != 0)
                                                *m_0188[w] = 1;
                                            m_0104 = KEY_REPEAT_FIRST;
                                        }
                                        if (m_0104 < 0)
                                        {
                                            if (m_010c == 1 && m_0098[w] > OPACITY_KEYS_ABOVE
                                                && (controls->select != 0 || leftClick == MOUSE_CLICK)
                                                && (page = m_0194[w]) != 0 && *page > m_01ac[w])
                                            {
                                                (*page)--;
                                                if (m_0188[w] != 0)
                                                    *m_0188[w] = 1;
                                                m_0104 = KEY_REPEAT_FIRST;
                                            }
                                            if (m_0104 < 0)
                                            {
                                                if (m_010c == 2 && m_0098[w] > OPACITY_KEYS_ABOVE
                                                    && (controls->select != 0 || leftClick == MOUSE_CLICK)
                                                    && (page = m_0194[w]) != 0 && *page < m_01a0[w])
                                                {
                                                    (*page)++;
                                                    if (m_0188[w] != 0)
                                                        *m_0188[w] = 1;
                                                    m_0104 = KEY_REPEAT_FIRST;
                                                }
                                                if (m_0104 < 0)
                                                {
                                                    if (m_0098[w] > OPACITY_KEYS_ABOVE
                                                        && (controls->cancel != 0 || rightClick == MOUSE_CLICK))
                                                    {
                                                        if (m_579c[w].entries[MENU_RESULT] != 0)
                                                            *m_579c[w].entries[MENU_RESULT] = 1;
                                                        m_0104 = KEY_REPEAT_FIRST;
                                                    }
                                                    if (m_0104 < 0 && m_010c == 0 && m_0098[w] > OPACITY_KEYS_ABOVE
                                                        && controls->alternate != 0)
                                                    {
                                                        int *alternate = m_579c[w].entries[MENU_RESULT + 1];
                                                        if (alternate != 0)
                                                            *alternate = 1;
                                                        m_0104 = KEY_REPEAT_FIRST;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (m_01dc[m_00a4[w]] != 0)
            m_0434[m_00a4[w]] = (char)m_0140[w];
        n--;
    } while (n >= 0);
    m_0000 = DAT_10f657b8;
    m_0004 = DAT_10f6555c;
    return 0;
}
