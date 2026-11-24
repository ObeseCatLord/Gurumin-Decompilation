// The script expression evaluator's operator step.

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "listbank.h"

// Operator codes: the first operator character plus the second (or 0) times 0x100, as ApplyOperator reads them.
#define OP_PREFIX 't'
#define OP_MODULO '%'
#define OP_MULTIPLY '*'
#define OP_ADD '+'
#define OP_SUBTRACT '-'
#define OP_DIVIDE '/'
#define OP_LESS '<'
#define OP_GREATER '>'
#define OP_EQUALS_SIGN 0x3d00
#define OP_LESS_EQUAL (OP_LESS | OP_EQUALS_SIGN)
#define OP_GREATER_EQUAL (OP_GREATER | OP_EQUALS_SIGN)
#define OP_EQUAL ('=' | OP_EQUALS_SIGN)
#define OP_NOT_EQUAL ('!' | OP_EQUALS_SIGN)
#define OP_MODULO_ASSIGN (OP_MODULO | OP_EQUALS_SIGN)
#define OP_MULTIPLY_ASSIGN (OP_MULTIPLY | OP_EQUALS_SIGN)
#define OP_ADD_ASSIGN (OP_ADD | OP_EQUALS_SIGN)
#define OP_SUBTRACT_ASSIGN (OP_SUBTRACT | OP_EQUALS_SIGN)
#define OP_DIVIDE_ASSIGN (OP_DIVIDE | OP_EQUALS_SIGN)
#define OP_AND ('&' | '&' << 8)
#define OP_OR ('|' | '|' << 8)
// Unary operators are tagged with OP_PREFIX: "t!" not, "t+" increment, "t-" decrement, "tp" plus, "tm" minus.
#define OP_NOT (OP_PREFIX | '!' << 8)
#define OP_INCREMENT (OP_PREFIX | '+' << 8)
#define OP_DECREMENT (OP_PREFIX | '-' << 8)
#define OP_PLUS (OP_PREFIX | 'p' << 8)
#define OP_MINUS (OP_PREFIX | 'm' << 8)

// The operands of the current nesting level.
#define LEFT operands[depth * LISTBANK_OPERANDS_PER_LEVEL]
#define RIGHT operands[depth * LISTBANK_OPERANDS_PER_LEVEL + 1]

// 0x10004b10 (ListBank::ApplyOperator). Applies the operator `op` (two characters, see the OP_ codes; "t" first
// = unary) to the operands at the current nesting level: slot 0 on the left, slot 1 on the right. The operands are
// first brought to one type: that of the left operand if it is a float or text, else a float when the right one
// is, else int (a unary operator uses the left operand's type). A text operand turned into a number reads as
// atol/atof; a number turned into text becomes its decimal text. The result goes to *result with its type in
// resultType: comparisons give 0 or 1 in the operands' type, `+` joins two texts (dropping the closing quote of
// the left one and the opening quote of the right one), and an operator that is not defined for text gives empty
// text. Returns 0.
int ListBank::ApplyOperator(const char *op)
{
    char type = SCRIPT_TYPE_INT;
    int count;
    if (op[0] == OP_PREFIX)
    {
        char leftType = operandTypes[depth * LISTBANK_OPERANDS_PER_LEVEL];
        if (leftType == SCRIPT_TYPE_FLOAT)
        {
            type = leftType;
            count = 1;
        }
        else if (leftType == SCRIPT_TYPE_TEXT)
        {
            type = leftType;
            count = 1;
        }
        else
        {
            if (leftType == SCRIPT_TYPE_INT)
                type = SCRIPT_TYPE_INT;
            count = 1;
        }
    }
    else
    {
        char leftType = operandTypes[depth * LISTBANK_OPERANDS_PER_LEVEL];
        if (leftType == SCRIPT_TYPE_FLOAT)
            type = leftType;
        else if (leftType == SCRIPT_TYPE_TEXT)
            type = leftType;
        else if (leftType == SCRIPT_TYPE_INT && operandTypes[depth * LISTBANK_OPERANDS_PER_LEVEL + 1] == SCRIPT_TYPE_FLOAT)
            type = SCRIPT_TYPE_FLOAT;
        else if (leftType == SCRIPT_TYPE_INT && operandTypes[depth * LISTBANK_OPERANDS_PER_LEVEL + 1] == SCRIPT_TYPE_INT)
            type = SCRIPT_TYPE_INT;
        else if (leftType == SCRIPT_TYPE_INT && operandTypes[depth * LISTBANK_OPERANDS_PER_LEVEL + 1] == SCRIPT_TYPE_TEXT)
            type = SCRIPT_TYPE_INT;
        count = 2;
    }
    for (int slot = 0; slot < count; slot++)
    {
        char slotType = operandTypes[depth * LISTBANK_OPERANDS_PER_LEVEL + slot];
        if (type != slotType)
        {
            if (type == SCRIPT_TYPE_INT)
            {
                if (slotType == SCRIPT_TYPE_FLOAT)
                    operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].i() = (int)operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].f();
                else if (slotType == SCRIPT_TYPE_TEXT)
                {
                    long number = atol(operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].text + 1);
                    operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].i() = number;
                }
            }
            else if (type == SCRIPT_TYPE_FLOAT)
            {
                if (slotType == SCRIPT_TYPE_INT)
                    operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].f() = (float)operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].i();
                else if (slotType == SCRIPT_TYPE_TEXT)
                {
                    float number = (float)atof(operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].text + 1);
                    operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].f() = (float)(double)number;
                }
            }
            else if (type == SCRIPT_TYPE_TEXT)
            {
                if (slotType == SCRIPT_TYPE_INT)
                    sprintf(operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].text, "%d");
                else if (slotType == SCRIPT_TYPE_FLOAT)
                    sprintf(operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].text, "%f", operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot].f());
            }
        }
    }
    switch (op[1] * 0x100 + op[0])
    {
    case OP_ADD:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = RIGHT.i() + LEFT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = RIGHT.f() + LEFT.f();
            return 0;
        }
        if (type == SCRIPT_TYPE_TEXT)
        {
            LEFT.text[strlen(LEFT.text) - 1] = 0;
            strcat(LEFT.text, RIGHT.text + 1);
            self->resultType = SCRIPT_TYPE_TEXT;
            strcpy((char *)self->result, LEFT.text);
            return 0;
        }
        break;
    case OP_SUBTRACT:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() - RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = LEFT.f() - RIGHT.f();
            return 0;
        }
        break;
    case OP_MULTIPLY:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = RIGHT.i() * LEFT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = RIGHT.f() * LEFT.f();
            return 0;
        }
        break;
    case OP_DIVIDE:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() / RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = LEFT.f() / RIGHT.f();
            return 0;
        }
        break;
    case OP_MODULO:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() % RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)((int)LEFT.f() % (int)RIGHT.f());
            return 0;
        }
        break;
    case OP_LESS:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() < RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)(int)(LEFT.f() < RIGHT.f());
            return 0;
        }
        break;
    case OP_NOT:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() == 0;
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)(int)(LEFT.f() == 0.0f);
            return 0;
        }
        break;
    case OP_GREATER:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() > RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)(int)(LEFT.f() > RIGHT.f());
            return 0;
        }
        break;
    case OP_AND:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = (LEFT.i() != 0 && RIGHT.i() != 0) ? 1 : 0;
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)((LEFT.f() != 0.0f && RIGHT.f() != 0.0f) ? 1 : 0);
            return 0;
        }
        break;
    case OP_DECREMENT:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() - 1;
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = LEFT.f() - 1.0f;
            return 0;
        }
        break;
    case OP_INCREMENT:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() + 1;
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = LEFT.f() + 1.0f;
            return 0;
        }
        break;
    case OP_NOT_EQUAL:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() != RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)(int)(LEFT.f() != RIGHT.f());
            return 0;
        }
        break;
    case OP_ADD_ASSIGN:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = RIGHT.i() + LEFT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = RIGHT.f() + LEFT.f();
            return 0;
        }
        if (type == SCRIPT_TYPE_TEXT)
        {
            LEFT.text[strlen(LEFT.text) - 1] = 0;
            strcat(LEFT.text, RIGHT.text + 1);
            self->resultType = SCRIPT_TYPE_TEXT;
            strcpy((char *)self->result, LEFT.text);
            return 0;
        }
        break;
    case OP_SUBTRACT_ASSIGN:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() - RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = LEFT.f() - RIGHT.f();
            return 0;
        }
        break;
    case OP_MULTIPLY_ASSIGN:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = RIGHT.i() * LEFT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = RIGHT.f() * LEFT.f();
            return 0;
        }
        break;
    case OP_DIVIDE_ASSIGN:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() / RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = LEFT.f() / RIGHT.f();
            return 0;
        }
        break;
    case OP_MODULO_ASSIGN:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() % RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)((int)LEFT.f() % (int)RIGHT.f());
            return 0;
        }
        break;
    case OP_LESS_EQUAL:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() <= RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)(int)(LEFT.f() <= RIGHT.f());
            return 0;
        }
        break;
    case OP_GREATER_EQUAL:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() >= RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)(int)(LEFT.f() >= RIGHT.f());
            return 0;
        }
        break;
    case OP_EQUAL:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i() == RIGHT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)(int)(LEFT.f() == RIGHT.f());
            return 0;
        }
        break;
    case OP_MINUS:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = -LEFT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = -LEFT.f();
            return 0;
        }
        break;
    case OP_OR:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = (LEFT.i() != 0 || RIGHT.i() != 0) ? 1 : 0;
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = (float)((LEFT.f() != 0.0f || RIGHT.f() != 0.0f) ? 1 : 0);
            return 0;
        }
        break;
    case OP_PLUS:
        if (type == SCRIPT_TYPE_INT)
        {
            self->resultType = SCRIPT_TYPE_INT;
            *self->result = LEFT.i();
            return 0;
        }
        if (type == SCRIPT_TYPE_FLOAT)
        {
            self->resultType = SCRIPT_TYPE_FLOAT;
            *(float *)self->result = LEFT.i();
            return 0;
        }
        break;
    }
    if (type == SCRIPT_TYPE_TEXT)
    {
        self->resultType = SCRIPT_TYPE_TEXT;
        *(char *)self->result = 0;
    }
    return 0;
}
