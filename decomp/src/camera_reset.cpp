// Resetting the camera state when a map starts: the camera vectors are zeroed and the camera flags and blend
// counters cleared (most of these globals are only named where they are used).

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Fill(const float &value);               // 0x101acc40: sets all four components
};

extern "C" int DAT_10203440;
extern "C" int DAT_10203444;
extern "C" float DAT_10203448;
extern "C" float DAT_1020344c;
extern "C" float DAT_10203450;
extern "C" float DAT_10203454;
extern "C" float DAT_10203458;
extern "C" float DAT_1020345c;
extern "C" float DAT_10203460;
extern "C" char DAT_10faa678;
extern "C" char DAT_10faaa80;
extern "C" int DAT_10fabe88;
extern "C" char DAT_10fabe8c;
extern "C" int DAT_10fe01d0;
extern "C" int DAT_10feb3d8;
extern "C" int DAT_10ff19e0;
extern "C" int DAT_10ff1df0;
extern "C" int DAT_10ff2ff0;
extern "C" int DAT_10ff2ff4;
extern "C" int DAT_11018624;
extern "C" int DAT_11018728;
extern "C" int DAT_1101872c;
extern "C" int DAT_11018730;
extern "C" int DAT_11018734;
extern "C" int DAT_11018738;
extern "C" int DAT_1101c800;  // camera heading target
extern "C" int DAT_1101d888;
extern "C" int DAT_1101d88c;  // frames left of a camera cut blend
extern "C" int DAT_1101d890;
extern "C" int DAT_1101d894;
extern "C" int DAT_1101d898;
extern "C" int DAT_1101d89c;
extern "C" int DAT_1101d8a0;
extern "C" int DAT_1101d8a4;
extern "C" int DAT_1101d8a8;
extern "C" int DAT_1101d8ac;
extern "C" int DAT_1101d8b0;
extern "C" int DAT_1101d8b4;
extern "C" char DAT_1101d8b8;
extern "C" int DAT_1101d9b8;
extern "C" int DAT_1101ddd0;
extern "C" int DAT_1101ddd4;
extern "C" int DAT_1101ddd8;
extern "C" int DAT_1101dddc;
extern "C" int DAT_1101dde0;
extern "C" int DAT_1101dde4;
extern "C" int DAT_1101dde8;
extern "C" int DAT_1101ddec;
extern "C" int DAT_1101ddf0;
extern "C" int DAT_1101de08;
extern "C" int DAT_1101de0c;
extern "C" int DAT_1101de10;
extern "C" int DAT_1101de14;
extern "C" int DAT_1101de18;
extern "C" int DAT_1101de20;
extern "C" int DAT_1101de28;
extern "C" int DAT_1101de2c;
extern "C" char DAT_1101de38;
extern "C" Vec4 DAT_118e69e8;  // camera position
extern "C" Vec4 DAT_118e6c78;  // goal of the camera target
extern "C" Vec4 DAT_118e6c88;
extern "C" Vec4 DAT_118e6c98;
extern "C" Vec4 DAT_118e6ca8;
extern "C" Vec4 DAT_118e6cb8;  // camera target
extern "C" Vec4 DAT_118e6cd8;
extern "C" Vec4 DAT_11af3e28;  // goal of the camera position
extern "C" Vec4 DAT_11af3e38;
extern "C" Vec4 DAT_11af3e48;
extern "C" Vec4 DAT_11af3e58;
extern "C" Vec4 DAT_11af3f48;
extern "C" Vec4 DAT_11af3f58;  // the camera-space push of the player

// 0x1016a980 (Camera_Reset). Clears the camera globals: the camera position and target vectors and their goals,
// the blend and cut counters (DAT_10203444 = -100, DAT_10203440 = 1), and sets the scale factors to 1 and
// DAT_10203450 to 12. Returns 0.
extern "C" int FUN_1016a980(void)
{
    DAT_10fabe8c = 0;
    DAT_10faaa80 = 0;
    DAT_1101de20 = 0;
    DAT_1101dde4 = 0;
    DAT_10faa678 = 0;
    DAT_1101de38 = 0;
    DAT_10203450 = 12.0f;
    DAT_10203448 = 1.0f;
    DAT_10fe01d0 = 0;
    DAT_10203454 = 1.0f;
    DAT_10203458 = 1.0f;
    DAT_1020345c = 1.0f;
    DAT_10203460 = 1.0f;
    DAT_1101de28 = 0;
    DAT_118e69e8.Fill(0.0f);
    DAT_118e6cb8.Fill(0.0f);
    DAT_11af3e28.Fill(0.0f);
    DAT_1101de2c = 0;
    DAT_118e6c78.Fill(0.0f);
    DAT_1101d888 = 0;
    DAT_10203440 = 1;
    DAT_1101d88c = 0;
    DAT_1101d890 = 0;
    DAT_1101d894 = 0;
    DAT_1101d898 = 0;
    DAT_10203444 = -100;
    DAT_1101d89c = 0;
    DAT_1101d8a0 = 0;
    DAT_1101d8a4 = 0;
    DAT_1101d8a8 = 0;
    DAT_1101d8ac = 0;
    DAT_1101d8b0 = 0;
    DAT_1101d8b4 = 0;
    DAT_1101d8b8 = 0;
    DAT_1101d9b8 = 0;
    DAT_1101ddd0 = 0;
    DAT_1101ddd4 = 0;
    DAT_1101ddd8 = 0;
    DAT_1101dddc = 0;
    DAT_1101dde0 = 0;
    DAT_1101dde8 = 0;
    DAT_1101ddec = 0;
    DAT_1020344c = 1.0f;
    DAT_1101ddf0 = 0;
    DAT_1101de08 = 0;
    DAT_1101de0c = 0;
    DAT_1101de10 = 0;
    DAT_1101de14 = 0;
    DAT_1101de18 = 0;
    DAT_1101c800 = 0;
    DAT_10ff19e0 = 0;
    DAT_10ff1df0 = 0;
    DAT_11018624 = 0;
    DAT_11018730 = 0;
    DAT_1101872c = 0;
    DAT_11018734 = 0;
    DAT_11018728 = 0;
    DAT_10ff2ff4 = 0;
    DAT_11018738 = 0;
    DAT_10fabe88 = 0;
    DAT_10feb3d8 = 0;
    DAT_10ff2ff0 = 0;
    DAT_11af3f48.Fill(0.0f);
    DAT_11af3f58.Fill(0.0f);
    DAT_11af3f48.Fill(0.0f);
    DAT_11af3e28.Fill(0.0f);
    DAT_118e6c78.Fill(0.0f);
    DAT_11af3e38.Fill(0.0f);
    DAT_118e6c88.Fill(0.0f);
    DAT_11af3e48.Fill(0.0f);
    DAT_118e6c98.Fill(0.0f);
    DAT_11af3e58.Fill(0.0f);
    DAT_118e6ca8.Fill(0.0f);
    DAT_118e6cd8.Fill(0.0f);
    DAT_118e69e8.Fill(0.0f);
    DAT_118e6cb8.Fill(0.0f);
    return 0;
}
