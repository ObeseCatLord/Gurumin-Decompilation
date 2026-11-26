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

// Property number that reads or writes all four listener properties at once (EAXLISTENER_ALLPARAMETERS).
const unsigned long EAX_LISTENER_ALL_PARAMETERS = 0;

// The four listener properties in the order of EAX_LISTENER_ALL_PARAMETERS.
struct EaxListenerParameters
{
    unsigned long environment;   // 0x00: environment number
    float volume;                // 0x04: listener volume
    float decayTime;             // 0x08: decay time of the room
    float damping;               // 0x0c: damping of the room
};

// Property numbers of the EAX 1.0 listener set.
const unsigned long EAX_LISTENER_ENVIRONMENT = 1;
const unsigned long EAX_LISTENER_VOLUME = 2;
const unsigned long EAX_LISTENER_DECAY_TIME = 3;
const unsigned long EAX_LISTENER_DAMPING = 4;
// Property number of the reverb mix in the EAX 1.0 buffer set.
const unsigned long EAX_BUFFER_REVERB_MIX = 1;

extern "C" const GUID g_eaxListenerPropertySet;   // EN 0x00861ebc: DSPROPSETID of the EAX 1.0 listener properties
extern "C" const GUID g_eaxBufferPropertySet;     // EN 0x00861eec: DSPROPSETID of the EAX 1.0 buffer properties
extern "C" const GUID g_ksPropertySetInterfaceId;   // EN 0x00861f8c: IID_IKsPropertySet

// The one buffer property of EAX_BUFFER_ALL_PARAMETERS.
struct EaxBufferParameters
{
    float reverbMix;   // 0x00: reverb mix of the buffer
};

// Property number that reads or writes all buffer properties at once (EAXBUFFER_ALLPARAMETERS).
const unsigned long EAX_BUFFER_ALL_PARAMETERS = 0;

// The EAX properties of one sound source (a DirectSound buffer).
struct EaxBuffer
{
    IKsPropertySet *propertySet;   // 0x00: the buffer's property set (0 = EAX not available)

    EaxBuffer *Attach(IUnknown *buffer);   // EN 0x7c15d0 (EN-only, sound_eax_attach.cpp): asks `buffer` for its property set and keeps it only when both buffer properties are readable and writable; returns this
    float GetReverbMix();          // EN 0x7c1680: the reverb mix, EAX_FLOAT_UNAVAILABLE when the read fails
    int SupportsProperty(unsigned long property);   // EN 0x7c16d0: 1 when the driver can both read and write buffer property `property`
    HRESULT SetParameters(EaxBufferParameters *parameters);   // EN 0x7c1710 (sound_eax_set.cpp, EN-only): writes all buffer properties at once
    HRESULT GetParameters(EaxBufferParameters *parameters);   // EN 0x7c1650 (sound_eax_get_all.cpp, EN-only): reads all buffer properties at once
    HRESULT SetReverbMix(float mix);                          // EN 0x7c1730 (sound_eax_set.cpp, EN-only): writes the reverb mix
};

// The EAX listener properties.
struct EaxListener
{
    IKsPropertySet *propertySet;   // 0x00: the property set (0 = EAX not available)

    EaxListener *Attach(IUnknown *source);   // EN 0x7c1750 (EN-only, sound_eax_attach.cpp): asks `source` for its property set and keeps it only when all five listener properties are readable and writable; returns this
    int SupportsProperty(unsigned long property);    // EN 0x7c1930: 1 when the driver can both read and write listener property `property`
    float GetDamping();            // EN 0x7c1820: the damping, EAX_FLOAT_UNAVAILABLE when the read fails
    float GetDecayTime();          // EN 0x7c1860: the decay time, EAX_FLOAT_UNAVAILABLE when the read fails
    unsigned long GetEnvironment();// EN 0x7c18a0: the environment number, 0 when the read fails
    float GetVolume();             // EN 0x7c18e0: the listener volume, EAX_FLOAT_UNAVAILABLE when the read fails
    void SetAll(unsigned long environment, float volume, float decayTime, float damping);   // EN 0x7c19f0: writes all four properties
    HRESULT SetParameters(EaxListenerParameters *parameters);   // EN 0x7c1970 (sound_eax_set.cpp, EN-only): writes all four properties from a structure
    HRESULT GetParameters(EaxListenerParameters *parameters);   // EN 0x7c17f0 (sound_eax_get_all.cpp, EN-only): reads all four properties into a structure
    HRESULT SetDamping(float damping);                          // EN 0x7c1990 (sound_eax_set.cpp, EN-only): writes the damping
    HRESULT SetDecayTime(float decayTime);                      // EN 0x7c19b0 (sound_eax_set.cpp, EN-only): writes the decay time
    HRESULT SetEnvironment(unsigned long environment);          // EN 0x7c19d0 (sound_eax_set.cpp, EN-only): writes the environment number
    HRESULT SetVolume(float volume);                            // EN 0x7c1a40 (sound_eax_set.cpp, EN-only): writes the listener volume
};

#endif
