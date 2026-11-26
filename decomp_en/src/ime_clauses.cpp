// Construction of the IME context and its clause queries (EN build, EN-only: no JP twin found).
#include "ime.h"

// Size in bytes of the clause table the IME fills: the start and the end of the first clause.
const int CLAUSE_TABLE_BYTES = 8;

// ImeContext::ImeContext: an empty context (no window, no input context, no candidate list). Runs when the game's
// global context is constructed.
ImeContext::ImeContext()
{
    window = 0;
    context = 0;
    buffer = 0;
    m_0010 = 0;
}

// ImeContext::GetReadingClauseEnd: asks the IME for the clause table of the reading string and returns the end of its
// first clause, 0 when the IME returns nothing. No direct caller.
DWORD ImeContext::GetReadingClauseEnd()
{
    DWORD clauses[2];
    LONG copied = ImmGetCompositionStringA(context, GCS_COMPREADCLAUSE, clauses, CLAUSE_TABLE_BYTES);
    return copied ? clauses[1] : 0;
}

// ImeContext::GetClauseEnd: the same for the clause table of the composition string. No direct caller.
DWORD ImeContext::GetClauseEnd()
{
    DWORD clauses[2];
    LONG copied = ImmGetCompositionStringA(context, GCS_COMPCLAUSE, clauses, CLAUSE_TABLE_BYTES);
    return copied ? clauses[1] : 0;
}
