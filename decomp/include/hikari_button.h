// The light platform with a call button (behaviour of the per-kind update 0x100f9670, hikari_button.cpp). It is the
// route-riding light of char_hikari.h with a "Box01" button model on it: the light waits at its start with the button
// red, the player stepping off it presses the button ("button" node) and the light sets off along its route.
#ifndef GURUMIN_HIKARI_BUTTON_H
#define GURUMIN_HIKARI_BUTTON_H

#include "char_hikari.h"

#define HIKARI_WIDE_UNIT 100000        // route point kind 10 packs x * 100000 + y
#define BUTTON_NODE "Box01"            // the button's model node
#define BUTTON_MODEL "O_015k.it3"      // loaded as a child of the light when it has no button node yet
#define BUTTON_MODEL_SIZE 0x40
#define BUTTON_PRESS_NODE "button"
#define BUTTON_MOTION_PRESS "up"
#define BUTTON_MOTION_ARMED "red"
#define BUTTON_MOTION_IDLE "blue"
#define BUTTON_MOTION_OFF "stop"
#define LIFT_MOTION_RUN "anime"

#define BUTTON_READY_PAUSED 3          // CharState::hikariReady while the light is switched off
#define BUTTON_MODE_START 99           // CharState::behaviour: set the light up again
#define BUTTON_MODE_WAITING 0x457      // waiting for the player to leave the platform
#define BUTTON_WAIT_DISTANCE 50.0f     // the player this far from the platform presses the button
#define BUTTON_RIDER_LIMIT 150.0f      // the player nearer than this rides the platform
#define BUTTON_RIDER_LIFT 20.0f        // the player is lifted this much when the platform stops under him
#define BUTTON_LIFT_RISING 2
#define BUTTON_SOUND_START 0x37b
#define BUTTON_SOUND_ARRIVE 0x37c
#define BUTTON_PLAYER_SLOT 0

extern "C" int FUN_10065b10(int index, int sound, int always, int optional);   // Character_PlaySound

#endif
