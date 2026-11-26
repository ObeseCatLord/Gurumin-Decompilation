// EN-only launcher: the Othello ("Osero") move rules: playing a move on the packed board (Ose_PlayMove).
// Draft, not matched yet (open.tsv): the original keeps this out of line from COseDlg::OnTimer although both live in
// the same object; a definition next to OnTimer gets inlined there, so the draft lives in its own file.
#include "launcher_dialog.h"

// Column steps (EN 0x00908990) and row steps (EN 0x009089b0, the next 32 bytes) of the 8 directions, starting at the
// left and turning clockwise.
int g_oseDirectionColumn[OSE_DIRECTIONS] = { -1, -1, 0, 1, 1, 1, 0, -1 };
int g_oseDirectionRow[OSE_DIRECTIONS] = { 0, -1, -1, -1, 0, 1, 1, 1 };

// Plays a move of `color` at (column, row) on the packed board `board`: unpacks it, and when the cell is empty
// flips the opponent pieces enclosed in each of the 8 directions, then repacks the board. Returns 1 when something was
// flipped (legal move), otherwise 0 and the board is left alone. Called by OnTimer (computer) and OnLButtonDown.
int Ose_PlayMove(int *board, unsigned int column, unsigned int row, int color)
{
    unsigned char cells[OSE_CELL_COUNT];

    int cell = 0;
    for (int word = 0; word < OSE_BOARD_WORDS; word++) {
        for (int shift = 0; shift < 32; shift += 2) {
            cells[cell++] = (board[word] >> shift) & 3;
        }
    }
    int flipped = 0;
    int index = row + column * OSE_BOARD_SIZE;
    unsigned char *target = &cells[index];
    if (cells[index] == OSE_CELL_EMPTY) {
        for (unsigned int direction = 0; direction < OSE_DIRECTIONS; direction++) {
            int columnStep = g_oseDirectionColumn[direction];
            int rowStep = g_oseDirectionRow[direction];
            int adjacent = 1;   // the first cell met must be an opponent piece
            unsigned int c = column;
            unsigned int r = row;
            for (int distance = 1; distance < OSE_BOARD_SIZE; distance++) {
                c += columnStep;
                r += rowStep;
                if (c <= OSE_BOARD_SIZE && r <= OSE_BOARD_SIZE) {
                    unsigned char piece = cells[r + c * OSE_BOARD_SIZE];
                    if (adjacent) {
                        if (piece == OSE_CELL_EMPTY || piece == color) {
                            break;
                        }
                    } else {
                        if (piece == OSE_CELL_EMPTY) {
                            break;
                        }
                        if (piece == color) {
                            flipped = 1;
                            if (distance > 0) {
                                unsigned char *flip = target;
                                for (int n = 0; n < distance; n++) {
                                    *flip = (unsigned char)color;
                                    flip += rowStep + columnStep * OSE_BOARD_SIZE;
                                }
                            }
                        }
                    }
                    adjacent = 0;
                }
            }
        }
        if (flipped) {
            cell = 0;
            for (int word = 0; word < OSE_BOARD_WORDS; word++) {
                *board = 0;
                int packed = 0;
                for (int slot = 0; slot < 16; slot += 4) {
                    packed |= cells[cell] << (slot * 2);
                    packed |= cells[cell + 1] << (slot * 2 + 2);
                    packed |= cells[cell + 2] << (slot * 2 + 4);
                    packed |= cells[cell + 3] << (slot * 2 + 6);
                    cell += 4;
                }
                *board = packed;
                board++;
            }
            return 1;
        }
    }
    return 0;
}
