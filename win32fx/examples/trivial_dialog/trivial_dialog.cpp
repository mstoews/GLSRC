//
// trivial_dialog.cpp - win32fx consumed from outside gl.sln (plan M3 task 8,
// acceptance 6.7 #9), the way the AP / AR projects will consume it.
//
//   trivial_dialog.exe                    the dialog, modal
//   trivial_dialog.exe /selftest          the checks below, results in a message box
//   trivial_dialog.exe /selftest /quiet   the same, no message box; exit code = failures
//                                         (from cmd: start /wait trivial_dialog.exe /selftest /quiet
//                                          then echo %errorlevel%)
//
// The self-test is the plan's two-instance dispatch test (risk P2): two live
// instances of one dialog class, and two MDI children of one window class,
// each receive their own messages. It also drives the custom controls with
// WM_SETTEXT / WM_CHAR.
//
#include <Windows.h>

#include "win32fx.h"
#include "resource.h"

#include <string>

namespace {

class TrivialDialog : public fx::Dialog {
public:
    explicit TrivialDialog(const wchar_t* name) : name_(name) {}
    int pings = 0;

protected:
    bool OnInitDialog() override
    {
        SetDlgItemTextW(hwnd(), IDC_NAME, name_);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND) override
    {
        if (id != IDC_PING || code != BN_CLICKED) return false;
        SetDlgItemInt(hwnd(), IDC_PINGS, static_cast<UINT>(++pings), FALSE);
        return true;
    }

    BEGIN_MSG_MAP(TrivialDialog)
        ON_MSG(WM_APP, OnWhoAmI)
    END_MSG_MAP()

    LRESULT OnWhoAmI(WPARAM, LPARAM) { return reinterpret_cast<LRESULT>(this); }

private:
    const wchar_t* name_;
};

class ProbeChild : public fx::MdiChild {
public:
    int commands = 0;

protected:
    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != 42) return false;
        ++commands;
        return true;
    }
};

class ProbeFrame : public fx::MdiFrame {
public:
    HWND lastActivated = nullptr;
    void OnMdiChildActivate(HWND child) override { lastActivated = child; }

protected:
    bool OnDestroy() override { return true; }   // not the main window: no WM_QUIT
};

struct Results {
    int failures = 0;
    std::wstring log;
    void check(bool ok, const wchar_t* what)
    {
        log += ok ? L"ok      " : L"FAILED  ";
        log += what;
        log += L"\n";
        if (!ok) ++failures;
    }
};

void typeText(HWND ctl, const wchar_t* text)
{
    SendMessageW(ctl, EM_SETSEL, 0, -1);
    SendMessageW(ctl, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L""));
    for (const wchar_t* p = text; *p; ++p) SendMessageW(ctl, WM_CHAR, *p, 0);
}

void dialogInstances(HINSTANCE inst, Results& r)
{
    TrivialDialog a(L"A"), b(L"B");
    HWND ha = a.CreateModeless(inst, IDD_TRIVIAL, nullptr);
    HWND hb = b.CreateModeless(inst, IDD_TRIVIAL, nullptr);
    r.check(ha && hb && ha != hb, L"two modeless instances of one Dialog class");
    if (!ha || !hb) return;

    const auto ping = [](HWND dlg) {
        SendMessageW(dlg, WM_COMMAND, MAKEWPARAM(IDC_PING, BN_CLICKED),
                     reinterpret_cast<LPARAM>(GetDlgItem(dlg, IDC_PING)));
    };
    ping(ha);
    ping(ha);
    ping(hb);
    r.check(a.pings == 2 && b.pings == 1, L"WM_COMMAND reaches the instance it was sent to");
    r.check(SendMessageW(ha, WM_APP, 0, 0) == reinterpret_cast<LRESULT>(&a) &&
                SendMessageW(hb, WM_APP, 0, 0) == reinterpret_cast<LRESULT>(&b),
            L"message map runs on its own instance, result via DWLP_MSGRESULT");
    r.check(fx::Dialog::FromHandle(ha) == &a && fx::Dialog::FromHandle(hb) == &b, L"Dialog::FromHandle");

    ShowWindow(hb, SW_SHOW);
    SetActiveWindow(hb);
    SendMessageW(GetDlgItem(hb, IDC_PING), BM_CLICK, 0, 0);
    r.check(a.pings == 2 && b.pings == 2, L"a click on B's tbSuperBtn reaches B only");

    // Custom controls, on dialog A.
    HWND account = GetDlgItem(ha, IDC_ACCOUNT), phone = GetDlgItem(ha, IDC_PHONE);
    HWND date = GetDlgItem(ha, IDC_DATE), amount = GetDlgItem(ha, IDC_AMOUNT), qty = GetDlgItem(ha, IDC_QUANTITY);
    r.check(fx::GetText(account).empty() && fx::GetText(amount).empty(), L"template text is the mask, not shown");
    wchar_t mask[32] = {};
    SendMessageW(account, EPM_GETMASK, 32, reinterpret_cast<LPARAM>(mask));
    r.check(std::wstring(mask) == L"NNNNNNN", L"EPM_GETMASK returns the template mask");
    const LRESULT code = SendMessageW(account, WM_GETDLGCODE, 0, 0);
    r.check((code & (DLGC_WANTCHARS | DLGC_WANTARROWS)) == (DLGC_WANTCHARS | DLGC_WANTARROWS),
            L"tbPic WM_GETDLGCODE has DLGC_WANTCHARS | DLGC_WANTARROWS");

    typeText(account, L"AB12345X");
    r.check(fx::GetText(account) == L"AB12345", L"tbPic NNNNNNN: at most 7 letters/digits");
    typeText(account, L"A-1");
    r.check(fx::GetText(account) == L"A1", L"tbPic NNNNNNN: '-' rejected");
    typeText(phone, L"5551234567");
    r.check(fx::GetText(phone) == L"(555) 123 - 4567", L"tbPic phone mask: literals inserted");

    SetWindowTextW(date, L"19950131");
    r.check(fx::GetText(date) == L"01/31/95", L"tbDate shows YYYYMMDD as MM/DD/YY");
    r.check(fx::DateGetYmd(date) == L"19950131", L"DateGetYmd reads it back as YYYYMMDD");
    typeText(date, L"123199");
    r.check(fx::GetText(date) == L"12/31/99" && fx::DateGetYmd(date) == L"19991231", L"tbDate typing inserts '/'");
    typeText(date, L"023099");
    r.check(fx::DateGetYmd(date).empty(), L"DateGetYmd: Feb 30 is not a date");

    SetWindowTextW(amount, L"     1234.5");
    r.check(fx::GetText(amount) == L"1,234.50", L"tbFloat formats a padded cb4sqlite value");
    double v = 0;
    r.check(FloatGetValue(amount, &v) && v == 1234.5, L"FloatGetValue ignores separators");
    typeText(amount, L"12.345");
    r.check(fx::GetText(amount) == L"12.34", L"tbFloat .99: third decimal rejected");
    FloatSetDlgItemValue(ha, IDC_AMOUNT, -0.5);
    r.check(fx::GetText(amount) == L"-0.50", L"FloatSetValue");

    IntSetDlgItemValue(ha, IDC_QUANTITY, 42);
    LONG n = 0;
    r.check(IntGetDlgItemValue(ha, IDC_QUANTITY, &n) && n == 42, L"IntSetValue / IntGetValue");
    typeText(qty, L"12345");
    r.check(fx::GetText(qty) == L"1234", L"tbInteger 9999: fifth digit rejected");
    typeText(qty, L"1.5");
    r.check(fx::GetText(qty) == L"15", L"tbInteger: '.' rejected");

    DestroyWindow(ha);
    DestroyWindow(hb);
    r.check(a.hwnd() == nullptr && b.hwnd() == nullptr, L"modeless dialogs detach on WM_NCDESTROY");
}

void mdiInstances(HINSTANCE inst, Results& r)
{
    ProbeFrame frame;
    r.check(frame.CreateFrame(inst, L"TrivialProbeFrame", L"probe", nullptr, -1) != nullptr, L"MdiFrame created");
    if (!frame.hwnd()) return;
    ProbeChild one, two;
    HWND h1 = one.CreateChild(frame, L"TrivialProbeChild", L"one");
    HWND h2 = two.CreateChild(frame, L"TrivialProbeChild", L"two");
    r.check(h1 && h2 && h1 != h2, L"two MDI children of one window class");
    SendMessageW(h1, WM_COMMAND, 42, 0);
    SendMessageW(h2, WM_COMMAND, 42, 0);
    SendMessageW(h2, WM_COMMAND, 42, 0);
    r.check(one.commands == 1 && two.commands == 2, L"WM_COMMAND reaches the MDI child it was sent to");
    r.check(fx::Window::FromHandle(h1) == &one && fx::Window::FromHandle(h2) == &two,
            L"Window::FromHandle (lpParam arrived through MDICREATESTRUCT)");
    SendMessageW(frame.mdiClient(), WM_MDIACTIVATE, reinterpret_cast<WPARAM>(h1), 0);
    r.check(frame.lastActivated == h1 && frame.activeChild() == h1, L"OnMdiChildActivate hook");
    DestroyWindow(frame);
    r.check(one.hwnd() == nullptr && two.hwnd() == nullptr && frame.hwnd() == nullptr,
            L"frame and children detach when destroyed");
}

} // namespace

int WINAPI wWinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE, _In_ LPWSTR cmdLine, _In_ int)
{
    if (!fx::RegisterCustomControls(inst)) return 100;
    const std::wstring args = cmdLine ? cmdLine : L"";
    if (args.find(L"/selftest") == std::wstring::npos) {
        TrivialDialog dlg(L"modal");
        return static_cast<int>(dlg.DoModal(inst, IDD_TRIVIAL, nullptr));
    }
    Results r;
    dialogInstances(inst, r);
    mdiInstances(inst, r);
    if (args.find(L"/quiet") == std::wstring::npos) {
        r.log += r.failures ? fx::Format(L"\n%d check(s) FAILED", r.failures) : std::wstring(L"\nAll checks passed");
        MessageBoxW(nullptr, r.log.c_str(), L"win32fx self-test", r.failures ? MB_ICONERROR : MB_ICONINFORMATION);
    }
    return r.failures;
}
