// Globals with Windows types: the game window and module handles, tick counts and the COM/DirectX ids the game
// passes to CoCreateInstance and DirectInput. One declaration each; include this header instead of redeclaring them.
#ifndef GURUMIN_WIN_VARS_H
#define GURUMIN_WIN_VARS_H

#include <windows.h>

extern "C" const GUID DAT_101fcc0c;                       // GUID_SysKeyboard
extern "C" const GUID DAT_101fcc1c;                       // GUID_SysMouse
extern "C" const GUID DAT_101fccbc;                       // IID_IDirectInput8A
extern "C" const GUID DAT_101fcccc;                       // IID_IDirectDrawMediaStream
extern "C" const GUID DAT_101fccdc;                       // CLSID_AMMultiMediaStream
extern "C" const GUID DAT_101fccec;                       // IID_IAMMultiMediaStream
extern "C" const GUID DAT_101fccfc;                       // MSPID_PrimaryVideo
extern "C" DWORD DAT_104c00e4;                            // tick count when the BGM track was started
extern "C" DWORD DAT_104c032c;                            // tick count of the last loading-screen update
extern "C" HINSTANCE DAT_10f140c8;                        // the module instance
extern "C" int DAT_10f654d8;                              // next frame deadline (timeGetTime milliseconds)
extern "C" DWORD DAT_10f654e0;                            // time of the last frame-rate measurement
extern "C" HWND DAT_10f6569c;                             // the game window
extern "C" DWORD DAT_10f66bb8;                            // IME sentence mode saved by ime.cpp
extern "C" DWORD DAT_10f66dd8;                            // IME conversion mode saved by ime.cpp

#endif
