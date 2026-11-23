// TextureManager: the renderer object that owns the textures (the one instance is DAT_10836c70); it does not use
// `this` in most methods.
#ifndef GURUMIN_TEXTURE_MANAGER_H
#define GURUMIN_TEXTURE_MANAGER_H

struct tagRECT;   // RECT, windows.h

struct TextureManager
{
    int Present(const tagRECT *source);   // 0x1013edc0 (Render_Present)
    int ResetFrameTimer();                // 0x1013eee0 (FrameTimer_ResetNext, engine_1013e.cpp)
    void M_1013ef00();                    // 0x1013ef00
    int M_10173480();
    int M_101734d0();
    int M_10173510();
    int CreateDevice();   // 0x10148b00 (Render_CreateDevice, src_od/render_device.cpp)
    int OnLostDevice();   // 0x1014b2a0 (render_reset.cpp Render_OnLostDevice)
    int OnResetDevice();  // 0x1013fbb0 (Render_OnResetDevice)
    int Shutdown();       // 0x1015c730: unloads the textures and shuts the renderer down (render_shutdown.cpp)
    int PumpAndRecover(); // 0x1015c850: the message pump with device-lost recovery
    int PumpMessages();   // 0x1013ec90 (engine_1013e.cpp): handles at most 30 pending messages
    int PaceFrame();      // 0x1014ade0 (frame_pacer.cpp): waits for the 60 Hz deadline
    int PresentFrame();   // 0x1015c960: ends the scene and flips the back buffer after the pre-present pass
    int PresentSimple();  // 0x1015c9b0: ends the scene and flips the back buffer
    void M_10175e70();    // unloads all textures, then Shutdown (texture_mgr.cpp)
};

extern "C" TextureManager DAT_10836c70;   // the renderer

#endif
