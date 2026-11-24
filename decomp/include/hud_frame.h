// The frame drawing (Hud_DrawFrame, hud_frame.cpp) and the globals only it reads: the scene passes, the HUD and the
// overlays drawn over them.
#ifndef GURUMIN_HUD_FRAME_H
#define GURUMIN_HUD_FRAME_H

struct FontRenderer;   // font_renderer.h
struct Matrix4;        // matrix4.h

// One entry of the marker table DAT_108d9758 (0x38 bytes, 32 entries; the list ends at the first entry after the
// first whose `phase` is 0): the strip of tick marks highlights the entry whose phase equals the strip's position.
struct StripMarker
{
    int phase;               // 0x00: position on the 480 step strip, 0 = end of the list
    int m_0004[13];
};

extern "C" StripMarker DAT_108d9758[32];                  // the strip markers
extern "C" float DAT_104bff24[25];                        // left half of the bar levels (key-held gauges), 0..320
extern "C" float DAT_104bffa4[25];                        // right half of the bar levels
extern "C" int DAT_102028d8;                              // texture slot of ico01.bmp (-1 = not loaded yet)
extern "C" int DAT_10836eb8;                              // width of the movie shown in the movie mode 3
extern "C" int DAT_10836ebc;                              // its height
extern "C" char DAT_10f08afb;                             // 1 while the screen copy texture holds a picture

extern "C" int FUN_1015d510(void);                        // Ime_IsOpen
extern "C" int FUN_1015fbf0(FontRenderer *renderer, int x, int y, int a, int b, int c, int d, int colour,
                            int selectedColour, int composingColour);   // Ime_DrawComposition
extern "C" int FUN_1015fa90(void);                        // number of visible IME candidates
extern "C" int FUN_1015fac0(FontRenderer *renderer);      // width of the widest visible candidate
extern "C" int FUN_1015fb20(FontRenderer *renderer, int x, int y, int colour, int selColour);   // draws the candidates
extern "C" void FUN_1000f4d0(void);                       // hud_fade.cpp: the dark panel over the HUD
extern "C" void FUN_1000cc30(void);                       // Hud_DrawNameBanners (name_banners.cpp)
extern "C" int FUN_1000eed0(void);                        // Message_DrawPanel (message_panel.cpp)
extern "C" int FUN_100236c0(void);                        // Overlay_DrawModel (overlay_draw.cpp)
extern "C" int FUN_1007d0a0(void);                        // TextWindows_Draw (text_windows_draw.cpp)

// 0x1008c680 (Hud_DrawFrame, hud_frame.cpp): draws one game frame: the scene passes, the HUD and the overlays.
// Returns 1.
int Hud_DrawFrame(void);

#endif
