// The script interpreter's statement runner.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listbank.h"
#include "script_statements.h"

// "ネストがない" ("there is no nest") followed by "break" / "continue", Shift-JIS.
#define NO_NEST_BREAK_TEXT "\x83l\x83X\x83g\x82\xaa\x82\xc8\x82\xa2" "break\x81" "B"
#define NO_NEST_CONTINUE_TEXT "\x83l\x83X\x83g\x82\xaa\x82\xc8\x82\xa2" "continue\x81" "B"

// Wanted result type meaning "leave the value as it is".
#define KEEP_RESULT_TYPE -1
// The label a switch runs when no case matches.
#define DEFAULT_LABEL "default"
// The word that continues an `if`.
#define ELSE_WORD "else"

// Whether the value the last expression left in `result` counts as true: a number other than 0, or text that
// reads as a number other than 0.
static __forceinline int ResultIsTrue(ListBank *bank)
{
    if (bank->resultType == 0)
        return *bank->result != 0;
    if (bank->resultType == 1)
        return *(float *)bank->result != 0.0f;
    if (bank->resultType == 2)
        return atol((char *)bank->result) != 0;
    return 0;
}

// 0x10089210 (ListBank::RunStatements). Runs the statements of the script text at cursor[depth], one nesting level
// deeper than the caller's (the level is released again on return). It ends at the closing brace of the block
// (`openBraces` is the number of braces already open: 0 when the caller is at a statement, which then ends at a
// `;` or after the first brace block), or at the end of the text. `if`, `for`, `while` and `switch` run their
// bodies by calling it again; break, continue and return are left to the caller in `flow` (break and continue
// outside a loop report an error when `wantType` is given: the nested block of a switch or loop passes
// KEEP_RESULT_TYPE). On leaving through `return` (or at the end of the block), the value in `result` is
// converted to `wantType` (0 = int, 1 = float, 2 = quoted text) unless that is KEEP_RESULT_TYPE. Returns 0.
int ListBank::RunStatements(char wantType, int openBraces)
{
    char statement[0x100];
    char word[0x100];
    int condition;
    int braces;
    int sawBlock;
    compiling = 1;
    flow = FLOW_NORMAL;
    braces = 0;
    sawBlock = 0;
    if (openBraces != 0)
    {
        braces = openBraces;
        sawBlock = 1;
    }
    condition = 0;
    while (cursor[depth] < text + textLength)
    {
        if (failed != 0)
            return 0;
        ReadStatement(&cursor[depth], statement);
        if (callback != 0)
            callback(0, cursor[depth] - text);
        if (statement[0] != 0)
        {
            if (statement[0] == '{')
            {
                braces++;
                sawBlock = 1;
            }
            else
            {
                if (braces == 0)
                {
                    if (statement[0] == ';')
                        goto finish;
                }
                else if (statement[0] == ';')
                    goto nextStatement;
                if (statement[0] == '}')
                    braces--;
                else
                {
                    int kind = CompileStatement(statement);
                    if (kind == STATEMENT_RETURN)
                    {
                        CompileCall(statementParts[depth][0], 0);
                        flow = FLOW_RETURN;
                        goto finish;
                    }
                    if (kind == STATEMENT_BREAK)
                    {
                        if (wantType == KEEP_RESULT_TYPE)
                        {
                            flow = FLOW_BREAK;
                            goto leave;
                        }
                        ReportError(NO_NEST_BREAK_TEXT);
                    }
                    else if (kind == STATEMENT_CONTINUE)
                    {
                        if (wantType == KEEP_RESULT_TYPE)
                        {
                            flow = FLOW_CONTINUE;
                            goto leave;
                        }
                        ReportError(NO_NEST_CONTINUE_TEXT);
                    }
                    else if (kind == STATEMENT_SWITCH)
                    {
                        char *found;
                        CompileCall(statementParts[depth][0], 0);
                        if (resultType == 0)
                            condition = *result;
                        else if (resultType == 1)
                            condition = (int)*(float *)result;
                        else if (resultType == 2)
                            condition = atol((char *)result);
                        sprintf(word, "%d", condition);
                        if (FindCase(word, cursor[depth], &found) == 0)
                        {
                            sprintf(word, DEFAULT_LABEL);
                            if (FindLabel(word, cursor[depth], &found) != 0)
                                goto runCase;
                        }
                        else
                        {
                        runCase:
                            depth++;
                            cursor[depth] = found;
                            RunStatements(KEEP_RESULT_TYPE, 1);
                            if (failed != 0)
                                return 0;
                            if (flow == FLOW_RETURN)
                                goto finish;
                        }
                        flow = FLOW_NORMAL;
                        SkipStatement(&cursor[depth]);
                    }
                    else if (kind == STATEMENT_WHILE)
                    {
                        for (;;)
                        {
                            if (failed != 0)
                                return 0;
                            CompileCall(statementParts[depth][0], 0);
                            condition = ResultIsTrue(this);
                            if (condition == 0)
                                break;
                            depth++;
                            cursor[depth] = cursor[depth - 1];
                            RunStatements(KEEP_RESULT_TYPE, 0);
                            if (failed != 0)
                                return 0;
                            if (flow == FLOW_RETURN)
                                goto finish;
                            if (flow == FLOW_BREAK)
                                break;
                            flow = FLOW_NORMAL;
                        }
                        flow = FLOW_NORMAL;
                        SkipStatement(&cursor[depth]);
                    }
                    else if (kind == STATEMENT_FOR)
                    {
                        CompileCall(statementParts[depth][0], 0);
                        for (;;)
                        {
                            if (failed != 0)
                                return 0;
                            CompileCall(statementParts[depth][1], 0);
                            condition = ResultIsTrue(this);
                            if (condition == 0)
                                break;
                            depth++;
                            cursor[depth] = cursor[depth - 1];
                            RunStatements(KEEP_RESULT_TYPE, 0);
                            if (failed != 0)
                                return 0;
                            if (flow == FLOW_RETURN)
                                goto finish;
                            CompileCall(statementParts[depth][2], 0);
                            if (flow == FLOW_BREAK)
                                break;
                            flow = FLOW_NORMAL;
                        }
                        flow = FLOW_NORMAL;
                        SkipStatement(&cursor[depth]);
                    }
                    else if (kind == STATEMENT_IF)
                    {
                        CompileCall(statementParts[depth][0], 0);
                        condition = ResultIsTrue(this);
                        if (condition != 0)
                        {
                            depth++;
                            cursor[depth] = cursor[depth - 1];
                            RunStatements(KEEP_RESULT_TYPE, 0);
                            if (failed != 0)
                                return 0;
                            if (flow == FLOW_RETURN)
                                goto finish;
                            if (flow == FLOW_BREAK)
                            {
                                if (wantType == KEEP_RESULT_TYPE)
                                {
                                    flow = FLOW_BREAK;
                                    goto leave;
                                }
                                ReportError(NO_NEST_BREAK_TEXT);
                            }
                            else if (flow == FLOW_CONTINUE)
                            {
                                if (wantType == KEEP_RESULT_TYPE)
                                {
                                    flow = FLOW_CONTINUE;
                                    goto leave;
                                }
                                ReportError(NO_NEST_CONTINUE_TEXT);
                            }
                        }
                        SkipStatement(&cursor[depth]);
                        char *rest = cursor[depth];
                        ReadWord(&rest, word);
                        if (strcmp(word, ELSE_WORD) == 0)
                        {
                            cursor[depth] = rest;
                            if (condition == 0)
                            {
                                depth++;
                                cursor[depth] = cursor[depth - 1];
                                RunStatements(KEEP_RESULT_TYPE, 0);
                                if (failed != 0)
                                    return 0;
                                if (flow == FLOW_RETURN)
                                    goto finish;
                                if (flow == FLOW_BREAK)
                                {
                                    if (wantType == KEEP_RESULT_TYPE)
                                    {
                                        flow = FLOW_BREAK;
                                        goto leave;
                                    }
                                    ReportError(NO_NEST_BREAK_TEXT);
                                }
                                else if (flow == FLOW_CONTINUE)
                                {
                                    if (wantType == KEEP_RESULT_TYPE)
                                    {
                                        flow = FLOW_CONTINUE;
                                        goto leave;
                                    }
                                    ReportError(NO_NEST_CONTINUE_TEXT);
                                }
                            }
                            SkipStatement(&cursor[depth]);
                        }
                    }
                }
            }
        }
    nextStatement:
        if (braces == 0 && sawBlock != 0)
            break;
    }
finish:
    if (wantType != KEEP_RESULT_TYPE && wantType != resultType)
    {
        if (wantType == 0)
        {
            if (resultType == 1)
                *result = (int)*(float *)result;
            else if (resultType == 2)
                *result = atol((char *)result + 1);
        }
        else if (wantType == 1)
        {
            if (resultType == 0)
                *(float *)result = (float)*result;
            else if (resultType == 2)
                *(float *)result = (float)atof((char *)result + 1);
        }
        else if (wantType == 2)
        {
            if (resultType == 0)
                sprintf((char *)result, "\"%d\"", *result);
            else if (resultType == 1)
                sprintf((char *)result, "\"%f\"", *(float *)result);
        }
        resultType = wantType;
    }
leave:
    FreeBank(depth);
    depth--;
    return 0;
}
