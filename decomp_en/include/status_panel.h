// The status panel drawn over the screen's lower right corner (EN build; JP: status_panel.cpp). This is the one
// declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_STATUS_PANEL_H
#define GURUMIN_EN_STATUS_PANEL_H

// Text lines of the panel and characters of each.
const int STATUS_PANEL_LINE_COUNT = 8;
const int STATUS_PANEL_LINE_SIZE = 0x50;

// Panel state, 0 = hidden; the text shows in states 8..22. EN 0x00916fe0 (JP DAT_1020c330).
extern "C" int g_statusPanelState;
// The panel's text lines. EN 0x00918660 (JP DAT_1020da50).
extern "C" char g_statusPanelLines[STATUS_PANEL_LINE_COUNT][STATUS_PANEL_LINE_SIZE];
// When 4 or more, the last line is yellow. EN 0x00918890 (JP DAT_1020dc80).
extern "C" char g_statusPanelLevel;

// Draws the panel with the fade `alpha`. EN 0x5cefe0 (JP 0x1001e340).
extern "C" int StatusPanel_Draw(float alpha);

#endif
