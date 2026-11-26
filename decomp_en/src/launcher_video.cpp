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
    ON_CBN_SELCHANGE(IDC_FOG_EFFECT, &CVideoDlg::OnCombo401)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    ON_WM_NCPAINT()
    ON_WM_DRAWITEM()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_GAMMA_SETTINGS, &CVideoDlg::OnGammaSettings)
    ON_CBN_SELCHANGE(IDC_COMBO_415, &CVideoDlg::OnSelchangeCombo415)
    ON_BN_CLICKED(IDC_USE_BACKBUFFER, &CVideoDlg::OnUseBackBuffer)
    ON_BN_CLICKED(IDC_ADVANCED, &CVideoDlg::OnAdvancedOption)
END_MESSAGE_MAP()

// Draws the page's controls with a transparent background.
HBRUSH CVideoDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    pDC->SetBkMode(TRANSPARENT);
    return hbr;
}

// Owner-draw items: the default processing.
void CVideoDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

// Background erase: the default processing.
BOOL CVideoDlg::OnEraseBkgnd(CDC *pDC)
{
    return CDialog::OnEraseBkgnd(pDC);
}

// Paint: only creates (and releases) the paint context; the dialog bitmap is drawn by the main dialog.
void CVideoDlg::OnPaint()
{
    CPaintDC dc(this);
}

// The non-client area is left undrawn.
void CVideoDlg::OnNcPaint()
{
}

// Gamma check box: nothing to do beyond the exchange the control already performs.
void CVideoDlg::OnGammaSettings()
{
}

// Handler of the 0x401 notification: nothing to do.
void CVideoDlg::OnCombo401()
{
}

// "Use BackBuffer" check box: Cross Fade and Blur Effect need the back buffer, so both are disabled and cleared while
// it is off.
void CVideoDlg::OnUseBackBuffer()
{
    UpdateData(TRUE);
    if (m_useBackBuffer == 0) {
        m_crossFadeCtrl.EnableWindow(FALSE);
        m_blurEffectCtrl.EnableWindow(FALSE);
        m_crossFadeCtrl.SendMessage(BM_SETCHECK, 0, 0);
        m_blurEffectCtrl.SendMessage(BM_SETCHECK, 0, 0);
    } else {
        m_crossFadeCtrl.EnableWindow(TRUE);
        m_blurEffectCtrl.EnableWindow(TRUE);
    }
}

// Selecting row 1 of the list in IDC_COMBO_415 warns about DirectSound3D (the Japanese text says that with
// DirectSound3D some sound cards make sound effects and voices hard to hear, and that volume differs between
// devices); the list in IDC_COMBO_414 is enabled only then. The strings are Shift-JIS in the exe, kept as escapes.
void CVideoDlg::OnSelchangeCombo415()
{
    if (m_combo415Ctrl.GetCurSel() == 1) {
        MessageBox(
        "DirextSound3D\x82\xf0\x8eg\x97p\x82\xb7\x82\xe9\x82\xc6\x81"
        "A\x83T\x83"
        "E\x83\x93\x83h\x83J\x81[\x83h\x82\xc9\x82\xe6\x82\xc1\x82\xc4\x82\xcd\x82r.\x82"
        "d.\x82\xe2\x89\xb9\x90\xba\x82\xaa\x95\xb7\x82\xab\x8e\xe6\x82\xe8\x93\xef\x82\xad\x82\xc8\x82\xe9\x82\xb1\x82\xc6\x82\xaa\x82\xa0\x82\xe8\x82\xdc\x82\xb7\x81"
        "B\n\x89\xb9\x97\xca\x82\xc9\x82\xe0\x8c\xc2\x91\xcc\x8d\xb7\x82\xaa\x82\xa0\x82\xe8\x82\xdc\x82\xb7\x82\xcc\x82\xc5\x81"
        "A\x82\xa0\x82\xe7\x82\xa9\x82\xb6\x82\xdf\x82\xb2\x97\xb9\x8f\xb3\x89\xba\x82\xb3\x82\xa2\x81"
        "B",
        "\x8am\x94"
        "F", 0);
        m_combo414Ctrl.EnableWindow(TRUE);
    } else {
        m_combo414Ctrl.EnableWindow(FALSE);
    }
}

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
