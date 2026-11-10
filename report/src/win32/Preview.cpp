//
// Preview.cpp - see report/win32/Preview.h.
//
#include "report/win32/Preview.h"

#include "report/Layout.h"
#include "report/win32/Gdi.h"
#include "report/win32/Print.h"
#include "report/win32/PrinterSetup.h"

#include <windowsx.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace rpt::win32 {

namespace {

constexpr wchar_t kPreviewClass[] = L"rptPreview";
constexpr wchar_t kViewClass[] = L"rptPageView";

enum : WORD { IDB_PRINT = 101, IDB_FIRST, IDB_PREV, IDB_NEXT, IDB_LAST, IDB_ZOOM, IDB_OPTIONS, IDB_CLOSE };
constexpr int kButtons = 8;

enum class Zoom { FitWidth, FitPage, Actual };

const wchar_t* zoomText(Zoom z)
{
    switch (z) {
    case Zoom::FitWidth: return L"Fit width";
    case Zoom::FitPage:  return L"Fit page";
    default:             return L"100 %";
    }
}

int scaled(HWND h, int px96) { return MulDiv(px96, static_cast<int>(h ? GetDpiForWindow(h) : 96), 96); }

class PreviewWindow;

// The grey desk with the page on it; scrolls when the page is larger.
class PageView : public win32fx::Window {
public:
    explicit PageView(PreviewWindow& owner) : owner_(owner) {}

    HWND CreateView(HWND parent, HINSTANCE inst);
    void Reset();    // back to the top left, then Update
    void Update();   // scroll ranges and a repaint

protected:
    bool OnSize(UINT, int, int) override { Update(); return true; }
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    double scale() const;   // device pixels per twip
    SIZE content() const;   // the desk, in pixels
    void scrollTo(int x, int y);
    void onScroll(int bar, WORD code);
    void paint();

    PreviewWindow& owner_;
    int sx_ = 0, sy_ = 0;
    bool updating_ = false;
};

class PreviewWindow : public win32fx::MdiChild {
public:
    PreviewWindow(Report report, Rebuild rebuild) : report_(std::move(report)), rebuild_(std::move(rebuild)) {}

    bool selfOwned = false;   // set once the window exists: OnFinalMessage deletes

    const PageSetup& setup() const noexcept { return setup_; }
    const Page& page() const { return pages_[static_cast<size_t>(page_)]; }
    const FontSet& fonts() const noexcept { return fonts_; }
    Zoom zoom() const noexcept { return zoom_; }
    int pageIndex() const noexcept { return page_; }
    int pageCount() const noexcept { return static_cast<int>(pages_.size()); }

    void Go(int page);
    void Print();
    void CycleZoom();
    std::wstring title() const { return Widen(report_.title.empty() ? std::string("Report") : report_.title); }

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnSize(UINT type, int cx, int cy) override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnDestroy() override;
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;
    void OnFinalMessage() override
    {
        if (selfOwned) delete this;
    }

private:
    void Relayout();
    void UpdateStrip();
    void Options();

    Report report_;
    Rebuild rebuild_;
    std::vector<Page> pages_;
    PageSetup setup_;
    FontSet fonts_;
    int page_ = 0;
    Zoom zoom_ = Zoom::FitWidth;
    PageView view_{*this};
    HWND buttons_[kButtons] = {};
    HWND label_ = nullptr;
    HFONT uiFont_ = nullptr;
    int stripHeight_ = 0;
};

// ---- PageView --------------------------------------------------------------

HWND PageView::CreateView(HWND parent, HINSTANCE inst)
{
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc = ClassInfo(inst, kViewClass);
        wc.hbrBackground = nullptr;   // paint() covers everything
        registered = RegisterClassExW(&wc) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }
    return Create(0, kViewClass, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP, 0, 0, 0, 0,
                  parent, nullptr, inst);
}

double PageView::scale() const
{
    const PageSetup& s = owner_.setup();
    RECT rc{};
    GetClientRect(hwnd(), &rc);
    const int gap = scaled(hwnd(), 16);
    const double w = (std::max)(1L, rc.right - 2 * gap), h = (std::max)(1L, rc.bottom - 2 * gap);
    const double fitW = w / (std::max)(1, s.width), fitH = h / (std::max)(1, s.height);
    switch (owner_.zoom()) {
    case Zoom::FitWidth: return (std::max)(0.005, fitW);
    case Zoom::FitPage:  return (std::max)(0.005, (std::min)(fitW, fitH));
    default:             return static_cast<double>(GetDpiForWindow(hwnd())) / 1440.0;
    }
}

SIZE PageView::content() const
{
    const PageSetup& s = owner_.setup();
    const double k = scale();
    const int gap = scaled(hwnd(), 16);
    return {static_cast<LONG>(std::lround(s.width * k)) + 2 * gap, static_cast<LONG>(std::lround(s.height * k)) + 2 * gap};
}

void PageView::Reset()
{
    sx_ = sy_ = 0;
    Update();
}

void PageView::Update()
{
    if (!hwnd() || updating_) return;
    updating_ = true;
    // Twice: a scroll bar appearing changes the client size.
    for (int pass = 0; pass < 2; ++pass) {
        RECT rc{};
        GetClientRect(hwnd(), &rc);
        const SIZE c = content();
        SCROLLINFO si{};
        si.cbSize = sizeof si;
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        sx_ = std::clamp(sx_, 0, static_cast<int>((std::max)(0L, c.cx - rc.right)));
        sy_ = std::clamp(sy_, 0, static_cast<int>((std::max)(0L, c.cy - rc.bottom)));
        si.nMax = c.cx - 1;
        si.nPage = static_cast<UINT>(rc.right);
        si.nPos = sx_;
        SetScrollInfo(hwnd(), SB_HORZ, &si, TRUE);
        si.nMax = c.cy - 1;
        si.nPage = static_cast<UINT>(rc.bottom);
        si.nPos = sy_;
        SetScrollInfo(hwnd(), SB_VERT, &si, TRUE);
    }
    updating_ = false;
    InvalidateRect(hwnd(), nullptr, FALSE);
}

void PageView::scrollTo(int x, int y)
{
    sx_ = x;
    sy_ = y;
    Update();
}

void PageView::onScroll(int bar, WORD code)
{
    SCROLLINFO si{};
    si.cbSize = sizeof si;
    si.fMask = SIF_ALL;
    GetScrollInfo(hwnd(), bar, &si);
    const int line = scaled(hwnd(), 40);
    int pos = si.nPos;
    switch (code) {
    case SB_LINEUP:        pos -= line; break;
    case SB_LINEDOWN:      pos += line; break;
    case SB_PAGEUP:        pos -= static_cast<int>(si.nPage); break;
    case SB_PAGEDOWN:      pos += static_cast<int>(si.nPage); break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION: pos = si.nTrackPos; break;
    case SB_TOP:           pos = 0; break;
    case SB_BOTTOM:        pos = si.nMax; break;
    default:               return;
    }
    if (bar == SB_VERT) scrollTo(sx_, pos);
    else scrollTo(pos, sy_);
}

void PageView::paint()
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd(), &ps);
    RECT rc{};
    GetClientRect(hwnd(), &rc);
    if (rc.right > 0 && rc.bottom > 0) {
        HDC mem = CreateCompatibleDC(dc);
        HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
        HGDIOBJ oldBmp = SelectObject(mem, bmp);
        FillRect(mem, &rc, GetSysColorBrush(COLOR_APPWORKSPACE));

        const PageSetup& s = owner_.setup();
        const double k = scale();
        const int gap = scaled(hwnd(), 16);
        const int pw = static_cast<int>(std::lround(s.width * k)), ph = static_cast<int>(std::lround(s.height * k));
        const int left = (std::max)(gap, (static_cast<int>(rc.right) - pw) / 2) - sx_;
        const int top = gap - sy_;
        const int shadow = scaled(hwnd(), 3);
        RECT sh{left + shadow, top + shadow, left + pw + shadow, top + ph + shadow};
        FillRect(mem, &sh, GetSysColorBrush(COLOR_3DDKSHADOW));
        RECT paper{left, top, left + pw, top + ph};
        FillRect(mem, &paper, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

        const int saved = SaveDC(mem);
        IntersectClipRect(mem, paper.left, paper.top, paper.right, paper.bottom);
        SetTwipsMapping(mem, k * 1440.0, k * 1440.0, {left, top});
        DrawPage(mem, owner_.page(), owner_.fonts());
        RestoreDC(mem, saved);

        BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, oldBmp);
        DeleteObject(bmp);
        DeleteDC(mem);
    }
    EndPaint(hwnd(), &ps);
}

bool PageView::OnMessage(UINT msg, WPARAM wp, LPARAM, LRESULT& result)
{
    result = 0;
    switch (msg) {
    case WM_PAINT:
        paint();
        return true;
    case WM_ERASEBKGND:
        result = 1;
        return true;
    case WM_VSCROLL:
        onScroll(SB_VERT, LOWORD(wp));
        return true;
    case WM_HSCROLL:
        onScroll(SB_HORZ, LOWORD(wp));
        return true;
    case WM_MOUSEWHEEL: {
        const int delta = GET_WHEEL_DELTA_WPARAM(wp);
        RECT rc{};
        GetClientRect(hwnd(), &rc);
        if (content().cy > rc.bottom) {
            scrollTo(sx_, sy_ - delta * 3 * scaled(hwnd(), 40) / WHEEL_DELTA);
        } else if (delta < 0) {
            owner_.Go(owner_.pageIndex() + 1);
        } else if (delta > 0) {
            owner_.Go(owner_.pageIndex() - 1);
        }
        return true;
    }
    case WM_KEYDOWN: {
        const int line = scaled(hwnd(), 40);
        const bool ctrl = GetKeyState(VK_CONTROL) < 0;
        switch (wp) {
        case VK_NEXT:  owner_.Go(owner_.pageIndex() + 1); return true;
        case VK_PRIOR: owner_.Go(owner_.pageIndex() - 1); return true;
        case VK_HOME:  owner_.Go(0); return true;
        case VK_END:   owner_.Go(owner_.pageCount() - 1); return true;
        case VK_UP:    scrollTo(sx_, sy_ - line); return true;
        case VK_DOWN:  scrollTo(sx_, sy_ + line); return true;
        case VK_LEFT:  scrollTo(sx_ - line, sy_); return true;
        case VK_RIGHT: scrollTo(sx_ + line, sy_); return true;
        case 'P':
            if (ctrl) {
                owner_.Print();
                return true;
            }
            return false;
        default:
            return false;
        }
    }
    case WM_LBUTTONDOWN:
        SetFocus(hwnd());
        return true;
    default:
        return false;
    }
}

// ---- PreviewWindow ---------------------------------------------------------

bool PreviewWindow::OnCreate(LPCREATESTRUCTW cs)
{
    HINSTANCE inst = cs->hInstance;
    SetClassLongPtrW(hwnd(), GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(GetSysColorBrush(COLOR_BTNFACE)));
    const UINT dpi = GetDpiForWindow(hwnd());
    NONCLIENTMETRICSW ncm{};
    ncm.cbSize = sizeof ncm;
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0, dpi);
    uiFont_ = CreateFontIndirectW(&ncm.lfMessageFont);

    const wchar_t* text[kButtons] = {L"&Print...", L"|<", L"<", L">", L">|", zoomText(zoom_), L"&Options...", L"&Close"};
    for (int i = 0; i < kButtons; ++i) {
        DWORD style = WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON;
        if (i != IDB_OPTIONS - IDB_PRINT || rebuild_) style |= WS_VISIBLE;
        buttons_[i] = CreateWindowExW(0, L"BUTTON", text[i], style, 0, 0, 0, 0, hwnd(),
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDB_PRINT + i)), inst, nullptr);
        SendMessageW(buttons_[i], WM_SETFONT, reinterpret_cast<WPARAM>(uiFont_), FALSE);
    }
    label_ = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, 0, 0, hwnd(),
                             nullptr, inst, nullptr);
    SendMessageW(label_, WM_SETFONT, reinterpret_cast<WPARAM>(uiFont_), FALSE);
    // An exception from WM_CREATE fails CreateMDIWindowW (Window.h).
    if (!view_.CreateView(hwnd(), inst)) throw std::runtime_error("The preview's page view could not be created.");
    AddPrinterListener(hwnd());
    Relayout();
    return false;   // DefMDIChildProc too
}

bool PreviewWindow::OnSize(UINT, int cx, int cy)
{
    const HWND h = hwnd();
    stripHeight_ = scaled(h, 34);
    const int y = scaled(h, 4), bh = scaled(h, 26);
    int x = scaled(h, 6);
    auto place = [&](HWND ctl, int w96, int after96) {
        const int w = scaled(h, w96);
        MoveWindow(ctl, x, y, w, bh, TRUE);
        x += w + scaled(h, after96);
    };
    place(buttons_[0], 72, 10);      // Print...
    place(buttons_[1], 32, 2);       // |<
    place(buttons_[2], 32, 2);       // <
    place(label_, 96, 2);            // n of m
    place(buttons_[3], 32, 2);       // >
    place(buttons_[4], 32, 10);      // >|
    place(buttons_[5], 80, 10);      // zoom
    if (rebuild_) place(buttons_[6], 84, 10);   // Options...
    place(buttons_[7], 64, 0);       // Close
    MoveWindow(view_.hwnd(), 0, stripHeight_, cx, (std::max)(0, cy - stripHeight_), TRUE);
    return false;   // DefMDIChildProc keeps the MDI maximize state
}

bool PreviewWindow::OnCommand(WORD id, WORD code, HWND ctl)
{
    if (!ctl || code != BN_CLICKED) return false;
    switch (id) {
    case IDB_PRINT:   Print(); break;
    case IDB_FIRST:   Go(0); break;
    case IDB_PREV:    Go(page_ - 1); break;
    case IDB_NEXT:    Go(page_ + 1); break;
    case IDB_LAST:    Go(pageCount() - 1); break;
    case IDB_ZOOM:    CycleZoom(); break;
    case IDB_OPTIONS: Options(); break;
    case IDB_CLOSE:
        SendMessageW(GetParent(hwnd()), WM_MDIDESTROY, reinterpret_cast<WPARAM>(hwnd()), 0);
        return true;
    default:
        return false;
    }
    if (hwnd()) SetFocus(view_.hwnd());
    return true;
}

bool PreviewWindow::OnDestroy()
{
    RemovePrinterListener(hwnd());
    if (uiFont_) {
        DeleteObject(uiFont_);
        uiFont_ = nullptr;
    }
    return false;
}

bool PreviewWindow::OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result)
{
    MdiChild::OnMessage(msg, wp, lp, result);   // reports WM_MDIACTIVATE to the frame
    switch (msg) {
    case WM_SETFOCUS:
        if (view_.hwnd()) SetFocus(view_.hwnd());
        return false;
    case kPrinterChanged:
        Relayout();
        result = 0;
        return true;
    default:
        return false;
    }
}

void PreviewWindow::Relayout()
{
    GdiMeasure measure(CreatePrinterIC(report_.orientation));
    setup_ = CurrentPageSetup(report_.orientation);
    pages_ = Layout(report_, setup_, measure);
    if (pages_.empty()) pages_.emplace_back();
    page_ = std::clamp(page_, 0, pageCount() - 1);
    UpdateStrip();
    view_.Update();
}

void PreviewWindow::Go(int page)
{
    const int to = std::clamp(page, 0, pageCount() - 1);
    if (to == page_) return;
    page_ = to;
    UpdateStrip();
    view_.Reset();
}

void PreviewWindow::UpdateStrip()
{
    const std::wstring text = std::to_wstring(page_ + 1) + L" of " + std::to_wstring(pageCount());
    SetWindowTextW(label_, text.c_str());
    const bool notFirst = page_ > 0, notLast = page_ + 1 < pageCount();
    EnableWindow(buttons_[1], notFirst);
    EnableWindow(buttons_[2], notFirst);
    EnableWindow(buttons_[3], notLast);
    EnableWindow(buttons_[4], notLast);
}

void PreviewWindow::Print()
{
    HWND owner = frame() ? frame()->hwnd() : hwnd();
    PrintReport(owner, report_, pageCount());
}

void PreviewWindow::Options()
{
    if (!rebuild_) return;
    std::optional<Report> rebuilt = rebuild_(frame() ? frame()->hwnd() : hwnd());
    if (!rebuilt || !hwnd()) return;
    report_ = std::move(*rebuilt);
    SetWindowTextW(hwnd(), title().c_str());
    page_ = 0;
    Relayout();
    view_.Reset();
}

void PreviewWindow::CycleZoom()
{
    zoom_ = zoom_ == Zoom::FitWidth ? Zoom::FitPage : zoom_ == Zoom::FitPage ? Zoom::Actual : Zoom::FitWidth;
    SetWindowTextW(buttons_[5], zoomText(zoom_));
    view_.Reset();
}

PreviewWindow* previewFrom(HWND hwnd) noexcept
{
    return hwnd ? dynamic_cast<PreviewWindow*>(win32fx::Window::FromHandle(hwnd)) : nullptr;
}

} // namespace

HWND OpenPreview(win32fx::MdiFrame& frame, Report report, Rebuild rebuild)
{
    auto* w = new PreviewWindow(std::move(report), std::move(rebuild));
    HWND h = w->CreateChild(frame, kPreviewClass, w->title().c_str());
    if (!h) {
        delete w;
        return nullptr;
    }
    w->selfOwned = true;
    SetFocus(h);
    return h;
}

bool IsPreview(HWND hwnd) noexcept { return previewFrom(hwnd) != nullptr; }

bool PrintActivePreview(win32fx::MdiFrame& frame)
{
    PreviewWindow* p = previewFrom(frame.activeChild());
    if (!p) return false;
    p->Print();
    return true;
}

int PreviewPage(HWND hwnd) noexcept
{
    const PreviewWindow* p = previewFrom(hwnd);
    return p ? p->pageIndex() : -1;
}

int PreviewPageCount(HWND hwnd) noexcept
{
    const PreviewWindow* p = previewFrom(hwnd);
    return p ? p->pageCount() : -1;
}

bool CyclePreviewZoom(HWND hwnd)
{
    PreviewWindow* p = previewFrom(hwnd);
    if (!p) return false;
    p->CycleZoom();
    return true;
}

} // namespace rpt::win32
