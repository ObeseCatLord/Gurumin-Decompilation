// The fighter of kind 0x2d (enemy_fighter.cpp, run from Char_UpdateByKind 0x100f9670): a duelling enemy that circles
// the player, picks one of its attack steps (slashes, kicks, a dash, a ranged shot) and plays it with swing effects.
// Its twin model (the slot two further on, DAT_10db6ee8[index + 2]) fades in and out of the visible one when it is
// hit; slot 0x3a is the first fighter and the one that plays the defeat sequence with the camera.
#ifndef GURUMIN_ENEMY_FIGHTER_H
#define GURUMIN_ENEMY_FIGHTER_H

#include "kind_counter.h"
#include "char_tables.h"
#include "enemy_voice.h"
#include "actor_scan.h"
#include "volume_fade.h"

// 0x100bdad0 (Enemy_UpdateFighter; C++ linkage: the original has an EH frame for its matrices and effect options).
// One frame of the fighter in slot `index`. Returns 0 (1 while the first fighter's defeat sequence runs).
int Enemy_UpdateFighter(int index);

// The behaviour words of CharState (DAT_1020b430[index].words[...], 0x04 apart): the fighter's variables.
#define CHAR_FIGHTER_KIND 0x2d
#define FIGHTER_WORD_STEP 1           // 0x04: attack step being played (0..0x15, see FIGHTER_STEP_*)
#define FIGHTER_WORD_FRAME 3          // 0x0c: frames the step has run (-1 while a new one is picked)
#define FIGHTER_WORD_HEALTH_MARK 4    // 0x10: health at the last check (the guard reacts when it drops)
#define FIGHTER_WORD_GUARD_FRAMES 5   // 0x14: frames left of the guard stance after a hit
#define FIGHTER_WORD_SIDE 7           // 0x1c: 0 or 1: the side it circles the player on
#define FIGHTER_WORD_BLOCKED 8        // 0x20: 1 while a wall or a gap is ahead of it
#define FIGHTER_WORD_PROTECT 10       // 0x28: frames of protection after its health changed
#define FIGHTER_WORD_PREVIOUS_STEP 11 // 0x2c: the step before the one now played
#define FIGHTER_WORD_DEFEAT_FRAMES 13 // 0x34: frames of the defeat sequence

// The attack steps (CharState word 1).
#define FIGHTER_STEP_CHASE_NEAR 0
#define FIGHTER_STEP_GUARD 1
#define FIGHTER_STEP_CIRCLE 4
#define FIGHTER_STEP_JUMP_BACK 5
#define FIGHTER_STEP_KICK_FIRST 6
#define FIGHTER_STEP_STRIKE_FIRST 0xc
#define FIGHTER_STEP_STAGGER 0x15

// Variables beyond the 0x40 bytes of a CharState record: one int every 0x40 bytes like KindCounter, each laid over
// the CharState table (see char_tables.h). Used only by the fighter unless said otherwise.
extern "C" KindCounter DAT_1020b470[CHAR_SLOT_COUNT];     // 1 while a motion is held (forced motion below)
extern "C" KindCounter DAT_1020b474[CHAR_SLOT_COUNT];     // layer of the held motion
extern "C" KindCounter DAT_1020b478[CHAR_SLOT_COUNT];     // frame of the held motion
extern "C" KindCounter DAT_1020b47c[CHAR_SLOT_COUNT];     // 1 once the held motion started
extern "C" KindCounter DAT_1020b480[CHAR_SLOT_COUNT];     // 1 while it may not be moved (an attack is on)
extern "C" KindCounter DAT_1020b484[CHAR_SLOT_COUNT];     // 1 while the swing trail is shown
extern "C" KindCounter DAT_1020b488[CHAR_SLOT_COUNT];     // health still to be given back
extern "C" KindCounter DAT_1020b48c[CHAR_SLOT_COUNT];     // the value of CharSlot::stats[STAT_VALUE_4] to restore
extern "C" KindCounter DAT_1020b490[CHAR_SLOT_COUNT];     // how often the held motion raised its aggression
extern "C" KindCounter DAT_1020b494[CHAR_SLOT_COUNT];     // 1 = down, 2 = down and not recovering (the clone phase)
extern "C" KindCounter DAT_1020b4a0[CHAR_SLOT_COUNT];     // counter of the screen fade of the other fighters
extern "C" KindCounter DAT_1020b4a4[CHAR_SLOT_COUNT];     // 1 once the defeat sequence of this fighter started
extern "C" KindCounter DAT_1020b4a8[CHAR_SLOT_COUNT];     // step of the swap between the model and its twin (0 = none)
extern "C" KindCounter DAT_1020b4ac[CHAR_SLOT_COUNT];     // frames of the model fade after the swap
extern "C" KindCounter DAT_1020b4b0[CHAR_SLOT_COUNT];     // hit flash counter (negative while it fades)

extern "C" int FUN_10012450(int index, int attach, const char *name);         // 0x10012450 (char_attach.cpp): loads a child model

#endif
