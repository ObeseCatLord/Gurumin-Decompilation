// EN-only launcher: the application object (CLauncherApp) and the launcher's message pump. Falcom code on top of static
// MFC; see NOTES.md "Launcher (MFC)". InitInstance (EN 0x006e7470) is not reconstructed yet.
#include "launcher_dialog.h"
#include <stdio.h>

// The one application object; MFC finds it through this global when the exe starts.
CLauncherApp theApp;

// Builds the application object (MFC names it after the executable).
CLauncherApp::CLauncherApp()
{
}

BEGIN_MESSAGE_MAP(CLauncherApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

const int PUMP_MESSAGE_LIMIT = 30;   // messages dispatched per call before the pump hands control back

// Path of the log file the pump writes when it sees WM_QUIT, the mode it opens it with and the text it writes.
static const char QUIT_LOG_PATH[] = "c:\\gya.txt";
static const char QUIT_LOG_MODE[] = "wt";
static const char QUIT_LOG_TEXT[] = "wm_quit";

// Dispatches up to PUMP_MESSAGE_LIMIT pending messages so the launcher window stays responsive while it works.
// After a WM_QUIT it writes a note to the log file and returns 0; otherwise returns 1.
extern "C" int Launcher_PumpMessages(void)
{
    MSG msg;
    int dispatched = 0;

    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) && dispatched < PUMP_MESSAGE_LIMIT) {
        dispatched++;
        if (msg.message == WM_QUIT) {
            FILE *log = fopen(QUIT_LOG_PATH, QUIT_LOG_MODE);
            fprintf(log, QUIT_LOG_TEXT);
            fclose(log);
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 1;
}
