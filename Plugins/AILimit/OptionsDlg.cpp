#include "OptionsDlg.h"
#include "resource.h"
#include "ApiClient.h"
#include "Utils.h"
#include "AILimitPlugin.h"
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <process.h>
#include <cctype>

COptionsDlg::COptionsDlg(CAIConfig* cfg) : m_cfg(cfg) { if (cfg) m_tmp = *cfg; }

void COptionsDlg::SetLiveData(const GoData* go1, const GoData* go2, const OrData* orData) {
    m_liveGo1 = go1; m_liveGo2 = go2; m_liveOr = orData;
}

void COptionsDlg::SetLiveDs(const DsData* ds, const double* spent) {
    m_liveDs = ds; m_liveSpent = spent;
}

static std::wstring ZhErr(const std::string& err) {
    std::string low = err;
    for (auto& c : low) c = (char)tolower((unsigned char)c);
    if (low.find("401") != std::string::npos || low.find("403") != std::string::npos ||
        low.find("unauthorized") != std::string::npos || low.find("forbidden") != std::string::npos ||
        low.find("invalid") != std::string::npos || low.find("denied") != std::string::npos)
        return L"密钥无效或已过期";
    if (low.find("402") != std::string::npos) return L"额度不足，请续费";
    if (low.find("429") != std::string::npos || low.find("rate") != std::string::npos) return L"触发限流，稍后再试";
    if (low.find("timeout") != std::string::npos || low.find("connect") != std::string::npos ||
        low.find("failed") != std::string::npos) return L"网络连接失败，请检查网络";
    if (low == "empty key") return L"密钥为空";
    if (low == "empty endpoint" || low == "bad endpoint") return L"请先填写Go端点";
    std::wstring w = Utils::Utf8ToWide(err);
    if (w.size() > 120) w = w.substr(0, 120) + L"…";
    return w;
}

static std::wstring GoRemainText(const GoData& d) {
    int remain = 100 - d.monthly.percent;
    if (remain < 0) remain = 0;
    wchar_t buf[32];
    swprintf_s(buf, L"%d%%", remain);
    return buf;
}

static std::wstring GoResetText(const GoData& d) {
    if (!d.monthly.resetText.empty()) return d.monthly.resetText + L"(M)";
    if (!d.weekly.resetText.empty()) return d.weekly.resetText + L"(M)";
    if (!d.rolling.resetText.empty()) return d.rolling.resetText;
    return L"--";
}

static std::wstring OrBalanceText(const OrData& d) {
    wchar_t buf[32];
    swprintf_s(buf, L"$%.0f", d.remaining);
    return buf;
}

static std::wstring DsSym(const std::string& currency) {
    return (currency == "CNY") ? L"¥" : L"$";
}

static std::wstring DsTotalText(const DsData& d) {
    wchar_t buf[32];
    swprintf_s(buf, L"%ls%.0f", DsSym(d.currency).c_str(), d.total);
    return buf;
}

static std::wstring DsSpentText(const DsData& d, double spent) {
    if (spent > 0.001) {
        wchar_t buf[32];
        swprintf_s(buf, L"-%ls%.1f", DsSym(d.currency).c_str(), spent);
        return buf;
    }
    return L"-";
}

void COptionsDlg::RefreshPreview(HWND hDlg) {
    bool hide = m_tmp.hideOnExpired;
    std::wstring top, bot;
    auto addTop = [&](const std::wstring& s) { if (!top.empty()) top += L"  "; top += s; };
    auto addBot = [&](const std::wstring& s) { if (!bot.empty()) bot += L"  "; bot += s; };
    const GoData* g1 = (m_liveGo1 && m_liveGo1->valid) ? m_liveGo1 : (m_hasTestGo1 ? &m_testGo1 : nullptr);
    bool g1Fresh = (m_liveGo1 && m_liveGo1->valid);
    const GoData* g2 = (m_liveGo2 && m_liveGo2->valid) ? m_liveGo2 : (m_hasTestGo2 ? &m_testGo2 : nullptr);
    bool g2Fresh = (m_liveGo2 && m_liveGo2->valid);
    const OrData* orD = (m_liveOr && m_liveOr->valid) ? m_liveOr : (m_hasTestOr ? &m_testOr : nullptr);
    const DsData* dsD = (m_liveDs && m_liveDs->valid) ? m_liveDs : (m_hasTestDs ? &m_testDs : nullptr);
    bool dsFresh = (m_liveDs && m_liveDs->valid);
    double dsSpent = (m_liveSpent && dsFresh) ? *m_liveSpent : 0;
    if (m_tmp.showGoTop) {
        if (!g1) addTop(L"--（示例）");
        else if (hide && !g1->valid) addTop(L"[Go1 已隐藏]");
        else if (hide && CAILimitPlugin::IsGoExhausted(*g1)) addTop(L"[Go1 已隐藏]");
        else addTop(GoRemainText(*g1) + (g1Fresh ? L"" : L"（刚测得）"));
    }
    if (m_tmp.showGo2Top) {
        if (!g2) addTop(L"--（示例）");
        else if (hide && CAILimitPlugin::IsGoExhausted(*g2)) addTop(L"[Go2 已隐藏]");
        else addTop(GoRemainText(*g2) + (g2Fresh ? L"" : L"（刚测得）"));
    }
    if (m_tmp.showOrTop) {
        if (!orD) addTop(L"$9（示例）");
        else if (hide && CAILimitPlugin::IsOrExhausted(*orD)) addTop(L"[OR 已隐藏]");
        else addTop(OrBalanceText(*orD));
    }
    if (m_tmp.showDsTop) {
        if (!dsD) addTop(L"¥110（示例）");
        else if (hide && CAILimitPlugin::IsDsExhausted(*dsD)) addTop(L"[DS 已隐藏]");
        else addTop(DsTotalText(*dsD) + (dsFresh ? L"" : L"（刚测得）"));
    }
    if (m_tmp.showGoBottom) {
        if (!g1) addBot(L"24d（示例）");
        else if (hide && (!g1->valid || CAILimitPlugin::IsGoExhausted(*g1))) addBot(L"[Go1 已隐藏]");
        else addBot(GoResetText(*g1) + (g1Fresh ? L"" : L"（刚测得）"));
    }
    if (m_tmp.showGo2Bottom) {
        if (!g2) addBot(L"28d（示例）");
        else if (hide && CAILimitPlugin::IsGoExhausted(*g2)) addBot(L"[Go2 已隐藏]");
        else addBot(GoResetText(*g2) + (g2Fresh ? L"" : L"（刚测得）"));
    }
    if (m_tmp.showOrBottom) {
        if (!orD || orD->usageDaily <= 0.001) addBot(L"-");
        else { wchar_t b[32]; swprintf_s(b, L"-$%.1f", orD->usageDaily); addBot(b); }
    }
    if (m_tmp.showDsBottom) {
        if (!dsD) addBot(L"-¥2.3（示例）");
        else if (hide && CAILimitPlugin::IsDsExhausted(*dsD)) addBot(L"[DS 已隐藏]");
        else addBot(DsSpentText(*dsD, dsSpent) + (dsFresh ? L"" : L"（刚测得）"));
    }
    if (top.empty()) top = L"(上排无显示项)";
    if (bot.empty()) bot = L"(下排无显示项)";
    SetDlgItemTextW(hDlg, IDC_STATIC_PREVIEW_TOP, top.c_str());
    SetDlgItemTextW(hDlg, IDC_STATIC_PREVIEW_BOT, bot.c_str());
}

int COptionsDlg::MatchPreset() const {
    bool a = m_tmp.showGoTop, b = m_tmp.showGoBottom, c = m_tmp.showGo2Top;
    bool d = m_tmp.showGo2Bottom, e = m_tmp.showOrTop, f = m_tmp.showOrBottom;
    bool g = m_tmp.showDsTop, h = m_tmp.showDsBottom;
    if (a && b && !c && !d && e && !f && g && !h) return 0;
    if (a && !b && !c && !d && !e && !f && !g && !h) return 1;
    if (a && b && c && d && e && f && g && h) return 2;
    return 3;
}

void COptionsDlg::ApplyPreset(HWND hDlg, int sel) {
    auto set = [&](bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h) {
        CheckDlgButton(hDlg, IDC_CHECK_GO_TOP, a ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_GO_BOTTOM, b ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_GO2_TOP, c ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_GO2_BOTTOM, d ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_OR_TOP, e ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_OR_BOTTOM, f ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_DS_TOP, g ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHECK_DS_BOTTOM, h ? BST_CHECKED : BST_UNCHECKED);
    };
    if (sel == 0) set(true, true, false, false, true, false, true, false);
    else if (sel == 1) set(true, false, false, false, false, false, false, false);
    else if (sel == 2) set(true, true, true, true, true, true, true, true);
    else return;
    SaveFromUI(hDlg);
    RefreshPreview(hDlg);
}

struct TestOut {
    int kind;
    int gen;
    bool ok;
    GoData go;
    OrData orD;
    DsData ds;
    std::wstring text;
};

struct TestReq {
    COptionsDlg* self;
    HWND hDlg;
    int kind;
    int gen;
    std::wstring key;
    std::wstring ep;
};

static unsigned __stdcall TestThread(void* p) {
    TestReq* r = (TestReq*)p;
    TestOut* o = new TestOut();
    o->kind = r->kind; o->gen = r->gen;
    if (r->kind == 3) {
        std::string err;
        o->ok = FetchOr(r->key, o->orD, err, r->ep);
        if (o->ok) {
            wchar_t b[128];
            swprintf_s(b, L"OR 可用：余额 $%.0f，已用 %.1f%%", o->orD.remaining, o->orD.usage);
            o->text = b;
        } else o->text = L"OR 测试失败：" + ZhErr(err);
    } else if (r->kind == 4) {
        std::string err;
        o->ok = FetchDs(r->key, o->ds, err, r->ep);
        if (o->ok) {
            wchar_t b[160];
            swprintf_s(b, L"DS 可用：总额 %ls%.2f（赠 %ls%.2f）",
                DsSym(o->ds.currency).c_str(), o->ds.total,
                DsSym(o->ds.currency).c_str(), o->ds.granted);
            o->text = b;
        } else o->text = L"DS 测试失败：" + ZhErr(err);
    } else {
        std::string err;
        o->ok = FetchGo(r->key, o->go, err, r->ep);
        const wchar_t* nm = (r->kind == 1) ? L"Go1" : L"Go2";
        if (o->ok) {
            int remain = 100 - o->go.maxPercent; if (remain < 0) remain = 0;
            wchar_t b[256];
            swprintf_s(b, L"%ls 可用：剩余 %d%%，%ls", nm, remain, o->go.summary.c_str());
            o->text = b;
        } else {
            o->text = std::wstring(nm) + L" 测试失败：" + ZhErr(err);
        }
    }
    HWND h = r->hDlg;
    delete r;
    if (!PostMessageW(h, WM_APP_TEST_DONE, 0, (LPARAM)o)) delete o;
    return 0;
}

void COptionsDlg::OnTestGo(HWND hDlg, int kind) {
    int idEdit = (kind == 1) ? IDC_EDIT_GO_KEY1 : (kind == 2 ? IDC_EDIT_GO_KEY2 : (kind == 3 ? IDC_EDIT_OR_KEY : IDC_EDIT_DS_KEY));
    int idBtn = (kind == 1) ? IDC_BTN_TEST_GO1 : (kind == 2 ? IDC_BTN_TEST_GO2 : (kind == 3 ? IDC_BTN_TEST_OR : IDC_BTN_TEST_DS));
    wchar_t buf[4096];
    GetDlgItemTextW(hDlg, idEdit, buf, 4096);
    std::wstring key = buf;
    if (key.empty()) { SetDlgItemTextW(hDlg, IDC_STATIC_TEST_RESULT, L"密钥为空，请先填写"); return; }
    GetDlgItemTextW(hDlg, (kind == 3) ? IDC_EDIT_ENDPOINT_OR : ((kind == 4) ? IDC_EDIT_ENDPOINT_DS : IDC_EDIT_ENDPOINT_GO), buf, 4096);
    TestReq* r = new TestReq();
    r->self = this; r->hDlg = hDlg; r->kind = kind;
    r->gen = ++m_testGen; r->key = key; r->ep = buf;
    EnableWindow(GetDlgItem(hDlg, idBtn), FALSE);
    SetDlgItemTextW(hDlg, IDC_STATIC_TEST_RESULT, L"测试中…");
    uintptr_t th = _beginthreadex(nullptr, 0, TestThread, r, 0, nullptr);
    if (th) CloseHandle((HANDLE)th);
    else { delete r; EnableWindow(GetDlgItem(hDlg, idBtn), TRUE); }
}

void COptionsDlg::OnTestDone(HWND hDlg, WPARAM, LPARAM lParam) {
    TestOut* o = (TestOut*)lParam;
    if (!o) return;
    int idBtn = (o->kind == 1) ? IDC_BTN_TEST_GO1 : (o->kind == 2 ? IDC_BTN_TEST_GO2 : (o->kind == 3 ? IDC_BTN_TEST_OR : IDC_BTN_TEST_DS));
    EnableWindow(GetDlgItem(hDlg, idBtn), TRUE);
    if (o->gen != m_testGen) { delete o; return; }
    SetDlgItemTextW(hDlg, IDC_STATIC_TEST_RESULT, o->text.c_str());
    if (o->ok) {
        if (o->kind == 1) { m_testGo1 = o->go; m_hasTestGo1 = true; }
        else if (o->kind == 2) { m_testGo2 = o->go; m_hasTestGo2 = true; }
        else if (o->kind == 3) { m_testOr = o->orD; m_hasTestOr = true; }
        else { m_testDs = o->ds; m_hasTestDs = true; }
        SaveFromUI(hDlg);
        RefreshPreview(hDlg);
    }
    delete o;
}

void COptionsDlg::OnToggleKey(HWND hDlg, int idx) {
    int idEdit = (idx == 0) ? IDC_EDIT_GO_KEY1 : (idx == 1 ? IDC_EDIT_GO_KEY2 : (idx == 2 ? IDC_EDIT_OR_KEY : IDC_EDIT_DS_KEY));
    int idBtn = (idx == 0) ? IDC_BTN_SHOW_GO1 : (idx == 1 ? IDC_BTN_SHOW_GO2 : (idx == 2 ? IDC_BTN_SHOW_OR : IDC_BTN_SHOW_DS));
    m_showKey[idx] = !m_showKey[idx];
    SendMessageW(GetDlgItem(hDlg, idEdit), EM_SETPASSWORDCHAR, m_showKey[idx] ? 0 : (WPARAM)L'●', 0);
    SetDlgItemTextW(hDlg, idBtn, m_showKey[idx] ? L"隐" : L"显");
    SetFocus(GetDlgItem(hDlg, idEdit));
    SendMessageW(GetDlgItem(hDlg, idEdit), EM_SETSEL, 100000, 100000);
}

INT_PTR CALLBACK COptionsDlg::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    COptionsDlg* self = nullptr;
    if (msg == WM_INITDIALOG) {
        SetWindowLongPtrW(hDlg, DWLP_USER, (LONG_PTR)lParam);
        self = (COptionsDlg*)lParam;
        self->OnInit(hDlg);
        return TRUE;
    }
    self = (COptionsDlg*)GetWindowLongPtrW(hDlg, DWLP_USER);
    if (!self) return FALSE;
    if (msg == WM_APP_TEST_DONE) { self->OnTestDone(hDlg, wParam, lParam); return TRUE; }
    switch (msg) {
    case WM_COMMAND:
        {
            int id = LOWORD(wParam);
            int code = HIWORD(wParam);
            if (id == IDC_BTN_GOTO_GO || id == IDC_BTN_GOTO_GO2) { ShellExecuteW(nullptr, L"open", L"https://opencode.ai", nullptr, nullptr, SW_SHOW); return TRUE; }
            if (id == IDC_BTN_GOTO_OR) { ShellExecuteW(nullptr, L"open", L"https://openrouter.ai/keys", nullptr, nullptr, SW_SHOW); return TRUE; }
            if (id == IDC_BTN_GOTO_DS) { ShellExecuteW(nullptr, L"open", L"https://platform.deepseek.com/api_keys", nullptr, nullptr, SW_SHOW); return TRUE; }
            if (id == IDC_BTN_SHOW_GO1) { self->OnToggleKey(hDlg, 0); return TRUE; }
            if (id == IDC_BTN_SHOW_GO2) { self->OnToggleKey(hDlg, 1); return TRUE; }
            if (id == IDC_BTN_SHOW_OR) { self->OnToggleKey(hDlg, 2); return TRUE; }
            if (id == IDC_BTN_SHOW_DS) { self->OnToggleKey(hDlg, 3); return TRUE; }
            if (id == IDC_BTN_APPLY) { self->OnApply(hDlg); return TRUE; }
            if (code == BN_CLICKED && (
                id == IDC_CHECK_GO_TOP || id == IDC_CHECK_GO_BOTTOM ||
                id == IDC_CHECK_GO2_TOP || id == IDC_CHECK_GO2_BOTTOM ||
                id == IDC_CHECK_OR_TOP || id == IDC_CHECK_OR_BOTTOM ||
                id == IDC_CHECK_DS_TOP || id == IDC_CHECK_DS_BOTTOM ||
                id == IDC_CHECK_HIDE_EXPIRED || id == IDC_CHECK_SHOW_GO_LABEL ||
                id == IDC_CHECK_BATTERY_AUTO || id == IDC_CHECK_SHOW_RESET_TIME)) {
                self->SaveFromUI(hDlg);
                HWND hPreset = GetDlgItem(hDlg, IDC_COMBO_PRESET);
                if (hPreset) ComboBox_SetCurSel(hPreset, self->MatchPreset());
                self->RefreshPreview(hDlg);
                return TRUE;
            }
            if (id == IDC_COMBO_PRESET && code == CBN_SELCHANGE) {
                HWND hPreset = GetDlgItem(hDlg, IDC_COMBO_PRESET);
                int sel = ComboBox_GetCurSel(hPreset);
                self->ApplyPreset(hDlg, sel);
                return TRUE;
            }
            if ((id == IDC_COMBO_DISPLAY_MODE || id == IDC_COMBO_RESET_FORMAT || id == IDC_COMBO_COLOR_MODE) && code == CBN_SELCHANGE) {
                self->SaveFromUI(hDlg);
                self->RefreshPreview(hDlg);
                return TRUE;
            }
            switch (id) {
            case IDOK: self->OnOk(hDlg); EndDialog(hDlg, IDOK); return TRUE;
            case IDCANCEL: EndDialog(hDlg, IDCANCEL); return TRUE;
            case IDC_BTN_TEST_GO1: self->OnTestGo(hDlg, 1); return TRUE;
            case IDC_BTN_TEST_GO2: self->OnTestGo(hDlg, 2); return TRUE;
            case IDC_BTN_TEST_OR: self->OnTestGo(hDlg, 3); return TRUE;
            case IDC_BTN_TEST_DS: self->OnTestGo(hDlg, 4); return TRUE;
            }
        }
        break;
    }
    return FALSE;
}

INT_PTR COptionsDlg::Show(HWND hParent) {
    extern HINSTANCE g_hInst;
    return DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_AI_OPTIONS), hParent, DlgProc, (LPARAM)this);
}

void COptionsDlg::OnInit(HWND hDlg) {
    HWND hPreset = GetDlgItem(hDlg, IDC_COMBO_PRESET);
    if (hPreset) {
        ComboBox_AddString(hPreset, L"推荐（Go上+下+OR上+DS上）");
        ComboBox_AddString(hPreset, L"极简（仅Go上）");
        ComboBox_AddString(hPreset, L"详细（全部）");
        ComboBox_AddString(hPreset, L"自定义");
    }
    HWND hSpin = GetDlgItem(hDlg, IDC_SPIN_INTERVAL);
    if (hSpin) SendMessageW(hSpin, UDM_SETRANGE, 0, MAKELPARAM(3600, 30));
    HWND hSpinW = GetDlgItem(hDlg, IDC_SPIN_WIDTH);
    if (hSpinW) SendMessageW(hSpinW, UDM_SETRANGE, 0, MAKELPARAM(300, 50));
    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_RESET_FORMAT);
    if (hCombo) {
        ComboBox_AddString(hCombo, L"Auto (50m / 1d 21h)");
        ComboBox_AddString(hCombo, L"Days only");
    }
    HWND hDisp = GetDlgItem(hDlg, IDC_COMBO_DISPLAY_MODE);
    if (hDisp) {
        ComboBox_AddString(hDisp, L"进度条");
        ComboBox_AddString(hDisp, L"简约文字");
        ComboBox_AddString(hDisp, L"环形");
    }
    HWND hColor = GetDlgItem(hDlg, IDC_COMBO_COLOR_MODE);
    if (hColor) {
        ComboBox_AddString(hColor, L"单色");
        ComboBox_AddString(hColor, L"三段色");
    }
    LoadToUI(hDlg);
    SendMessageW(GetDlgItem(hDlg, IDC_EDIT_ENDPOINT_GO), EM_SETCUEBANNER, TRUE, (LPARAM)L"Go额度接口地址，如 https://host/v1/usage");
    if (hPreset) ComboBox_SetCurSel(hPreset, MatchPreset());
    RefreshPreview(hDlg);
}

void COptionsDlg::LoadToUI(HWND hDlg) {
    SetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY1, m_tmp.goKey1.c_str());
    SetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY2, m_tmp.goKey2.c_str());
    SetDlgItemTextW(hDlg, IDC_EDIT_OR_KEY, m_tmp.orKey.c_str());
    SetDlgItemTextW(hDlg, IDC_EDIT_DS_KEY, m_tmp.dsKey.c_str());
    SetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY1_NAME, m_tmp.goKey1Name.c_str());
    SetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY2_NAME, m_tmp.goKey2Name.c_str());
    SetDlgItemInt(hDlg, IDC_EDIT_INTERVAL, m_tmp.intervalSec, FALSE);
    SetDlgItemInt(hDlg, IDC_EDIT_WIDTH, m_tmp.widthPx, FALSE);
    SetDlgItemTextW(hDlg, IDC_EDIT_ENDPOINT_GO, m_tmp.endpointGo.c_str());
    SetDlgItemTextW(hDlg, IDC_EDIT_ENDPOINT_OR, m_tmp.endpointOr.c_str());
    SetDlgItemTextW(hDlg, IDC_EDIT_ENDPOINT_DS, m_tmp.endpointDs.c_str());

    CheckDlgButton(hDlg, IDC_CHECK_GO_TOP, m_tmp.showGoTop ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_GO_BOTTOM, m_tmp.showGoBottom ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_GO2_TOP, m_tmp.showGo2Top ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_GO2_BOTTOM, m_tmp.showGo2Bottom ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_OR_TOP, m_tmp.showOrTop ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_OR_BOTTOM, m_tmp.showOrBottom ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_DS_TOP, m_tmp.showDsTop ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_DS_BOTTOM, m_tmp.showDsBottom ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hDlg, IDC_CHECK_SHOW_RESET_TIME, m_tmp.showResetTime ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_HIDE_EXPIRED, m_tmp.hideOnExpired ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_SHOW_GO_LABEL, m_tmp.showGoLabel ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECK_BATTERY_AUTO, m_tmp.batteryAuto ? BST_CHECKED : BST_UNCHECKED);
    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_RESET_FORMAT);
    if (hCombo) ComboBox_SetCurSel(hCombo, m_tmp.resetTimeFormat);
    HWND hDisp = GetDlgItem(hDlg, IDC_COMBO_DISPLAY_MODE);
    if (hDisp) ComboBox_SetCurSel(hDisp, m_tmp.displayMode < 0 || m_tmp.displayMode > 2 ? 0 : m_tmp.displayMode);
    HWND hColor = GetDlgItem(hDlg, IDC_COMBO_COLOR_MODE);
    if (hColor) ComboBox_SetCurSel(hColor, m_tmp.colorMode ? 1 : 0);
}

void COptionsDlg::SaveFromUI(HWND hDlg) {
    wchar_t buf[4096];
    GetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY1, buf, 4096); m_tmp.goKey1 = buf;
    GetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY2, buf, 4096); m_tmp.goKey2 = buf;
    GetDlgItemTextW(hDlg, IDC_EDIT_OR_KEY, buf, 4096); m_tmp.orKey = buf;
    GetDlgItemTextW(hDlg, IDC_EDIT_DS_KEY, buf, 4096); m_tmp.dsKey = buf;
    GetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY1_NAME, buf, 4096); m_tmp.goKey1Name = buf;
    GetDlgItemTextW(hDlg, IDC_EDIT_GO_KEY2_NAME, buf, 4096); m_tmp.goKey2Name = buf;
    m_tmp.intervalSec = GetDlgItemInt(hDlg, IDC_EDIT_INTERVAL, nullptr, FALSE);
    if (m_tmp.intervalSec < 30) m_tmp.intervalSec = 30;
    if (m_tmp.intervalSec > 3600) m_tmp.intervalSec = 3600;
    m_tmp.widthPx = GetDlgItemInt(hDlg, IDC_EDIT_WIDTH, nullptr, FALSE);
    if (m_tmp.widthPx < 50) m_tmp.widthPx = 50;
    if (m_tmp.widthPx > 300) m_tmp.widthPx = 300;
    GetDlgItemTextW(hDlg, IDC_EDIT_ENDPOINT_GO, buf, 4096); m_tmp.endpointGo = buf;
    GetDlgItemTextW(hDlg, IDC_EDIT_ENDPOINT_OR, buf, 4096); m_tmp.endpointOr = buf;
    GetDlgItemTextW(hDlg, IDC_EDIT_ENDPOINT_DS, buf, 4096); m_tmp.endpointDs = buf;

    m_tmp.showGoTop = IsDlgButtonChecked(hDlg, IDC_CHECK_GO_TOP) == BST_CHECKED;
    m_tmp.showGoBottom = IsDlgButtonChecked(hDlg, IDC_CHECK_GO_BOTTOM) == BST_CHECKED;
    m_tmp.showGo2Top = IsDlgButtonChecked(hDlg, IDC_CHECK_GO2_TOP) == BST_CHECKED;
    m_tmp.showGo2Bottom = IsDlgButtonChecked(hDlg, IDC_CHECK_GO2_BOTTOM) == BST_CHECKED;
    m_tmp.showOrTop = IsDlgButtonChecked(hDlg, IDC_CHECK_OR_TOP) == BST_CHECKED;
    m_tmp.showOrBottom = IsDlgButtonChecked(hDlg, IDC_CHECK_OR_BOTTOM) == BST_CHECKED;
    m_tmp.showDsTop = IsDlgButtonChecked(hDlg, IDC_CHECK_DS_TOP) == BST_CHECKED;
    m_tmp.showDsBottom = IsDlgButtonChecked(hDlg, IDC_CHECK_DS_BOTTOM) == BST_CHECKED;

    m_tmp.showResetTime = IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_RESET_TIME) == BST_CHECKED;
    m_tmp.hideOnExpired = IsDlgButtonChecked(hDlg, IDC_CHECK_HIDE_EXPIRED) == BST_CHECKED;
    m_tmp.showGoLabel = IsDlgButtonChecked(hDlg, IDC_CHECK_SHOW_GO_LABEL) == BST_CHECKED;
    m_tmp.batteryAuto = IsDlgButtonChecked(hDlg, IDC_CHECK_BATTERY_AUTO) == BST_CHECKED;
    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_RESET_FORMAT);
    if (hCombo) m_tmp.resetTimeFormat = ComboBox_GetCurSel(hCombo);
    HWND hDisp = GetDlgItem(hDlg, IDC_COMBO_DISPLAY_MODE);
    if (hDisp) m_tmp.displayMode = ComboBox_GetCurSel(hDisp);
    HWND hColor = GetDlgItem(hDlg, IDC_COMBO_COLOR_MODE);
    if (hColor) m_tmp.colorMode = ComboBox_GetCurSel(hColor) == 1 ? 1 : 0;
}

void COptionsDlg::OnOk(HWND hDlg) {
    SaveFromUI(hDlg);
    *m_cfg = m_tmp;
    m_cfg->Save(m_cfg->configPath);
    if (m_onApply) m_onApply();
}

void COptionsDlg::OnApply(HWND hDlg) {
    SaveFromUI(hDlg);
    *m_cfg = m_tmp;
    m_cfg->Save(m_cfg->configPath);
    if (m_onApply) m_onApply();
    RefreshPreview(hDlg);
}
