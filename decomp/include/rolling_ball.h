// The rolling ball scene (rolling_ball.cpp, behaviour update 0x1007a370) and the two globals that start it.
#ifndef GURUMIN_ROLLING_BALL_H
#define GURUMIN_ROLLING_BALL_H

extern "C" int DAT_10c63524;   // ball scene phase: 1 = a character is to be dropped in, 0 = the ball node is to be found, 2 = dropped
extern "C" int DAT_10c655e8;   // number of the ball scene (script flag 0x19a + this is set when the dropped-in character lands)

// 0x1007a370 (Char_RollingBall, rolling_ball.cpp)
int Char_RollingBall(int index);

#endif
