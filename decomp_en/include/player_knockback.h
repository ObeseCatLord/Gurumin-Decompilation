// The player knock-back state (EN build; JP: Player_KnockBack in APP.dll 0x10055ee0 and the DAT_ globals it uses). This
// is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_PLAYER_KNOCKBACK_H
#define GURUMIN_EN_PLAYER_KNOCKBACK_H

extern "C" int g_playerIndex;                 // EN 0x00a207b8 (JP DAT_104bf8b4): slot of g_characterModels the player controls
extern "C" int g_modeBlockingKnockBack;       // EN 0x009e15f4 (JP DAT_104c0068): game state; 1 blocks the knock-back
extern "C" int g_stateBlockingKnockBackA;     // EN 0x00915a00 (JP DAT_104bfdf0): non-zero blocks the knock-back
extern "C" unsigned char g_playerStateByte;   // EN 0x009e3366 (JP DAT_104c009a): player state, 8 and above block the knock-back
extern "C" unsigned char g_playerHitFlagA;    // EN 0x009e3393 (JP DAT_104c010c): cleared by the knock-back
extern "C" unsigned char g_playerHitFlagB;    // EN 0x00a13968 (JP DAT_104c0154): set to 1 by the knock-back
extern "C" int g_getUpPending;                // EN 0x0091d68c (JP DAT_104bfe30): 1 = the player gets up instead of being knocked back
extern "C" int g_knockBackBlocked;            // EN 0x0091da24 (JP DAT_104bfe3c): non-zero blocks the knock-back
extern "C" unsigned char g_playerActionFlag;  // EN 0x009e3368 (JP DAT_104c009c): cleared when a knock-back or get-up starts
extern "C" int g_hitShakeFrames;              // EN 0x00bc885c (JP DAT_1020d430): set to 15 by the knock-back
extern "C" int g_hitShakeKind;                // EN 0x00bc8864 (JP DAT_1032d6a4): set to 9 by the knock-back
extern "C" int g_hitShakeStep;                // EN 0x00bc886c (JP DAT_1020d4b8): cleared by the knock-back
extern "C" int g_hitShakeOffset;              // EN 0x00bc8858 (JP DAT_104bfdb0): set to -30 by the knock-back
extern "C" int g_carryActive;                 // EN 0x00914544 (JP DAT_104bfdc8): 1 while the player carries an object
extern "C" int g_carryState;                  // EN 0x00914548 (JP DAT_104bfdcc): cleared when the carried object is dropped
extern "C" unsigned char g_carryDropped;      // EN 0x009e335c (JP DAT_104c0090): set to 1 when the carried object is dropped
extern "C" int g_carryTimer;                  // EN 0x0091454c (JP DAT_104bfdd0): cleared by the knock-back
extern "C" unsigned char g_knockBackCountdown;  // EN 0x009e336a (JP DAT_104c009e): frames left, counts down every call
extern "C" unsigned char g_sceneResetFlag;    // EN 0x00a14f84 (JP DAT_104c0171): cleared with the battle state reset
extern "C" int g_idleRestartPending;          // EN 0x00bc8ea0 (JP DAT_104687b0): set to 1 when the player's idle motion is on
extern "C" unsigned char g_knockBackSoundFlag;  // EN 0x0092710d (JP DAT_1021a360): cleared by the knock-back
extern "C" int g_knockBackStarted;            // EN 0x00a15290 (JP DAT_104c01e8): set to 1 by the knock-back
extern "C" int g_menuBlockingKnockBack;       // EN 0x0091dbcc (JP DAT_104bfe4c): non-zero clears g_playerHitFlagA
extern "C" int g_stateClearingHitFlag;        // EN 0x0091dda8 (JP DAT_104bfe50): non-zero clears g_playerHitFlagA
extern "C" unsigned char g_playerRecovered;   // EN 0x00a13969 (JP DAT_104c0155): cleared when the knock-back ends

#endif
