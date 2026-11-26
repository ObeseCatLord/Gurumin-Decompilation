// EAX 1.0 property access of the EN build's sound engine: wrappers around the IKsPropertySet of a DirectSound buffer.
// Only the property set pointer (the first member of each wrapper) is recovered. This is the one declaration of them in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_SOUND_EAX_H
#define GURUMIN_EN_SOUND_EAX_H

#include <windows.h>
#include <dsound.h>

// KSPROPERTY_SUPPORT_GET and _SET together: the driver can read and write the property.
const unsigned long EAX_SUPPORT_GET_AND_SET = 3;

// Value a float getter returns when the driver has no such property (-1e30).
const float EAX_FLOAT_UNAVAILABLE = -1.0e30f;

// Property numbers of the EAX 1.0 listener set.
const unsigned long EAX_LISTENER_ENVIRONMENT = 1;
const unsigned long EAX_LISTENER_VOLUME = 2;
const unsigned long EAX_LISTENER_DECAY_TIME = 3;
const unsigned long EAX_LISTENER_DAMPING = 4;
// Property number of the reverb mix in the EAX 1.0 buffer set.
const unsigned long EAX_BUFFER_REVERB_MIX = 1;

extern "C" const GUID g_eaxListenerPropertySet;   // EN 0x00861ebc: DSPROPSETID of the EAX 1.0 listener properties
extern "C" const GUID g_eaxBufferPropertySet;     // EN 0x00861eec: DSPROPSETID of the EAX 1.0 buffer properties

// The EAX properties of one sound source (a DirectSound buffer).
struct EaxBuffer
{
    IKsPropertySet *propertySet;   // 0x00: the buffer's property set (0 = EAX not available)

    float GetReverbMix();          // EN 0x7c1680: the reverb mix, EAX_FLOAT_UNAVAILABLE when the read fails
    int SupportsProperty(unsigned long property);   // EN 0x7c16d0: 1 when the driver can both read and write buffer property `property`
};

// The EAX listener properties.
struct EaxListener
{
    IKsPropertySet *propertySet;   // 0x00: the property set (0 = EAX not available)

    int SupportsProperty(unsigned long property);    // EN 0x7c1930: 1 when the driver can both read and write listener property `property`
    float GetDamping();            // EN 0x7c1820: the damping, EAX_FLOAT_UNAVAILABLE when the read fails
    float GetDecayTime();          // EN 0x7c1860: the decay time, EAX_FLOAT_UNAVAILABLE when the read fails
    unsigned long GetEnvironment();// EN 0x7c18a0: the environment number, 0 when the read fails
    float GetVolume();             // EN 0x7c18e0: the listener volume, EAX_FLOAT_UNAVAILABLE when the read fails
};

#endif
