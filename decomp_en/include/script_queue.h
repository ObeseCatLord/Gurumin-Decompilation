// Queue of scripts to run (EN build; JP: script_queue.cpp and script_vars.h). The queued script numbers are kept in the
// ring g_history (dialogs.h), written at g_historyNext. This is the one declaration of these globals in decomp_en:
// include this header, never redeclare them.
#ifndef GURUMIN_EN_SCRIPT_QUEUE_H
#define GURUMIN_EN_SCRIPT_QUEUE_H

// Script texts by script number (0 = no such script). EN 0x0132df6c (JP DAT_10c5e1c4).
extern "C" char *g_scriptTexts[];
// Read index of the ring g_history (the next queued script to run). EN 0x00920024 (JP DAT_104bfe84).
extern "C" int g_historyRead;
// 1 while a queued script runs. EN 0x00bbc498 (JP DAT_10492d88).
extern "C" int g_queuedScriptRunning;
// Script run state, cleared before and after a queued script is loaded and compiled (not recovered). EN 0x00bc81f4 and
// 0x00bc81ec (JP DAT_1049eae4, DAT_1049eadc).
extern "C" int g_scriptStateA;
extern "C" int g_scriptStateB;
// Script flag byte, cleared around a queued script (not recovered). EN 0x00bc8870 (JP DAT_104c0338).
extern "C" char g_scriptFlagByte;

// Runs every queued script in order and empties the queue. EN 0x5b8cb0 (JP 0x100fc3e0).
extern "C" int Script_RunQueue(void);

#endif
