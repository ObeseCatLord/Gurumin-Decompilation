// Constructor of the movie player (EN build, EN-only: no JP twin found).
#include "mmv_player.h"

// Starts the player empty: the movie file is closed, the movie does not loop, no movie is held (ownsFrame, playing and
// state are 0) and neither the movie memory nor the frame buffer is allocated. No direct caller (the movie player of
// the scenes is a global constructed at start-up).
MmvPlayer::MmvPlayer()
{
    loop = 0;
    memory = 0;
    ownsFrame = 0;
    playing = 0;
    frame = 0;
    state = 0;
}
