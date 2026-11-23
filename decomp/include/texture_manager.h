// TextureManager: the renderer object that owns the textures; it does not use `this` in most methods.
#ifndef GURUMIN_TEXTURE_MANAGER_H
#define GURUMIN_TEXTURE_MANAGER_H

struct TextureManager
{
    int M_10173480();
    int M_101734d0();
    int M_10173510();
    int OnLostDevice();   // 0x1014b2a0 (render_reset.cpp Render_OnLostDevice)
    int OnResetDevice();  // 0x1013fbb0 (Render_OnResetDevice)
    int Shutdown();       // 0x1015c730: unloads the textures and shuts the renderer down (render_shutdown.cpp)
    int PumpAndRecover(); // 0x1015c850: the message pump with device-lost recovery
    void M_10175e70();    // unloads all textures, then Shutdown (texture_mgr.cpp)
};

#endif
