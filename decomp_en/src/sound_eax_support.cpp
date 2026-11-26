// EAX 1.0 property support queries of the sound engine (EN build, EN-only: no JP twin found).
#include "sound_eax.h"

// Asks the driver whether buffer property `property` can be both read and written. Returns 1 when it can. No direct caller.
int EaxBuffer::SupportsProperty(unsigned long property)
{
    unsigned long support = 0;
    HRESULT result = propertySet->QuerySupport(g_eaxBufferPropertySet, property, &support);
    if (result < 0 || (support & EAX_SUPPORT_GET_AND_SET) != EAX_SUPPORT_GET_AND_SET)
        return 0;
    return 1;
}

// Asks the driver whether listener property `property` can be both read and written. Returns 1 when it can. No direct caller.
int EaxListener::SupportsProperty(unsigned long property)
{
    unsigned long support = 0;
    HRESULT result = propertySet->QuerySupport(g_eaxListenerPropertySet, property, &support);
    if (result < 0 || (support & EAX_SUPPORT_GET_AND_SET) != EAX_SUPPORT_GET_AND_SET)
        return 0;
    return 1;
}
