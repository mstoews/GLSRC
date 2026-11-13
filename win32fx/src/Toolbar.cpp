//
// Toolbar.cpp - see Toolbar.h.
//

#include "Toolbar.h"
#include "controls/CustomControls.h"

#pragma comment(lib, "comctl32.lib")

namespace win32fx {

namespace {

// The bar's edge (UI refresh): kBorder px of white, then kGap px of the
// parent's background, the same on every side, so the buttons sit in the
// middle of the bar. Both at 96 DPI. The edge is the toolbar's non-client
// area, so the control lays its buttons out inside it unchanged.
constexpr int kBorder = 2;
constexpr int kGap = 2;
constexpr UINT_PTR kEdgeSubclass = 1;

int scaledTo(HWND h, int px96) { return MulDiv(px96, static_cast<int>(GetDpiForWindow(h)), 96); }

void paintEdge(HWND h)
{
    RECT wr{}, cr{};
    GetWindowRect(h, &wr);
    GetClientRect(h, &cr);
    MapWindowPoints(h, nullptr, reinterpret_cast<POINT*>(&cr), 2);
    OffsetRect(&cr, -wr.left, -wr.top);
    OffsetRect(&wr, -wr.left, -wr.top);
    HDC dc = GetWindowDC(h);
    if (!dc) return;
    ExcludeClipRect(dc, cr.left, cr.top, cr.right, cr.bottom);
    // The gap shows what the transparent toolbar shows: the parent's background.
    HBRUSH back = reinterpret_cast<HBRUSH>(GetClassLongPtrW(GetParent(h), GCLP_HBRBACKGROUND));
    FillRect(dc, &wr, back ? back : GetSysColorBrush(COLOR_APPWORKSPACE));
    const int b = scaledTo(h, kBorder);
    HBRUSH white = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    const RECT sides[] = {{wr.left, wr.top, wr.right, wr.top + b},
                          {wr.left, wr.bottom - b, wr.right, wr.bottom},
                          {wr.left, wr.top, wr.left + b, wr.bottom},
                          {wr.right - b, wr.top, wr.right, wr.bottom}};
    for (const RECT& r : sides) FillRect(dc, &r, white);
    ReleaseDC(h, dc);
}

LRESULT CALLBACK edgeProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR)
{
    switch (msg) {
    case WM_NCCALCSIZE:
        if (wp) {
            const int e = scaledTo(h, kBorder + kGap);
            RECT& r = reinterpret_cast<NCCALCSIZE_PARAMS*>(lp)->rgrc[0];
            InflateRect(&r, -e, -e);
            if (r.bottom < r.top) r.bottom = r.top;
            if (r.right < r.left) r.right = r.left;
            return 0;
        }
        break;
    case WM_NCPAINT:
        paintEdge(h);
        return 0;
    case WM_NCDESTROY:
        RemoveWindowSubclass(h, edgeProc, kEdgeSubclass);
        break;
    }
    return DefSubclassProc(h, msg, wp, lp);
}

} // namespace

CommandBar::~CommandBar()
{
    if (hwnd_ && IsWindow(hwnd_)) DestroyWindow(hwnd_);
    if (images_) ImageList_Destroy(images_);
}

HWND CommandBar::Create(HWND parent, UINT id, HINSTANCE inst, int iconPx)
{
    inst_ = inst;
    parent_ = parent;
    id_ = id;
    iconPx96_ = iconPx;
    // CCS_NORESIZE | CCS_NOPARENTALIGN: MdiFrame::Dock places and sizes it.
    hwnd_ = CreateWindowExW(0, TOOLBARCLASSNAMEW, nullptr,
                            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS |
                                CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN,
                            0, 0, 0, 0, parent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)), inst, nullptr);
    if (!hwnd_) return nullptr;
    SetWindowSubclass(hwnd_, edgeProc, kEdgeSubclass, 0);
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    SendMessageW(hwnd_, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    iconPx_ = MulDiv(iconPx, static_cast<int>(GetDpiForWindow(parent)), 96);
    if (images_) ImageList_Destroy(images_);
    images_ = ImageList_Create(iconPx_, iconPx_, ILC_COLOR32 | ILC_MASK, 16, 4);
    SendMessageW(hwnd_, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(images_));
    // No text under the icons: the strings show only as tooltips.
    SendMessageW(hwnd_, TB_SETMAXTEXTROWS, 0, 0);
    return hwnd_;
}

void CommandBar::AddButton(int picture, WORD command, const std::wstring& tip)
{
    buttons_.push_back({picture, command, tip});
    addButton(buttons_.back());
}

void CommandBar::AddSeparator()
{
    buttons_.push_back({0, 0, {}});
    addButton(buttons_.back());
}

void CommandBar::addButton(const Button& button)
{
    if (!hwnd_) return;
    TBBUTTON b{};
    if (button.picture == 0) {
        b.fsStyle = BTNS_SEP;
    } else {
        b.iBitmap = I_IMAGENONE;
        if (HICON icon = LoadModernIcon(inst_, MAKEINTRESOURCEW(button.picture), iconPx_)) {
            if (images_) b.iBitmap = ImageList_AddIcon(images_, icon);
            DestroyIcon(icon);
        }
        b.idCommand = button.command;
        b.fsState = TBSTATE_ENABLED;
        b.fsStyle = BTNS_BUTTON;
        b.iString = reinterpret_cast<INT_PTR>(button.tip.c_str());   // buttons_ is a list: it stays put
    }
    SendMessageW(hwnd_, TB_ADDBUTTONSW, 1, reinterpret_cast<LPARAM>(&b));
}

HWND CommandBar::Rebuild()
{
    if (hwnd_ && IsWindow(hwnd_)) DestroyWindow(hwnd_);
    hwnd_ = nullptr;
    if (!Create(parent_, id_, inst_, iconPx96_)) return nullptr;
    for (const Button& b : buttons_) addButton(b);
    return hwnd_;
}

int CommandBar::Height() const
{
    if (!hwnd_) return 0;
    // The buttons, with the edge above and below them (edgeProc).
    return HIWORD(SendMessageW(hwnd_, TB_GETBUTTONSIZE, 0, 0)) + 2 * scaledTo(hwnd_, kBorder + kGap);
}

} // namespace win32fx
