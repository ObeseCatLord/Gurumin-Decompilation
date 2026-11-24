// Removing a wave enemy from play (EN build).
#include "char.h"

// Retires character `index` once: moves it out of the world (matrix x = -100000 - x), records its two-digit formation
// number (values[1]: tens = row, ones = column) in the 10x10 wave grid, marks the state as retired (state = 1) and hides
// its marker. Called by the NPC dispatcher when a wave enemy is released. Returns 0. EN 0x6d64e0 (JP 0x1001b7b0).
extern "C" int Char_RetireWaveEnemy(int index)
{
    if (g_charState[index].state == 0)
    {
        g_charInPlay[index] = 0;
        g_charState[index].state = 1;
        g_waveEnemiesAlive = 1;
        g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] = CHAR_RETIRE_OFFSET - g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X] * 1.0f;
        int formation = g_charState[index].values[1];
        g_charState[index].values[3] = formation % WAVE_GRID_SIZE;
        g_waveGrid[formation / WAVE_GRID_SIZE % WAVE_GRID_SIZE][formation % WAVE_GRID_SIZE] = (char)index;
        g_charMarkers[index].y = CHAR_MARKER_HIDDEN;
    }
    return 0;
}
