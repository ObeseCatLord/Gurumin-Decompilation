// Greeting state of the characters (EN build; JP: char_tables.h, char_vars.h).
#ifndef GURUMIN_EN_NPC_GREET_H
#define GURUMIN_EN_NPC_GREET_H

#include "char.h"

extern "C" int g_greetingArea;                       // EN 0x009e33b4 (JP DAT_104c00e8): the map area (greetings only happen in some areas)
extern "C" int g_greetCountdown[CHAR_SLOT_COUNT];    // EN 0x0091d7a0 (JP DAT_10254390): greeting countdown per character (GREET_NOW = greet now)

extern "C" int g_greetingForgetTimer;                // EN 0x00915ec8 (JP DAT_104c0378): frames until the greeted character is forgotten once the player walked away (0 = not counting)
extern "C" int g_greetingVoice;                      // EN 0x00915cb0 (JP DAT_104c0368): voice line number of the greeted character (0 = none)
// Keeps the voice of the character the player stands next to (see npc_greeting.cpp). Returns 0. EN 0x5c1080 (JP 0x100ba040).
extern "C" int Npc_PlayGreeting(void);

// Greeting of character `index`: counts down while the player is near and then makes the character turn to the player and
// show a greeting balloon. Returns 0. EN 0x5c1840 (JP 0x100b8220).
extern "C" int Npc_Greet(int index);

// Idle NPC that reacts when the player comes near: speaks now and then, and when it reacts wakes nearby enemies, plays a sound,
// shows a prompt and turns to the player. Returns 0. EN 0x6d5a80 (JP 0x100b5550, see npc_greet_near.cpp).
extern "C" int Npc_GreetWhenNear(int index);

#endif
