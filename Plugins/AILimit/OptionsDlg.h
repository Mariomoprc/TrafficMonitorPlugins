#pragma once
#include <windows.h>
#include <functional>
#include "Config.h"
#include "ApiClient.h"

class COptionsDlg {
public:
    COptionsDlg(CAIConfig* cfg);
    INT_PTR Show(HWND hParent);
    void SetLiveData(const GoData* go1, const GoData* go2, const OrData* orData);
    void SetLiveDs(const DsData* ds, const double* spent);
    void SetOnApply(std::function<void()> cb) { m_onApply = cb; }
private:
    CAIConfig* m_cfg;
    CAIConfig m_tmp;
    const GoData* m_liveGo1 = nullptr;
    const GoData* m_liveGo2 = nullptr;
    const OrData* m_liveOr = nullptr;
    const DsData* m_liveDs = nullptr;
    const double* m_liveSpent = nullptr;
    std::function<void()> m_onApply;
    int m_testGen = 0;
    bool m_showKey[4] = { false, false, false, false };
    bool m_hasTestGo1 = false;
    bool m_hasTestGo2 = false;
    bool m_hasTestOr = false;
    bool m_hasTestDs = false;
    GoData m_testGo1;
    GoData m_testGo2;
    OrData m_testOr;
    DsData m_testDs;
    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnInit(HWND hDlg);
    void OnOk(HWND hDlg);
    void OnApply(HWND hDlg);
    void OnTestGo(HWND hDlg, int kind);
    void OnTestDone(HWND hDlg, WPARAM wParam, LPARAM lParam);
    void OnToggleKey(HWND hDlg, int idx);
    int MatchPreset() const;
    void ApplyPreset(HWND hDlg, int sel);
    void RefreshPreview(HWND hDlg);
    void LoadToUI(HWND hDlg);
    void SaveFromUI(HWND hDlg);
};
