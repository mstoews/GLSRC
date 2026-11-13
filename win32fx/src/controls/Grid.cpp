//
// Grid.cpp - tbSpread: a report-mode ListView in a container window. See
// controls/Grid.h for the interface and the SS* calls it replaces.
//
// The container (class "tbSpread") owns a GridState in its window extra
// bytes. The ListView child sends its WM_NOTIFY to the container, which
// answers custom draw (row and cell colours) and turns clicks and keys into
// GRN_* WM_COMMANDs to the container's parent.
//
// Keyboard: the ListView is subclassed so that, inside a dialog, Enter goes
// to the list (NM_RETURN, a pick) instead of the default button. The cell
// editor is a tbFloat child of the ListView, subclassed for Enter / Tab /
// Esc / Up / Down; losing the focus to anything else commits it, through a
// posted message so the editor isn't destroyed inside its own WM_KILLFOCUS.
//
// Font and DPI (UI refresh spec 5.1, 5.2): in a dialog the grid takes the
// dialog's font (WM_SETFONT). Elsewhere (no parent font) it makes its own,
// the system message font at its DPI. After a DPI change
// (WM_DPICHANGED_AFTERPARENT) it takes the parent's new font, or remakes its
// own. Whenever the font changes, the column widths (set by the screens in
// character widths) scale by the change in the width of "0".
//

#include "ControlsInternal.h"
#include "controls/Grid.h"

#include <CommCtrl.h>

#include <algorithm>
#include <map>
#include <memory>
#include <utility>

namespace win32fx::detail {

namespace {

constexpr wchar_t kClass[] = L"tbSpread";
constexpr UINT kListId = 1;
constexpr UINT kEditorId = 2;
constexpr UINT_PTR kListSubclass = 1;
constexpr UINT_PTR kEditorSubclass = 2;
constexpr UINT kEndEdit = WM_USER + 100;   // wParam: commit; lParam: the editor it was posted for

struct Colors {
    COLORREF back;
    COLORREF fore;
};

struct EditSpec {
    std::wstring mask;
    DWORD tbStyle = 0;
};

struct GridState {
    HWND self = nullptr;
    HWND list = nullptr;
    int stretchCol = 0;                              // 1-based; 0 none
    bool hasDefault = false;
    Colors defaults{};
    std::map<int, Colors> rowColors;                 // 0-based row
    std::map<std::pair<int, int>, Colors> cellColors;   // (row, col), 0-based
    std::map<int, EditSpec> editable;                // 0-based col
    HWND editor = nullptr;
    int editRow = -1, editCol = -1;                  // 0-based
    bool ending = false;
    int activeCol = 0;                               // 0-based
    int lastKey = 0;
    int changedRow = 0, changedCol = 0;              // 1-based
    HFONT ownFont = nullptr;                         // made by the grid when its parent has none
};

LRESULT CALLBACK GridProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);

GridState* stateOf(HWND h) noexcept
{
    if (!h || reinterpret_cast<WNDPROC>(GetClassLongPtrW(h, GCLP_WNDPROC)) != GridProc) return nullptr;
    return reinterpret_cast<GridState*>(GetWindowLongPtrW(h, 0));
}

void notify(GridState& st, WORD code)
{
    HWND parent = GetParent(st.self);
    if (parent)
        SendMessageW(parent, WM_COMMAND, MAKEWPARAM(static_cast<WORD>(GetDlgCtrlID(st.self)), code),
                     reinterpret_cast<LPARAM>(st.self));
}

int rowCount(const GridState& st) { return ListView_GetItemCount(st.list); }

int columnCount(const GridState& st)
{
    HWND header = ListView_GetHeader(st.list);
    return header ? Header_GetItemCount(header) : 0;
}

void addRows(GridState& st, int rows)
{
    for (int n = rowCount(st); n < rows; ++n) {
        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_IMAGE;
        item.iItem = n;
        item.pszText = const_cast<wchar_t*>(L"");
        item.iImage = I_IMAGENONE;
        ListView_InsertItem(st.list, &item);
    }
}

void stretch(GridState& st)
{
    const int col = st.stretchCol - 1;
    const int count = columnCount(st);
    if (col < 0 || col >= count) return;
    RECT rc{};
    GetClientRect(st.list, &rc);
    int used = 0;
    for (int c = 0; c < count; ++c)
        if (c != col) used += ListView_GetColumnWidth(st.list, c);
    const int rest = (rc.right - rc.left) - used;
    if (rest > 20) ListView_SetColumnWidth(st.list, col, rest);
}

int selectedRow(const GridState& st) { return ListView_GetNextItem(st.list, -1, LVNI_SELECTED); }

void selectRow(GridState& st, int row)   // 0-based; -1 clears
{
    ListView_SetItemState(st.list, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
    if (row < 0 || row >= rowCount(st)) return;
    ListView_SetItemState(st.list, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(st.list, row, FALSE);
}

std::wstring cellText(const GridState& st, int row, int col)   // 0-based
{
    wchar_t buf[512] = {};
    ListView_GetItemText(st.list, row, col, buf, 512);
    return buf;
}

void setCell(GridState& st, int row, int col, const std::wstring& text)   // 0-based, row exists
{
    ListView_SetItemText(st.list, row, col, const_cast<wchar_t*>(text.c_str()));
}

// ---- the cell editor --------------------------------------------------------

void endEdit(GridState& st, bool commit, bool refocus)
{
    if (!st.editor || st.ending) return;
    st.ending = true;
    HWND e = st.editor;
    const int row = st.editRow, col = st.editCol;
    bool changed = false;
    if (commit) {
        double v = 0;
        std::wstring text;
        if (FloatGetValue(e, &v)) {
            FloatSetValue(e, v);   // the mask's decimals and separators
            text = WindowText(e);
        }
        if (row < rowCount(st) && text != cellText(st, row, col)) {
            setCell(st, row, col, text);
            changed = true;
        }
    }
    st.editor = nullptr;
    st.editRow = st.editCol = -1;
    if (refocus) SetFocus(st.list);
    DestroyWindow(e);
    st.ending = false;
    if (changed) {
        st.changedRow = row + 1;
        st.changedCol = col + 1;
        notify(st, GRN_CELLCHANGED);
    }
}

bool beginEdit(GridState& st, int row, int col);   // 0-based

LRESULT CALLBACK EditorProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR ref)
{
    auto* st = reinterpret_cast<GridState*>(ref);
    switch (msg) {
    case WM_GETDLGCODE:
        return DefSubclassProc(h, msg, wp, lp) | DLGC_WANTALLKEYS;
    case WM_KEYDOWN:
        switch (wp) {
        case VK_RETURN:
        case VK_TAB:
            endEdit(*st, true, true);
            return 0;
        case VK_ESCAPE:
            endEdit(*st, false, true);
            return 0;
        case VK_UP:
        case VK_DOWN: {
            const int row = st->editRow + (wp == VK_UP ? -1 : 1), col = st->editCol;
            endEdit(*st, true, true);
            if (row >= 0 && row < rowCount(*st)) {
                selectRow(*st, row);
                beginEdit(*st, row, col);
            }
            return 0;
        }
        default:
            break;
        }
        break;
    case WM_CHAR:
        if (wp == L'\r' || wp == L'\t' || wp == 0x1B) return 0;   // handled on WM_KEYDOWN; no beep
        break;
    case WM_KILLFOCUS:
        if (!st->ending) PostMessageW(st->self, kEndEdit, TRUE, reinterpret_cast<LPARAM>(h));
        break;
    case WM_NCDESTROY:
        RemoveWindowSubclass(h, EditorProc, kEditorSubclass);
        break;
    default:
        break;
    }
    return DefSubclassProc(h, msg, wp, lp);
}

bool beginEdit(GridState& st, int row, int col)
{
    const auto spec = st.editable.find(col);
    if (spec == st.editable.end() || row < 0 || row >= rowCount(st)) return false;
    endEdit(st, true, false);
    ListView_EnsureVisible(st.list, row, FALSE);
    RECT rc{};
    if (!ListView_GetSubItemRect(st.list, row, col, col == 0 ? LVIR_LABEL : LVIR_BOUNDS, &rc)) return false;
    auto inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(st.self, GWLP_HINSTANCE));
    HWND e = CreateWindowExW(0, L"tbFloat", spec->second.mask.c_str(), WS_CHILD | spec->second.tbStyle, rc.left,
                             rc.top, rc.right - rc.left, rc.bottom - rc.top, st.list,
                             reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditorId)), inst, nullptr);
    if (!e) return false;
    SendMessageW(e, WM_SETFONT, SendMessageW(st.list, WM_GETFONT, 0, 0), FALSE);
    SetWindowTextW(e, cellText(st, row, col).c_str());
    SetWindowSubclass(e, EditorProc, kEditorSubclass, reinterpret_cast<DWORD_PTR>(&st));
    st.editor = e;
    st.editRow = row;
    st.editCol = col;
    st.activeCol = col;
    ShowWindow(e, SW_SHOW);
    SetFocus(e);
    SendMessageW(e, EM_SETSEL, 0, -1);
    return true;
}

// The first editable column, or the active one if it is editable.
int editColumn(const GridState& st)
{
    if (st.editable.count(st.activeCol)) return st.activeCol;
    return st.editable.empty() ? -1 : st.editable.begin()->first;
}

// ---- the list -----------------------------------------------------------------

LRESULT CALLBACK ListProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR)
{
    switch (msg) {
    case WM_GETDLGCODE: {
        LRESULT code = DefSubclassProc(h, msg, wp, lp);
        const auto* m = reinterpret_cast<const MSG*>(lp);
        if (m && m->message == WM_KEYDOWN && m->wParam == VK_RETURN) code |= DLGC_WANTMESSAGE;
        return code;
    }
    case WM_NCDESTROY:
        RemoveWindowSubclass(h, ListProc, kListSubclass);
        break;
    default:
        break;
    }
    return DefSubclassProc(h, msg, wp, lp);
}

LRESULT customDraw(const GridState& st, NMLVCUSTOMDRAW* cd)
{
    switch (cd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT:
        return CDRF_NOTIFYSUBITEMDRAW;
    case CDDS_ITEMPREPAINT | CDDS_SUBITEM: {
        const int row = static_cast<int>(cd->nmcd.dwItemSpec), col = cd->iSubItem;
        const Colors* c = nullptr;
        if (auto it = st.cellColors.find({row, col}); it != st.cellColors.end()) c = &it->second;
        else if (auto r = st.rowColors.find(row); r != st.rowColors.end()) c = &r->second;
        else if (st.hasDefault) c = &st.defaults;
        cd->clrText = c ? c->fore : CLR_DEFAULT;
        cd->clrTextBk = c ? c->back : CLR_DEFAULT;
        return CDRF_NEWFONT;
    }
    default:
        return CDRF_DODEFAULT;
    }
}

LRESULT onNotify(GridState& st, const NMHDR* nm)
{
    if (nm->hwndFrom != st.list) return 0;
    switch (nm->code) {
    case NM_CUSTOMDRAW:
        return customDraw(st, reinterpret_cast<NMLVCUSTOMDRAW*>(const_cast<NMHDR*>(nm)));
    case NM_CLICK: {
        const auto* ia = reinterpret_cast<const NMITEMACTIVATE*>(nm);
        if (ia->iItem >= 0) st.activeCol = std::max(0, ia->iSubItem);
        return 0;
    }
    case NM_DBLCLK: {
        const auto* ia = reinterpret_cast<const NMITEMACTIVATE*>(nm);
        if (ia->iItem < 0) return 0;
        st.activeCol = std::max(0, ia->iSubItem);
        selectRow(st, ia->iItem);
        if (!beginEdit(st, ia->iItem, st.activeCol)) notify(st, GRN_PICK);
        return 0;
    }
    case NM_RETURN: {
        const int row = selectedRow(st);
        if (row < 0) return 0;
        if (!beginEdit(st, row, editColumn(st))) notify(st, GRN_PICK);
        return 0;
    }
    case NM_RCLICK: {
        const auto* ia = reinterpret_cast<const NMITEMACTIVATE*>(nm);
        if (ia->iItem >= 0) {
            st.activeCol = std::max(0, ia->iSubItem);
            selectRow(st, ia->iItem);
        }
        notify(st, GRN_RCLICK);
        return 0;
    }
    case LVN_ITEMCHANGED: {
        const auto* lv = reinterpret_cast<const NMLISTVIEW*>(nm);
        if ((lv->uChanged & LVIF_STATE) && (lv->uNewState & LVIS_SELECTED) && !(lv->uOldState & LVIS_SELECTED))
            notify(st, GRN_SELCHANGE);
        return 0;
    }
    case LVN_KEYDOWN: {
        const auto* kd = reinterpret_cast<const NMLVKEYDOWN*>(nm);
        st.lastKey = kd->wVKey;
        if (kd->wVKey == VK_F2) {
            const int row = selectedRow(st);
            if (row >= 0) beginEdit(st, row, editColumn(st));
        }
        notify(st, GRN_KEYDOWN);
        return 0;
    }
    case LVN_BEGINSCROLL:
        endEdit(st, true, false);
        return 0;
    default:
        return 0;
    }
}

int charWidth(const GridState& st) { return st.list ? std::max(1, ListView_GetStringWidth(st.list, L"0")) : 8; }

// `font` for the list (and its header); the columns scale with it.
void applyFont(GridState& st, HFONT font, BOOL redraw)
{
    if (!st.list) return;
    const int before = charWidth(st);
    SendMessageW(st.list, WM_SETFONT, reinterpret_cast<WPARAM>(font), redraw);
    const int after = charWidth(st);
    if (after == before) return;
    for (int c = 0; c < columnCount(st); ++c)
        if (c + 1 != st.stretchCol)
            ListView_SetColumnWidth(st.list, c, MulDiv(ListView_GetColumnWidth(st.list, c), after, before));
    stretch(st);
}

// The grid's own font, for a parent without one: the message font at its DPI.
void useOwnFont(GridState& st, HWND h)
{
    NONCLIENTMETRICSW ncm{};
    ncm.cbSize = sizeof ncm;
    if (!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0, GetDpiForWindow(h))) return;
    HFONT old = st.ownFont;
    st.ownFont = CreateFontIndirectW(&ncm.lfMessageFont);
    applyFont(st, st.ownFont, TRUE);
    if (old) DeleteObject(old);
}

HFONT parentFont(HWND h) { return reinterpret_cast<HFONT>(SendMessageW(GetParent(h), WM_GETFONT, 0, 0)); }

LRESULT onCreate(GridState& st, HWND h)
{
    const DWORD style = static_cast<DWORD>(GetWindowLongPtrW(h, GWL_STYLE));
    auto inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(h, GWLP_HINSTANCE));
    RECT rc{};
    GetClientRect(h, &rc);
    st.list = CreateWindowExW(0, WC_LISTVIEWW, L"",
                              WS_CHILD | WS_VISIBLE | (style & WS_TABSTOP) | LVS_REPORT | LVS_SHOWSELALWAYS |
                                  LVS_SINGLESEL | LVS_SHAREIMAGELISTS,
                              0, 0, rc.right, rc.bottom, h, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kListId)),
                              inst, nullptr);
    if (!st.list) return -1;
    ListView_SetExtendedListViewStyle(st.list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER |
                                                   LVS_EX_SUBITEMIMAGES);
    SetWindowSubclass(st.list, ListProc, kListSubclass, 0);
    if (!parentFont(h)) useOwnFont(st, h);   // a dialog sends its font itself
    return 0;
}

LRESULT dispatch(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE) {
        auto st = std::make_unique<GridState>();
        st->self = h;
        SetWindowLongPtrW(h, 0, reinterpret_cast<LONG_PTR>(st.release()));
        // The dialog manager tabs into the list instead of stopping at the container.
        SetWindowLongPtrW(h, GWL_EXSTYLE, GetWindowLongPtrW(h, GWL_EXSTYLE) | WS_EX_CONTROLPARENT);
        return DefWindowProcW(h, msg, wp, lp);
    }
    auto* st = reinterpret_cast<GridState*>(GetWindowLongPtrW(h, 0));
    if (msg == WM_NCDESTROY) {
        SetWindowLongPtrW(h, 0, 0);
        if (st && st->ownFont) DeleteObject(st->ownFont);   // the list is gone by now
        delete st;
        return DefWindowProcW(h, msg, wp, lp);
    }
    if (!st) return DefWindowProcW(h, msg, wp, lp);

    switch (msg) {
    case WM_CREATE:
        return onCreate(*st, h);
    case WM_SIZE:
        if (st->list) {
            endEdit(*st, true, false);
            MoveWindow(st->list, 0, 0, LOWORD(lp), HIWORD(lp), TRUE);
            stretch(*st);
        }
        return 0;
    case WM_SETFONT:
        applyFont(*st, reinterpret_cast<HFONT>(wp), static_cast<BOOL>(lp));
        if (st->ownFont && reinterpret_cast<HFONT>(wp) != st->ownFont) {   // the parent's from now on
            DeleteObject(st->ownFont);
            st->ownFont = nullptr;
        }
        return 0;
    case WM_DPICHANGED_AFTERPARENT:
        if (HFONT pf = parentFont(h)) {
            if (st->list && pf != reinterpret_cast<HFONT>(SendMessageW(st->list, WM_GETFONT, 0, 0)))
                SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(pf), TRUE);
        } else {
            useOwnFont(*st, h);
        }
        return DefWindowProcW(h, msg, wp, lp);
    case WM_GETFONT:
        return st->list ? SendMessageW(st->list, WM_GETFONT, 0, 0) : 0;
    case WM_SETFOCUS:
        if (st->list) SetFocus(st->editor ? st->editor : st->list);
        return 0;
    case WM_ENABLE:
        if (st->list) EnableWindow(st->list, static_cast<BOOL>(wp));
        return 0;
    case WM_NOTIFY:
        return onNotify(*st, reinterpret_cast<const NMHDR*>(lp));
    case kEndEdit:
        if (reinterpret_cast<HWND>(lp) == st->editor) endEdit(*st, wp != 0, false);
        return 0;
    default:
        return DefWindowProcW(h, msg, wp, lp);
    }
}

LRESULT CALLBACK GridProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    try {
        return dispatch(h, msg, wp, lp);
    } catch (...) {   // std::bad_alloc only; nothing may reach user32
        return msg == WM_NCCREATE ? FALSE : (msg == WM_CREATE ? -1 : DefWindowProcW(h, msg, wp, lp));
    }
}

} // namespace

bool RegisterGrid(HINSTANCE inst)
{
    WNDCLASSEXW existing{};
    existing.cbSize = sizeof existing;
    if (GetClassInfoExW(inst, kClass, &existing)) return true;
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    wc.style = CS_GLOBALCLASS | CS_DBLCLKS;
    wc.lpfnWndProc = GridProc;
    wc.cbWndExtra = static_cast<int>(sizeof(LONG_PTR));
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClass;
    return RegisterClassExW(&wc) != 0;
}

} // namespace win32fx::detail

// ---- fx::Grid ---------------------------------------------------------------------

namespace win32fx {

using detail::GridState;

namespace {

GridState* gs(HWND h) noexcept { return detail::stateOf(h); }

bool inRange(int value, int count) { return value >= 1 && value <= count; }

} // namespace

HWND Grid::Create(HWND parent, int id, int x, int y, int cx, int cy, DWORD style)
{
    auto inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(parent, GWLP_HINSTANCE));
    return CreateWindowExW(0, L"tbSpread", L"", style | WS_CHILD, x, y, cx, cy, parent,
                           reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), inst, nullptr);
}

HWND Grid::list() const noexcept
{
    const GridState* st = gs(hwnd_);
    return st ? st->list : nullptr;
}

bool Grid::IsGrid() const noexcept { return gs(hwnd_) != nullptr && gs(hwnd_)->list != nullptr; }

void Grid::SetColumns(const std::vector<GridColumn>& columns)
{
    GridState* st = gs(hwnd_);
    if (!st || !st->list) return;
    while (detail::columnCount(*st) > 0) ListView_DeleteColumn(st->list, 0);
    for (size_t i = 0; i < columns.size(); ++i) {
        const GridColumn& c = columns[i];
        LVCOLUMNW col{};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
        col.fmt = c.align == GridAlign::Right ? LVCFMT_RIGHT : c.align == GridAlign::Center ? LVCFMT_CENTER
                                                                                           : LVCFMT_LEFT;
        col.cx = c.width;
        col.pszText = const_cast<wchar_t*>(c.header.c_str());
        col.iSubItem = static_cast<int>(i);
        ListView_InsertColumn(st->list, static_cast<int>(i), &col);
    }
    detail::stretch(*st);
}

int Grid::Columns() const
{
    const GridState* st = gs(hwnd_);
    return st && st->list ? detail::columnCount(*st) : 0;
}

void Grid::SetColumnWidth(int col, int pixels)
{
    GridState* st = gs(hwnd_);
    if (!st || !inRange(col, detail::columnCount(*st))) return;
    ListView_SetColumnWidth(st->list, col - 1, pixels);
    if (col != st->stretchCol) detail::stretch(*st);
}

int Grid::ColumnWidth(int col) const
{
    const GridState* st = gs(hwnd_);
    return st && inRange(col, detail::columnCount(*st)) ? ListView_GetColumnWidth(st->list, col - 1) : 0;
}

int Grid::CharWidth() const
{
    const GridState* st = gs(hwnd_);
    return st && st->list ? std::max(1, ListView_GetStringWidth(st->list, L"0")) : 8;
}

void Grid::SetStretchColumn(int col)
{
    GridState* st = gs(hwnd_);
    if (!st) return;
    st->stretchCol = col;
    detail::stretch(*st);
}

void Grid::SetMaxRows(int rows)
{
    GridState* st = gs(hwnd_);
    if (!st || !st->list || rows < 0) return;
    detail::endEdit(*st, true, false);
    detail::addRows(*st, rows);
    for (int n = detail::rowCount(*st); n > rows; --n) ListView_DeleteItem(st->list, n - 1);
    detail::stretch(*st);
}

int Grid::Rows() const
{
    const GridState* st = gs(hwnd_);
    return st && st->list ? detail::rowCount(*st) : 0;
}

void Grid::Clear()
{
    GridState* st = gs(hwnd_);
    if (!st || !st->list) return;
    detail::endEdit(*st, false, false);
    ListView_DeleteAllItems(st->list);
    st->rowColors.clear();
    st->cellColors.clear();
}

void Grid::SetData(int col, int row, const std::wstring& text)
{
    GridState* st = gs(hwnd_);
    if (!st || !inRange(col, detail::columnCount(*st)) || row < 1) return;
    detail::addRows(*st, row);
    detail::setCell(*st, row - 1, col - 1, text);
}

std::wstring Grid::GetData(int col, int row) const
{
    const GridState* st = gs(hwnd_);
    if (!st || !inRange(col, detail::columnCount(*st)) || !inRange(row, detail::rowCount(*st))) return {};
    return detail::cellText(*st, row - 1, col - 1);
}

void Grid::SetColor(int col, int row, COLORREF back, COLORREF fore)
{
    GridState* st = gs(hwnd_);
    if (!st || !st->list) return;
    const detail::Colors c{back, fore};
    if (col == kAll && row == kAll) {
        st->hasDefault = true;
        st->defaults = c;
        st->rowColors.clear();
        st->cellColors.clear();
    } else if (col == kAll) {
        if (row < 1) return;
        st->rowColors[row - 1] = c;
        for (auto it = st->cellColors.begin(); it != st->cellColors.end();)
            it = it->first.first == row - 1 ? st->cellColors.erase(it) : std::next(it);
    } else if (row == kAll) {
        for (int r = 0; r < detail::rowCount(*st); ++r) st->cellColors[{r, col - 1}] = c;
    } else if (col >= 1 && row >= 1) {
        st->cellColors[{row - 1, col - 1}] = c;
    }
    InvalidateRect(st->list, nullptr, FALSE);
}

void Grid::SetImageList(HIMAGELIST images)
{
    GridState* st = gs(hwnd_);
    if (st && st->list) ListView_SetImageList(st->list, images, LVSIL_SMALL);
}

void Grid::SetCellImage(int col, int row, int image)
{
    GridState* st = gs(hwnd_);
    if (!st || !inRange(col, detail::columnCount(*st)) || row < 1) return;
    detail::addRows(*st, row);
    LVITEMW item{};
    item.mask = LVIF_IMAGE;
    item.iItem = row - 1;
    item.iSubItem = col - 1;
    item.iImage = image < 0 ? I_IMAGENONE : image;
    ListView_SetItem(st->list, &item);
}

void Grid::SetRedraw(bool on)
{
    GridState* st = gs(hwnd_);
    if (!st || !st->list) return;
    SendMessageW(st->list, WM_SETREDRAW, on ? TRUE : FALSE, 0);
    if (on) {
        detail::stretch(*st);   // rows may have brought in the vertical scroll bar
        InvalidateRect(st->list, nullptr, TRUE);
    }
}

int Grid::ActiveRow() const
{
    const GridState* st = gs(hwnd_);
    return st && st->list ? detail::selectedRow(*st) + 1 : 0;
}

int Grid::ActiveCol() const
{
    const GridState* st = gs(hwnd_);
    return st ? st->activeCol + 1 : 0;
}

void Grid::SetActiveCell(int col, int row)
{
    GridState* st = gs(hwnd_);
    if (!st || !st->list) return;
    if (col >= 1) st->activeCol = col - 1;
    detail::selectRow(*st, row - 1);
}

void Grid::SetColumnEditable(int col, const std::wstring& floatMask, DWORD tbStyle)
{
    GridState* st = gs(hwnd_);
    if (!st || col < 1) return;
    st->editable[col - 1] = detail::EditSpec{floatMask, tbStyle & 0xFFFF};
}

bool Grid::BeginEdit(int col, int row)
{
    GridState* st = gs(hwnd_);
    if (!st || !st->list || col < 1 || row < 1) return false;
    detail::selectRow(*st, row - 1);
    return detail::beginEdit(*st, row - 1, col - 1);
}

void Grid::EndEdit(bool commit)
{
    GridState* st = gs(hwnd_);
    if (st) detail::endEdit(*st, commit, true);
}

bool Grid::IsEditing() const
{
    const GridState* st = gs(hwnd_);
    return st && st->editor != nullptr;
}

int Grid::LastKey() const
{
    const GridState* st = gs(hwnd_);
    return st ? st->lastKey : 0;
}

int Grid::LastChangedRow() const
{
    const GridState* st = gs(hwnd_);
    return st ? st->changedRow : 0;
}

int Grid::LastChangedCol() const
{
    const GridState* st = gs(hwnd_);
    return st ? st->changedCol : 0;
}

} // namespace win32fx
