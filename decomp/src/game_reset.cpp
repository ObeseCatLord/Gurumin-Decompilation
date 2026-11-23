// Resetting the game state for a new game or before loading one (called by Game_Startup and Game_LoadData): every
// game global is cleared, the HUD textures are released, the camera, sound, scene and script systems reset and the
// player's parameters set back to their starting values.

#include "texture_manager.h"
#include "tri_batch.h"
#include "mmv_player.h"
#include "last_position.h"
#include <string.h>
#include "char_slot.h"
#include "char_state.h"

#include "vec4.h"

#include "matrix4.h"

Matrix4 IdentityMatrix();                        // 0x101b0ba0

#include "game_reset.h"

#include "node.h"

#include "model_bank.h"

#include "effect_manager.h"


#include "listbank.h"
#include "anim_model.h"

#include "camera_vars.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_vars.h"
#include "script_vars.h"
#include "sound_vars.h"
#include "ui_vars.h"
#include "world.h"
extern "C" int DAT_10202464;
extern "C" int DAT_1020d860[0x60];
extern "C" int DAT_10219c40[0x40];
extern "C" int DAT_10219fc0[0x40];
extern "C" int DAT_1021f640[0x20];
extern "C" int DAT_10255b08[0x40];
extern "C" int DAT_1030bbf8[0x40];
extern "C" int DAT_1042de28[0x20];
extern "C" int DAT_1042e0a8[0x40];
extern "C" int DAT_104463f8[0x80];
extern "C" int DAT_10447fd8[0x80];
extern "C" int DAT_104481d8[0x80];
extern "C" int DAT_10453208[0x4000];
extern "C" int DAT_10467c20[0x80];
extern "C" int DAT_10468230[0x80];
extern "C" GameObject DAT_104b7200;
extern "C" int DAT_104bfb08[0x80];
#include "win_vars.h"
extern "C" EmptyObject DAT_1065cb28;
extern "C" Vec4 DAT_108029b8;
extern "C" SmallRecord DAT_10860de8[0x40];

extern "C" int FUN_10001b40(void);               // SoundArc_Message221
extern "C" int FUN_10174c70(int slot, int force);  // Texture_Release
extern "C" void FUN_10188fb0(void);              // Texture_ReloadMissing
extern "C" int TestDat_Load(void);               // 0x1000beb0
int FUN_1014b1c0(short value);                   // type_parse.cpp
extern "C" int FUN_10001bf0(int stream, int fadeTime);
extern "C" int FUN_10161920(const char *name);   // Texture_FindByName
extern "C" int FUN_10023d30(void);
extern "C" int FUN_100154c0(int value);
extern "C" int FUN_1015fd30(void);
extern "C" int FUN_1016b010(int value);
extern "C" int FUN_1013fb10(int a, int b, int c);
extern "C" void FUN_101997f0(void);
extern "C" int FUN_1016a980(void);               // Camera_Reset
extern "C" int FUN_101a5860(void);
extern "C" void FUN_10161980(void);
extern "C" int FUN_1000d9d0(void);

// 0x1003e710 (Game_Reset). Clears the game globals and resets the subsystems for a new game (see above).
extern "C" void FUN_1003e710(void)
{
  DAT_104c0360 = 0;
  DAT_104c0148 = 0;
  DAT_102559f4 = 0;
  DAT_102559f8 = 0;
  DAT_102559fc = 0;
  DAT_10255a00 = 0;
  DAT_104c033a = 0;
  DAT_10219bb8 = 0;
  DAT_104c0330 = 0;
  DAT_104c0331 = 0;
  if (DAT_104c01b0 == 0) {
    for (int i = 0; i < 0x44c; i++)
      DAT_10447338[i] = -11;
    if (FUN_10001b40() != 0)
      FUN_10001b40();
  }
  DAT_104c030a = 0;
  DAT_104c030c = 0;
  DAT_104c0310 = 0;
  DAT_104c0334 = 0;
  DAT_10203420 = 1.0f;
  DAT_10f654f0 = 0;
  if (DAT_104b7210 >= 0)
    FUN_10174c70(DAT_104b7210, 0);
  DAT_104b7210 = -1;
  for (int i = 0; i < 0xf; i++)
  {
    if (DAT_10202808[i] >= 0)
      FUN_10174c70(DAT_10202808[i], 0);
    DAT_10202808[i] = -1;
  }
  FUN_10188fb0();
  memset(DAT_10202808, 0xff, sizeof(DAT_10202808));
  DAT_104c0319 = 0;
  DAT_104c0318 = 0;
  DAT_104c0309 = 0;
  DAT_104c0308 = 0;
  DAT_1021fb40 = 0;
  DAT_1021fb44 = 0;
  DAT_104c02f4 = 0;
  DAT_104c02f8 = 0;
  DAT_104c02fc = 0;
  DAT_104c02ee = 0;
  DAT_104c02f0 = 0;
  DAT_104c0300 = 0;
  DAT_104c0301 = 0;
  DAT_104c0304 = 0;
  DAT_104c02ec = 0;
  DAT_104c02e7 = 0;
  DAT_104c02e6 = 0;
  DAT_1101d87c = 0;
  TestDat_Load();
  DAT_104c023e = 0;
  DAT_104c023c = 0;
  FUN_1014b1c0(0);
  DAT_10202800 = 1;
  DAT_10f654f8 = 0;
  DAT_10f654fc = 0;
  DAT_104bfd58 = 0;
  DAT_10202358 = 0xff;
  DAT_104bfd54 = 0;
  if (DAT_104c01b0 == 0)
  {
    FUN_10001bf0(0, 1);
    FUN_10001bf0(1, 1);
  }
  DAT_104c013c = 0;
  DAT_104c0140 = FUN_10161920("lens05");
  DAT_1021cffc.SetBuffer(DAT_10223a58, 0x400);
  memset(DAT_10254c40, 0, sizeof(DAT_10254c40));
  DAT_10804bf8.Reset();
  DAT_102027e4 = 180.0f;
  DAT_104c0224 = 0;
  DAT_1083631c.Fill(0.0f);
  for (int i = 0; i < 0x10; i++)
    DAT_10802ad8[i].Fill(0.0f);
  DAT_104c0228 = 0;
  DAT_104c022c = 0;
  DAT_104c022d = 0;
  DAT_10802bd8.Fill(0.0f);
  memset(DAT_10254310, 0, sizeof(DAT_10254310));
  memset(DAT_1020d438, 0, sizeof(DAT_1020d438));
  DAT_104c022e = 0;
  DAT_102027e8 = 1.0f;
  DAT_104c0234 = 0;
  DAT_104c0232 = 0;
  DAT_104bfe20 = 0;
  DAT_10202464 = 0;
  DAT_10202468 = 0;
  DAT_1020246c = 0;
  DAT_10202470 = 0;
  DAT_10202474 = 0;
  DAT_104c0210 = 0;
  DAT_104c0214 = 0;
  DAT_104c0218 = 0;
  DAT_104bfe90 = 0;
  DAT_104c021c = 0;
  DAT_104c0224 = 0;
  DAT_1083631c.Fill(0.0f);
  DAT_104c011d = 0;
  DAT_104c01fc = 0;
  DAT_104c020c = 0;
  DAT_104c01ec = 0;
  DAT_104c01f0 = 0;
  DAT_102027e4 = 180.0f;
  DAT_104c0204 = 0;
  DAT_104c01dc = 0;
  DAT_104c01e0 = 0;
  DAT_104c01e4 = 0;
  DAT_104c01e8 = 0;
  FUN_10023d30();
  DAT_104c01a8 = 0;
  DAT_104c01ac = 0;
  DAT_104c01d8 = 0;
  DAT_104c01cc = 0;
  DAT_104c01c8 = 0;
  DAT_104c01a4 = 0;
  DAT_104c019c = 0;
  DAT_104c03a4 = 0;
  DAT_102028dc = 1;
  DAT_104c03b8 = 0;
  DAT_104c03bc = 0;
  DAT_104c0190 = 0;
  if (DAT_10255f08 != 0)
    DAT_10255f08->Release();
  if (DAT_10255f0c != 0)
    DAT_10255f0c->Release();
  DAT_10255f08 = 0;
  DAT_10255f0c = 0;
  DAT_104c01a0 = 0;
  DAT_104c018c = 0;
  DAT_102024ac = -1;
  DAT_10836ec8 = IdentityMatrix();
  DAT_10daaa80.Reset();
  DAT_104c0350 = 0;
  DAT_104c037c = 0;
  DAT_104c0380 = 0;
  DAT_104bfd84 = 0;
  DAT_104bfd10 = 0;
  DAT_104c016c = 0;
  DAT_1049eae4 = 1;
  DAT_104687b8.FreeAll();
  DAT_1065cb28.M_1013edb0();
  memset(DAT_10c62250, 0, sizeof(DAT_10c62250));
  memset(DAT_10c62650, 0, sizeof(DAT_10c62650));
  if (DAT_104c0180 == 0)
  {
    DAT_104bf8c0 = 0;
    DAT_104bf8bc = 0;
  }
  DAT_104c0194 = 0;
  DAT_104b7200.M_100082e0();
  DAT_10daaae8[0] = DAT_10db4408;
  FUN_100154c0(DAT_10daaae8[0].helmet);
  DAT_10f654b4 = 0;
  DAT_10f654b8 = 0;
  DAT_104c0128 = 0;
  FUN_1015fd30();
  memset(DAT_10c62edc, 0, sizeof(DAT_10c62edc));
  DAT_104bfd08 = 0;
  DAT_104bf8d4 = 0;
  DAT_104bf8c8 = 0;
  DAT_104bf8f0 = 0.0f;
  DAT_10f08ab0.Close();
  DAT_104c009a = 0;
  DAT_1020235c = 1;
  FUN_1016b010(0);
  FUN_1013fb10(0,0,0);
  DAT_104bfe6c = 0;
  DAT_104c0088 = 0;
  DAT_104bfda8 = 0;
  DAT_104bfd28 = 0;
  DAT_104bfd78 = 0;
  DAT_104bfe1c = 0;
  DAT_104bfe78 = 0;
  DAT_1101ddf4 = 0;
  for (int i = 0; i < 0x100; i++)
  {
    DAT_1030bdf8[i] = 0;
    DAT_10c66ef0[i].Fill(0.0f);
    DAT_102195f0[i] = 0;
    DAT_104469f8[i] = 0;
    DAT_1021b628[i] = 0;
    DAT_10866df8[i].Fill(0.0f);
    DAT_10446eb0[i] = 0;
  }
  DAT_10836c70.M_1013ef00();
  FUN_101997f0();
  FUN_1016a980();
  FUN_101a5860();
  FUN_10161980();
  DAT_104b7200.M_10007f20();
  memset(DAT_1021b548, 0, sizeof(DAT_1021b548));
  memset(DAT_10467e20, 0, sizeof(DAT_10467e20));
  DAT_1020aee8 = 0;
  DAT_1020d648 = 0;
  DAT_1030b664 = 0;
  DAT_104687b0 = 0;
  DAT_1020d430 = 0;
  DAT_10468128 = 0;
  DAT_1020d4b8 = 0;
  DAT_1021f338 = 0;
  DAT_10802ac8.Fill(0.0f);
  DAT_10836c84.Fill(0.0f);
  DAT_104d29e8.Fill(0.0f);
  DAT_104d29f8.Fill(0.0f);
  DAT_10c67f50.Fill(0.0f);
  DAT_10db9368 = 0;
  DAT_10db6ee8[0].Unload();
  for (int i = 0; i < 0x80; i++)
  {
    DAT_10837fc8[i].Fill(0.0f);
    DAT_10836fc8[i].Fill(0.0f);
    DAT_104686b0[i] = 0;
    DAT_10210d30[i] = 0;
    DAT_10218fc0[i] = 0;
    DAT_10210b30[i] = 0;
    DAT_1021f9c0[i] = 0;
    DAT_10255a88[i] = 0;
    DAT_1021f340[i] = 0;
    DAT_10253e98[i] = 0;
    DAT_102550e0[i] = 0;
    DAT_10254090[i] = 0;
    DAT_1021f6c0[i] = 0;
    DAT_102239d0[i] = 0;
    DAT_10467288[i] = 0;
    DAT_1030c278[i] = 0;
    DAT_10447dd8[i] = 0;
    DAT_1020b128[i] = 0;
    DAT_10219ec0[i] = 0;
    DAT_1021b3c8[i] = 0;
    DAT_102191c0[i] = 0;
    DAT_1032d4b8[i] = 0;
  }
  DAT_104bf888 = 0;
  DAT_104bf88c = 0;
  DAT_10202348 = 1;
  DAT_104bf894 = 0;
  DAT_104bf898 = 0;
  DAT_104bf89c = 0;
  DAT_104bf8a0 = 0;
  DAT_104bf8a4 = 0;
  DAT_104bf8a8 = 0;
  DAT_104bf8ac = 0;
  DAT_104bf8b0 = 0;
  DAT_104bf8b4 = 0;
  DAT_104bf8b8 = 0;
  DAT_104bf8c4 = 0;
  DAT_104bf8c8 = 0;
  DAT_104bf8cc = 0;
  DAT_104bf8d0 = 0;
  DAT_104bf8d4 = 0;
  DAT_104bf8d8 = 0;
  DAT_104bf8dc = 0;
  DAT_104bf8e0 = 0;
  DAT_104bf8e4 = 0;
  DAT_104bf8e8 = 0.0f;
  DAT_104bf8ec = 0;
  DAT_104bf8f0 = 0.0f;
  DAT_1020234c = 0.5f;
  DAT_104bfaf8 = 0;
  DAT_104bfafc = 0;
  DAT_104bfb00 = 0;
  DAT_104bfd08 = 0;
  DAT_104bfd0c = 0;
  DAT_104bfd10 = 0;
  DAT_104bfd14 = 0;
  DAT_104bfd18 = 0;
  DAT_104bfd1c = 0;
  DAT_104bfd20 = 0;
  DAT_10202350 = -1;
  DAT_104bfd24 = 0;
  DAT_104bfd28 = 0;
  DAT_104bfd2c = 0;
  DAT_104bfd30 = 0;
  DAT_104bfd34 = 0;
  DAT_104bfd38 = 0;
  DAT_10202354 = -1;
  DAT_104bfd3c = 0;
  DAT_104bfd40 = 0;
  DAT_104bfd44 = 0;
  DAT_104bfd48 = 0;
  DAT_104bfd4c = 0;
  DAT_104bfd50 = 0;
  DAT_104bfd5c = 0;
  DAT_104bfd60 = 0;
  DAT_104bfd64 = 0;
  DAT_104bfd68 = 0;
  DAT_104bfd6c = 0;
  DAT_104bfd70 = 0;
  DAT_104bfd71 = 0;
  DAT_104bfd72 = 0;
  DAT_104bfd73 = 0;
  DAT_104bfd74 = 0;
  DAT_104bfd78 = 0;
  DAT_104bfd7c = 0;
  DAT_104bfd80 = 0;
  memset(DAT_104bfd88, 0, sizeof(DAT_104bfd88));
  DAT_104bfda8 = 0;
  DAT_104bfdac = 0;
  DAT_104bfdb0 = 0;
  DAT_104bfdb4 = 0;
  DAT_104bfdb8 = 0;
  DAT_104bfdbc = 0;
  DAT_104bfdc0 = 0;
  DAT_1020235c = 1;
  DAT_104bfdc4 = 0;
  DAT_104bfdc8 = 0;
  DAT_104bfdcc = 0;
  DAT_104bfdd0 = 0;
  DAT_104bfdd4 = 0;
  DAT_104bfdd8 = 0;
  DAT_104bfddc = 0;
  DAT_1020245c = -4;
  DAT_104bfde0 = 0;
  DAT_104bfde4 = 0;
  DAT_104bfde8 = 0;
  DAT_104bfdec = 0;
  DAT_104bfdf0 = 0;
  DAT_104bfdf4 = 0;
  DAT_104bfdf8 = 0;
  DAT_104bfdfc = 0;
  DAT_104bfe00 = 0;
  DAT_104bfe04 = 0;
  DAT_104bfe08 = 0;
  DAT_104bfe0c = 0;
  DAT_104bfe10 = 0;
  DAT_104bfe20 = 0;
  DAT_104bfe14 = 0;
  DAT_10202460 = 1.0f;
  DAT_104bfe18 = 0;
  DAT_104bfe1c = 0;
  memset(DAT_104485e8, 0, sizeof(DAT_104485e8));
  DAT_104bfe24 = 0.0f;
  DAT_104bfe28 = 0;
  DAT_104bfe2c = 0;
  DAT_104bfe30 = 0;
  DAT_104bfe34 = 0;
  DAT_104bfe38 = 0;
  DAT_104bfe3c = 0;
  DAT_104bfe40 = 0;
  DAT_104bfe44 = 0;
  DAT_104bfe48 = 0;
  DAT_104bfe4c = 0;
  DAT_104bfe50 = 0;
  DAT_104bfe54 = 0;
  DAT_104bfe58 = 0;
  DAT_104bfe5c = 0;
  DAT_104bfe60 = 0;
  DAT_104bfe64 = 0;
  DAT_104bfe68 = 0;
  DAT_104bfe6c = 0;
  DAT_104bfe70 = 0;
  DAT_104bfe74 = 0;
  DAT_104bfe78 = 0;
  DAT_104bfe7c = 0;
  DAT_1020247c[0] = -1;
  DAT_10202480 = -1;
  DAT_10202484 = -1;
  DAT_10202488 = -1;
  DAT_1020248c = -1;
  DAT_10202490 = -1;
  DAT_10202494 = -1;
  DAT_10202498 = -1;
  DAT_104bfe84 = 0;
  DAT_104bfe88 = 0;
  DAT_104bfe8c = 0;
  DAT_104bfe90 = 0;
  DAT_104bfe94 = 0;
  DAT_104bfe98 = 0;
  DAT_104bfe9c = 0;
  DAT_104bfea0 = 0;
  DAT_104bfea4 = 0;
  DAT_104bfea8 = 0;
  DAT_104bfea9 = 0;
  DAT_104bfeac = 0;
  DAT_104bfeb0 = 0;
  DAT_104bfeb4 = 0;
  DAT_104bfeb8 = 0;
  DAT_104bfeb9 = 0;
  DAT_104bfeba = 0;
  DAT_104bfec0 = 0;
  DAT_1020249c = -1;
  DAT_104bff00 = 0;
  DAT_104bff04 = 0;
  DAT_102024a0 = -1;
  DAT_104bff08 = 0;
  DAT_104bff0c = 0;
  DAT_104bff10 = 0;
  DAT_104bff14 = 0;
  DAT_104bff18 = 0;
  DAT_102024a4 = 1.0f;
  DAT_104c0020 = 0;
  DAT_104c0024 = 0;
  DAT_104c0028 = 0;
  DAT_104c002c = 0;
  DAT_104c0030 = 0;
  DAT_104c0034 = 0;
  DAT_102024a8 = 1;
  DAT_104c0038 = 0;
  DAT_104c003c = 0;
  DAT_104c0040 = 0;
  DAT_104c0044 = 0;
  DAT_104c0048 = 0;
  DAT_104c004c = 0;
  DAT_104c0050 = 0;
  DAT_104c0054 = 0;
  DAT_104c0058 = 0;
  DAT_104c005c = 0;
  DAT_104c0060 = 0;
  DAT_104c0064 = 0;
  DAT_104c0068 = 0;
  DAT_104c006c = 0;
  DAT_104c0070 = 0;
  DAT_104c0074 = 0;
  DAT_102024b0 = -1;
  DAT_104c0078 = 0;
  DAT_104c007c = 0;
  DAT_104c0088 = 0;
  DAT_102024b4 = 1;
  DAT_104c008c = 0;
  DAT_104c0091 = 0;
  DAT_104c0092 = 0;
  DAT_104c0093 = 0;
  DAT_104c0094 = 0;
  DAT_104c0095 = 0;
  DAT_104c0096 = 0;
  DAT_104c0097 = 0;
  DAT_104c0098 = 0;
  DAT_104c0099 = 0;
  DAT_104c009a = 0;
  DAT_104c009b = 0;
  DAT_104c009c = 0;
  DAT_104c009d = 0;
  DAT_104c009e = 0;
  DAT_104c00a0 = 0;
  DAT_104c00a4 = 0;
  DAT_104c00a8 = 0;
  DAT_102024b8 = 0xffff;
  DAT_102024bc = 0xffff;
  DAT_104c00ac = 0;
  DAT_104c00b0 = 0;
  DAT_104c00b2 = 0;
  DAT_104c00b3 = 0;
  DAT_104c00b4 = 0;
  DAT_104c00b5 = 0;
  DAT_104c00b6 = 0;
  DAT_104c00b7 = 0;
  DAT_104c00b8 = 0;
  DAT_104c00bc = 0;
  DAT_104c00bd = 0;
  DAT_104c00be = 0;
  DAT_104c00bf = 0;
  DAT_104c00c4 = 0;
  DAT_104c00c8 = 0;
  DAT_104c00cc = 0;
  DAT_104c00d0 = 0;
  DAT_104c00d4 = 0;
  DAT_104c00d5 = 0;
  DAT_104c00d8 = 0;
  DAT_104c0138 = 0;
  DAT_104c00dc = 0;
  DAT_104c00e0 = 0;
  DAT_104c00e4 = 0;
  DAT_104c00e8 = 0;
  DAT_104c00ec = 0;
  DAT_104c00f0 = 0;
  DAT_104c00f4 = 0;
  DAT_102024c0 = 1;
  DAT_102024c4 = 1;
  DAT_102024c8 = 1;
  DAT_104c00f8 = 0;
  DAT_104c0104 = 0;
  DAT_104c0100 = 0;
  DAT_104c00fc = 0;
  DAT_104c0108 = 0;
  DAT_104c010c = 0;
  DAT_104c0110 = 0;
  DAT_104c0114 = 0;
  DAT_104c0118 = 0;
  DAT_104c0119 = 0;
  DAT_104c011a = 0;
  DAT_104c011b = 0;
  DAT_104c011c = 0;
  DAT_104c0120 = 0;
  DAT_104c0121 = 0;
  DAT_104c0124 = 0;
  DAT_104c0128 = 0;
  DAT_104c012c = 0;
  DAT_104c0130 = 0;
  DAT_104c0131 = 0;
  DAT_104c0132 = 0;
  DAT_104c0133 = 0;
  DAT_104c0134 = 0;
  DAT_104c0135 = 0;
  DAT_104c0154 = 0;
  DAT_104c0155 = 0;
  DAT_104c0156 = 0;
  DAT_104c0158[0] = 0;
  DAT_104c0158[1] = 0;
  DAT_104c0158[2] = 0;
  DAT_104c0158[3] = 0;
  DAT_104c015c = 0;
  DAT_104c0160 = 0;
  DAT_104c0164 = 0;
  DAT_104c0168 = 0;
  DAT_104c016c = 0;
  DAT_104c0170 = 0;
  DAT_104c0171 = 0;
  DAT_104c0172 = 0;
  DAT_104c0173 = 0;
  DAT_1030b664 = 0;
  DAT_104687b0 = 0;
  DAT_1020d430 = 0;
  DAT_10468128 = 0;
  DAT_1020d4b8 = 0;
  DAT_1021f338 = 0;
  DAT_10802ac8.Fill(0.0f);
  DAT_10836c84.Fill(0.0f);
  DAT_104d29e8.Fill(0.0f);
  DAT_104d29f8.Fill(0.0f);
  DAT_10c67f50.Fill(0.0f);
  DAT_1049f0f0 = 0;
  DAT_108029b8.Fill(0.0f);
  memset(DAT_1020dcf8, 0, sizeof(DAT_1020dcf8));
  DAT_1020b22c = 0;
  DAT_10f08d1c.Fill(0.0f);
  DAT_1065cb08.Fill(0.0f);
  DAT_1021bbf0 = 0;
  DAT_1021bbf4 = 0;
  DAT_10223a50 = 0;
  DAT_1030b868 = 0;
  DAT_10edbde8.Fill(0.0f);
  DAT_10254088 = 0;
  DAT_10210b28 = 0;
  DAT_10daaad4.Fill(0.0f);
  memset(DAT_10315140, 0, sizeof(DAT_10315140));
  memset(DAT_10255a08, 0, sizeof(DAT_10255a08));
  memset(DAT_1042e2a8, 0, sizeof(DAT_1042e2a8));
  memset(DAT_10450808, 0, sizeof(DAT_10450808));
  memset(DAT_103154b0, 0, sizeof(DAT_103154b0));
  memset(DAT_1049f200, 0, sizeof(DAT_1049f200));
  memset(DAT_1042e3f0, 0, sizeof(DAT_1042e3f0));
  DAT_102559f0 = 0;
  DAT_1042d9e4 = 0;
  DAT_10467f20 = 0;
  DAT_10254590 = 0;
  DAT_10edbd38.Fill(0.0f);
  memset(DAT_1032d6a8, 0, sizeof(DAT_1032d6a8));
  memset(DAT_10467488, 0, sizeof(DAT_10467488));
  DAT_1032d69c = 1;
  for (int i = 0; i < 0x80; i++)
  {
    DAT_10837fc8[i].Fill(0.0f);
    DAT_1021f9c0[i] = 0;
    DAT_10255a88[i] = 0;
    DAT_1021f340[i] = 0;
    DAT_10253e98[i] = 0;
    DAT_102550e0[i] = 0;
    DAT_10254090[i] = 0;
    DAT_104bf8f8[i] = 0;
    DAT_1021f740[i] = 0;
    DAT_1021d138[i] = 0;
    DAT_104bfb08[i] = 0;
    DAT_104481d8[i] = 0;
    DAT_10467c20[i] = 0;
    DAT_10802bf8[i].position.Fill(0.0f);
    DAT_10802bf8[i].step.Fill(0.0f);
  }
  memset(DAT_1020b430, 0, sizeof(DAT_1020b430));
  memset(DAT_10453208, 0, sizeof(DAT_10453208));
  memset(DAT_1021bbc0, 0, sizeof(DAT_1021bbc0));
  memset(DAT_1032d5b8, 0, sizeof(DAT_1032d5b8));
  memset(DAT_103150c0, 0, sizeof(DAT_103150c0));
  memset(DAT_1042de28, 0, sizeof(DAT_1042de28));
  memset(DAT_10219fc0, 0, sizeof(DAT_10219fc0));
  memset(DAT_10219c40, 0, sizeof(DAT_10219c40));
  memset(DAT_1042e0a8, 0, sizeof(DAT_1042e0a8));
  memset(DAT_10255b08, 0, sizeof(DAT_10255b08));
  memset(DAT_1030bbf8, 0, sizeof(DAT_1030bbf8));
  DAT_1032d6a0 = 0;
  DAT_1030c478 = 0;
  DAT_1030c479 = 0;
  DAT_1030c47a = 0;
  DAT_1030c47b = 0;
  DAT_1049f1f8 = 0;
  DAT_1049f1f9 = 0;
  DAT_1049f1fa = 0;
  DAT_1049f1fb = 0;
  for (int i = 0; i < 4; i++)
  {
    DAT_1065ca08[i].SetIdentity();
    DAT_10c67ef0[i].Fill(0.0f);
  }
  memset(DAT_1042d918, 0, sizeof(DAT_1042d918));
  DAT_10da9150.Fill(0.0f);
  DAT_10868818.Fill(0.0f);
  memset(DAT_1020aefc, 0, sizeof(DAT_1020aefc));
  memset(DAT_102550c0, 0, sizeof(DAT_102550c0));
  memset(DAT_1049ef90, 0, sizeof(DAT_1049ef90));
  memset(DAT_10468430, 0, sizeof(DAT_10468430));
  memset(DAT_1032d7a8, 0, sizeof(DAT_1032d7a8));
  memset(DAT_10253f18, 0, sizeof(DAT_10253f18));
  memset(DAT_1021b5c8, 0, sizeof(DAT_1021b5c8));
  memset(DAT_104472b0, 0, sizeof(DAT_104472b0));
  memset(DAT_1020d860, 0, sizeof(DAT_1020d860));
  memset(DAT_10315350, 0, 0x60);   // the original clears only the first 0x60 bytes
  DAT_1032d868 = 0;
  DAT_102548b8 = 0;
  DAT_1021a360 = 0;
  DAT_10edbdd8.Fill(0.0f);
  DAT_1046812c = 0;
  DAT_102199b0 = 0;
  DAT_10210b2c = 0;
  DAT_1030c990 = 0;
  DAT_104463f0 = 0;
  DAT_108029a8.Fill(0.0f);
  memset(DAT_10255040, 0, sizeof(DAT_10255040));
  memset(DAT_102196f0, 0, sizeof(DAT_102196f0));
  memset(DAT_1020d5c8, 0, sizeof(DAT_1020d5c8));
  DAT_1049ef8c = 0;
  DAT_10447dd0 = 0;
  DAT_10210fb8 = 0;
  DAT_10446ea8 = 0;
  DAT_10467710 = 0;
  DAT_103088b4 = 0;
  DAT_1030b660 = 0;
  memset(DAT_10447338, 0, sizeof(DAT_10447338));
  memset(DAT_10836fc8, 0, sizeof(DAT_10836fc8));
  memset(DAT_10467288, 0, sizeof(DAT_10467288));
  DAT_10c67f30.Fill(0.0f);
  memset(DAT_10254110, 0, sizeof(DAT_10254110));
  DAT_102024b0 = -1;
  DAT_104c0078 = 0;
  DAT_104c007c = 0;
  DAT_104c0088 = 0;
  DAT_102024b4 = 1;
  DAT_104c008c = 0;
  DAT_104c0091 = 0;
  DAT_104c0092 = 0;
  DAT_104c0093 = 0;
  DAT_104c0094 = 0;
  DAT_104c0095 = 0;
  DAT_104c0096 = 0;
  DAT_104c0097 = 0;
  DAT_104c0098 = 0;
  DAT_104c0099 = 0;
  DAT_104c009a = 0;
  DAT_104c009b = 0;
  DAT_104c009c = 0;
  DAT_104c009d = 0;
  DAT_104c009e = 0;
  DAT_104c00a0 = 0;
  DAT_104c00a4 = 0;
  DAT_104c00a8 = 0;
  DAT_102024b8 = 0xffff;
  DAT_102024bc = 0xffff;
  DAT_104c00ac = 0;
  DAT_104c00b0 = 0;
  DAT_104c00b2 = 0;
  DAT_104c00b3 = 0;
  DAT_104c00b4 = 0;
  DAT_104c00b5 = 0;
  DAT_104c00b6 = 0;
  DAT_104c00b7 = 0;
  DAT_104c00b8 = 0;
  DAT_104c00bc = 0;
  DAT_104c00bd = 0;
  DAT_104c00be = 0;
  DAT_104c00bf = 0;
  DAT_104c00c0 = 0;
  DAT_104c00c4 = 0;
  DAT_104c00c8 = 0;
  DAT_104c00cc = 0;
  DAT_104c00d0 = 0;
  DAT_104c00d4 = 0;
  DAT_104c00d5 = 0;
  DAT_104c00d8 = 0;
  DAT_104c00dc = 0;
  DAT_104c00e0 = 0;
  DAT_104c00e4 = 0;
  DAT_104c00e8 = 0;
  DAT_104c00ec = 0;
  DAT_104c00f0 = 0;
  DAT_104c00f4 = 0;
  DAT_102024c0 = 0;
  DAT_102024c4 = 0;
  DAT_102024c8 = 1;
  DAT_104c0108 = 0;
  DAT_104c010c = 0;
  DAT_104c0110 = 0;
  DAT_104c0114 = 0;
  DAT_104c0118 = 0;
  DAT_104c0119 = 0;
  DAT_104c011a = 0;
  DAT_104c011b = 0;
  DAT_104c011c = 0;
  DAT_104c0120 = 0;
  DAT_104c0124 = 0;
  DAT_104c0128 = 0;
  DAT_104c012c = 0;
  DAT_104c0132 = 0;
  DAT_104c0133 = 0;
  DAT_104c0134 = 0;
  FUN_1000d9d0();
  for (int i = 0; i < 0x40; i++)
  {
    DAT_10860de8[i].m_00f4 = 0;
    DAT_10860de8[i].m_00f8 = 0;
    DAT_10860de8[i].m_0000 = 0;
    DAT_10860de8[i].m_0004 = 0;
  }
  memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
  memset(DAT_10447fd8, 0, sizeof(DAT_10447fd8));
  memset(DAT_1021f640, 0, sizeof(DAT_1021f640));
  memset(DAT_10256218, 0, sizeof(DAT_10256218));
  memset(DAT_10253c58, 0, sizeof(DAT_10253c58));
  memset(DAT_104463f8, 0, sizeof(DAT_104463f8));
  memset(DAT_10468230, 0, sizeof(DAT_10468230));
  memset(DAT_1032d67c, 0, sizeof(DAT_1032d67c));
  DAT_1032d4b0 = 0.0f;
}

