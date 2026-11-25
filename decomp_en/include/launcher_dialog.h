// EN-only launcher/config dialogs ("Gurumin Ver.1.4 (STEAM)"): the MFC dialog classes Falcom wrote on top of static MFC.
//
// This header is the ONLY place that pulls in the MFC headers (afxwin.h, afxcmn.h). It is included by the launcher
// source files (src/launcher_dialog.cpp and friends) and by nothing else, so no shared game header ever sees MFC.
//
// Build setup (see NOTES.md "Launcher (MFC)"): the game links MFC statically (/MT, no _AFXDLL), so the MFC headers
// are used exactly as the build flags deliver them. VS2013 Community Update 4 ships them in VC\atlmfc\include
// (vcvars32.bat puts that directory on INCLUDE). The exe has no _UNICODE and no _MBCS define: the launcher uses the
// ANSI entry points (SendMessageA, SetWindowTextA) and char strings.
#ifndef GURUMIN_EN_LAUNCHER_DIALOG_H
#define GURUMIN_EN_LAUNCHER_DIALOG_H

// The Update 4 headers declare one more CDialog member than the MFC the game was built with (the 12.0.21005 MBCS
// libraries): `BOOL m_bClosedByEndDialog;` sits between m_hWndTop and m_pOccDialogInfo, which makes sizeof(CDialog)
// 0x98 instead of the 0x94 that every Falcom dialog class in the exe is laid out on (first derived member at 0x94).
// Turn that one declaration into an unnamed zero-width bit-field so the layout matches the exe; the macro is removed
// again right after the includes. MFC code that uses the member is in the MFC library, not in these headers.
#define m_bClosedByEndDialog :0
#include <afxwin.h>
#include <afxcmn.h>
#undef m_bClosedByEndDialog

// ---- Dialog template ids (RT_DIALOG resources of the exe) ------------------------------------------------------
enum LauncherDialogId {
    IDD_VIDEO_DIALOG = 0x82,     // "Graphics" / "Audio" settings page (CVideoDlg)
    IDD_PAD_DIALOG = 0x81,       // gamepad button assignment page (CPadDlg)
    IDD_NET_DIALOG = 0x83,       // network / lobby server page (CNetDlg)
    IDD_OSE_DIALOG = 0x84,       // "Osero" (Othello) mini game page (COseDlg)
    IDD_TITLE_DIALOG = 0x8b,     // title / progress page (CTitDlg)
    IDD_INSTALL_DIALOG = 0x8d,   // "installing, please wait" notice (CInstDlg)
    IDD_ADVANCED_DIALOG = 0x8e,  // "Advanced Option" dialog (CAdvancedOptionDlg)
    IDD_KEYBOARD_DIALOG = 0x8f   // keyboard key assignment page (CKbDlg)
};

// ---- Control ids of the Graphics / Audio page (names from the dialog template, RT_DIALOG 130) ---------------------
// The Advanced Option dialog (RT_DIALOG 142) reuses the same ids for its own copies of the graphics switches.
enum LauncherControlId {
    IDC_PLAY_VOICES = 0x3eb,            // "Play Voices" check box
    IDC_HIGH_QUALITY_TEXTURE = 0x3ec,   // "High Quality Texture" check box
    IDC_WINDOWED = 0x3ed,               // "Windowed" check box
    IDC_RESOLUTION = 0x3ee,             // "Resolution" combo box
    IDC_USE_BACKBUFFER = 0x3ef,         // "Use BackBuffer" check box
    IDC_DIRECTX81 = 0x3f0,              // "DirectX 8.1" radio button (first of the renderer group)
    IDC_DELAY_MS = 0x3f2,               // edit box: delay in milliseconds
    IDC_AUDIO_ENABLE = 0x3f3,           // "Enable" radio button (first of the audio on/off group)
    IDC_UNCOMPRESSED_TEXTURES = 0x3f7,  // "Uncompressed Textures" check box
    IDC_REFLECTIONS = 0x3f8,            // "Reflections" check box
    IDC_DIRECT_SOUND_3D = 0x3fb,        // "DirectSound3D" check box
    IDC_DEPTH_OF_FIELD = 0x3fc,         // "Depth Of Field" check box
    IDC_CROSS_FADE = 0x3fe,             // "Closs Fade" (sic) check box
    IDC_BLUR_EFFECT = 0x3ff,            // "Blur Effect" check box
    IDC_CACHE_AT_STARTUP = 0x400,       // generate the cache at start-up check box
    IDC_FOG_EFFECT = 0x401,             // "Fog Effect" check box
    IDC_HW_VERTEX_SHADER = 0x402,       // hardware vertex shader priority check box
    IDC_COMBO_403 = 0x403,              // combo box (memory/quality list, purpose not yet identified)
    IDC_PRESET = 0x404,                 // "Preset" combo box
    IDC_SHADOW_QUALITY = 0x405,         // "Shadow Quality" combo box (Low, Medium, High, Ultra)
    IDC_SHADOW_COMPLEXITY = 0x406,      // "Shadow Complexity" combo box (Low, Medium, High, Ultra)
    IDC_COMBO_407 = 0x407,              // combo box (memory/quality list, purpose not yet identified)
    IDC_SYNC = 0x408,                   // synchronise check box
    IDC_GAMMA_SETTINGS = 0x409,         // "Gamma Settings" check box
    IDC_COMPLEXITY_SHADER = 0x40a,      // "Complexity Shader" check box
    IDC_COLOR_32BIT = 0x40b,            // "32bitColor" check box
    IDC_DIRECTX9 = 0x40c,               // "DirectX 9" radio button (second of the renderer group)
    IDC_REFLECTION_EFFECT = 0x40d,      // "Reflaction Effect" (sic) check box
    IDC_COMBO_40E = 0x40e,              // combo box (purpose not yet identified)
    IDC_COMBO_40F = 0x40f,              // combo box (purpose not yet identified)
    IDC_CONTRAST = 0x410,               // "Contrast" slider
    IDC_BRIGHTNESS = 0x411,             // "Brightness" slider
    IDC_BGM_VOLUME = 0x412,             // "BGM Volume" slider
    IDC_SE_VOLUME = 0x413,              // sound-effect volume slider
    IDC_COMBO_414 = 0x414,              // combo box (purpose not yet identified)
    IDC_COMBO_415 = 0x415,              // combo box (purpose not yet identified)
    IDC_SKIP_VIDEO = 0x416,             // "Skip Video" check box
    IDC_VSYNC_OFF = 0x417,              // "Vsync Off" check box
    IDC_ADVANCED = 0x41f,               // "Advanced..." button
    IDC_TEXTURE_FILTERING = 0x425,      // "Texture Filtering" combo box
    IDC_MULTI_SAMPLING = 0x426          // "Multi Sampling" combo box
};

// ---- Control ids of the keyboard page (RT_DIALOG 143): one combo box per action, plus three mouse options --------
enum LauncherKeyboardControlId {
    IDC_KB_MOVE_LEFT = 0x3ff,          // "Move Left" key list
    IDC_KB_DASH = 0x3fe,               // "Dash" key list
    IDC_KB_MOVE_FORWARD = 0x400,       // "Move Forward" key list
    IDC_KB_ATTACK = 0x401,             // "Attack / OK / Action" key list
    IDC_KB_CAMERA_FORWARD = 0x402,     // "Camera: Show Forward" key list
    IDC_KB_MOVE_RIGHT = 0x404,         // "Move Right" key list
    IDC_KB_MOVE_BACKWARD = 0x405,      // "Move Backward" key list
    IDC_KB_JUMP = 0x406,               // "Jump / Cancel" key list
    IDC_KB_CAMERA_ENEMY = 0x407,       // "Camera: Show Enemy" key list
    IDC_KB_CAMERA_LEFT = 0x40f,        // "Camera: Show Left" key list
    IDC_KB_MENU = 0x415,               // "Menu On / Off" key list
    IDC_KB_CAMERA_RIGHT = 0x418,       // "Camera: Show Right" key list
    IDC_KB_SWAP_ATTACK_MOVE = 0x41b,   // "Swap Attack / Move Button" check box
    IDC_KB_WHEEL_CAMERA = 0x41c,       // "Rotate Camera With Wheel" check box
    IDC_KB_DASH_DOUBLE_CLICK = 0x41e   // "Dash: Double Right Click" check box
};

// Graphics / Audio settings page of the launcher (dialog template IDD_VIDEO_DIALOG). It is a plain wizard-style
// dialog: every setting is an int field exchanged with its control by DoDataExchange, and a second set of CWnd
// objects subclasses the same controls. Created by the launcher's tab dialog; its "Advanced..." button opens the
// Advanced Option dialog (CAdvancedOptionDlg).
class CVideoDlg : public CDialog {
public:
    CVideoDlg(CWnd *pParent = NULL);   // EN 0x006f06f0

    enum { IDD = IDD_VIDEO_DIALOG };

    int m_windowed;                  // 0x094 IDC_WINDOWED check state
    int m_highQualityTexture;        // 0x098 IDC_HIGH_QUALITY_TEXTURE check state
    int m_hwVertexShader;            // 0x09c IDC_HW_VERTEX_SHADER check state
    int m_resolution;                // 0x0a0 IDC_RESOLUTION selected row (-1 none)
    int m_uncompressedTextures;      // 0x0a4 IDC_UNCOMPRESSED_TEXTURES check state
    int m_audioEnable;               // 0x0a8 IDC_AUDIO_ENABLE radio group index (-1 none)
    int m_00ac;                      // 0x0ac not touched by the constructor or DoDataExchange
    int m_textureFiltering;          // 0x0b0 IDC_TEXTURE_FILTERING selected row
    int m_multiSampling;             // 0x0b4 IDC_MULTI_SAMPLING selected row
    int m_depthOfField;              // 0x0b8 IDC_DEPTH_OF_FIELD check state
    int m_reflections;               // 0x0bc IDC_REFLECTIONS check state
    int m_crossFade;                 // 0x0c0 IDC_CROSS_FADE check state
    int m_shadowQuality;             // 0x0c4 IDC_SHADOW_QUALITY selected row
    int m_shadowComplexity;          // 0x0c8 IDC_SHADOW_COMPLEXITY selected row
    int m_combo403;                  // 0x0cc IDC_COMBO_403 selected row
    int m_preset;                    // 0x0d0 IDC_PRESET selected row
    int m_blurEffect;                // 0x0d4 IDC_BLUR_EFFECT check state
    int m_cacheAtStartup;            // 0x0d8 IDC_CACHE_AT_STARTUP check state
    int m_combo407;                  // 0x0dc IDC_COMBO_407 selected row
    CButton m_cacheAtStartupCtrl;    // 0x0e0
    CComboBox m_combo403Ctrl;        // 0x154
    CComboBox m_combo407Ctrl;        // 0x1c8
    int m_fogEffect;                 // 0x23c IDC_FOG_EFFECT check state
    int m_delayMs;                   // 0x240 IDC_DELAY_MS value
    int m_contrast;                  // 0x244 IDC_CONTRAST slider position
    int m_brightness;                // 0x248 IDC_BRIGHTNESS slider position
    int m_gammaSettings;             // 0x24c IDC_GAMMA_SETTINGS check state
    int m_sync;                      // 0x250 IDC_SYNC check state
    int m_complexityShader;          // 0x254 IDC_COMPLEXITY_SHADER check state
    int m_useBackBuffer;             // 0x258 IDC_USE_BACKBUFFER check state
    int m_color32bit;                // 0x25c IDC_COLOR_32BIT check state
    int m_directX81;                 // 0x260 IDC_DIRECTX81 radio group index
    int m_directX9;                  // 0x264 IDC_DIRECTX9 radio group index
    int m_bgmVolume;                 // 0x268 IDC_BGM_VOLUME slider position
    int m_seVolume;                  // 0x26c IDC_SE_VOLUME slider position
    int m_directSound3D;             // 0x270 IDC_DIRECT_SOUND_3D check state
    int m_combo40e;                  // 0x274 IDC_COMBO_40E selected row
    int m_combo40f;                  // 0x278 IDC_COMBO_40F selected row
    int m_combo414;                  // 0x27c IDC_COMBO_414 selected row
    int m_reflectionEffect;          // 0x280 IDC_REFLECTION_EFFECT check state
    CSliderCtrl m_bgmVolumeCtrl;     // 0x284
    CButton m_directSound3DCtrl;     // 0x2f8
    CComboBox m_combo414Ctrl;        // 0x36c
    CButton m_syncCtrl;              // 0x3e0
    CEdit m_delayMsCtrl;             // 0x454
    CComboBox m_combo40fCtrl;        // 0x4c8
    CComboBox m_combo40eCtrl;        // 0x53c
    int m_05b0;                      // 0x5b0 zeroed by the constructor, otherwise unused
    CButton m_audioEnableCtrl;       // 0x5b4
    CButton m_gammaSettingsCtrl;     // 0x628
    CSliderCtrl m_contrastCtrl;      // 0x69c
    CSliderCtrl m_brightnessCtrl;    // 0x710
    CButton m_0784;                  // 0x784 constructed but never bound by DoDataExchange
    CButton m_hwVertexShaderCtrl;    // 0x7f8
    CButton m_color32bitCtrl;        // 0x86c
    CButton m_directX81Ctrl;         // 0x8e0
    int m_0954;                      // 0x954 zeroed by the constructor, otherwise unused
    CComboBox m_shadowQualityCtrl;   // 0x958
    CComboBox m_shadowComplexityCtrl;   // 0x9cc
    CButton m_highQualityTextureCtrl;   // 0xa40
    CButton m_depthOfFieldCtrl;         // 0xab4
    CButton m_uncompressedTexturesCtrl; // 0xb28
    CButton m_reflectionsCtrl;          // 0xb9c
    CButton m_crossFadeCtrl;            // 0xc10
    CButton m_blurEffectCtrl;           // 0xc84
    CButton m_fogEffectCtrl;            // 0xcf8
    CButton m_complexityShaderCtrl;     // 0xd6c
    CButton m_useBackBufferCtrl;        // 0xde0
    CButton m_reflectionEffectCtrl;     // 0xe54
    CComboBox m_presetCtrl;             // 0xec8
    CButton m_windowedCtrl;             // 0xf3c
    CComboBox m_resolutionCtrl;         // 0xfb0
    int m_playVoices;                   // 0x1024 IDC_PLAY_VOICES check state
    CButton m_playVoicesCtrl;           // 0x1028
    CComboBox m_combo415Ctrl;           // 0x109c
    int m_combo415;                     // 0x1110 IDC_COMBO_415 selected row
    CButton m_skipVideoCtrl;            // 0x1114
    int m_skipVideo;                    // 0x1188 IDC_SKIP_VIDEO check state
    int m_vsyncOff;                     // 0x118c IDC_VSYNC_OFF check state
    CButton m_vsyncOffCtrl;             // 0x1190

protected:
    virtual void DoDataExchange(CDataExchange *pDX);   // EN 0x006f0cf0
    afx_msg void OnAdvancedOption();                     // EN 0x006f1550 (IDC_ADVANCED button)
    DECLARE_MESSAGE_MAP()
};

// Advanced Option dialog opened by the "Advanced..." button of the Graphics page: eleven graphics switches that the
// page copies in on construction. Constructed by CVideoDlg::OnAdvancedOption (EN 0x006f1550).
class CAdvancedOptionDlg : public CDialog {
    DECLARE_DYNAMIC(CAdvancedOptionDlg)
public:
    // Copies the graphics switches out of the Graphics page. EN 0x006e5d60.
    CAdvancedOptionDlg(CVideoDlg *config, CWnd *pParent = NULL);
    virtual ~CAdvancedOptionDlg();   // EN 0x006e5ed0 (out of line; the scalar deleting form is 0x006e5f70)

    // Template id used by the wizard-style DDX; EN 0x006e5d60 passes IDD_ADVANCED_DIALOG to CDialog.
    enum { IDD = IDD_ADVANCED_DIALOG };

    int m_highQualityTexture;     // 0x094 IDC_HIGH_QUALITY_TEXTURE check state
    int m_uncompressedTextures;   // 0x098 IDC_UNCOMPRESSED_TEXTURES check state
    int m_crossFade;              // 0x09c IDC_CROSS_FADE check state
    int m_fogEffect;              // 0x0a0 IDC_FOG_EFFECT check state
    int m_useBackBuffer;          // 0x0a4 IDC_USE_BACKBUFFER check state
    int m_shadowQuality;          // 0x0a8 IDC_SHADOW_QUALITY selected row (0 Low .. 3 Ultra)
    int m_depthOfField;           // 0x0ac IDC_DEPTH_OF_FIELD check state
    int m_reflections;            // 0x0b0 IDC_REFLECTIONS check state
    int m_blurEffect;             // 0x0b4 IDC_BLUR_EFFECT check state
    int m_complexityShader;       // 0x0b8 IDC_COMPLEXITY_SHADER check state
    int m_shadowComplexity;       // 0x0bc IDC_SHADOW_COMPLEXITY selected row (0 Low .. 3 Ultra)
    CButton m_reflectionEffectCtrl;       // 0x0c0 IDC_REFLECTION_EFFECT control
    int m_reflectionEffect;               // 0x134 IDC_REFLECTION_EFFECT check state
    CVideoDlg *m_config;                  // 0x138 the Graphics page the values were copied from
    CComboBox m_shadowQualityCtrl;        // 0x13c IDC_SHADOW_QUALITY control
    CComboBox m_shadowComplexityCtrl;     // 0x1b0 IDC_SHADOW_COMPLEXITY control

protected:
    virtual void DoDataExchange(CDataExchange *pDX);   // EN 0x006e60b0
    virtual BOOL OnInitDialog();                        // EN 0x006e6340
    afx_msg void OnBnClickedOk();                       // EN 0x006e6240 (IDOK handler)
    DECLARE_MESSAGE_MAP()
};

// "Installing, please wait" notice shown while the launcher updates the game (dialog template IDD_INSTALL_DIALOG).
// A bare dialog: no members and nothing exchanged.
class CInstDlg : public CDialog {
    DECLARE_DYNAMIC(CInstDlg)
public:
    CInstDlg(CWnd *pParent = NULL);   // EN 0x006eb600
    virtual ~CInstDlg();              // folded into the scalar deleting destructor EN 0x006eb630

    enum { IDD = IDD_INSTALL_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);   // EN 0x006eb660 (empty)
    DECLARE_MESSAGE_MAP()
};

// Keyboard key assignment page (dialog template IDD_KEYBOARD_DIALOG): one drop-down list per action, each holding
// the same list of keys, and the page keeps the assignments unique (picking a key that another action already uses
// clears that other action back to row 0 "not assigned").
class CKbDlg : public CDialog {
    DECLARE_DYNAMIC(CKbDlg)
public:
    CKbDlg(CWnd *pParent = NULL);   // EN 0x006eb6a0
    virtual ~CKbDlg();              // folded into the scalar deleting destructor EN 0x006eb770

    enum { IDD = IDD_KEYBOARD_DIALOG };

    int m_dash;                    // 0x094 IDC_KB_DASH selected row
    int m_cameraEnemy;             // 0x098 IDC_KB_CAMERA_ENEMY selected row
    int m_cameraLeft;              // 0x09c IDC_KB_CAMERA_LEFT selected row
    int m_cameraRight;             // 0x0a0 IDC_KB_CAMERA_RIGHT selected row
    int m_cameraForward;           // 0x0a4 IDC_KB_CAMERA_FORWARD selected row
    int m_menu;                    // 0x0a8 IDC_KB_MENU selected row
    int m_jump;                    // 0x0ac IDC_KB_JUMP selected row
    int m_attack;                  // 0x0b0 IDC_KB_ATTACK selected row
    int m_moveForward;             // 0x0b4 IDC_KB_MOVE_FORWARD selected row
    int m_moveRight;               // 0x0b8 IDC_KB_MOVE_RIGHT selected row
    int m_moveLeft;                // 0x0bc IDC_KB_MOVE_LEFT selected row
    int m_moveBackward;            // 0x0c0 IDC_KB_MOVE_BACKWARD selected row
    int m_swapAttackMove;          // 0x0c4 IDC_KB_SWAP_ATTACK_MOVE check state
    int m_wheelCamera;             // 0x0c8 IDC_KB_WHEEL_CAMERA check state
    int m_dashDoubleClick;         // 0x0cc IDC_KB_DASH_DOUBLE_CLICK check state

    // Returns true when the player should not start: a key list is still on row 0 and the player refused the
    // default input setup offered by the warning box. EN 0x006eb860.
    bool IsStartRefused(CWnd *owner);

protected:
    virtual void DoDataExchange(CDataExchange *pDX);   // EN 0x006eb950
    virtual BOOL OnInitDialog();                        // EN 0x006ebe60
    // Called after the key list `sourceId` changed to row `row`: the list that already shows `row` is reset to row 0.
    // EN 0x006eb7a0.
    void ClearDuplicateKey(int row, int sourceId);
    afx_msg void OnSelchangeMoveForward();    // EN 0x006ebc80
    afx_msg void OnSelchangeMoveLeft();       // EN 0x006ebc30
    afx_msg void OnSelchangeMoveRight();      // EN 0x006ebd70
    afx_msg void OnSelchangeMoveBackward();   // EN 0x006ebdc0
    afx_msg void OnSelchangeMenu();           // EN 0x006ebb40
    afx_msg void OnSelchangeDash();           // EN 0x006ebbe0
    afx_msg void OnSelchangeJump();           // EN 0x006ebe10
    afx_msg void OnSelchangeAttack();         // EN 0x006ebcd0
    afx_msg void OnSelchangeCameraForward();  // EN 0x006ebd20
    afx_msg void OnSelchangeCameraLeft();     // EN 0x006ebaf0
    afx_msg void OnSelchangeCameraRight();    // EN 0x006ebb90
    afx_msg void OnSelchangeCameraEnemy();    // EN 0x006ebaa0
    DECLARE_MESSAGE_MAP()
};

// Loads the launcher settings from bin\gurumin.def into the global configuration. `section` 0 loads everything,
// 1 only the pad assignments, 2 only the keyboard assignments (EN 0x006e81d0, not yet reconstructed).
void Launcher_LoadConfig(int section);

#endif
