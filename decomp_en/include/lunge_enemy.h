// The hand-swinging stalker (EN build; JP: decomp/include/lunge_enemy.h). Declared once here. Used by src/enemy_lunger.cpp.
#ifndef GURUMIN_EN_LUNGE_ENEMY_H
#define GURUMIN_EN_LUNGE_ENEMY_H

// One frame of the lunging stalker `index`: stalks, circles, retreats and lunges with a swing of its hand. Returns 0.
// C++ linkage (the original has an exception frame). EN 0x6a3ef0 (JP 0x100ad0f0).
int Enemy_StalkAndLunge(int index);

#endif
