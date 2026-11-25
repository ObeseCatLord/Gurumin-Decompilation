// EN-only launcher: the Graphics / Audio settings page (CVideoDlg). Falcom MFC dialog code (ClassWizard style) from
// the Steam build; see NOTES.md "Launcher (MFC)".
#include "launcher_dialog.h"

// CVideoDlg


// Builds the Graphics / Audio page. The wizard-style constructor zeroes the exchanged values in the member initialiser
// list (in declaration order, between the control objects) and sets the first few in the body.
CVideoDlg::CVideoDlg(CWnd *pParent)
    : CDialog(IDD_VIDEO_DIALOG, pParent)
    , m_textureFiltering(0)
    , m_multiSampling(0)
    , m_depthOfField(0)
    , m_reflections(0)
    , m_crossFade(0)
    , m_shadowQuality(0)
    , m_shadowComplexity(0)
    , m_combo403(0)
    , m_preset(0)
    , m_blurEffect(0)
    , m_cacheAtStartup(0)
    , m_combo407(0)
    , m_fogEffect(0)
    , m_delayMs(0)
    , m_contrast(0)
    , m_brightness(0)
    , m_gammaSettings(0)
    , m_sync(0)
    , m_complexityShader(0)
    , m_useBackBuffer(0)
    , m_color32bit(0)
    , m_directX81(0)
    , m_directX9(0)
    , m_bgmVolume(0)
    , m_seVolume(0)
    , m_directSound3D(0)
    , m_combo40e(0)
    , m_combo40f(0)
    , m_combo414(0)
    , m_reflectionEffect(0)
    , m_05b0(0)
    , m_0954(0)
    , m_playVoices(0)
    , m_combo415(0)
    , m_skipVideo(0)
    , m_vsyncOff(0)
{
    m_windowed = 0;
    m_highQualityTexture = 0;
    m_hwVertexShader = 0;
    m_resolution = -1;
    m_uncompressedTextures = 0;
    m_audioEnable = -1;
    m_textureFiltering = 0;
    m_multiSampling = 0;
}

BEGIN_MESSAGE_MAP(CVideoDlg, CDialog)
    ON_BN_CLICKED(IDC_ADVANCED, &CVideoDlg::OnAdvancedOption)
END_MESSAGE_MAP()

// Exchanges every setting with its control, then subclasses the controls that the page drives directly.
void CVideoDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Check(pDX, IDC_WINDOWED, m_windowed);
    DDX_Check(pDX, IDC_HIGH_QUALITY_TEXTURE, m_highQualityTexture);
    DDX_Check(pDX, IDC_HW_VERTEX_SHADER, m_hwVertexShader);
    DDX_CBIndex(pDX, IDC_RESOLUTION, m_resolution);
    DDX_Check(pDX, IDC_UNCOMPRESSED_TEXTURES, m_uncompressedTextures);
    DDX_Radio(pDX, IDC_AUDIO_ENABLE, m_audioEnable);
    DDX_CBIndex(pDX, IDC_TEXTURE_FILTERING, m_textureFiltering);
    DDX_CBIndex(pDX, IDC_MULTI_SAMPLING, m_multiSampling);
    DDX_Check(pDX, IDC_DEPTH_OF_FIELD, m_depthOfField);
    DDX_Check(pDX, IDC_REFLECTIONS, m_reflections);
    DDX_Check(pDX, IDC_CROSS_FADE, m_crossFade);
    DDX_CBIndex(pDX, IDC_SHADOW_QUALITY, m_shadowQuality);
    DDX_CBIndex(pDX, IDC_SHADOW_COMPLEXITY, m_shadowComplexity);
    DDX_CBIndex(pDX, IDC_COMBO_403, m_combo403);
    DDX_CBIndex(pDX, IDC_PRESET, m_preset);
    DDX_Check(pDX, IDC_BLUR_EFFECT, m_blurEffect);
    DDX_Check(pDX, IDC_CACHE_AT_STARTUP, m_cacheAtStartup);
    DDX_CBIndex(pDX, IDC_COMBO_407, m_combo407);
    DDX_Control(pDX, IDC_CACHE_AT_STARTUP, m_cacheAtStartupCtrl);
    DDX_Control(pDX, IDC_COMBO_403, m_combo403Ctrl);
    DDX_Control(pDX, IDC_COMBO_407, m_combo407Ctrl);
    DDX_Check(pDX, IDC_FOG_EFFECT, m_fogEffect);
    DDX_Text(pDX, IDC_DELAY_MS, m_delayMs);
    DDX_Slider(pDX, IDC_CONTRAST, m_contrast);
    DDX_Slider(pDX, IDC_BRIGHTNESS, m_brightness);
    DDX_Check(pDX, IDC_GAMMA_SETTINGS, m_gammaSettings);
    DDX_Check(pDX, IDC_SYNC, m_sync);
    DDX_Check(pDX, IDC_COMPLEXITY_SHADER, m_complexityShader);
    DDX_Check(pDX, IDC_USE_BACKBUFFER, m_useBackBuffer);
    DDX_Check(pDX, IDC_COLOR_32BIT, m_color32bit);
    DDX_Radio(pDX, IDC_DIRECTX81, m_directX81);
    DDX_Radio(pDX, IDC_DIRECTX9, m_directX9);
    DDX_Slider(pDX, IDC_BGM_VOLUME, m_bgmVolume);
    DDX_Slider(pDX, IDC_SE_VOLUME, m_seVolume);
    DDX_Check(pDX, IDC_DIRECT_SOUND_3D, m_directSound3D);
    DDX_CBIndex(pDX, IDC_COMBO_40E, m_combo40e);
    DDX_CBIndex(pDX, IDC_COMBO_40F, m_combo40f);
    DDX_CBIndex(pDX, IDC_COMBO_414, m_combo414);
    DDX_Check(pDX, IDC_REFLECTION_EFFECT, m_reflectionEffect);
    DDX_Control(pDX, IDC_BGM_VOLUME, m_bgmVolumeCtrl);
    DDX_Control(pDX, IDC_DIRECT_SOUND_3D, m_directSound3DCtrl);
    DDX_Control(pDX, IDC_COMBO_414, m_combo414Ctrl);
    DDX_Control(pDX, IDC_SYNC, m_syncCtrl);
    DDX_Control(pDX, IDC_DELAY_MS, m_delayMsCtrl);
    DDX_Control(pDX, IDC_COMBO_40F, m_combo40fCtrl);
    DDX_Control(pDX, IDC_COMBO_40E, m_combo40eCtrl);
    DDX_Control(pDX, IDC_AUDIO_ENABLE, m_audioEnableCtrl);
    DDX_Control(pDX, IDC_GAMMA_SETTINGS, m_gammaSettingsCtrl);
    DDX_Control(pDX, IDC_CONTRAST, m_contrastCtrl);
    DDX_Control(pDX, IDC_BRIGHTNESS, m_brightnessCtrl);
    DDX_Control(pDX, IDC_HW_VERTEX_SHADER, m_hwVertexShaderCtrl);
    DDX_Control(pDX, IDC_COLOR_32BIT, m_color32bitCtrl);
    DDX_Control(pDX, IDC_DIRECTX81, m_directX81Ctrl);
    DDX_Control(pDX, IDC_SHADOW_QUALITY, m_shadowQualityCtrl);
    DDX_Control(pDX, IDC_SHADOW_COMPLEXITY, m_shadowComplexityCtrl);
    DDX_Control(pDX, IDC_HIGH_QUALITY_TEXTURE, m_highQualityTextureCtrl);
    DDX_Control(pDX, IDC_DEPTH_OF_FIELD, m_depthOfFieldCtrl);
    DDX_Control(pDX, IDC_UNCOMPRESSED_TEXTURES, m_uncompressedTexturesCtrl);
    DDX_Control(pDX, IDC_REFLECTIONS, m_reflectionsCtrl);
    DDX_Control(pDX, IDC_CROSS_FADE, m_crossFadeCtrl);
    DDX_Control(pDX, IDC_BLUR_EFFECT, m_blurEffectCtrl);
    DDX_Control(pDX, IDC_FOG_EFFECT, m_fogEffectCtrl);
    DDX_Control(pDX, IDC_COMPLEXITY_SHADER, m_complexityShaderCtrl);
    DDX_Control(pDX, IDC_USE_BACKBUFFER, m_useBackBufferCtrl);
    DDX_Control(pDX, IDC_REFLECTION_EFFECT, m_reflectionEffectCtrl);
    DDX_Control(pDX, IDC_PRESET, m_presetCtrl);
    DDX_Control(pDX, IDC_WINDOWED, m_windowedCtrl);
    DDX_Control(pDX, IDC_RESOLUTION, m_resolutionCtrl);
    DDX_Check(pDX, IDC_PLAY_VOICES, m_playVoices);
    DDX_Control(pDX, IDC_PLAY_VOICES, m_playVoicesCtrl);
    DDX_Control(pDX, IDC_COMBO_415, m_combo415Ctrl);
    DDX_CBIndex(pDX, IDC_COMBO_415, m_combo415);
    DDX_Control(pDX, IDC_SKIP_VIDEO, m_skipVideoCtrl);
    DDX_Check(pDX, IDC_SKIP_VIDEO, m_skipVideo);
    DDX_Check(pDX, IDC_VSYNC_OFF, m_vsyncOff);
    DDX_Control(pDX, IDC_VSYNC_OFF, m_vsyncOffCtrl);
}

// "Advanced..." button: runs the Advanced Option dialog modally on a copy of the page's graphics switches.
void CVideoDlg::OnAdvancedOption()
{
    CAdvancedOptionDlg dlg(this);
    dlg.DoModal();
}
