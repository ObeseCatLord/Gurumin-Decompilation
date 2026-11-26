// EAX 1.0 property getters of the sound engine (EN build, EN-only: no JP twin found).
#include "sound_eax.h"

// Reads the reverb mix of the buffer; EAX_FLOAT_UNAVAILABLE when the property set refuses. No direct caller.
float EaxBuffer::GetReverbMix()
{
    float value = EAX_FLOAT_UNAVAILABLE;
    unsigned long bytesReturned;
    propertySet->Get(g_eaxBufferPropertySet, EAX_BUFFER_REVERB_MIX, 0, 0, &value, sizeof(value), &bytesReturned);
    return value;
}

// Reads the damping of the listener's room; EAX_FLOAT_UNAVAILABLE when the property set refuses. No direct caller.
float EaxListener::GetDamping()
{
    float value = EAX_FLOAT_UNAVAILABLE;
    unsigned long bytesReturned;
    propertySet->Get(g_eaxListenerPropertySet, EAX_LISTENER_DAMPING, 0, 0, &value, sizeof(value), &bytesReturned);
    return value;
}

// Reads the decay time of the listener's room; EAX_FLOAT_UNAVAILABLE when the property set refuses. No direct caller.
float EaxListener::GetDecayTime()
{
    float value = EAX_FLOAT_UNAVAILABLE;
    unsigned long bytesReturned;
    propertySet->Get(g_eaxListenerPropertySet, EAX_LISTENER_DECAY_TIME, 0, 0, &value, sizeof(value), &bytesReturned);
    return value;
}

// Reads the environment number of the listener; 0 when the property set refuses. No direct caller.
unsigned long EaxListener::GetEnvironment()
{
    unsigned long value = 0;
    unsigned long bytesReturned;
    propertySet->Get(g_eaxListenerPropertySet, EAX_LISTENER_ENVIRONMENT, 0, 0, &value, sizeof(value), &bytesReturned);
    return value;
}

// Reads the volume of the listener's room; EAX_FLOAT_UNAVAILABLE when the property set refuses. No direct caller.
float EaxListener::GetVolume()
{
    float value = EAX_FLOAT_UNAVAILABLE;
    unsigned long bytesReturned;
    propertySet->Get(g_eaxListenerPropertySet, EAX_LISTENER_VOLUME, 0, 0, &value, sizeof(value), &bytesReturned);
    return value;
}
