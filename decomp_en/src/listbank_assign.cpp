// EN script parser (ListBank): storing a value into a variable. JP twin: ListBank::AssignValue in
// decomp/src/listbank.cpp (not matched there).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listbank.h"

// "定義されてない識別子です。" ("This identifier is not defined."), Shift-JIS.
#define UNDEFINED_NAME_TEXT "\x92\xe8\x8b`\x82\xb3\x82\xea\x82\xc4\x82\xc8\x82\xa2\x8e\xaf\x95\xca\x8eq\x82\xc5\x82\xb7\x81" "B"

// Stores `value` (of `type`) into the variable `name`: an element of a registered keyword array (int or float
// elements, the index from the name's first [ ]), or a declared script variable (int, float or text; text receives
// numbers formatted as "%d" / "%f" in quotes, and numbers parse the quoted text after its quote). Values are
// converted to the variable's type. Reports an undefined name. Returns 1 once the variable was found, 0 otherwise or
// after an error. Called by the statement runner for assignments.
int ListBank::AssignValue(const char *name, const ScriptValue *value, char type)
{
    if (failed != 0)
        return 0;
    ScriptValue v = *value;
    {
        int index;
        int element;
        if (FindKeyword(name, &index) != 0)
        {
            FirstIndexValue(name, &element);
            if (m_367b4[index] == 0)
            {
                if (type == SCRIPT_TYPE_INT)
                {
                    m_36734[index][element] = v.i();
                    return 1;
                }
                if (type == SCRIPT_TYPE_FLOAT)
                {
                    m_36734[index][element] = (int)v.f();
                    return 1;
                }
            }
            else
            {
                if (type == SCRIPT_TYPE_INT)
                {
                    ((float *)m_36734[index])[element] = (float)v.i();
                    return 1;
                }
                if (type == SCRIPT_TYPE_FLOAT)
                    ((float *)m_36734[index])[element] = (float)(int)v.f();
            }
            return 1;
        }
    }
    {
        int index;
        int group;
        int element;
        if (FindVariable(name, &index, &group, &element) != 0)
        {
            if (variableTypes[group][index] == SCRIPT_TYPE_INT)
            {
                if (type == SCRIPT_TYPE_INT)
                {
                    values[group][index].ints[element] = v.i();
                    return 1;
                }
                if (type == SCRIPT_TYPE_FLOAT)
                {
                    values[group][index].ints[element] = (int)v.f();
                    return 1;
                }
                if (type == SCRIPT_TYPE_TEXT)
                {
                    values[group][index].ints[element] = atol(&v.text[1]);
                    return 1;
                }
            }
            else if (variableTypes[group][index] == SCRIPT_TYPE_FLOAT)
            {
                if (type == SCRIPT_TYPE_INT)
                {
                    values[group][index].floats[element] = (float)v.i();
                    return 1;
                }
                if (type == SCRIPT_TYPE_FLOAT)
                {
                    values[group][index].floats[element] = v.f();
                    return 1;
                }
                if (type == SCRIPT_TYPE_TEXT)
                {
                    values[group][index].floats[element] = (float)atof(&v.text[1]);
                    return 1;
                }
            }
            else if (variableTypes[group][index] == SCRIPT_TYPE_TEXT)
            {
                if (type == SCRIPT_TYPE_INT)
                {
                    sprintf(values[group][index].texts[element], "\"%d\"", v.i());
                    return 1;
                }
                if (type == SCRIPT_TYPE_FLOAT)
                {
                    sprintf(values[group][index].texts[element], "\"%f\"", v.f());
                    return 1;
                }
                if (type == SCRIPT_TYPE_TEXT)
                    strcpy(values[group][index].texts[element], v.text);
            }
            return 1;
        }
    }
    ReportError(UNDEFINED_NAME_TEXT);
    return 0;
}
