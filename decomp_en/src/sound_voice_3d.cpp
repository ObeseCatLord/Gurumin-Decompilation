// 3D placement of the sound engine's voices (EN build; JP twin: sound3d.dll, not matched yet): the DirectSound
// listener and the pan of software-positioned voices.
#include <math.h>

#include "sound_engine.h"
#include "sound_voice.h"

// Sets the cooperative level of the voice's DirectSound device to exclusive for `window`. Returns 0. Called when the
// game window changes (the voice that owns the device).
int SoundVoice::SetCooperativeLevel(HWND window)
{
    if (directSound != 0)
        directSound->SetCooperativeLevel(window, DSSCL_EXCLUSIVE);
    return 0;
}

// Places the DirectSound 3D listener: rolloff factor, position, front and top vectors (velocity zero). A front vector
// of all zeros becomes (x, -1, z); a top vector that ends in zero ends in -1. Remembers the position and the front
// vector for the 3D voices. Returns 1. Called by the "place listener" command.
int SoundVoice::SetListener(float rolloff, float x, float y, float z, float frontX, float frontY, float frontZ,
                            float topX, float topY, float topZ)
{
    if (listener != 0)
    {
        DS3DLISTENER parameters;
        parameters.dwSize = sizeof(DS3DLISTENER);
        listener->GetAllParameters(&parameters);
        parameters.flRolloffFactor = rolloff;
        parameters.flDopplerFactor = LISTENER_DOPPLER_FACTOR;
        parameters.vVelocity.x = 0;
        parameters.vVelocity.y = 0;
        parameters.vVelocity.z = 0;
        parameters.vPosition.x = x;
        parameters.vPosition.y = y;
        parameters.vPosition.z = z;
        if (frontX == 0 && frontY == 0 && frontZ == 0)
            frontY = LISTENER_DEFAULT_AXIS;
        parameters.vOrientFront.z = frontZ;
        parameters.vOrientFront.x = frontX;
        parameters.vOrientFront.y = frontY;
        if (topZ == 0)
            topZ = LISTENER_DEFAULT_AXIS;
        parameters.vOrientTop.x = topX;
        parameters.vOrientTop.y = topY;
        parameters.vOrientTop.z = topZ;
        listener->SetAllParameters(&parameters, DS3D_IMMEDIATE);
        g_soundListenerPosition[0] = x;
        g_soundListenerPosition[1] = y;
        g_soundListenerPosition[2] = z;
        g_soundListenerFront[0] = frontX;
        g_soundListenerFront[1] = frontY;
        g_soundListenerFront[2] = frontZ;
    }
    return 1;
}

// Full circle in radians; angles are wrapped into -TWO_PI..TWO_PI.
static const float SOUND_TWO_PI = 6.283185f;
// Distance (scaled) at which a software-positioned voice is inaudible, and the units the fade runs over.
static const float SOUND_PAN_MAX_DISTANCE = 800.0f;
static const float SOUND_PAN_DISTANCE_STEPS = 11.0f;
// Volume units lost per step of distance.
static const float SOUND_PAN_VOLUME_PER_STEP = 100.0f;
// Pan units (hundredths of a decibel) per unit of the directional factor; its square is scaled by 1.6.
static const float SOUND_PAN_SCALE = 1500.0f;
static const double SOUND_PAN_SQUARE_SCALE = 1.6;
// Range of the DirectSound pan value.
static const float SOUND_PAN_LEFT = -10000.0f;
static const float SOUND_PAN_RIGHT = 10000.0f;

// Derives volume and pan of a voice that has no 3D buffer from the listener: the volume falls with the distance
// (scaled), the pan follows the squared sine of the angle between the direction to the sound and the listener's front
// vector, signed. Applies both to the DirectSound buffer. Returns 0. Called whenever the voice or the listener moves.
int SoundVoice::UpdatePanFromListener()
{
    if (buffer == 0)
        return 0;
    if (is3D == 0)
        return 0;
    float dx = position[0] - g_soundListenerPosition[0];
    float dy = position[1] - g_soundListenerPosition[1];
    float dz = position[2] - g_soundListenerPosition[2];
    float distance = (float)sqrt((double)(dx * dx + dy * dy + dz * dz));
    float fade = (SOUND_PAN_MAX_DISTANCE - distance * distanceScale) / SOUND_PAN_DISTANCE_STEPS;
    double soundArc = atan2((double)dx, (double)dy);
    double frontArc = atan2((double)g_soundListenerFront[0], (double)g_soundListenerFront[1]);
    float frontAngle = (float)frontArc;
    while (frontAngle > SOUND_TWO_PI)
        frontAngle -= SOUND_TWO_PI;
    while (frontAngle < -SOUND_TWO_PI)
        frontAngle += SOUND_TWO_PI;
    float soundAngle = (float)soundArc;
    while (soundAngle > SOUND_TWO_PI)
        soundAngle -= SOUND_TWO_PI;
    while (soundAngle < -SOUND_TWO_PI)
        soundAngle += SOUND_TWO_PI;
    float side = (float)sin((double)(soundAngle - frontAngle));
    double directional = (double)(side * side) * SOUND_PAN_SQUARE_SCALE;
    if (side < 0)
        directional = -directional;
    float pan = (float)directional;
    float level = 0;
    float volumeLevel = (float)SOUND_VOLUME_FULL - fade * SOUND_PAN_VOLUME_PER_STEP;
    if (volumeLevel >= 0)
        level = volumeLevel;
    ApplyVolume((int)level, 1);
    pan = pan * SOUND_PAN_SCALE;
    if (pan < SOUND_PAN_LEFT)
        pan = SOUND_PAN_LEFT;
    else if (pan > SOUND_PAN_RIGHT)
        pan = SOUND_PAN_RIGHT;
    buffer->SetPan((int)pan);
    return 0;
}
