// Queue of scripts to run (EN build). Ported from the matched JP source decomp/src/script_queue.cpp.
#include <string.h>

#include "dialogs.h"
#include "listbank.h"
#include "script_queue.h"

// Script_RunQueue: runs every queued script in order and empties the queue: each script text is loaded into the script
// parser and its main() is compiled and run. Returns 0. Called once per frame by the main loop.
extern "C" int Script_RunQueue(void)
{
    while (g_historyNext != g_historyRead)
    {
        char *script = g_scriptTexts[g_history[g_historyRead]];
        if (script != 0)
        {
            g_scriptStateA = 0;
            g_scriptParser.LoadText(script, strlen(script));
            g_scriptStateA = 0;
            g_queuedScriptRunning = 1;
            g_scriptStateB = 0;
            g_gameBuiltinsRegistered = 0;
            g_scriptFlagByte = 0;
            g_scriptParser.CompileStatement("main();");   // the entry point of the script text
            g_scriptFlagByte = 0;
            g_gameBuiltinsRegistered = 1;
            g_queuedScriptRunning = 0;
        }
        g_historyRead = (g_historyRead + 1) & HISTORY_MASK;
    }
    return 0;
}
