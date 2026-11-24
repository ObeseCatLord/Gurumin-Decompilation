// EN-only launcher/config dialog ("Gurumin Ver.1.4 (STEAM)", Graphics / Audio settings), Falcom code on top of static MFC.
// MFC itself is library code and is not declared here: the VS2013 install on the build machine (WDExpress) ships no
// atlmfc headers, so only the pieces that do not need MFC types are reconstructed (see launcher.cpp).
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

#endif
