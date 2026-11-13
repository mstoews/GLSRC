//
// VoucherLists.cpp - see VoucherLists.h.
//
// Translation notes (plan per-port checklist):
//   - TGLDistList's tbSpread is fx::Grid. Amounts are shown with two
//     decimals and thousands separators (SSSetTypeFloat FS_SEPARATOR); the
//     original wrote the padded field text into float cells, which the grid
//     formatted.
//   - TAPDistList::SetPosition/SavePosition used GL.INI [GL_DISTLIST]; the
//     port does too. Without a saved position it opens below the dialog.
//   - Closing the line list hides it, as the other lists.
//   - TGLToolBox drew its own small caption (WMPaint/WMNchittest); the port
//     is a WS_EX_TOOLWINDOW, whose caption is already small. It has no close
//     box: the dialog's Show/Hide check box controls it, as before.
//
#include "VoucherLists.h"

#include "Globals.h"
#include "Ini.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr wchar_t kIni[] = L"GL.INI";
constexpr wchar_t kLineClass[] = L"GL_DISTLIST";   // TGLDistList::GetClassName and its GL.INI section
constexpr wchar_t kToolClass[] = L"GLToolBox";
constexpr int kListId = ID_DISTLIST;

bool registerClass(const wchar_t* name)
{
    WNDCLASSEXW existing{};
    existing.cbSize = sizeof existing;
    if (GetClassInfoExW(hInst, name, &existing)) return true;
    WNDCLASSEXW wc = fx::Window::ClassInfo(hInst, name);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    return RegisterClassExW(&wc) != 0;
}

int iniInt(const wchar_t* key)
{
    return static_cast<int>(GetPrivateProfileIntW(kLineClass, key, 0, IniPath(kIni).c_str()));
}

constexpr COLORREF kWhite = RGB(255, 255, 255), kBlack = RGB(0, 0, 0), kRed = RGB(255, 0, 0);
constexpr COLORREF kPickBack = RGB(192, 220, 192), kBlue = RGB(0, 0, 255);

} // namespace

std::wstring FormatAmount(double v, bool blankZero)
{
    const long long c = std::llround(v * 100.0);
    if (c == 0 && blankZero) return {};
    const unsigned long long a = static_cast<unsigned long long>(c < 0 ? -c : c);
    std::wstring whole = std::to_wstring(a / 100);
    for (int i = static_cast<int>(whole.size()) - 3; i > 0; i -= 3) whole.insert(static_cast<size_t>(i), L",");
    wchar_t frac[4];
    std::swprintf(frac, 4, L"%02llu", a % 100);
    return (c < 0 ? L"-" : L"") + whole + L"." + frac;
}

// ---- LineList -------------------------------------------------------------

LineList::~LineList()
{
    if (hwnd()) DestroyWindow(hwnd());
}

bool LineList::Create(HWND owner, std::function<void(int line)> onPick)
{
    owner_ = owner;
    onPick_ = std::move(onPick);
    if (!registerClass(kLineClass)) return false;
    RECT dlg{};
    GetWindowRect(owner, &dlg);
    int left = iniInt(L"LEFT"), top = iniInt(L"TOP"), right = iniInt(L"RIGHT"), bottom = iniInt(L"BOTTOM");
    if (left == 0 || top == 0 || right == 0 || bottom == 0) {
        left = dlg.left;
        top = dlg.bottom;
        right = dlg.right;
        bottom = top + 200;
    }
    if (!fx::Window::Create(WS_EX_TOOLWINDOW, kLineClass, fx::LoadResString(hInst, 8156).c_str(),   // "Distribution List"
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN, left, top,
                            right - left, bottom - top, owner, nullptr, hInst))
        return false;
    ShowWindow(hwnd(), SW_SHOWNOACTIVATE);
    return true;
}

bool LineList::OnCreate(LPCREATESTRUCTW)
{
    grid_ = fx::Grid(fx::Grid::Create(hwnd(), kListId, 0, 0, 0, 0));
    if (!grid_.IsGrid()) return false;
    // TGLDistList::SetupWindow: widths 3 / (rest) / 6 / 4 / 13 / 13 characters.
    const int ch = grid_.CharWidth();
    auto text = [](UINT id) { return fx::LoadResString(hInst, id); };
    grid_.SetColumns({{text(6067), 4 * ch},                                  // "Ln"
                      {text(6044), 30 * ch},                                 // "Description"
                      {text(6068), 8 * ch},                                  // "Acct"
                      {text(6054), 5 * ch},                                  // "Dept"
                      {text(6069), 14 * ch, fx::GridAlign::Right},           // "Debit"
                      {text(6070), 14 * ch, fx::GridAlign::Right}});         // "Credit"
    grid_.SetStretchColumn(2);
    return false;
}

bool LineList::OnSize(UINT, int cx, int cy)
{
    if (!grid_.hwnd()) return false;
    MoveWindow(grid_.hwnd(), 0, 0, cx, cy, TRUE);
    return true;
}

bool LineList::OnDestroy()
{
    SavePosition();
    grid_ = fx::Grid();
    return false;
}

bool LineList::OnCommand(WORD id, WORD code, HWND)
{
    if (id != kListId || code != GRN_PICK) return false;
    const int row = grid_.ActiveRow();
    const int line = row >= 1 && row <= static_cast<int>(lineOfRow_.size()) ? lineOfRow_[static_cast<size_t>(row - 1)] : 0;
    if (onPick_) onPick_(line);   // TGLDistList::SelectCell
    return true;
}

bool LineList::OnMessage(UINT msg, WPARAM, LPARAM, LRESULT& result)
{
    switch (msg) {
    case WM_CLOSE:
        ShowWindow(hwnd(), SW_HIDE);
        if (owner_) SetFocus(owner_);
        result = 0;
        return true;
    case WM_SETFOCUS:
        if (grid_.list()) SetFocus(grid_.list());
        result = 0;
        return true;
    default:
        return false;
    }
}

void LineList::Clear()
{
    lineOfRow_.clear();
    if (!grid_.IsGrid()) return;
    grid_.Clear();
}

void LineList::Show(const std::vector<VoucherLine>& lines, double debit, double credit)
{
    if (!grid_.IsGrid()) return;
    grid_.SetRedraw(false);
    grid_.Clear();
    grid_.SetColor(fx::Grid::kAll, fx::Grid::kAll, kWhite, kBlack);
    lineOfRow_.clear();
    int row = 0;
    for (const VoucherLine& l : lines) {
        ++row;
        wchar_t ln[8];
        std::swprintf(ln, 8, L"%3d", l.line);   // iJournal.assignLong(i, 3)
        grid_.SetData(1, row, ln);
        grid_.SetData(2, row, fx::Utf8ToWide(l.descript));
        grid_.SetData(3, row, fx::Utf8ToWide(l.account));
        grid_.SetData(4, row, fx::Utf8ToWide(l.dept));
        grid_.SetData(5, row, FormatAmount(l.debit));
        grid_.SetData(6, row, FormatAmount(l.credit));
        lineOfRow_.push_back(l.line);
    }
    ++row;
    if (Balanced(debit, credit)) {
        grid_.SetData(2, row, L"Total");
    } else {
        grid_.SetData(2, row, L"Voucher Out Of Balance");
        grid_.SetColor(2, row, kWhite, kRed);   // RGBCOLOR_WHITE on RGBCOLOR_RED
    }
    grid_.SetData(5, row, FormatAmount(debit, false));
    grid_.SetData(6, row, FormatAmount(credit, false));
    grid_.SetColor(5, row, kPickBack, kBlue);
    grid_.SetColor(6, row, kPickBack, kBlue);
    grid_.SetRedraw(true);
}

void LineList::SavePosition() const
{
    RECT rc{};
    if (!hwnd() || !GetWindowRect(hwnd(), &rc)) return;
    WriteIniString(kIni, kLineClass, L"LEFT", std::to_wstring(rc.left).c_str());
    WriteIniString(kIni, kLineClass, L"TOP", std::to_wstring(rc.top).c_str());
    WriteIniString(kIni, kLineClass, L"RIGHT", std::to_wstring(rc.right).c_str());
    WriteIniString(kIni, kLineClass, L"BOTTOM", std::to_wstring(rc.bottom).c_str());
}

// ---- ToolBox --------------------------------------------------------------

namespace {

// TGLToolBox::SetupWindow / SetBitmap: two columns of 30 x 25 buttons, in
// this order (position 1 top left, 2 top right, 3 second row left, ...).
// 30 x 25 px at 96 DPI, scaled (UI refresh spec 5.1).
constexpr int kTools[] = {ID_TOP, ID_BOTTOM, ID_NEXT, ID_PREVIOUS, ID_NEW,
                          ID_SLIPLIST, IDOK, ID_DELETE, ID_VIEW, IDCANCEL};
constexpr int kToolW = 30, kToolH = 25;

} // namespace

ToolBox::~ToolBox()
{
    if (hwnd()) DestroyWindow(hwnd());
}

bool ToolBox::Create(HWND owner)
{
    owner_ = owner;
    if (!registerClass(kToolClass)) return false;
    RECT dlg{};
    GetWindowRect(owner, &dlg);
    const UINT dpi = GetDpiForWindow(owner);
    auto px = [dpi](int v) { return MulDiv(v, static_cast<int>(dpi), 96); };
    RECT rc{0, 0, 2 * px(kToolW), 5 * px(kToolH)};
    const DWORD style = WS_POPUP | WS_CAPTION | WS_CLIPCHILDREN;
    AdjustWindowRectExForDpi(&rc, style, FALSE, WS_EX_TOOLWINDOW, dpi);
    return fx::Window::Create(WS_EX_TOOLWINDOW, kToolClass, L"ToolBox", style, dlg.right - px(110),
                              dlg.top + px(20), rc.right - rc.left, rc.bottom - rc.top, owner, nullptr, hInst);
}

bool ToolBox::OnCreate(LPCREATESTRUCTW)
{
    for (int i = 0; i < static_cast<int>(sizeof kTools / sizeof kTools[0]); ++i) {
        // "tbSuperBtn" as the text: SuperBtn.cpp shows only the picture.
        HWND b = CreateWindowExW(0, L"tbSuperBtn", L"tbSuperBtn", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd(),
                                 reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTools[i])), hInst, nullptr);
        if (b)
            SendMessageW(b, SBM_SETPICT, SUPERBTN_PICT_BITMAP,
                         reinterpret_cast<LPARAM>(MAKEINTRESOURCEW(kTools[i])));
    }
    placeButtons();
    return false;
}

void ToolBox::placeButtons()
{
    const int dpi = static_cast<int>(GetDpiForWindow(hwnd()));
    const int w = MulDiv(kToolW, dpi, 96), h = MulDiv(kToolH, dpi, 96);
    for (int i = 0; i < static_cast<int>(sizeof kTools / sizeof kTools[0]); ++i)
        if (HWND b = GetDlgItem(hwnd(), kTools[i])) MoveWindow(b, (i % 2) * w, (i / 2) * h, w, h, TRUE);
}

bool ToolBox::OnCommand(WORD id, WORD code, HWND)
{
    if (code != BN_CLICKED || !owner_) return false;
    // TGLToolBox's handlers each sent the parent the same command.
    PostMessageW(owner_, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), 0);
    return true;
}

bool ToolBox::OnMessage(UINT msg, WPARAM, LPARAM lp, LRESULT& result)
{
    if (msg == WM_MOUSEACTIVATE) {   // clicking a tool leaves the dialog active
        result = MA_NOACTIVATE;
        return true;
    }
    if (msg == WM_DPICHANGED) {   // moved to a monitor with another scaling (UI refresh spec 5.2)
        const RECT* r = reinterpret_cast<const RECT*>(lp);
        SetWindowPos(hwnd(), nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        placeButtons();   // the buttons reload their icons themselves (WM_DPICHANGED_AFTERPARENT)
        result = 0;
        return true;
    }
    return false;
}
