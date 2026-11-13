//
// ResourceCheck.cpp - see ResourceCheck.h.
//
//   gl.exe /checkres           report in a message box
//   gl.exe /checkres /quiet    exit code = failures
//                              (from cmd: start /wait gl.exe /checkres /quiet
//                               then echo %errorlevel%)
//
// The dialog check also measures each caption against its control (UI
// refresh spec 4.6, D6): see checkTextFit. SaveScreenshots is gl.exe
// /screenshots (App.cpp).
//
#include "ResourceCheck.h"

#include "PortIds.h"

#include "win32fx.h"

#include <dwmapi.h>
#include <wincodec.h>

#include <algorithm>
#include <cwchar>
#include <filesystem>
#include <string>
#include <vector>

#pragma comment(lib, "dwmapi.lib")        // DwmFlush, DwmGetWindowAttribute
#pragma comment(lib, "windowscodecs.lib") // CLSID_WICImagingFactory, the PNG encoder
#pragma comment(lib, "ole32.lib")

#ifndef PW_RENDERFULLCONTENT
#define PW_RENDERFULLCONTENT 0x00000002
#endif

#include "LegacyIds.h"   // last: see LegacyIds.h (the picture IDs below)

namespace {

struct Report {
    int failures = 0;
    std::wstring log;
    void check(bool ok, const std::wstring& what)
    {
        log += ok ? L"ok      " : L"FAILED  ";
        log += what;
        log += L"\n";
        if (!ok) ++failures;
    }
};

INT_PTR CALLBACK ProbeProc(HWND, UINT, WPARAM, LPARAM) { return FALSE; }

// The buttons a screen gives a picture (MainDialog::SetBitmap: SBM_SETPICT with
// the bitmap of the same ID), so the text-fit check and the screenshots see
// the buttons as the screen shows them. Keep in step with the SetBitmap calls.
enum class Pics { None, Main };   // Main: MainDialog::OnInitDialog's set (kMainPictures)

const int kMainPictures[] = {IDCANCEL, IDOK, ID_NEW, ID_TOP, ID_BOTTOM, ID_NEXT, ID_PREVIOUS, ID_DELETE, ID_SLIPLIST};

struct DialogCase {
    int id;
    const wchar_t* what;
    bool hasGrid;   // contains a tbSpread grid (fx::Grid, Spec #2 S2-M0): checked to hold its ListView
    Pics pics = Pics::None;
    std::vector<int> morePics = {};   // the screen's own SetBitmap calls
    std::vector<std::pair<int, int>> borrowedPics = {};   // {button, picture}: a picture of another ID
};

// Spec 6.2 as corrected in M4. The three named in the M4 exit criteria come first.
const DialogCase kDialogs[] = {
    {1010, L"Chart of Accounts (TGLAcctsAdd / TGLAcctsDelete)", false, Pics::Main, {ID_GLINQ, ID_SETDATA}},
    {1158, L"Login (TGLAccess)", false, Pics::Main, {ID_ADD}},
    {1021, L"Company Profile (TGLProfile)", false, Pics::Main, {4000, 1125}},   // ID_SETPERIODS, ID_SETACCOUNTS
    {1008, L"About", false},
    {1028, L"Set Period Dates (TGLSetPeriodDates)", false, Pics::Main},
    {1032, L"Toolbar (TToolBar)", false},
    {1033, L"Account Setup (TGLSetAccounts)", false, Pics::Main},
    {1034, L"Cash Accounts (TAPCashAccts)", false, Pics::Main, {2236}},   // ID_CASHACCTS
    {1035, L"List Window Parameters (TSetQuery)", false},
    {2057, L"Module bar (TVToolBar)", false},
    {1159, L"User Access Update (TGLAdministration)", false, Pics::Main,
     {ID_MODIFY, ID_SEARCH, ID_ADD, ID_VIEW, 1094}},   // 1094: Login.cpp's ID_RESET
    {1011, L"Cash Accounts list (TAPCashAcctsList)", true},
    {1160, L"Menu Access (TAccessMenus)", true, Pics::Main},
    // Spec #2 S2-M2.
    {1023, L"Department Codes (TGLDeptCodes)", false, Pics::Main, {ID_GLINQ}},
    {1006, L"Medical Reps (TGLMedReps)", false, Pics::Main},
    {1040, L"Doctor Codes (TGLDrCodes)", false, Pics::Main},
    {1007, L"Product Codes (TGLProd)", false, Pics::Main},
    // Spec #2 S2-M3.
    {1013, L"Voucher Entry (TGLBatchEntry, TGLPrior)", false, Pics::Main, {4095, 1121, 3068}},   // Add, List, Print
    // Spec #2 S2-M4.
    {1061, L"Post General Ledger Voucher (TGLPostBatch)", true, Pics::Main},
    {1015, L"Recover General Ledger Voucher (TGLRecoverBatch)", true, Pics::Main},
    {1043, L"Cancel General Ledger Voucher (TGLCancelBatch)", true, Pics::Main},
    // Spec #2 S2-M5.
    {1029, L"Transaction Inquiry (TGLTransInq)", true, Pics::Main, {2000, ID_SETDATA}},   // 2000: kSearch
    {1017, L"Budget Data (TGLBudget)", true, Pics::Main, {ID_GLINQ, ID_SETDATA}},
    // Spec #3 S3-M7.
    {1022, L"Report list (TGLCustomReport, D9)", true},
    // Spec #4 S4-M1.
    {4002, L"Asset Types (TGLTypes)", false, Pics::Main, {1103}},   // kTypesButton (no bitmap 1103: no picture)
    // Spec #4 S4-M2.
    {4004, L"Fixed Assets Add/Modify (TGLFixedAssets)", false, Pics::Main, {1103, 1074}},
    // Spec #4 S4-M3.
    {4003, L"Create Voucher (TGLCreateDeprSlip)", false, Pics::Main},
    // Spec #4 S4-M4.
    {1001, L"Disposal (TGLFixedDelete)", false, Pics::Main, {1074}},
    // Spec #5 S5-M1.
    {1041, L"Vendors (TAPVendorAdd)", false, Pics::Main, {ID_SETDATA}, {{4082, ID_NEW}, {3043, 4054}}},
    // The port's own (gl/res/port.rc, spec #3 3).
    {IDD_SAMPLE_OPTIONS, L"Sample Report options (port.rc, S3-M1)", false},
    {IDD_TB_OPTIONS, L"Trial balance options (port.rc, S3-M2)", false},
    {IDD_STMT_OPTIONS, L"Financial statement options (port.rc, S3-M3)", false},
    {IDD_STMT_LAYOUT, L"Statement Layout (port.rc, S3-M4)", false},   // its grid is IDC_SL_GRID, not ID_DISTLIST
};

void applyPictures(HWND dlg, const DialogCase& d)
{
    auto set = [dlg](int id) {
        SendDlgItemMessageW(dlg, id, SBM_SETPICT, SUPERBTN_PICT_BITMAP, static_cast<LPARAM>(id));
    };
    if (d.pics == Pics::Main)
        for (const int id : kMainPictures) set(id);
    for (const int id : d.morePics) set(id);
    for (const auto& [button, picture] : d.borrowedPics)
        SendDlgItemMessageW(dlg, button, SBM_SETPICT, SUPERBTN_PICT_BITMAP, static_cast<LPARAM>(picture));
}

bool isChildTemplate(HWND dlg) { return (GetWindowLongPtrW(dlg, GWL_STYLE) & WS_CHILD) != 0; }

// ---- text fit (UI refresh spec 4.6) -------------------------------------------
//
// Each caption is measured in its control's font with DrawText(DT_CALCRECT)
// and compared with the room the control leaves for it. The allowances are
// what the v6 controls draw around the text at 96 DPI, scaled:
//   static            the client rectangle; word-wrapped unless SS_LEFTNOWORDWRAP
//                     or SS_SIMPLE, and then the lines must fit its height
//   check box, radio  16 px for the box and the gap
//   group box         16 px: the caption's indent and its gaps in the frame
//   push button       4 px each side, plus the picture and a 4 px gap
// Picture-only buttons (BS_BITMAP / BS_ICON, or the text "tbSuperBtn") and
// controls without text are skipped. Child templates (the toolbar 1032 and the
// module bar 2057) are skipped: code places their controls.

struct FitProblem {
    int id;
    std::wstring text;
    int need, room;
    bool lines;   // need/room are heights: the text wraps onto more lines than fit
};

bool same(const wchar_t* a, const wchar_t* b)   // ignoring case, as window class names compare
{
    return CompareStringOrdinal(a, -1, b, -1, TRUE) == CSTR_EQUAL;
}

std::wstring windowText(HWND h)
{
    std::wstring s(static_cast<size_t>(GetWindowTextLengthW(h)) + 1, L'\0');
    s.resize(static_cast<size_t>(GetWindowTextW(h, s.data(), static_cast<int>(s.size()))));
    return s;
}

SIZE imageSize(HWND button)
{
    SIZE sz{};
    if (auto bmp = reinterpret_cast<HBITMAP>(SendMessageW(button, BM_GETIMAGE, IMAGE_BITMAP, 0))) {
        BITMAP bm{};
        if (GetObjectW(bmp, sizeof bm, &bm)) sz = {bm.bmWidth, bm.bmHeight};
    } else if (auto icon = reinterpret_cast<HICON>(SendMessageW(button, BM_GETIMAGE, IMAGE_ICON, 0))) {
        ICONINFO ii{};
        if (GetIconInfo(icon, &ii)) {
            BITMAP bm{};
            if (GetObjectW(ii.hbmColor ? ii.hbmColor : ii.hbmMask, sizeof bm, &bm))
                sz = {bm.bmWidth, ii.hbmColor ? bm.bmHeight : bm.bmHeight / 2};
            if (ii.hbmColor) DeleteObject(ii.hbmColor);
            if (ii.hbmMask) DeleteObject(ii.hbmMask);
        }
    }
    return sz;
}

// The text's extent in `ctl`'s font: one line, or wrapped at `wrapWidth` (> 0).
RECT measure(HWND ctl, const std::wstring& text, UINT format, int wrapWidth)
{
    RECT rc{0, 0, wrapWidth > 0 ? wrapWidth : 0, 0};
    HDC dc = GetDC(ctl);
    HFONT font = reinterpret_cast<HFONT>(SendMessageW(ctl, WM_GETFONT, 0, 0));
    if (!font) font = reinterpret_cast<HFONT>(SendMessageW(GetParent(ctl), WM_GETFONT, 0, 0));
    const HGDIOBJ old = font ? SelectObject(dc, font) : nullptr;
    DrawTextW(dc, text.c_str(), -1, &rc,
              DT_CALCRECT | DT_EXPANDTABS | format | (wrapWidth > 0 ? DT_WORDBREAK : DT_SINGLELINE));
    if (old) SelectObject(dc, old);
    ReleaseDC(ctl, dc);
    return rc;
}

std::vector<FitProblem> checkTextFit(HWND dlg)
{
    std::vector<FitProblem> problems;
    for (HWND ctl = GetWindow(dlg, GW_CHILD); ctl; ctl = GetWindow(ctl, GW_HWNDNEXT)) {
        const std::wstring text = windowText(ctl);
        if (text.empty()) continue;
        wchar_t cls[32] = {};
        GetClassNameW(ctl, cls, 32);
        const LONG_PTR style = GetWindowLongPtrW(ctl, GWL_STYLE);
        const int id = GetDlgCtrlID(ctl);
        const int dpi = static_cast<int>(GetDpiForWindow(ctl));
        auto px = [dpi](int v) { return MulDiv(v, dpi, 96); };
        RECT client{};
        GetClientRect(ctl, &client);
        const int width = client.right, height = client.bottom;

        if (same(cls, L"Static")) {
            const LONG_PTR type = style & SS_TYPEMASK;
            if (type != SS_LEFT && type != SS_CENTER && type != SS_RIGHT && type != SS_LEFTNOWORDWRAP &&
                type != SS_SIMPLE)
                continue;   // pictures, frames, owner-drawn
            const UINT prefix = (style & SS_NOPREFIX) ? DT_NOPREFIX : 0;
            if (type == SS_LEFTNOWORDWRAP || type == SS_SIMPLE) {
                const RECT r = measure(ctl, text, prefix, 0);
                if (r.right > width) problems.push_back({id, text, r.right, width, false});
            } else {
                const RECT r = measure(ctl, text, prefix, width);
                if (r.right > width) problems.push_back({id, text, r.right, width, false});   // a word too long
                else if (r.bottom > height) problems.push_back({id, text, r.bottom, height, true});
            }
            continue;
        }
        if (!same(cls, L"Button") && !same(cls, L"tbSuperBtn")) continue;
        const LONG_PTR type = style & BS_TYPEMASK;
        int room = width;
        switch (type) {
        case BS_CHECKBOX: case BS_AUTOCHECKBOX: case BS_3STATE: case BS_AUTO3STATE:
        case BS_RADIOBUTTON: case BS_AUTORADIOBUTTON:
            room = width - px(16);
            break;
        case BS_GROUPBOX:
            room = width - px(16);
            break;
        case BS_PUSHBUTTON: case BS_DEFPUSHBUTTON: {
            if (style & (BS_BITMAP | BS_ICON)) continue;   // picture only
            if (text == L"tbSuperBtn") continue;
            const SIZE image = imageSize(ctl);
            room = width - 2 * px(4) - (image.cx ? image.cx + px(4) : 0);
            break;
        }
        default:
            continue;
        }
        if (style & BS_MULTILINE) {
            const RECT r = measure(ctl, text, 0, (std::max)(room, 1));
            if (r.bottom > height) problems.push_back({id, text, r.bottom, height, true});
        } else {
            const RECT r = measure(ctl, text, 0, 0);
            if (r.right > room) problems.push_back({id, text, r.right, room, false});
        }
    }
    return problems;
}

void checkDialogs(HINSTANCE inst, Report& r)
{
    // A hidden owner/parent: 1032 and 2057 are WS_CHILD templates.
    HWND parent = CreateWindowExW(0, L"STATIC", L"resource check", WS_POPUP, 0, 0, 400, 300, nullptr,
                                  nullptr, inst, nullptr);
    for (const DialogCase& d : kDialogs) {
        const std::wstring name = fx::Format(L"dialog %d, %s", d.id, d.what);
        const bool present = FindResourceW(inst, MAKEINTRESOURCEW(d.id), MAKEINTRESOURCEW(5)) != nullptr;  // RT_DIALOG
        if (!present) {
            r.check(false, name + L": template missing");
            continue;
        }
        HWND dlg = CreateDialogParamW(inst, MAKEINTRESOURCEW(d.id), parent, ProbeProc, 0);
        r.check(dlg != nullptr, name);
        if (dlg && d.hasGrid) r.check(fx::Grid(GetDlgItem(dlg, 1075)).IsGrid(), name + L": tbSpread grid (ID_DISTLIST)");
        if (dlg && !isChildTemplate(dlg)) {
            applyPictures(dlg, d);
            const auto problems = checkTextFit(dlg);
            for (const FitProblem& p : problems)
                r.check(false, fx::Format(p.lines ? L"dialog %d, control %d \"%s\": text wraps to %d px high, room %d px"
                                                  : L"dialog %d, control %d \"%s\": text too wide: %d px, room %d px",
                                          d.id, p.id, p.text.c_str(), p.need, p.room));
            if (problems.empty()) r.check(true, fx::Format(L"dialog %d: every caption fits", d.id));
        }
        if (dlg) DestroyWindow(dlg);
    }
    if (parent) DestroyWindow(parent);
}

void checkMenus(HINSTANCE inst, Report& r)
{
    for (const wchar_t* name : {L"MANILA", L"GLE", L"GLSUB"}) {
        fx::MenuGuard menu(LoadMenuW(inst, name));
        r.check(static_cast<bool>(menu), fx::Format(L"menu %s", name));
        if (menu && wcscmp(name, L"MANILA") == 0) {
            wchar_t text[32] = {};
            GetMenuStringW(menu.get(), 7, text, 32, MF_BYPOSITION);
            r.check(std::wstring(text) == L"&Window", L"MANILA: Window menu at position 7");
        }
        // Spec #3 D9: Custom Report has its own command; File > Print keeps 3068.
        if (menu && wcscmp(name, L"GLE") != 0) {
            wchar_t text[32] = {};
            GetMenuStringW(menu.get(), CM_REPORT_LIST, text, 32, MF_BYCOMMAND);
            r.check(std::wstring(text) == L"&Custom Report...", fx::Format(L"%s: Custom Report is CM_REPORT_LIST", name));
        }
    }
}

BOOL CALLBACK collectName(HMODULE, LPCWSTR, LPWSTR name, LONG_PTR param)
{
    auto* names = reinterpret_cast<std::vector<std::wstring>*>(param);
    names->push_back(IS_INTRESOURCE(name) ? fx::Format(L"#%u", static_cast<unsigned>(reinterpret_cast<ULONG_PTR>(name)))
                                          : std::wstring(name));
    return TRUE;
}

LPCWSTR resName(const std::wstring& n)
{
    return n[0] == L'#' ? MAKEINTRESOURCEW(std::stoi(n.substr(1))) : n.c_str();
}

void checkImages(HINSTANCE inst, Report& r)
{
    std::vector<std::wstring> bitmaps;
    EnumResourceNamesW(inst, MAKEINTRESOURCEW(2), collectName, reinterpret_cast<LONG_PTR>(&bitmaps));  // RT_BITMAP
    int loaded = 0;
    for (const auto& n : bitmaps) {
        fx::BitmapGuard bmp(static_cast<HBITMAP>(LoadImageW(inst, resName(n), IMAGE_BITMAP, 0, 0, 0)));
        if (bmp) {
            ++loaded;
        } else {
            r.check(false, L"bitmap " + n);
        }
    }
    r.check(bitmaps.size() == 83 && loaded == 83, fx::Format(L"%d of %d bitmaps load (83 expected)", loaded,
                                                              static_cast<int>(bitmaps.size())));
    for (const wchar_t* n : {L"STATUSBAR", L"PLUS", L"MINUS"}) {
        fx::BitmapGuard bmp(static_cast<HBITMAP>(LoadImageW(inst, n, IMAGE_BITMAP, 0, 0, 0)));
        r.check(static_cast<bool>(bmp), fx::Format(L"bitmap %s", n));
    }
    for (const wchar_t* n : {L"GL", L"SHEET"}) {
        r.check(LoadIconW(inst, n) != nullptr, fx::Format(L"icon %s", n));
    }
    r.check(LoadIconW(inst, MAKEINTRESOURCEW(1111)) != nullptr, L"icon 1111");

    // UI refresh (spec 4.2): every P<resource> icon (icons/fluent/map.csv) loads
    // at 16 and 48 px, and replaces a bitmap that exists.
    std::vector<std::wstring> icons;
    EnumResourceNamesW(inst, MAKEINTRESOURCEW(14), collectName, reinterpret_cast<LONG_PTR>(&icons));   // RT_GROUP_ICON
    int modern = 0;
    for (const auto& n : icons) {
        if (n.size() < 2 || n[0] != L'P' || n[1] == L'#') continue;   // e.g. "PPLUS", "P4055"; not GL, SHEET, #1111
        ++modern;
        const std::wstring bitmap = n.substr(1);
        const bool numeric = bitmap.find_first_not_of(L"0123456789") == std::wstring::npos;
        const LPCWSTR res = numeric ? MAKEINTRESOURCEW(std::stoi(bitmap)) : bitmap.c_str();
        HICON icon16 = fx::LoadModernIcon(inst, res, 16), icon48 = fx::LoadModernIcon(inst, res, 48);
        const bool hasBitmap = FindResourceW(inst, res, MAKEINTRESOURCEW(2)) != nullptr;   // RT_BITMAP
        r.check(icon16 && icon48 && hasBitmap, L"icon " + n + (hasBitmap ? L"" : L": no bitmap " + bitmap));
        if (icon16) DestroyIcon(icon16);
        if (icon48) DestroyIcon(icon48);
    }
    r.check(modern == 31, fx::Format(L"%d modern icons (31 expected: icons/fluent/map.csv)", modern));
    r.check(LoadAcceleratorsW(inst, L"GL") != nullptr, L"accelerators GL");
}

void checkStrings(HINSTANCE inst, Report& r)
{
    r.check(fx::LoadResString(inst, 4024) == L"MM/DD/YY", L"string 4024 = MM/DD/YY (tbDate display format)");
    r.check(fx::LoadResString(inst, 1021) == L"Are you sure you wish to delete this account?", L"string 1021");
    r.check(!fx::LoadResString(inst, 8095).empty(), L"string 8095");
}

} // namespace

int RunResourceCheck(HINSTANCE inst, bool quiet)
{
    Report r;
    checkDialogs(inst, r);
    checkMenus(inst, r);
    checkImages(inst, r);
    checkStrings(inst, r);
    if (!quiet) {
        r.log += r.failures ? fx::Format(L"\n%d check(s) FAILED", r.failures) : std::wstring(L"\nAll checks passed");
        MessageBoxW(nullptr, r.log.c_str(), L"gl.exe resource check", r.failures ? MB_ICONERROR : MB_ICONINFORMATION);
    }
    return r.failures;
}

// ---- screenshots (UI refresh spec 4.6) ---------------------------------------

namespace {

template <class T>
struct Com {
    T* p = nullptr;
    Com() = default;
    Com(const Com&) = delete;
    Com& operator=(const Com&) = delete;
    ~Com()
    {
        if (p) p->Release();
    }
    T* operator->() const noexcept { return p; }
};

bool savePng(HBITMAP bmp, int w, int h, const std::wstring& path)
{
    Com<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
                                reinterpret_cast<void**>(&factory.p))))
        return false;
    Com<IWICBitmap> source;
    Com<IWICStream> stream;
    Com<IWICBitmapEncoder> encoder;
    Com<IWICBitmapFrameEncode> frame;
    WICPixelFormatGUID format = GUID_WICPixelFormat24bppBGR;
    return SUCCEEDED(factory->CreateBitmapFromHBITMAP(bmp, nullptr, WICBitmapIgnoreAlpha, &source.p)) &&
           SUCCEEDED(factory->CreateStream(&stream.p)) &&
           SUCCEEDED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE)) &&
           SUCCEEDED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder.p)) &&
           SUCCEEDED(encoder->Initialize(stream.p, WICBitmapEncoderNoCache)) &&
           SUCCEEDED(encoder->CreateNewFrame(&frame.p, nullptr)) && SUCCEEDED(frame->Initialize(nullptr)) &&
           SUCCEEDED(frame->SetSize(static_cast<UINT>(w), static_cast<UINT>(h))) &&
           SUCCEEDED(frame->SetPixelFormat(&format)) && SUCCEEDED(frame->WriteSource(source.p, nullptr)) &&
           SUCCEEDED(frame->Commit()) && SUCCEEDED(encoder->Commit());
}

HBITMAP createDib(HDC dc, int w, int h)
{
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;   // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    return CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
}

// Paints `wnd` and its children now and waits for the compositor, so that
// PrintWindow(PW_RENDERFULLCONTENT) sees the finished window.
void settle(HWND wnd)
{
    RedrawWindow(wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
    MSG msg;
    while (PeekMessageW(&msg, nullptr, WM_PAINT, WM_PAINT, PM_REMOVE)) DispatchMessageW(&msg);   // nothing else:
    DwmFlush();                                                                                   // no posted commands
}

} // namespace

bool SaveWindowPng(HWND wnd, const std::wstring& path)
{
    settle(wnd);
    RECT win{};
    GetWindowRect(wnd, &win);
    // The visible frame: GetWindowRect includes the invisible resize borders.
    RECT vis = win;
    if (FAILED(DwmGetWindowAttribute(wnd, DWMWA_EXTENDED_FRAME_BOUNDS, &vis, sizeof vis))) vis = win;
    const int w = win.right - win.left, h = win.bottom - win.top;
    const int vw = vis.right - vis.left, vh = vis.bottom - vis.top;
    if (w <= 0 || h <= 0 || vw <= 0 || vh <= 0) return false;

    HDC screen = GetDC(nullptr);
    HDC whole = CreateCompatibleDC(screen);
    HDC cropped = CreateCompatibleDC(screen);
    HBITMAP wholeBmp = createDib(screen, w, h);
    HBITMAP croppedBmp = createDib(screen, vw, vh);
    bool ok = false;
    if (whole && cropped && wholeBmp && croppedBmp) {
        const HGDIOBJ old1 = SelectObject(whole, wholeBmp);
        const HGDIOBJ old2 = SelectObject(cropped, croppedBmp);
        ok = PrintWindow(wnd, whole, PW_RENDERFULLCONTENT) &&
             BitBlt(cropped, 0, 0, vw, vh, whole, vis.left - win.left, vis.top - win.top, SRCCOPY);
        SelectObject(whole, old1);
        SelectObject(cropped, old2);
        ok = ok && savePng(croppedBmp, vw, vh, path);
    }
    if (wholeBmp) DeleteObject(wholeBmp);
    if (croppedBmp) DeleteObject(croppedBmp);
    if (whole) DeleteDC(whole);
    if (cropped) DeleteDC(cropped);
    ReleaseDC(nullptr, screen);
    return ok;
}

int SaveScreenshots(HINSTANCE inst, HWND frame, const std::wstring& dir, bool quiet)
{
    const HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    const UINT dpi = GetDpiForWindow(frame);
    auto file = [&](const std::wstring& name) {
        return (std::filesystem::path(dir) / fx::Format(L"%s-%u.png", name.c_str(), dpi)).wstring();
    };

    Report r;
    r.check(SaveWindowPng(frame, file(L"frame")), L"frame");
    for (const DialogCase& d : kDialogs) {
        HWND dlg = CreateDialogParamW(inst, MAKEINTRESOURCEW(d.id), frame, ProbeProc, 0);
        if (!dlg) {
            r.check(false, fx::Format(L"dialog %d: not created", d.id));
            continue;
        }
        if (isChildTemplate(dlg)) {   // 1032, 2057: in frame.png
            DestroyWindow(dlg);
            continue;
        }
        applyPictures(dlg, d);
        ShowWindow(dlg, SW_SHOWNOACTIVATE);
        r.check(SaveWindowPng(dlg, file(std::to_wstring(d.id))), fx::Format(L"dialog %d, %s", d.id, d.what));
        DestroyWindow(dlg);
    }
    if (SUCCEEDED(init)) CoUninitialize();

    if (!quiet) {
        r.log += fx::Format(L"\nSaved to %s, at %u DPI (%u%%).", dir.c_str(), dpi, dpi * 100 / 96);
        if (r.failures) r.log += fx::Format(L"\n%d screenshot(s) FAILED", r.failures);
        MessageBoxW(frame, r.log.c_str(), L"gl.exe screenshots", r.failures ? MB_ICONERROR : MB_ICONINFORMATION);
    }
    return r.failures;
}

