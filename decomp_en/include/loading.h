// State of the loading indicator of the EN build (JP: DAT_104c031c, DAT_104c0320, DAT_104c032c). This is the one
// declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_LOADING_H
#define GURUMIN_EN_LOADING_H

extern "C" unsigned long g_loadingLastTick;   // EN 0x00bc87cc (JP DAT_104c032c): tick count of the last loading-screen update
extern "C" float g_loadingSpinnerAngle;       // EN 0x00b91ec4 (JP DAT_104c031c): angle of the spinner
extern "C" float g_loadingProgress;           // EN 0x00bc869c (JP DAT_104c0320): loading progress in percent

// Redraws the loading indicator at most every 34 ms and handles window messages. Returns 1, or 0 once the game is
// quitting. EN 0x602f70 (JP 0x1000fb80).
extern "C" int Loading_Tick(void);

#endif
