// Globals of the scene frame (scene_frame.cpp) that no other header declares.
#ifndef GURUMIN_SCENE_FRAME_VARS_H
#define GURUMIN_SCENE_FRAME_VARS_H

extern "C" int DAT_10c64f78;      // 1 while the stamina gauge stands at its full value (400 with the aura, else 300)
extern "C" int DAT_10c65684;      // non-zero = the "@cr" root rotator turns the fast way
extern "C" int DAT_10c66d40[4];   // map 0xc: per switch, non-zero = the switch is held down
extern "C" short DAT_104477b0;    // sound arc command played when the sound event ends (-11 = none)
extern "C" short DAT_104477ae;    // second sound arc command (-11 = none)

extern "C" const unsigned short DAT_101d6564[];   // frames a person waits before waking, by delay class (150, 300, 450, 900)
extern "C" int DAT_10c64014;      // the player's health at the last frame (the timed run counts the hits)
extern "C" int DAT_10c635ac;      // 1 while the timed run's clock is stopped (cutscene or pause)

#endif
