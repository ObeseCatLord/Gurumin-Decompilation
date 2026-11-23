// The music's beat/spectrum data (read from the .de2 header by Bgm_Play) and the smoothed bar heights derived from it.
#ifndef GURUMIN_BGM_SPECTRUM_H
#define GURUMIN_BGM_SPECTRUM_H

#define BGM_SPECTRUM_BANDS 32                                 // bands per channel
#define BGM_SPECTRUM_FRAME_SIZE 0x50                          // bytes per frame (two channels of 0x28)
#define BGM_SPECTRUM_SAMPLES_PER_FRAME 0x5be                  // 44.1 kHz samples per spectrum frame

#define BGM_SPECTRUM_RIGHT_OFFSET 0x28                        // the right channel's levels follow the left's (and 8 other bytes)

extern "C" unsigned char DAT_102564a8[];                      // frames of the playing track: per frame a level (0-255) per band for the left, then the right channel (the .de2 header after its checks)
extern "C" float DAT_104bff20[2 * BGM_SPECTRUM_BANDS];        // smoothed band levels: left channel, then right
extern "C" int DAT_104c014c;                                  // non-zero = the music position is read from soundarc

// 0x1005c680 (Bgm_Update).
extern "C" int FUN_1005c680(void);

#endif
