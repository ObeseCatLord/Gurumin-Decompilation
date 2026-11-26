// EN script parser: keyword registration and operand saving (EN-only: no JP twin found).
#include "listbank.h"

// Registers keyword `index` of the parser: name, the array of variables the keyword reads and writes, and the element
// type of that array (0 = int, otherwise float). The game registers "ch" (the script flags) this way. Returns 0.
int ListBank::RegisterKeyword(int index, const char *name, int *array, char type)
{
    m_36734[index] = array;
    m_367b4[index] = type;
    char *destination = m_36334[index];
    while ((*destination++ = *name++) != '\0')
        ;
    return 0;
}

// Saves the last evaluated value (result and resultType) as operand `slot` of the current nesting level, so the
// next operand can be evaluated. Returns 0.
int ListBank::StoreOperand(int slot)
{
    operandTypes[depth * LISTBANK_OPERANDS_PER_LEVEL + slot] = resultType;
    operands[depth * LISTBANK_OPERANDS_PER_LEVEL + slot] = *(ScriptValue *)result;
    return 0;
}
