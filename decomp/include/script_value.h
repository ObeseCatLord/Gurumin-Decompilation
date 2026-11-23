// ScriptValue: declared here once (first used by src/listbank.cpp).
#ifndef GURUMIN_SCRIPT_VALUE_H
#define GURUMIN_SCRIPT_VALUE_H

// A value produced by the expression evaluator: an int (type 0), a float (type 1) or quoted text (type 2, the text
// still starts with its opening quote).
struct ScriptValue
{
    char text[0x100];
    int &i() { return *(int *)text; }
    float &f() { return *(float *)text; }
};

#endif
