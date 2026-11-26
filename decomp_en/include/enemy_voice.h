// Enemy voices with a cooldown (EN build; JP: Enemy_PlayVoice in APP.dll 0x100b9bb0). Declared once here.
#ifndef GURUMIN_EN_ENEMY_VOICE_H
#define GURUMIN_EN_ENEMY_VOICE_H

// Non-zero = the voice cooldown is off: voices requested as `limited` play at once. EN 0x01336a68 (JP DAT_10c66cc0).
extern "C" int g_voiceLimitsOff;

// Plays voice `sound` of enemy `index` unless its cooldown is running (`limited` voices are skipped entirely unless
// g_voiceLimitsOff is set); only some kinds of enemy speak, each keeps its cooldown in one word of the state of its own
// slot or of a neighbour. `reset` clears the cooldown first. After playing, the sound sets the cooldown (10 to 285
// frames). Returns 0. EN 0x67fcc0 (JP 0x100b9bb0). Called by the enemy behaviours.
extern "C" int Enemy_PlayVoice(int index, int sound, int reset, int limited);

#endif
