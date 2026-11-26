// Items and choices of a text window (EN build, EN-only: no JP twin found).
#include <string.h>
#include "font_renderer.h"
#include "text_window.h"

// TextWindow::AddItem: appends an item of kind `kind` at (x, y) with `choices` list entries, `selected` as the picked
// entry and the size itemW x itemH; clears the variable `result` when one is given. Returns 0. No direct caller
// recovered.
int TextWindow::AddItem(int *result, int x, int y, int kind, int choices, int selected, int itemW, int itemH)
{
    if (result != 0)
        *result = 0;
    itemSource[itemCount].value = result;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    choiceCount[itemCount] = choices;
    selectedChoice[itemCount] = selected;
    itemWidth[itemCount] = itemW;
    itemHeight[itemCount] = itemH;
    itemKind[itemCount] = kind;
    itemCount++;
    return 0;
}

// TextWindow::AddPanel: appends a panel item at (x, y) of size itemW x itemH, carrying `param`. Returns 0. No direct
// caller recovered.
int TextWindow::AddPanel(int param, int x, int y, int itemW, int itemH)
{
    itemSource[itemCount].value = 0;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    choiceCount[itemCount] = 0;
    selectedChoice[itemCount] = 0;
    itemWidth[itemCount] = itemW;
    itemHeight[itemCount] = itemH;
    itemKind[itemCount] = TEXT_ITEM_KIND_PANEL;
    itemParam[itemCount] = param;
    itemCount++;
    return 0;
}

// TextWindow::AddLabel: appends a label item showing `text` at (x, y); its size is the text measured with the window
// font and its label is a copy of the text. Returns 0. No direct caller recovered.
int TextWindow::AddLabel(const char *text, int x, int y)
{
    int textWidth;
    int textHeight;
    itemSource[itemCount].text = text;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    g_textWindowFont->MeasureWindowsText(text, TEXT_LABEL_SCALE, &textWidth, &textHeight);
    itemWidth[itemCount] = textWidth;
    itemHeight[itemCount] = textHeight;
    itemKind[itemCount] = TEXT_ITEM_KIND_LABEL;
    strcpy(itemLabel[itemCount], text);
    itemCount++;
    return 0;
}

// TextWindow::AddChoice: appends the choice (`text`, `value`) to the list of item `item`. The first choice becomes the
// item's label and the value of its result variable, and the selection is reset to it. Returns the index the choice
// got. No direct caller recovered.
int TextWindow::AddChoice(int item, const char *text, int value)
{
    TextWindowChoice *choice = choices[item];
    int index = choiceCount[item];
    choice += index;
    strcpy(choice->text, text);
    choice->value = value;
    if (choiceCount[item] == 0)
    {
        selectedChoice[item] = 0;
        strcpy(itemLabel[item], text);
        *choiceResult[item] = value;
        choiceCount[item]++;
        return index;
    }
    choiceCount[item]++;
    return index;
}
