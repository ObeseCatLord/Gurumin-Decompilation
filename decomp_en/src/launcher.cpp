// EN-only launcher/config dialog: the functions that need no MFC types.
#include "launcher.h"

// Converts a list-box row into the stored setting value (rows from 7 on skip two settings).
// Called by the launcher dialog when it saves its settings.
extern "C" int Launcher_ListRowToSetting(int row)
{
    if (row >= LAUNCHER_SKIPPED_LIST_FIRST) {
        row += LAUNCHER_SKIPPED_LIST_COUNT;
    }
    return row;
}

// Converts a stored setting value into the list-box row that shows it (values from 7 on sit two rows lower).
// Called by the launcher dialog when it fills its drop-down lists.
extern "C" int Launcher_SettingToListRow(int setting)
{
    return setting >= LAUNCHER_SKIPPED_LIST_FIRST ? setting - LAUNCHER_SKIPPED_LIST_COUNT : setting;
}

// Writes the launcher settings back to the configuration file. Stubbed out in the Steam build (the settings are
// written elsewhere), so it only returns 0. Called by the main dialog's Start, Close, Cancel and close-box handlers.
extern "C" int Launcher_SaveSettings()
{
    return 0;
}
