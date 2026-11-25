// EN-only launcher: the Advanced Option dialog (CAdvancedOptionDlg), opened from the Graphics page.
// Falcom MFC dialog code (ClassWizard style) from the Steam build; see NOTES.md "Launcher (MFC)". The original keeps
// each dialog class in its own source file (its destructor is an out-of-line call from the Graphics page), so the
// classes are split the same way here.
#include "launcher_dialog.h"



IMPLEMENT_DYNAMIC(CAdvancedOptionDlg, CDialog)

// Builds the dialog and copies the eleven graphics switches out of the Graphics page they were opened from.
// Called by CVideoDlg::OnAdvancedOption (EN 0x006f1550).
CAdvancedOptionDlg::CAdvancedOptionDlg(CVideoDlg *config, CWnd *pParent)
    : CDialog(IDD_ADVANCED_DIALOG, pParent)
{
    m_config = config;
    m_highQualityTexture = config->m_highQualityTexture;
    m_uncompressedTextures = config->m_uncompressedTextures;
    m_crossFade = config->m_crossFade;
    m_fogEffect = config->m_fogEffect;
    m_useBackBuffer = config->m_useBackBuffer;
    m_shadowQuality = config->m_shadowQuality;
    m_depthOfField = config->m_depthOfField;
    m_reflections = config->m_reflections;
    m_blurEffect = config->m_blurEffect;
    m_complexityShader = config->m_complexityShader;
    m_shadowComplexity = config->m_shadowComplexity;
    m_reflectionEffect = config->m_reflectionEffect;
}

// Destroys the two combo boxes and the check box control, then the dialog base. EN 0x006e5ed0.
CAdvancedOptionDlg::~CAdvancedOptionDlg()
{
}

// Exchanges the switches with their controls. The two combo boxes and the reflection check box are bound twice (the
// wizard-generated exchange for the value, then the control subclassing, repeated at the end).
void CAdvancedOptionDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Check(pDX, IDC_HIGH_QUALITY_TEXTURE, m_highQualityTexture);
    DDX_Check(pDX, IDC_UNCOMPRESSED_TEXTURES, m_uncompressedTextures);
    DDX_Check(pDX, IDC_CROSS_FADE, m_crossFade);
    DDX_Check(pDX, IDC_FOG_EFFECT, m_fogEffect);
    DDX_Check(pDX, IDC_USE_BACKBUFFER, m_useBackBuffer);
    DDX_CBIndex(pDX, IDC_SHADOW_QUALITY, m_shadowQuality);
    DDX_Check(pDX, IDC_DEPTH_OF_FIELD, m_depthOfField);
    DDX_Check(pDX, IDC_REFLECTIONS, m_reflections);
    DDX_Check(pDX, IDC_BLUR_EFFECT, m_blurEffect);
    DDX_Check(pDX, IDC_COMPLEXITY_SHADER, m_complexityShader);
    DDX_CBIndex(pDX, IDC_SHADOW_COMPLEXITY, m_shadowComplexity);
    DDX_Control(pDX, IDC_SHADOW_QUALITY, m_shadowQualityCtrl);
    DDX_Control(pDX, IDC_SHADOW_COMPLEXITY, m_shadowComplexityCtrl);
    DDX_Check(pDX, IDC_REFLECTION_EFFECT, m_reflectionEffect);
    DDX_Control(pDX, IDC_REFLECTION_EFFECT, m_reflectionEffectCtrl);
    DDX_Control(pDX, IDC_SHADOW_QUALITY, m_shadowQualityCtrl);
    DDX_Control(pDX, IDC_SHADOW_COMPLEXITY, m_shadowComplexityCtrl);
}

BEGIN_MESSAGE_MAP(CAdvancedOptionDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CAdvancedOptionDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// Sets the caption, fills the two quality lists (Low .. Ultra) and selects the current rows.
// Called by MFC when the dialog is created (DoModal).
BOOL CAdvancedOptionDlg::OnInitDialog()
{
    const char *qualityNames[4] = { "Low", "Medium", "High", "Ultra" };
    const char *complexityNames[4] = { "Low", "Medium", "High", "Ultra" };

    SetWindowText("Advanced Option");
    CenterWindow(NULL);
    for (int i = 0; i < 4; i++) {
        m_shadowQualityCtrl.AddString(qualityNames[i]);
        m_shadowComplexityCtrl.AddString(complexityNames[i]);
    }
    m_shadowQualityCtrl.SetCurSel(m_shadowQuality);
    m_shadowComplexityCtrl.SetCurSel(m_shadowComplexity);
    return TRUE;
}

// OK button: reads the controls back, writes the switches into the Graphics page, refreshes the page's controls and
// closes the dialog.
void CAdvancedOptionDlg::OnBnClickedOk()
{
    UpdateData(TRUE);
    m_config->m_highQualityTexture = m_highQualityTexture;
    m_config->m_uncompressedTextures = m_uncompressedTextures;
    m_config->m_crossFade = m_crossFade;
    m_config->m_fogEffect = m_fogEffect;
    m_config->m_useBackBuffer = m_useBackBuffer;
    m_config->m_shadowQuality = m_shadowQuality;
    m_config->m_depthOfField = m_depthOfField;
    m_config->m_reflections = m_reflections;
    m_config->m_blurEffect = m_blurEffect;
    m_config->m_complexityShader = m_complexityShader;
    m_config->m_shadowComplexity = m_shadowComplexity;
    m_config->m_reflectionEffect = m_reflectionEffect;
    m_config->UpdateData(FALSE);
    CDialog::OnOK();
}
