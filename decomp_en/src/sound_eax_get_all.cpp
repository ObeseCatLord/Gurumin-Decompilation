// EAX 1.0 whole-set property getters of the sound engine (EN build, EN-only: no JP twin found).
#include "sound_eax.h"

// Reads all buffer properties into `parameters`. Returns the result of the property set. No direct caller.
HRESULT EaxBuffer::GetParameters(EaxBufferParameters *parameters)
{
    unsigned long bytesReturned;
    return propertySet->Get(g_eaxBufferPropertySet, EAX_BUFFER_ALL_PARAMETERS, 0, 0, parameters, sizeof(*parameters), &bytesReturned);
}

// Reads all four listener properties into `parameters`. Returns the result of the property set. No direct caller.
HRESULT EaxListener::GetParameters(EaxListenerParameters *parameters)
{
    unsigned long bytesReturned;
    return propertySet->Get(g_eaxListenerPropertySet, EAX_LISTENER_ALL_PARAMETERS, 0, 0, parameters, sizeof(*parameters), &bytesReturned);
}
