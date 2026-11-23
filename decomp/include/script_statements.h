// Statement kinds ListBank::CompileStatement returns (besides 0 = error and 1 = nothing to execute: a label, a
// declaration or a call that was compiled). RunStatements acts on them.
#ifndef GURUMIN_SCRIPT_STATEMENTS_H
#define GURUMIN_SCRIPT_STATEMENTS_H

#define STATEMENT_IF 10
#define STATEMENT_FOR 11
#define STATEMENT_WHILE 12
#define STATEMENT_SWITCH 13
#define STATEMENT_BREAK 14
#define STATEMENT_CONTINUE 15
#define STATEMENT_RETURN 16

// ListBank::flow: how the statements that just ran ended.
#define FLOW_NORMAL 0
#define FLOW_BREAK 1
#define FLOW_CONTINUE 2
#define FLOW_RETURN 3

#endif
