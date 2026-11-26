// EAX 1.0 property setters of the sound engine (EN build, EN-only: no JP twin found).
#include "sound_eax.h"

// Writes all buffer properties from `parameters`. Returns the result of the property set. No direct caller.
HRESULT EaxBuffer::SetParameters(EaxBufferParameters *parameters)
{
    return propertySet->Set(g_eaxBufferPropertySet, EAX_BUFFER_ALL_PARAMETERS, 0, 0, parameters, sizeof(*parameters));
}

// Writes the reverb mix of the buffer. Returns the result of the property set. No direct caller.
HRESULT EaxBuffer::SetReverbMix(float mix)
{
    return propertySet->Set(g_eaxBufferPropertySet, EAX_BUFFER_REVERB_MIX, 0, 0, &mix, sizeof(mix));
}

// Writes all four listener properties from `parameters`. Returns the result of the property set. No direct caller.
HRESULT EaxListener::SetParameters(EaxListenerParameters *parameters)
{
    return propertySet->Set(g_eaxListenerPropertySet, EAX_LISTENER_ALL_PARAMETERS, 0, 0, parameters, sizeof(*parameters));
}

// Writes the damping of the listener's room. Returns the result of the property set. No direct caller.
HRESULT EaxListener::SetDamping(float damping)
{
    return propertySet->Set(g_eaxListenerPropertySet, EAX_LISTENER_DAMPING, 0, 0, &damping, sizeof(damping));
}

// Writes the decay time of the listener's room. Returns the result of the property set. No direct caller.
HRESULT EaxListener::SetDecayTime(float decayTime)
{
    return propertySet->Set(g_eaxListenerPropertySet, EAX_LISTENER_DECAY_TIME, 0, 0, &decayTime, sizeof(decayTime));
}

// Writes the environment number of the listener. Returns the result of the property set. No direct caller.
HRESULT EaxListener::SetEnvironment(unsigned long environment)
{
    return propertySet->Set(g_eaxListenerPropertySet, EAX_LISTENER_ENVIRONMENT, 0, 0, &environment, sizeof(environment));
}

// Writes the volume of the listener's room. Returns the result of the property set. No direct caller.
HRESULT EaxListener::SetVolume(float volume)
{
    return propertySet->Set(g_eaxListenerPropertySet, EAX_LISTENER_VOLUME, 0, 0, &volume, sizeof(volume));
}
