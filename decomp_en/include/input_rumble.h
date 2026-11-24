// Controller vibration of the EN build: an XInput pad (XInputSetState loaded at run time) or a DirectInput
// force-feedback effect (JP: dinput_min.h InputEffect/EffectDesc/PeriodicForce, ui_vars.h). This is the one
// declaration of these types and globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_INPUT_RUMBLE_H
#define GURUMIN_EN_INPUT_RUMBLE_H

// IDirectInputEffect (the joystick's force-feedback effect); only the slots used so far are named.
struct InputEffect
{
    virtual long __stdcall QueryInterface() = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
    virtual long __stdcall Initialize() = 0;
    virtual long __stdcall GetEffectGuid() = 0;
    virtual long __stdcall GetParameters() = 0;
    virtual long __stdcall SetParameters(void *effect, unsigned long flags) = 0;   // slot 6
    virtual long __stdcall Start(unsigned long iterations, unsigned long flags) = 0;   // slot 7
};

// DIEFFECT (the periodic effect's description).
struct EffectDesc
{
    unsigned long size;                   // 0x00: structure size
    unsigned long flags;                  // 0x04
    unsigned long duration;               // 0x08: microseconds
    unsigned long samplePeriod;           // 0x0c
    unsigned long gain;                   // 0x10
    unsigned long triggerButton;          // 0x14
    unsigned long triggerRepeatInterval;  // 0x18
    unsigned long axisCount;              // 0x1c
    unsigned long *axes;                  // 0x20
    long *directions;                     // 0x24
    void *envelope;                       // 0x28
    unsigned long typeSpecificSize;       // 0x2c
    void *typeSpecific;                   // 0x30
    unsigned long startDelay;             // 0x34
};

// DIPERIODIC.
struct PeriodicForce
{
    unsigned long magnitude;    // 0x00: 0..10000
    long offset;                // 0x04
    unsigned long phase;        // 0x08
    unsigned long period;       // 0x0c: microseconds
};

// XINPUT_VIBRATION: the speed of the two motors, 0..65535.
struct XInputVibration
{
    unsigned short leftMotor;   // 0x00: low-frequency motor
    unsigned short rightMotor;  // 0x02: high-frequency motor
};

// Strongest values of the two kinds of vibration; Input_Rumble divides them by its `divisor`.
const float XINPUT_MOTOR_MAX = 65535.0f;
const float FORCE_FEEDBACK_MAGNITUDE_MAX = 10000.0f;
// SetParameters flags: DIEP_DURATION | DIEP_TYPESPECIFICPARAMS.
const unsigned long EFFECT_UPDATE_DURATION_AND_TYPE = 0x101;
// Microseconds per millisecond (DirectInput effect durations are in microseconds).
const int MICROSECONDS_PER_MILLISECOND = 1000;

extern "C" int g_xinputActive;                     // EN 0x022280d4: non-zero when an XInput pad is in use
extern "C" unsigned long (__stdcall *g_xinputSetState)(unsigned long user, XInputVibration *vibration);   // EN 0x022280d0: XInputSetState
extern "C" unsigned long g_xinputUser;             // EN 0x022280e4: user index of the XInput pad
extern "C" unsigned long g_rumbleEndTime;          // EN 0x022280d8: GetTickCount() at which the XInput rumble stops
extern "C" InputEffect *g_forceFeedbackEffect;     // EN 0x022280ec: the DirectInput force-feedback effect (0 = none)
extern "C" EffectDesc g_forceFeedbackDesc;         // EN 0x022280f0: the effect's description
extern "C" PeriodicForce g_forceFeedbackPeriodic;  // EN 0x02228128: the effect's type-specific data

// Starts the vibration for `milliseconds` ms at 1 / `divisor` of the full strength. EN 0x7aee30 (JP 0x10142700).
extern "C" void Input_Rumble(float divisor, int milliseconds);

#endif
