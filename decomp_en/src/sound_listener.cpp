// The 3D sound listener follows the camera (EN build). Ported from the matched JP source decomp/src/sound_listener.cpp.
#include "camera.h"
#include "game_vars.h"
#include "sound_listener.h"

// The scale factors are written as literals: Vec4::operator* takes its factor by reference, so a named constant would
// not compile to the same code. Camera smoothing: 0.8 of the current vector plus 0.2 of the goal. The listener sits
// at 0.15 times the midpoint (0.5) between camera position and target, the sound world being smaller.
// Scale of the listener position kept in the copy (applied a second time to the position sent).
const float LISTENER_POSITION_SCALE = 0.15f;
// Volume and up-vector z the listener command is sent with.
const float LISTENER_VOLUME = 1.0f;
const float LISTENER_UP_Z = -1.0f;

// Sound_UpdateListener: moves the camera position and target 20 % towards their goals (unless locked), then places
// the listener at 0.15 times the point halfway between target and position with the view direction, sends it to
// the sound process (unless sound is muted) and keeps a copy (the position scaled by 0.15 once more). Returns 0.
// Called every frame by the game update.
extern "C" int Sound_UpdateListener(void)
{
    if (g_cameraSmoothingLocked == 0)
    {
        g_cameraPositionNow.Copy(g_cameraPositionNow * 0.8f + g_cameraPositionCopy * 0.2f);
        g_cameraTargetNow.Copy(g_cameraTargetNow * 0.8f + g_cameraTargetCopy * 0.2f);
    }
    // The listener has its own scope: its vectors share the stack with the smoothing temporaries.
    {
        Vec4 direction;
        direction.Copy(g_cameraTargetNow - g_cameraPositionNow);
        direction.Normalized3();
        Vec4 position;
        position.Copy(g_cameraTargetNow + (g_cameraPositionNow - g_cameraTargetNow) * 0.5f);
        position.Copy(position * 0.15f);
        if (g_soundMuted == 0)
            g_soundArcSetListener(LISTENER_VOLUME, position.x, position.y, position.z, direction.x, direction.y, direction.z,
                                  0, 0, LISTENER_UP_Z);
        g_listenerPosition.Set(position.x * LISTENER_POSITION_SCALE, position.y * LISTENER_POSITION_SCALE,
                               position.z * LISTENER_POSITION_SCALE, 0);
        g_listenerDirection.Set(direction.x, direction.y, direction.z, 0);
    }
    return 0;
}
