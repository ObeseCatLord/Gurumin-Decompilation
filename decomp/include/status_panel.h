// The status panel at the lower right (drawn by FUN_1001e340 in status_panel.cpp, filled by StatusPanel_UpdateText).
#ifndef GURUMIN_STATUS_PANEL_H
#define GURUMIN_STATUS_PANEL_H

extern "C" int DAT_1020c334;    // the map record the panel text was built for, -1 = none

extern "C" int FUN_1001e4b0(int recordFlag, int notCleared);  // 0x1001e4b0

#endif
