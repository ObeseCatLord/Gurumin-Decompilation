// EN-only launcher/config dialog ("Gurumin Ver.1.4 (STEAM)", Graphics / Audio settings), Falcom code on top of static MFC.
// This header holds the pieces that need no MFC types; the MFC dialog classes are in launcher_dialog.h (the only header
// that includes the MFC headers), which includes this one.
#ifndef GURUMIN_EN_LAUNCHER_H
#define GURUMIN_EN_LAUNCHER_H

// First list index that is skipped when the dialog converts between a list-box row and the stored setting value
// (two settings between them are not offered in the list).
const int LAUNCHER_SKIPPED_LIST_FIRST = 7;
// Number of consecutive stored values that the list does not show, starting at LAUNCHER_SKIPPED_LIST_FIRST.
const int LAUNCHER_SKIPPED_LIST_COUNT = 2;

// Converts a stored setting value into the list-box row that shows it (values from 7 on sit two rows lower).
// Called by the launcher dialog when it fills its drop-down lists. EN 0x006eb5f0.
extern "C" int Launcher_SettingToListRow(int setting);
// Converts a list-box row into the stored setting value (rows from 7 on are two settings higher).
// Called by the launcher dialog when it saves its settings. EN 0x006eb5e0.
extern "C" int Launcher_ListRowToSetting(int row);

// Writes the launcher settings back to the configuration file. Stubbed out in the Steam build (the settings are
// written elsewhere): it only returns 0. Called by the main dialog's Start, Close, Cancel and close-box handlers.
// EN 0x006efc30.
extern "C" int Launcher_SaveSettings();

// Dispatches up to 30 pending window messages. Returns 1 when the queue is drained (or the limit is hit) and 0 after
// a WM_QUIT, which it also logs to a file. Called by the launcher while it waits. EN 0x006e8fa0.
extern "C" int Launcher_PumpMessages(void);

#endif
