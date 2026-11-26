// Script parser report callback of the EN build (the ListBank callback: error text and error offset). This is the one
// declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_SCRIPT_REPORT_H
#define GURUMIN_EN_SCRIPT_REPORT_H

// Codes the parser passes to its callback (ListBank::ReportError and the statement runner).
const int SCRIPT_REPORT_VALUE = 0;      // `value` is a pointer the game keeps in g_scriptReportValue
const int SCRIPT_REPORT_ERROR_OFFSET = 5000;   // `value` is the offset of the error in the script text

// Frames the script error message stays on screen.
const int SCRIPT_ERROR_FRAMES = 150;

extern "C" char g_scriptErrorText[];    // EN 0x00bc86a0: the last error text the parser reported
extern "C" int g_scriptReportValue;     // EN 0x00a1ddf4: the value of the last SCRIPT_REPORT_VALUE report

// The parser callback: stores error texts and values, and shows the "script error" message when the offset arrives.
// Returns 1. EN 0x5e2e70 (see script_report.cpp).
extern "C" int ScriptParser_Report(int code, int value);

#endif
