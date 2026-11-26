// Frame timer, data paths and random table of the EN build (JP: decomp/include/win_vars.h, map_vars.h, script_vars.h).
#ifndef GURUMIN_EN_ENGINE_H
#define GURUMIN_EN_ENGINE_H

const int FRAME_MS = 16;                  // milliseconds of one 60 Hz frame (deadline step)
const int PATH_SIZE = 0x100;              // bytes of one formatted path
const int DATA_PATH_BUFFERS = 4;          // rotating buffers of MakeDataPath
const int FOLDER_PATH_BUFFERS = 16;       // rotating buffers of Path_Data
const int RANDOM_TABLE_SIZE = 0x10000;    // entries of the random table (the position wraps at 16 bits)

extern "C" unsigned long g_nextFrameDeadline;                   // EN 0x021e47c4 (JP DAT_10f654d8): timeGetTime() deadline of the next frame
extern "C" char g_dataFolder[];                                 // EN 0x021e8c48 (JP DAT_10f627d0): the game's data folder
extern "C" int g_dataPathCounter;                               // EN 0x021f32c0 (JP DAT_10f0edf8): count of MakeDataPath calls
extern "C" char g_dataPathBuffers[DATA_PATH_BUFFERS][PATH_SIZE];  // EN 0x021f22c0 (JP DAT_10f64368): results of MakeDataPath
extern "C" char g_folderNames[][PATH_SIZE];                     // EN 0x00908b38 (JP DAT_10202b40): data folder names
extern "C" int g_folderPathCounter;                             // EN 0x021e8e84 (JP DAT_10f1fd24): count of Path_Data calls
extern "C" char g_folderPathBuffers[FOLDER_PATH_BUFFERS][PATH_SIZE];  // EN 0x021f12c0 (JP DAT_10f19860): results of Path_Data
extern "C" unsigned short g_randomPosition;                     // EN 0x021a3604 (JP DAT_10f65558): position in the random table
extern "C" int g_randomTable[RANDOM_TABLE_SIZE];                // EN 0x021a3608 (JP DAT_10f20590): the random table

const int MESSAGES_PER_PUMP = 30;         // window messages handled per Message_Pump call
const int GAME_SCREEN_WIDTH = 640;        // width of the game's 2D coordinate space
const int GAME_SCREEN_HEIGHT = 480;       // height of the game's 2D coordinate space

extern "C" int g_frameBlendActive;                              // EN 0x021e8d54 (JP DAT_10f65498): 1 while the frame blend is active
extern "C" float g_frameBlendStrength;                          // EN 0x021e8d60 (JP DAT_10f6549c): current blend strength
extern "C" float g_frameBlendStep;                              // EN 0x021e8d64 (JP DAT_10f1fcf8): strength change per update
extern "C" float g_frameBlendLimit;                             // EN 0x021e8d68 (JP DAT_10f628d0): upper limit of the strength
extern "C" int g_frameBlendSupported;                           // EN 0x021e47e0 (JP DAT_10f654f4): non-zero when the device supports blending
extern "C" int g_singleFrameBuffer;                             // EN 0x00909338 (JP DAT_10202b0c): 1 when only one frame buffer exists

extern "C" int FrameTimer_ResetNext2(void);                     // EN 0x7a40d0 (JP 0x1013faf0)
extern "C" int FrameTimer_ResetNext(void);                      // EN 0x7a40f0 (JP 0x1013eee0)
extern "C" char *MakeDataPath(const char *name);                // EN 0x7a31a0 (JP 0x1013f910)
extern "C" const char *Path_Data(const char *name, int folder); // EN 0x7a31f0 (JP 0x1013f8a0)
extern "C" int FrameBlend_Start(int active, int target, int frames);  // EN 0x77c840 (JP 0x1013fb10)
extern "C" int Random(void);                                    // EN 0x7a3a30 (JP 0x1013f880)
// Loads the random table fontr.dat and stores `value` as the table position (C++ linkage). EN 0x7a4280 (JP 0x1014b1c0).
int FontRandom_Load(short value);

#endif
