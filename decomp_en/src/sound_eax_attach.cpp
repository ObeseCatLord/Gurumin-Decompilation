// Attaching the EAX property set of a DirectSound buffer (EN build, EN-only: no JP twin found).
#include "sound_eax.h"

// Property numbers of the EAX 1.0 buffer set that must both be supported: all parameters, and the reverb mix.
const unsigned long EAX_BUFFER_ALL_PARAMETERS = 0;

// EaxBuffer::Attach: queries `buffer` for IKsPropertySet and keeps it as the property set when the driver can both
// read and write the EAX buffer properties 0 and EAX_BUFFER_REVERB_MIX; otherwise propertySet is cleared (EAX not
// available). Returns this. No direct caller.
EaxBuffer *EaxBuffer::Attach(IUnknown *buffer)
{
    if (buffer->QueryInterface(g_ksPropertySetInterfaceId, (void **)&propertySet) >= 0)
    {
        unsigned long support = 0;
        if (propertySet->QuerySupport(g_eaxBufferPropertySet, EAX_BUFFER_ALL_PARAMETERS, &support) >= 0 &&
            (support & EAX_SUPPORT_GET_AND_SET) == EAX_SUPPORT_GET_AND_SET)
        {
            support = 0;
            if (propertySet->QuerySupport(g_eaxBufferPropertySet, EAX_BUFFER_REVERB_MIX, &support) >= 0 &&
                (support & EAX_SUPPORT_GET_AND_SET) == EAX_SUPPORT_GET_AND_SET)
                return this;
        }
    }
    propertySet = 0;
    return this;
}

// EaxListener::Attach: queries `source` for IKsPropertySet and keeps it as the property set when the driver can both
// read and write the EAX listener properties all-parameters, environment, volume, decay time and damping; otherwise
// propertySet is cleared (EAX not available). Returns this. No direct caller.
EaxListener *EaxListener::Attach(IUnknown *source)
{
    if (source->QueryInterface(g_ksPropertySetInterfaceId, (void **)&propertySet) >= 0)
    {
        unsigned long support = 0;
        if (propertySet->QuerySupport(g_eaxListenerPropertySet, EAX_LISTENER_ALL_PARAMETERS, &support) >= 0 &&
            (support & EAX_SUPPORT_GET_AND_SET) == EAX_SUPPORT_GET_AND_SET)
        {
            support = 0;
            if (propertySet->QuerySupport(g_eaxListenerPropertySet, EAX_LISTENER_ENVIRONMENT, &support) >= 0 &&
                (support & EAX_SUPPORT_GET_AND_SET) == EAX_SUPPORT_GET_AND_SET && SupportsProperty(EAX_LISTENER_VOLUME) &&
                SupportsProperty(EAX_LISTENER_DECAY_TIME) && SupportsProperty(EAX_LISTENER_DAMPING))
                return this;
        }
    }
    propertySet = 0;
    return this;
}
