// Script parser report callback (EN build, EN-only: no JP twin found; the JP callback is FUN_10017f20).
#include <stdio.h>
#include <string.h>
#include "game_vars.h"
#include "script_report.h"

// "%s(%d)": error text and offset (the parentheses are full-width).
#define SCRIPT_ERROR_DETAIL_FORMAT "\x25\x73\x81\x69\x25\x64\x81\x6a"
// Title of the on-screen error message: "Script error occurred (UPIPE)" in Shift-JIS.
#define SCRIPT_ERROR_TITLE "\x83\x58\x83\x4e\x83\x8a\x83\x76\x83\x67\x83\x47\x83\x89\x81\x5b\x94\xad\x90\xb6\x81\x69\x82\x74\x82\x6f\x82\x68\x82\x6f\x82\x64\x81\x6a"
// Bytes of the line of detail text shown under the title.
const int SCRIPT_ERROR_DETAIL_SIZE = 0x100;

// ScriptParser_Report: callback of the script parser. With SCRIPT_REPORT_ERROR_OFFSET it shows the last error text and
// the offset under the script error title; with SCRIPT_REPORT_VALUE it stores `value`; every other code copies the
// string `value` points to (the error text) into g_scriptErrorText. Returns 1.
extern "C" int ScriptParser_Report(int code, int value)
{
    if (code == SCRIPT_REPORT_ERROR_OFFSET)
    {
        char detail[SCRIPT_ERROR_DETAIL_SIZE];
        sprintf(detail, SCRIPT_ERROR_DETAIL_FORMAT, g_scriptErrorText, value);
        Message_Show(SCRIPT_ERROR_TITLE, detail, SCRIPT_ERROR_FRAMES);
        return 1;
    }
    if (code == SCRIPT_REPORT_VALUE)
    {
        g_scriptReportValue = value;
        return 1;
    }
    strcpy(g_scriptErrorText, (const char *)value);
    return 1;
}
