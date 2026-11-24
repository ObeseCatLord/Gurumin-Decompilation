// The script expression compiler: evaluates one expression, or a call of a script function or builtin.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listbank.h"

// "対応する括弧がありません。" ("There is no matching bracket."), Shift-JIS.
#define NO_MATCHING_BRACKET_TEXT "\x91\xce\x89\x9e\x82\xb7\x82\xe9\x8a\x87\x8c\xca\x82\xaa\x82\xa0\x82\xe8\x82\xdc\x82\xb9\x82\xf1\x81" "B"
// "参照引数には変数のみ指定できます。" ("Only a variable can be given for a reference argument."), Shift-JIS.
#define REFERENCE_NEEDS_VARIABLE_TEXT "\x8eQ\x8f\xc6\x88\xf8\x90\x94\x82\xc9\x82\xcd\x95\xcf\x90\x94\x82\xcc\x82\xdd\x8ew\x92\xe8\x82\xc5\x82\xab\x82\xdc\x82\xb7\x81" "B"
// "参照引数に関数は使えません。" ("A function cannot be a reference argument."), Shift-JIS.
#define REFERENCE_NO_FUNCTION_TEXT "\x8eQ\x8f\xc6\x88\xf8\x90\x94\x82\xc9\x8a\xd6\x90\x94\x82\xcd\x8eg\x82\xa6\x82\xdc\x82\xb9\x82\xf1\x81" "B"
// "引数の型が違います。キャストしてください。" ("The argument types are wrong. Please cast."), Shift-JIS.
#define ARGUMENT_TYPE_TEXT "\x88\xf8\x90\x94\x82\xcc\x8c^\x82\xaa\x88\xe1\x82\xa2\x82\xdc\x82\xb7\x81" "B\x83L\x83\x83\x83X\x83g\x82\xb5\x82\xc4\x82\xad\x82\xbe\x82\xb3\x82\xa2\x81" "B"
// "引数の数が違います。" ("The number of arguments is wrong."), Shift-JIS.
#define ARGUMENT_COUNT_TEXT "\x88\xf8\x90\x94\x82\xcc\x90\x94\x82\xaa\x88\xe1\x82\xa2\x82\xdc\x82\xb7\x81" "B"
// "定義されてない識別子です。" ("This identifier is not defined."), Shift-JIS.
#define UNDEFINED_IDENTIFIER_TEXT "\x92\xe8\x8b`\x82\xb3\x82\xea\x82\xc4\x82\xc8\x82\xa2\x8e\xaf\x95\xca\x8eq\x82\xc5\x82\xb7\x81" "B"
// "参照引数に定数は使えません。" ("A constant cannot be a reference argument."), Shift-JIS.
#define REFERENCE_NO_CONSTANT_TEXT "\x8eQ\x8f\xc6\x88\xf8\x90\x94\x82\xc9\x92\xe8\x90\x94\x82\xcd\x8eg\x82\xa6\x82\xdc\x82\xb9\x82\xf1\x81" "B"
// "バグがあります。" ("There is a bug."), Shift-JIS.
#define INTERNAL_BUG_TEXT "\x83o\x83O\x82\xaa\x82\xa0\x82\xe8\x82\xdc\x82\xb7\x81" "B"

// Tokens one expression can hold (the scan below counts to TOKEN_SCAN_LIMIT, as the original did), and the length of
// a token.
#define TOKEN_SLOTS 0x80
#define TOKEN_SCAN_LIMIT 0x100
#define TOKEN_SIZE 0x100
// Arguments of one call.
#define ARGUMENT_LIMIT 0x10
// Parameter types: a type of 10 or more is a reference parameter (the parameter type plus BY_REFERENCE_TYPE).
#define BY_REFERENCE_TYPE 10
#define PARAMETER_END (char)0xff
// builtinIds of a function written in the script.
#define SCRIPT_FUNCTION_ID -1
// resultType of an empty expression.
#define EMPTY_RESULT_TYPE (char)0xff

// Where a reference argument leaves the variable it names in the result buffer: group, slot and element.
#define REFERENCE_INDEX 0
#define REFERENCE_GROUP 4
#define REFERENCE_ELEMENT 8

typedef char (*TokenTable)[TOKEN_SIZE];

// The nesting level's operand slots hold the left and the right operand.
#define LEFT_SLOT(bank) ((bank)->depth * LISTBANK_OPERANDS_PER_LEVEL)

// The token of the n-th entry of the expression.
static __forceinline char *TokenAt(TokenTable tokens, const short *order, int n)
{
    return tokens[order[n]];
}

// Leaves the result of the expression just compiled as the left operand of the nesting level.
static __forceinline void SetLeftOperand(ListBank *bank, char type, const ScriptValue *value)
{
    bank->operandTypes[LEFT_SLOT(bank)] = type;
    bank->operands[LEFT_SLOT(bank)] = *value;
}

// Evaluates the left token, then the right one, and leaves them as the operands of the nesting level.
static __forceinline void SetBothOperands(ListBank *bank, char *left, char *right, ScriptValue *leftValue,
                                          char *leftType)
{
    bank->CompileCall(left, 0);
    *leftValue = *(ScriptValue *)bank->result;
    *leftType = bank->resultType;
    bank->CompileCall(right, 0);
    SetLeftOperand(bank, *leftType, leftValue);
    bank->operandTypes[LEFT_SLOT(bank) + 1] = bank->resultType;
    bank->operands[LEFT_SLOT(bank) + 1] = *(ScriptValue *)bank->result;
}

// Drops `count` entries from the expression's token order starting at `at`.
static __forceinline void DropTokens(short *order, int at, int count)
{
    for (int j = at; j < TOKEN_SLOTS - count; j++)
        order[j] = order[j + count];
}

// How a reduction treats its operands.
#define REDUCE_VALUE 0
#define REDUCE_ASSIGN 1
#define REDUCE_COMPOUND 2
#define REDUCE_COMMA 3

// Reduces the operator at entry `i` (the two characters c1, c2, c2 = 0 for a one-character operator) when it sits
// between two operands: both are evaluated, the operator applied, and the value is left as text in the left token
// (assignments store it into the left variable; the comma only drops its right operand). The scan then starts
// again at 0. Returns 1 when the operator was reduced.
static __forceinline int ReduceBinary(ListBank *bank, TokenTable tokens, short *order, int *i, int *n, char c1, char c2,
                                      int how, ScriptValue *leftValue, char *leftType)
{
    int at = *i;
    if (at + 1 < *n && TokenAt(tokens, order, at - 1)[TOKEN_IS_OPERATOR] == 0 &&
        TokenAt(tokens, order, at + 1)[TOKEN_IS_OPERATOR] == 0 && TokenAt(tokens, order, at)[0] == c1 &&
        TokenAt(tokens, order, at)[1] == c2)
    {
        char *left = TokenAt(tokens, order, at - 1);
        char *right = TokenAt(tokens, order, at + 1);
        char *op = TokenAt(tokens, order, at);
        if (how == REDUCE_VALUE)
        {
            SetBothOperands(bank, left, right, leftValue, leftType);
            bank->ApplyOperator(op);
            bank->M_10004950(left, bank->result, bank->resultType);
        }
        else if (how == REDUCE_ASSIGN)
        {
            bank->CompileCall(right, 0);
            bank->AssignValue(left, (ScriptValue *)bank->result, bank->resultType);
        }
        else if (how == REDUCE_COMPOUND)
        {
            SetBothOperands(bank, left, right, leftValue, leftType);
            bank->ApplyOperator(op);
            bank->AssignValue(left, (ScriptValue *)bank->result, bank->resultType);
        }
        if (bank->failed == 0)
            DropTokens(order, at, 2);
        *n -= 2;
        *i = 0;
        return 1;
    }
    return 0;
}

// Reduces the operator at entry `i` as a prefix operator on the operand after it: `code` is the unary operator's
// text ("t!", "tm", "tp"). Returns 1 when reduced.
static __forceinline int ReducePrefix(ListBank *bank, TokenTable tokens, short *order, int *i, int *n, char c,
                                      const char *code)
{
    int at = *i;
    if (at + 1 < *n && TokenAt(tokens, order, at + 1)[TOKEN_IS_OPERATOR] == 0 &&
        (at == 0 || TokenAt(tokens, order, at - 1)[TOKEN_IS_OPERATOR] == 1) && TokenAt(tokens, order, at)[0] == c)
    {
        char *operand = TokenAt(tokens, order, at + 1);
        char *op = TokenAt(tokens, order, at);
        bank->CompileCall(operand, 0);
        SetLeftOperand(bank, bank->resultType, (ScriptValue *)bank->result);
        bank->ApplyOperator(code);
        bank->M_10004950(op, bank->result, bank->resultType);
        if (bank->failed == 0)
            DropTokens(order, at + 1, 1);
        *n -= 1;
        *i = 0;
        return 1;
    }
    return 0;
}

// Reports an error exactly like ListBank::ReportError (which the original compiler expanded here twice).
static __forceinline int ReportErrorExpanded(ListBank *bank, const char *message)
{
    bank->failed = 1;
    printf("%s\n", message);
    if (bank->callback != 0)
        bank->callback(1, (int)message);
    if (bank->callback != 0)
        bank->callback(5000, bank->cursor[bank->depth] - bank->text);
    bank->FreeAll();
    return 0;
}

// 0x10089a10 (Script_CompileCall). Compiles and evaluates the expression `text`; its value lands in *result with
// its type in resultType (an empty expression gives type 0xff). The text is split into tokens (see ReadToken) and
// reduced in passes, tightest binding first: bracketed groups (compiled recursively), postfix ++ and --, prefix !
// - +, then * / %, + -, < > >= <= == !=, && ||, = += -= *= /= %=, and ','. Each reduction evaluates its operand
// tokens, applies the operator (ApplyOperator) and leaves the value as text in the left operand's token. One token
// left is a single operand: a call of a builtin or script function (the arguments are compiled one by one and the
// overload whose parameter count and types fit is chosen; a script function runs its body in a new nesting level
// with the parameters declared there, by-reference parameters naming the caller's variables), a keyword array
// element, a variable, a number or quoted text. With `asReference` set (an argument for a reference parameter) a
// variable is not read: its slot, group and element are left in the result buffer, and anything else is an error.
// Errors are reported through ReportError and leave an int 0. Returns 0.
int ListBank::CompileCall(const char *text, int asReference)
{
    char tokens[TOKEN_SLOTS][TOKEN_SIZE];
    char *name;
    ScriptValue leftValue;
    char leftType;
    {
        short order[TOKEN_SLOTS];
        int n = 0;
        int i;
        char *source = (char *)text;
        char *token = tokens[0];
        while (ReadToken(&source, token) != 0)
        {
            if (n >= TOKEN_SCAN_LIMIT)
                break;
            if (token[0] != ' ' && token[0] != '\t' && token[0] != '\n' && token[0] != '\r')
            {
                order[n] = (short)n;
                n++;
                token += TOKEN_SIZE;
            }
        }
        for (;;)
        {
            if (failed != 0)
                return 0;
            if (n == 0)
            {
                resultType = EMPTY_RESULT_TYPE;
                return 0;
            }
            for (i = 0; i < n; i++)
            {
                if (failed != 0)
                    return 0;
                char *group = TokenAt(tokens, order, i);
                if (group[0] == '(')
                {
                    if (asReference != 0)
                    {
                        ReportError(REFERENCE_NEEDS_VARIABLE_TEXT);
                        self->resultType = 0;
                        *self->result = 0;
                        return 0;
                    }
                    group[0] = ' ';
                    if (group[strlen(group) - 1] != ')')
                    {
                        ReportError(NO_MATCHING_BRACKET_TEXT);
                        strcpy(group, "0");
                    }
                    else
                    {
                        group[strlen(group) - 1] = 0;
                        CompileCall(group, 0);
                        M_10004950(group, result, resultType);
                    }
                }
            }
            if (n == 1)
            {
                name = TokenAt(tokens, order, 0);
                break;
            }
            if (asReference != 0)
            {
                ReportErrorExpanded(this, REFERENCE_NEEDS_VARIABLE_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
            i = 0;
            while (i < n)
            {
                if (failed != 0)
                    return 0;
                if (i > 0 && TokenAt(tokens, order, i - 1)[TOKEN_IS_OPERATOR] == 0)
                {
                    char *operand = TokenAt(tokens, order, i - 1);
                    char *op = TokenAt(tokens, order, i);
                    if (op[0] == '+' && op[1] == '+')
                    {
                        CompileCall(operand, 0);
                        SetLeftOperand(this, resultType, (ScriptValue *)result);
                        ApplyOperator("t+");
                        AssignValue(operand, (ScriptValue *)result, resultType);
                        if (failed == 0)
                            DropTokens(order, i, 1);
                        n--;
                        i = 0;
                        continue;
                    }
                    if (op[0] == '-' && op[1] == '-')
                    {
                        CompileCall(operand, 0);
                        SetLeftOperand(this, resultType, (ScriptValue *)result);
                        ApplyOperator("t-");
                        AssignValue(operand, (ScriptValue *)result, resultType);
                        if (failed == 0)
                            DropTokens(order, i, 1);
                        n--;
                        i = 0;
                        continue;
                    }
                }
                if (ReducePrefix(this, tokens, order, &i, &n, '!', "t!") || ReducePrefix(this, tokens, order, &i, &n, '-', "tm") ||
                    ReducePrefix(this, tokens, order, &i, &n, '+', "tp"))
                    continue;
                i++;
            }
            i = 0;
            while (i < n - 1)
            {
                if (failed != 0)
                    return 0;
                if (i > 0 && (ReduceBinary(this, tokens, order, &i, &n, '*', 0, REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '/', 0, REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '%', 0, REDUCE_VALUE, &leftValue, &leftType)))
                    continue;
                i++;
            }
            i = 0;
            while (i < n - 1)
            {
                if (failed != 0)
                    return 0;
                if (i > 0 && (ReduceBinary(this, tokens, order, &i, &n, '+', 0, REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '-', 0, REDUCE_VALUE, &leftValue, &leftType)))
                    continue;
                i++;
            }
            i = 0;
            while (i < n - 1)
            {
                if (failed != 0)
                    return 0;
                if (i > 0 && (ReduceBinary(this, tokens, order, &i, &n, '<', 0, REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '>', 0, REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '>', '=', REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '<', '=', REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '=', '=', REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '!', '=', REDUCE_VALUE, &leftValue, &leftType)))
                    continue;
                i++;
            }
            i = 0;
            while (i < n - 1)
            {
                if (failed != 0)
                    return 0;
                if (i > 0 && (ReduceBinary(this, tokens, order, &i, &n, '&', '&', REDUCE_VALUE, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '|', '|', REDUCE_VALUE, &leftValue, &leftType)))
                    continue;
                i++;
            }
            i = 0;
            while (i < n - 1)
            {
                if (failed != 0)
                    return 0;
                if (i > 0 && (ReduceBinary(this, tokens, order, &i, &n, '=', 0, REDUCE_ASSIGN, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '+', '=', REDUCE_COMPOUND, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '-', '=', REDUCE_COMPOUND, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '*', '=', REDUCE_COMPOUND, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '/', '=', REDUCE_COMPOUND, &leftValue, &leftType) ||
                              ReduceBinary(this, tokens, order, &i, &n, '%', '=', REDUCE_COMPOUND, &leftValue, &leftType)))
                    continue;
                i++;
            }
            i = 0;
            while (i < n - 1)
            {
                if (failed != 0)
                    return 0;
                if (i > 0 && ReduceBinary(this, tokens, order, &i, &n, ',', 0, REDUCE_COMMA, &leftValue, &leftType))
                    continue;
                i++;
            }
            if (n > 1)
            {
                ReportErrorExpanded(this, INTERNAL_BUG_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
        }
    }
    {
        while (*name == '(')
        {
            name++;
            name[strlen(name) - 1] = 0;
        }
        int selected = 0;
        int sameCount = 0;
        int m = 0;
        char word[LISTBANK_TEXT_SIZE];
        ReadWord(&name, word);
        int found[0x80];
        int matches = FindAll(word, found);
        if (matches != 0)
        {
            if (asReference != 0)
            {
                ReportError(REFERENCE_NO_FUNCTION_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
            char arguments[LISTBANK_TEXT_SIZE];
            strcpy(arguments, name);
            char *argumentCursor = arguments;
            if (arguments[0] != '(')
            {
                ReportError(NO_MATCHING_BRACKET_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
            arguments[0] = ' ';
            if (arguments[strlen(arguments) - 1] != ')')
            {
                ReportError(NO_MATCHING_BRACKET_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
            arguments[strlen(arguments) - 1] = 0;
            char argumentTypes[ARGUMENT_LIMIT];
            ScriptArgument argumentList[ARGUMENT_LIMIT];
            ScriptValue argumentValues[ARGUMENT_LIMIT];
            char argumentTexts[ARGUMENT_LIMIT][TOKEN_SIZE];
            int argumentCount;
            for (argumentCount = 0; argumentCount < ARGUMENT_LIMIT; argumentCount++)
            {
                if (failed != 0)
                    return 0;
                ReadArgument(&argumentCursor, argumentTexts[argumentCount]);
                if (argumentTexts[argumentCount][0] == 0)
                    break;
                CompileCall(argumentTexts[argumentCount], 0);
                argumentValues[argumentCount] = *(ScriptValue *)result;
                argumentList[argumentCount].text = argumentValues[argumentCount].text;
                argumentTypes[argumentCount] = resultType;
            }
            for (; m < matches; m++)
            {
                int candidate = found[m];
                if (builtinParamCounts[candidate] == argumentCount)
                {
                    sameCount = 1;
                    selected = candidate;
                    int typesFit = 0;
                    for (int a = 0; a < argumentCount; a++)
                    {
                        if (argumentTypes[a] == builtinParamTypes[candidate][a] % BY_REFERENCE_TYPE)
                            typesFit++;
                    }
                    if (typesFit == argumentCount)
                        break;
                }
            }
            if (m == matches)
            {
                if (sameCount != 0)
                    ReportError(ARGUMENT_TYPE_TEXT);
                else
                    ReportError(ARGUMENT_COUNT_TEXT);
                return 0;
            }
            if (builtinIds[selected] == SCRIPT_FUNCTION_ID)
            {
                depth++;
                if (builtinParamTypes[selected][0] != PARAMETER_END)
                {
                    char *parameterType = builtinParamTypes[selected];
                    char *parameterName = builtinParamNames[selected][0];
                    ScriptArgument *argument = argumentList;
                    char *argumentText = argumentTexts[0];
                    do
                    {
                        if (failed != 0)
                            return 0;
                        if (*parameterType >= BY_REFERENCE_TYPE)
                        {
                            CompileCall(argumentText, 1);
                            if (failed != 0)
                                return 0;
                            int index = self->result[REFERENCE_INDEX];
                            int group = self->result[REFERENCE_GROUP];
                            int element = self->result[REFERENCE_ELEMENT];
                            char type = variableTypes[group][index];
                            if (type == 0)
                                M_100046d0(parameterName, depth, (int)(values[group][index].ints + element), 0);
                            else if (type == 1)
                                M_100046d0(parameterName, depth, (int)(values[group][index].floats + element), 1);
                            else if (type == 2)
                                M_100046d0(parameterName, depth, (int)values[group][index].texts[element], 2);
                        }
                        else
                        {
                            DeclareVariable(parameterName, depth, *parameterType);
                            AssignValue(parameterName, (ScriptValue *)argument->text, *parameterType);
                        }
                        parameterName += 0x10;
                        argumentText += TOKEN_SIZE;
                        argument++;
                        parameterType++;
                    } while (*parameterType != PARAMETER_END);
                }
                cursor[depth] = builtinHandlers[selected].body;
                flow = 0;
                RunStatements(builtinReturnTypes[selected], 0);
                flow = 0;
                M_10004680(depth + 1);
                return 0;
            }
            if (builtinHandlers[selected].handler == 0)
                return 0;
            for (int a = 0; a < builtinParamCounts[selected]; a++)
            {
                if (builtinParamTypes[selected][a] % BY_REFERENCE_TYPE == SCRIPT_TYPE_TEXT)
                {
                    strcpy((char *)found, argumentValues[a].text + 1);
                    ((char *)found)[strlen((char *)found) - 1] = 0;
                    strcpy(argumentValues[a].text, (char *)found);
                }
                if (builtinParamTypes[selected][a] >= BY_REFERENCE_TYPE)
                {
                    CompileCall(argumentTexts[a], 1);
                    if (failed != 0)
                        return 0;
                }
            }
            builtinHandlers[selected].handler(builtinIds[selected], argumentList, this);
            for (int a = 0; a < builtinParamCounts[selected]; a++)
            {
                char type = builtinParamTypes[selected][a];
                if (type >= BY_REFERENCE_TYPE)
                {
                    if (type % BY_REFERENCE_TYPE == SCRIPT_TYPE_TEXT)
                    {
                        sprintf((char *)found, "\"%s\"", argumentList[a].text);
                        AssignValue(argumentTexts[a], (ScriptValue *)found, builtinParamTypes[selected][a] % BY_REFERENCE_TYPE);
                    }
                    else
                        AssignValue(argumentTexts[a], (ScriptValue *)argumentList[a].text, type % BY_REFERENCE_TYPE);
                }
            }
            return 0;
        }
        int keyword;
        if (M_100048f0(word, &keyword) != 0)
        {
            int element;
            FirstIndexValue(word, &element);
            char type = m_367b4[keyword];
            if (type == 0)
                self->resultType = type;
            else if (type == 1)
                self->resultType = type;
            else
                return 0;
            *self->result = m_36734[keyword][element];
            return 0;
        }
        int index;
        int group;
        int element;
        if (FindVariable(word, &index, &group, &element) != 0)
        {
            if (asReference != 0)
            {
                self->result[REFERENCE_INDEX] = index;
                self->result[REFERENCE_GROUP] = group;
                self->result[REFERENCE_ELEMENT] = element;
                return 0;
            }
            char type = variableTypes[group][index];
            if (type == 0)
            {
                self->resultType = 0;
                *self->result = values[group][index].ints[element];
            }
            else if (type == 1)
            {
                self->resultType = 1;
                *self->result = *(int *)(values[group][index].floats + element);
            }
            else if (type == 2)
            {
                self->resultType = 2;
                strcpy((char *)self->result, values[group][index].texts[element]);
            }
            return 0;
        }
        int intValue;
        float floatValue;
        if (M_10004a30(word, &intValue) != 0)
        {
            if (asReference != 0)
            {
                ReportError(REFERENCE_NO_CONSTANT_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
            if (failed != 0)
                return 0;
            self->resultType = 0;
            *self->result = intValue;
            return 0;
        }
        if (M_10004aa0(word, &floatValue) != 0)
        {
            if (asReference != 0)
            {
                ReportError(REFERENCE_NO_CONSTANT_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
            if (failed != 0)
                return 0;
            self->resultType = 1;
            *(float *)self->result = floatValue;
            return 0;
        }
        if (word[0] == '"' && word[strlen(word) - 1] == '"')
        {
            if (asReference != 0)
            {
                ReportError(REFERENCE_NO_CONSTANT_TEXT);
                self->resultType = 0;
                *self->result = 0;
                return 0;
            }
            if (failed != 0)
                return 0;
            self->resultType = 2;
            strcpy((char *)self->result, word);
            return 0;
        }
        if (failed != 0)
            return 0;
        ReportError(UNDEFINED_IDENTIFIER_TEXT);
        self->resultType = 0;
        *self->result = 0;
        return 0;
    }
}
