// Enemy voice lines (enemy_voice.cpp).
#ifndef GURUMIN_ENEMY_VOICE_H
#define GURUMIN_ENEMY_VOICE_H

// 0x100b9bb0 (Enemy_PlayVoice): plays voice line `sound` of character `index`; `reset` restarts it, `limited` keeps one at a time.
extern "C" int FUN_100b9bb0(int index, int sound, int reset, int limited);

#endif
