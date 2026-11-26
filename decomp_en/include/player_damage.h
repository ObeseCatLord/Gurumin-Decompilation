// The player taking damage (EN build; JP: decomp/src/player_damage.cpp). This is the one declaration of these globals
// and functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_PLAYER_DAMAGE_H
#define GURUMIN_EN_PLAYER_DAMAGE_H

#include "char.h"

extern "C" unsigned char g_charProtectFrames[CHAR_SLOT_COUNT];   // EN 0x009e1990 (JP DAT_10253e98): frames of protection left after a lava or trap hit
extern "C" char g_healthGaugeChange;                              // EN 0x009e33e6 (JP DAT_104c011d): last change shown by the health gauge (8 = a gain)

extern "C" int g_warpBoarded;                    // EN 0x009e339c (JP DAT_104c00c8): non-zero while the player is inside a warp (damage is ignored)
extern "C" char g_extraLife;                     // EN 0x009e338e (JP DAT_104c00cc): non-zero = an extra life is held; 2 = it was used up by this hit
extern "C" int g_invulnerable;                   // EN 0x009e3388 (JP DAT_104c00b8): non-zero = the player takes no damage
extern "C" char g_damageReduced;                 // EN 0x009e335f (JP DAT_104c0093): non-zero = damage is reduced to two thirds
extern "C" int g_gaugeFrozen;                    // EN 0x01336aa8 (JP DAT_10c66d00): non-zero = hits do not lower the drill gauge

// Plays voice line `sound` at `position` with `volume` unless the voices are off (g_voicesOff). Returns the result of
// Sound_PlayEffect, or 0 when the voices are off. EN 0x6e05d0.
extern "C" int Sound_PlayVoice(int sound, const Vec4 *position, float volume);

// Takes `damage` from the player's health unless a scene, menu or pause state blocks it. Returns 0. EN 0x5c9c50 (JP 0x1005dbf0).
extern "C" int Player_TakeDamage(int damage);

#endif
