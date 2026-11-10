//
// Print.cpp - see report/win32/Print.h.
//
#include "report/win32/Print.h"

#include "report/Layout.h"
#include "report/win32/Gdi.h"
#include "report/win32/PrinterSetup.h"

#include <commdlg.h>

#include <algorithm>
#include <string>
#include <vector>

namespace rpt::win32 {

namespace {

constexpr wchar_t kAbortClass[] = L"rptPrintAbort";
constexpr int kCancelId = IDCANCEL;

bool gAborted = false;   // one print job at a time: the owner is disabled meanwhile

void pump()
{
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {   // put it back for the main loop
            PostQuitMessage(static_cast<int>(msg.wParam));
            gAborted = true;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

BOOL CALLBACK abortProc(HDC, int)
{
    pump();
    return !gAborted;
}

LRESULT CALLBACK abortWndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD(wp) == kCancelId) {
            gAborted = true;
            EnableWindow(reinterpret_cast<HWND>(lp), FALSE);
        }
        return 0;
    case WM_CLOSE:
        gAborted = true;
        return 0;
    default:
        return DefWindowProcW(h, msg, wp, lp);
    }
}

// The "Printing page n of m" window: a caption, a line of text and Cancel.
class AbortWindow {
public:
    AbortWindow(HWND owner, const std::wstring& title)
    {
        HINSTANCE inst = GetModuleHandleW(nullptr);
        static bool registered = false;
        if (!registered) {
            WNDCLASSEXW wc{};
            wc.cbSize = sizeof wc;
            wc.lpfnWndProc = abortWndProc;
            wc.hInstance = inst;
            wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
            wc.lpszClassName = kAbortClass;
            registered = RegisterClassExW(&wc) != 0;
        }
        const UINT dpi = owner ? GetDpiForWindow(owner) : 96;
        auto px = [dpi](int v) { return MulDiv(v, static_cast<int>(dpi), 96); };
        NONCLIENTMETRICSW ncm{};
        ncm.cbSize = sizeof ncm;
        SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0, dpi);
        font_ = CreateFontIndirectW(&ncm.lfMessageFont);

        RECT rc{0, 0, px(300), px(96)};
        AdjustWindowRectExForDpi(&rc, WS_POPUP | WS_CAPTION, FALSE, WS_EX_DLGMODALFRAME, dpi);
        const int w = rc.right - rc.left, h = rc.bottom - rc.top;
        RECT o{};
        GetWindowRect(owner ? owner : GetDesktopWindow(), &o);
        hwnd_ = CreateWindowExW(WS_EX_DLGMODALFRAME, kAbortClass, title.c_str(), WS_POPUP | WS_CAPTION,
                                o.left + ((o.right - o.left) - w) / 2, o.top + ((o.bottom - o.top) - h) / 2, w, h,
                                owner, nullptr, inst, nullptr);
        text_ = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER, px(12), px(16), px(276), px(20),
                                hwnd_, nullptr, inst, nullptr);
        HWND cancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                                      px(110), px(52), px(80), px(26), hwnd_,
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelId)), inst, nullptr);
        SendMessageW(text_, WM_SETFONT, reinterpret_cast<WPARAM>(font_), FALSE);
        SendMessageW(cancel, WM_SETFONT, reinterpret_cast<WPARAM>(font_), FALSE);
        if (hwnd_) {
            ShowWindow(hwnd_, SW_SHOW);
            UpdateWindow(hwnd_);
        }
    }
    ~AbortWindow()
    {
        if (hwnd_) DestroyWindow(hwnd_);
        if (font_) DeleteObject(font_);
    }
    AbortWindow(const AbortWindow&) = delete;
    AbortWindow& operator=(const AbortWindow&) = delete;

    void setText(const std::wstring& text)
    {
        if (text_) SetWindowTextW(text_, text.c_str());
        pump();
    }

private:
    HWND hwnd_ = nullptr;
    HWND text_ = nullptr;
    HFONT font_ = nullptr;
};

void showError(HWND owner, const wchar_t* what)
{
    const std::wstring text = std::wstring(what) + L"\n\nWindows error " + std::to_wstring(GetLastError()) + L".";
    MessageBoxW(owner, text.c_str(), L"Print", MB_OK | MB_ICONEXCLAMATION);
}

} // namespace

PrintResult PrintReport(HWND owner, const Report& report, int pageCount)
{
    const PrinterHandles start = CopyPrinter(report.orientation);
    if (!start.devNames) {
        MessageBoxW(owner, L"No printer is installed.", L"Print", MB_OK | MB_ICONINFORMATION);
        return PrintResult::Failed;
    }

    PRINTDLGW pd{};
    pd.lStructSize = sizeof pd;
    pd.hwndOwner = owner;
    pd.hDevMode = start.devMode;
    pd.hDevNames = start.devNames;
    pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_NOSELECTION;
    pd.nMinPage = 1;
    pd.nMaxPage = static_cast<WORD>(std::clamp(pageCount > 0 ? pageCount : 9999, 1, 9999));
    pd.nFromPage = 1;
    pd.nToPage = pd.nMaxPage;
    pd.nCopies = 1;
    // On Windows 11 this shows the system print dialog, which runs in
    // another process (explorer.exe); this thread waits in PrintDlgW.
    if (!PrintDlgW(&pd)) {
        const DWORD err = CommDlgExtendedError();
        if (pd.hDevMode) GlobalFree(pd.hDevMode);
        if (pd.hDevNames) GlobalFree(pd.hDevNames);
        if (err == 0) return PrintResult::Cancelled;
        MessageBoxW(owner, (L"The Print dialog failed (error " + std::to_wstring(err) + L").").c_str(), L"Print",
                    MB_OK | MB_ICONEXCLAMATION);
        return PrintResult::Failed;
    }
    // The printer chosen becomes the session's (previews re-lay out for it).
    SetSessionPrinter(pd.hDevMode, pd.hDevNames);
    HDC dc = pd.hDC;
    if (!dc) {
        showError(owner, L"The printer could not be opened.");
        return PrintResult::Failed;
    }

    // Lay out again for this printer: its fonts and its paper.
    std::vector<Page> pages;
    {
        const int saved = SaveDC(dc);
        GdiMeasure measure(CreateCompatibleDC(dc));
        // A compatible memory DC has the printer's resolution and fonts.
        pages = Layout(report, PageSetupFromDC(dc), measure);
        RestoreDC(dc, saved);
    }
    const int count = static_cast<int>(pages.size());
    int from = 1, to = count;
    if (pd.Flags & PD_PAGENUMS) {
        from = (std::max)(1, static_cast<int>(pd.nFromPage));
        to = (std::min)(count, static_cast<int>(pd.nToPage));
    }

    const std::wstring title = Widen(report.title.empty() ? std::string("Report") : report.title);
    PrintResult result = PrintResult::Printed;
    gAborted = false;
    const bool ownerWasEnabled = owner && IsWindowEnabled(owner);
    if (owner) EnableWindow(owner, FALSE);
    {
        AbortWindow progress(owner, title);
        SetAbortProc(dc, abortProc);
        DOCINFOW di{};
        di.cbSize = sizeof di;
        di.lpszDocName = title.c_str();
        // A "print to file" printer (Microsoft Print to PDF) asks for the file in StartDocW.
        if (from > to || StartDocW(dc, &di) <= 0) {
            if (from <= to) showError(owner, L"The document could not be started.");
            result = from > to ? PrintResult::Cancelled : PrintResult::Failed;
        } else {
            const int dpiX = GetDeviceCaps(dc, LOGPIXELSX), dpiY = GetDeviceCaps(dc, LOGPIXELSY);
            const POINT origin = PaperOrigin(dc);
            FontSet fonts;
            for (int p = from; p <= to && !gAborted; ++p) {
                progress.setText(L"Printing page " + std::to_wstring(p - from + 1) + L" of " +
                                 std::to_wstring(to - from + 1));
                if (gAborted) break;
                if (StartPage(dc) <= 0) {
                    if (!gAborted) showError(owner, L"A page could not be started.");
                    result = PrintResult::Failed;
                    break;
                }
                SetTwipsMapping(dc, dpiX, dpiY, origin);
                DrawPage(dc, pages[static_cast<size_t>(p - 1)], fonts);
                if (EndPage(dc) <= 0) {
                    if (!gAborted) showError(owner, L"A page could not be printed.");
                    result = PrintResult::Failed;
                    break;
                }
            }
            if (gAborted || result == PrintResult::Failed) {
                AbortDoc(dc);
                if (result != PrintResult::Failed) result = PrintResult::Cancelled;
            } else {
                EndDoc(dc);
            }
        }
    }
    if (owner) {
        EnableWindow(owner, ownerWasEnabled ? TRUE : FALSE);
        SetActiveWindow(owner);
    }
    DeleteDC(dc);
    return result;
}

} // namespace rpt::win32
