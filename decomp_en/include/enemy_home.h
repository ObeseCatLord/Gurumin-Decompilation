// Returning-home test of an enemy (EN build; JP: enemy_home.h). This is the one declaration of this function in
// decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_ENEMY_HOME_H
#define GURUMIN_EN_ENEMY_HOME_H

// 1 when enemy `index` may head for its start position in direction (dirX, dirY): always when `distance` is below
// `minimum` or below `minimumHome`; otherwise only when nothing blocks the way to the start position and that way is
// less than 150 degrees away from the direction. The sixth and seventh parameters are not used. EN 0x5c15f0 (JP 0x10024d00).
extern "C" int Enemy_CanReturnHome(int index, float distance, float minimumHome, float dirX, float dirY, int unused6,
                                   int unused7, float minimum);

#endif
