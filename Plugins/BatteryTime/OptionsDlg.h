#pragma once
#include "DataManager.h"

class COptionsDlg : public CDialog
{
    DECLARE_DYNAMIC(COptionsDlg)

public:
    COptionsDlg(CWnd* pParent = nullptr);
    virtual ~COptionsDlg();

    SettingData m_data;

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_OPTIONS_DIALOG };
#endif

private:
    CComboBox m_progress_style_combo;
    CComboBox m_time_format_combo;
    CEdit m_progress_height_edit;
    CEdit m_progress_max_width_edit;
    CEdit m_font_size_edit;
    CEdit m_label_text_edit;
    CEdit m_low_battery_threshold_edit;
    CEdit m_custom_format_edit;

    CStatic m_preview_static;
    double m_preview_percent{ 75.0 };
    int m_preview_step{ 0 };

    void DrawPreview();
    void RefreshPreview();
    void SyncControls();
    std::wstring FormatTimeForPreview(double percent) const;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnBnClickedPreviewCheck();
    afx_msg void OnBnClickedProgressAboveRadio();
    afx_msg void OnBnClickedProgressBelowRadio();
    afx_msg void OnEnChangeProgressHeightEdit();
    afx_msg void OnEnChangeProgressMaxWidthEdit();
    afx_msg void OnCbnSelchangeProgressStyleCombo();
    afx_msg void OnEnChangeFontSizeEdit();
    afx_msg void OnCbnSelchangeTimeFormatCombo();
    afx_msg void OnBnClickedShowLabelCheck();
    afx_msg void OnEnChangeLabelTextEdit();
    afx_msg void OnBnClickedLowBatteryCheck();
    afx_msg void OnEnChangeLowBatteryThresholdEdit();
    afx_msg void OnBnClickedShowTooltipCheck();
    afx_msg void OnBnClickedShowChargingCheck();
    afx_msg void OnBnClickedApplyButton();
    afx_msg void OnBnClickedDefaultButton();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnPaintPreview();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnBnClickedShowDaysCheck();
    afx_msg void OnBnClickedShowSecondsCheck();
    afx_msg void OnBnClickedHideZeroCheck();
    afx_msg void OnEnChangeCustomFormatEdit();
};
