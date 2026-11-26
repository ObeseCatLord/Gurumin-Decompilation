// EN twin of decomp/include/listbank.h (same layout, checked on the EN functions ported so far).
// The script parser (JP DAT_104687b8, also seen as ResourceBanks and TokenReader: one object). It holds the declared
// script variables in 8 nesting levels ("groups") of up to 0x80 named entries, the registered builtin functions,
// the script text being parsed with one read cursor per nesting level, and an error flag that makes every reader
// fail once an error was reported. Methods: listbank.cpp, listbank_vars.cpp (FindVariable, RegisterBuiltin),
// resource_banks.cpp (FreeBank, FreeAll), type_parse.cpp (ReadType).
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_EN_LISTBANK_H
#define GURUMIN_EN_LISTBANK_H

#include <stdio.h>


struct ListBank;

// A value produced by the expression evaluator: an int (type 0), a float (type 1) or quoted text (type 2, the text
// still starts with its opening quote).
struct ScriptValue
{
    char text[0x100];
    int &i() { return *(int *)text; }
    float &f() { return *(float *)text; }
};

// Callback the host installs to receive parser messages: (1, message text) for errors, (5000, text offset) for
// the position of the error in the script text.
typedef void (*ParserCallback)(int code, int value);

// One argument of a builtin call: the evaluator passes a pointer to the argument's value (an int or a float), or the
// text itself for a string argument. The array of arguments has one entry per declared parameter.
union ScriptArgument
{
    int *i;
    float *f;
    char *text;
};

// A builtin script function: called with the builtin's id, its arguments and the parser (the result of the call
// is stored in bank->result, see resultType).
typedef void (*BuiltinHandler)(int id, ScriptArgument *args, ListBank *bank);

// Number of nesting levels (variable groups) and of entries per level.
#define LISTBANK_GROUP_COUNT 8
#define LISTBANK_GROUP_ENTRIES 0x80
// Size of one element of a text variable.
#define LISTBANK_TEXT_SIZE 0x100
// ReadToken: token[TOKEN_IS_OPERATOR] = 1 when the token read is an operator; the kind of token read last.
#define TOKEN_IS_OPERATOR 0xff
#define TOKEN_OPERAND 0
#define TOKEN_OPERATOR 1
#define TOKEN_START 2
// Longest statement or word the readers copy (the buffers hold this many characters plus two zero bytes).
#define READ_BUFFER_LIMIT 0x100
// Operand slots of the expression evaluator: a pair (left, right) per nesting level, at slot level * 16.
#define LISTBANK_OPERANDS_PER_LEVEL 16
#define LISTBANK_OPERAND_SLOTS (LISTBANK_GROUP_COUNT * LISTBANK_OPERANDS_PER_LEVEL)
// Value types: an int, a float, quoted text.
#define SCRIPT_TYPE_INT 0
#define SCRIPT_TYPE_FLOAT 1
#define SCRIPT_TYPE_TEXT 2

// Ids of the library builtins InitLibrary registers (all run by RunLibraryBuiltin, which switches on the id).
enum LibraryBuiltinId
{
    BUILTIN_SIN = 0x2774,
    BUILTIN_COS,
    BUILTIN_TAN,
    BUILTIN_ATAN,
    BUILTIN_ATAN2,
    BUILTIN_SQRT,
    BUILTIN_RAND,
    BUILTIN_ABS,
    BUILTIN_FABS,
    BUILTIN_ITOF,
    BUILTIN_FTOI,
    BUILTIN_ASIN,
    BUILTIN_ACOS,
    BUILTIN_SINH,
    BUILTIN_COSH,
    BUILTIN_TANH,
    BUILTIN_POW,
    BUILTIN_CEIL,
    BUILTIN_FLOOR,
    BUILTIN_EXP,
    BUILTIN_LOG,
    BUILTIN_LOG10,
    BUILTIN_MAX,
    BUILTIN_MIN,
    BUILTIN_STRCMP
};

// Runs the library builtins (math functions, rand, strcmp); the handler registered for every id above.
extern "C" void RunLibraryBuiltin(int id, ScriptArgument *args, ListBank *bank);   // EN 0x00647ad0

// A top-level `{ ... }` block of a script (its start in the text).
struct ScriptBlock
{
    char *start;              // first character after the opening brace
    char *end;                // first character after the closing brace
};

// The value slot of a declared variable: a constant's value, or the storage allocated for a variable's elements.
union ScriptVariableValue
{
    int value;                                // a constant's value (AppendEntry)
    int *ints;                                // int variable: one int per element
    float *floats;                            // float variable: one float per element
    char (*texts)[LISTBANK_TEXT_SIZE];        // text variable: LISTBANK_TEXT_SIZE characters per element
    void *storage;                            // the storage as allocated (FreeBank frees it unless borrowed)
};

// What a registered function runs: a builtin has a handler, a function declared in the script has its body text.
union ScriptFunction
{
    BuiltinHandler handler;
    char *body;               // first character after the parameter list (builtinIds = -1)
};

struct ListBank
{
    union
    {
        int *result;          // 0x00: where the last evaluated expression left its value (resultBuffer)
        char *resultText;     // 0x00: the same when the value is text
    };
    char resultType;          // 0x04: type of *result: 0 = int, 1 = float, 2 = quoted text
    char pad_0005[3];
    ParserCallback callback;  // 0x08: message callback (may be 0)
    char variableNames[LISTBANK_GROUP_COUNT][LISTBANK_GROUP_ENTRIES][0x10];  // 0x0c: declared variable names
    char variableTypes[LISTBANK_GROUP_COUNT][LISTBANK_GROUP_ENTRIES];        // 0x400c: 0 = int, 1 = float, 2 = text
    ScriptVariableValue values[LISTBANK_GROUP_COUNT][0x100];                 // 0x440c: value or storage of each
    int sizes[LISTBANK_GROUP_COUNT][LISTBANK_GROUP_ENTRIES][8];              // 0x640c: size of each array dimension
    char dimensions[LISTBANK_GROUP_COUNT][LISTBANK_GROUP_ENTRIES];           // 0xe40c: array dimensions (0 = none)
    char borrowed[LISTBANK_GROUP_COUNT][LISTBANK_GROUP_ENTRIES];             // 0xe80c: 1 = the value is not owned
                                                                             // (constants; FreeBank keeps it)
    char builtinNames[0x100][0x20];       // 0xec0c: names of the registered builtin functions
    char builtinReturnTypes[0x100];       // 0x10c0c: 0 = int, 1 = float, 2 = string
    char builtinParamTypes[0x100][0x10];  // 0x10d0c: parameter types (+10 = by reference, 0xff = end)
    short builtinIds[0x100];              // 0x11d0c
    ScriptFunction builtinHandlers[0x100];// 0x11f0c: the handler of a builtin, the body text of a script function
    char builtinParamNames[0x100][0x10][0x10];  // 0x1230c: parameter names of the declared script functions
    char builtinParamCounts[0x100];       // 0x2230c
    union
    {
        int resultBuffer[0x2070];             // 0x2240c: where Evaluate leaves its result (`result` points here)
        struct
        {
            ScriptValue resultValue;                          // 0x2240c: the same buffer as a value
            char operandTypes[LISTBANK_OPERAND_SLOTS];        // 0x2250c: type of each operand (ApplyOperator)
            ScriptValue operands[LISTBANK_OPERAND_SLOTS];     // 0x2258c: the operands, LISTBANK_OPERANDS_PER_LEVEL per level
        };
    };
    int pad_2a5cc;
    int compiling;            // 0x2a5d0: 1 while CompileText runs
    int builtinCount;         // 0x2a5d4: number of registered builtin functions
    int globalBuiltinCount;   // 0x2a5d8: builtins registered outside any script (kept between compiles)
    int variableCounts[8];    // 0x2a5dc: variables declared per nesting level
    union
    {
        ScriptBlock blocks[0x800];    // 0x2a5fc: top-level `{ }` blocks of the text
        struct
        {
            char pad_blocks[0x1000];
            char statementParts[LISTBANK_GROUP_COUNT][6][0x100];  // 0x2b5fc: per nesting level the condition or
                                                                  // the three parts of a `for` header (3 unused)
        };
    };
    int blockCount;           // 0x2e5fc
    char text[0x7d00];        // 0x2e600: script text being parsed
    int textLength;           // 0x36300
    char *cursor[8];          // 0x36304: read position in `text`, one per nesting level
    int depth;                // 0x36324: current nesting level
    int flow;                 // 0x36328: how the statements run last ended (FLOW_ codes, script_statements.h)
    int failed;               // 0x3632c: set once an error was reported; every reader then fails
    ListBank *self;           // 0x36330: set to this object by LoadText
    char m_36334[0x20][0x20]; // 0x36334: registered keywords (16 are searched; the init clears 32)
    int *m_36734[0x20];       // 0x36734: the variable arrays of the keywords ("ch" = the script flags)
    char m_367b4[0x20];       // 0x367b4: element type of each keyword array (0 = int, otherwise float)

    int ClearGroup(int group);
    int RegisterKeyword(int index, const char *name, int *array, char type);  // EN 0x6448f0 (EN-only, listbank_keywords.cpp): registers keyword `index` with its name, variable array and element type
    int StoreOperand(int slot);                                // EN 0x6462c0 (EN-only, listbank_keywords.cpp): saves the last result as operand `slot` of the current nesting level
    int AppendEntry(const char *text, int group, int value, char flag);
    int FindKeyword(const char *name, int *index);
    int M_10004950(char *buffer, void *value, char type);
    int HasParenthesis(const char *text);
    int M_10004a30(const char *text, int *value);
    int M_10004aa0(const char *text, float *value);
    // Reports a parse error: sets the error flag (every reader then fails), prints the message, passes it and the
    // error's offset in the script text to the callback, and frees the script buffers. Returns 0. Inline in the
    // original: some callers expand it, others call the one out-of-line copy.
    int ReportError(const char *message)
    {
        failed = 1;
        printf("%s\n", message);
        if (callback != 0)
            callback(1, (int)message);
        if (callback != 0)
            callback(5000, cursor[depth] - text);
        FreeAll();
        return 0;
    }
    int FindAll(const char *name, int *out);
    int FreeAll();                                             // 0x10007d50 (resource_banks.cpp)
    int FreeBank(int group);                                   // 0x10007ce0 (resource_banks.cpp)
    int ReadStatement(char **text, char *statement);           // 0x10035b10
    int SkipBlock(char **text, int *level, int block);         // 0x10035a30
    int ReadArgument(char **text, char *word);               // 0x10035d20
    char ReadType(char **text);                               // 0x100364a0 (type_parse.cpp)
    int ReadWord(char **text, char *word);
    int FindCase(const char *value, char *text, char **found);
    int FindLabel(const char *name, char *text, char **found);
    int SkipStatement(char **text);
    int FindVariable(const char *name, int *index, int *group, int *element);  // 0x10037090
    int AssignValue(const char *name, const struct ScriptValue *value, char type);
    int ReadTerm(char **text, char *word);
    int ReadToken(char **text, char *token);                  // 0x100360f0: next token of an expression
    int CompileStatement(char *statement);                     // 0x100ed9b0
    int DeclareFunction(char *statement);                      // 0x100366c0
    int DeclareFromText(char *declaration, int group);        // 0x100ed510
    int CompileText();
    int LoadText(const char *source, int length);
    int GetIndexPart(const char *name, char *part, int n);    // the n-th [ ] part of `name`
    int Evaluate(const char *expression, char type);
    int RunStatements(char wantType, int openBraces);          // 0x10089210 (script_run.cpp)
    int CompileCall(const char *text, int flags);              // 0x10089a10 (Script_CompileCall)
    int FirstIndexValue(const char *name, int *value);
    int DeclareVariable(const char *name, int group, char type);
    int RegisterBuiltin(const char *signature, short id, BuiltinHandler handler);  // 0x10036ca0
    int InitLibrary();
    int ApplyOperator(const char *op);                         // 0x10004b10 (script_operators.cpp)
};

// The game's script parser.
extern "C" ListBank g_scriptParser;                  // EN 0x00b91ec8 (JP DAT_104687b8)
// The script flags the keyword "ch" reads and writes (the same array as Game::scriptFlags; EN code addresses it
// directly). Declared here until game_vars.h, which this header does not own, carries it.
extern "C" int g_scriptFlags[];                      // EN 0x01332c84 (JP DAT_10c62edc): 4000 flags
extern "C" char g_scriptFlagKeyword[];               // EN 0x0084b234 (JP DAT_101dc1fc): "ch", the keyword of the flags
extern "C" int g_gameBuiltinsRegistered;             // EN 0x00b91ec0 (JP DAT_1032d69c): 1 once the game builtins are registered
// The game's parser message callback (error text and error offset; JP FUN_10017f20).
extern "C" void ScriptParser_Message(int code, int value);   // EN 0x005e2e70
// Sets up the parser: library and game builtins (listbank_builtins.cpp). EN 0x00644db0 (JP 0x1013ad90).
extern "C" void ScriptParser_RegisterGameBuiltins(void);

#endif
