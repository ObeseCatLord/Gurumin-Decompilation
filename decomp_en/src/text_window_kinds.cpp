// More item makers of a text window (EN build, EN-only: no JP twin found). The names are provisional: the kind number
// and the layout each maker gives the item are all that is known.
#include <string.h>
#include "font_renderer.h"
#include "text_window.h"

// TextWindow::AddPlainItem: appends an item of kind TEXT_ITEM_KIND_PLAIN at (x, y) whose size is `text` measured with
// the window font at scale 1.0 and whose label is a copy of `text`; clears the variable `result` when one is given.
// Returns 0. No direct caller recovered.
int TextWindow::AddPlainItem(int *result, int x, int y, const char *text)
{
    int textWidth;
    int textHeight;
    if (result != 0)
        *result = 0;
    itemSource[itemCount].value = result;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    g_textWindowFont->MeasureWindowsText(text, TEXT_LABEL_SCALE, &textWidth, &textHeight);
    itemWidth[itemCount] = textWidth;
    itemHeight[itemCount] = textHeight;
    itemKind[itemCount] = TEXT_ITEM_KIND_PLAIN;
    strcpy(itemLabel[itemCount], text);
    itemCount++;
    return 0;
}

// TextWindow::AddButton: appends an item of kind TEXT_ITEM_KIND_BUTTON at (x, y) that picks into `pickedInto`; its width
// is `text` measured at scale 0.8 plus a margin, its height is one line, and its label is a copy of `text`; clears the
// variable `result` when one is given. Returns 0. No direct caller recovered.
int TextWindow::AddButton(int *result, int *pickedInto, int x, int y, const char *text)
{
    int textWidth;
    int textHeight;
    if (result != 0)
        *result = 0;
    itemSource[itemCount].value = result;
    choiceResult[itemCount] = pickedInto;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    g_textWindowFont->MeasureWindowsText(text, TEXT_ITEM_MEASURE_SCALE, &textWidth, &textHeight);
    itemWidth[itemCount] = textWidth + TEXT_BUTTON_MARGIN;
    itemHeight[itemCount] = TEXT_ITEM_LINE_HEIGHT;
    itemKind[itemCount] = TEXT_ITEM_KIND_BUTTON;
    strcpy(itemLabel[itemCount], text);
    itemCount++;
    return 0;
}

// TextWindow::AddTextItem: appends an item of kind TEXT_ITEM_KIND_TEXT at (x, y) of size itemW x itemH that keeps the
// pointer `text`; a width of 0 takes the size from `text` measured at scale 0.8 (height one line) instead; clears the
// variable `result` when one is given. Returns 0. No direct caller recovered.
int TextWindow::AddTextItem(int *result, int x, int y, int itemW, int itemH, const char *text)
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
    itemKind[itemCount] = TEXT_ITEM_KIND_TEXT;
    itemText[itemCount] = text;
    itemCount++;
    return 0;
}

// TextWindow::AddChoiceListA: appends an empty choice list of kind TEXT_ITEM_KIND_LIST_A at (x, y) of size itemW x itemH
// with room for 256 choices (fill it with AddChoice) that picks into `pickedInto`; clears the variable `result` when
// one is given. Returns the index of the new item. No direct caller recovered.
int TextWindow::AddChoiceListA(int *result, int *pickedInto, int x, int y, int itemW, int itemH)
{
    if (result != 0)
        *result = 0;
    itemSource[itemCount].value = result;
    choiceResult[itemCount] = pickedInto;
    itemKind[itemCount] = TEXT_ITEM_KIND_LIST_A;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    itemWidth[itemCount] = itemW;
    itemHeight[itemCount] = itemH;
    choices[itemCount] = new TextWindowChoice[TEXT_LIST_CHOICE_CAPACITY];
    choiceCount[itemCount] = 0;
    itemLabel[itemCount][0] = 0;
    itemCount++;
    return itemCount - 1;
}

// TextWindow::AddChoiceListB: AddChoiceListA with kind TEXT_ITEM_KIND_LIST_B. Returns the index of the new item. No
// direct caller recovered.
int TextWindow::AddChoiceListB(int *result, int *pickedInto, int x, int y, int itemW, int itemH)
{
    if (result != 0)
        *result = 0;
    itemSource[itemCount].value = result;
    choiceResult[itemCount] = pickedInto;
    itemKind[itemCount] = TEXT_ITEM_KIND_LIST_B;
    itemX[itemCount] = x;
    itemY[itemCount] = y;
    itemWidth[itemCount] = itemW;
    itemHeight[itemCount] = itemH;
    choices[itemCount] = new TextWindowChoice[TEXT_LIST_CHOICE_CAPACITY];
    itemLabel[itemCount][0] = 0;
    choiceCount[itemCount] = 0;
    itemCount++;
    return itemCount - 1;
}
