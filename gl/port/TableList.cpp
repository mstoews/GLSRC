//
// TableList.cpp - see TableList.h.
//
// Translation notes (plan per-port checklist):
//   - TListWindow is a tbSpread; the port's grid is fx::Grid (Spec #2 S2-M0).
//     The matched row is selected rather than painted green, and
//     TListWindow::GetKey's hand-built seek buffer (which leaked a malloc per
//     keystroke) is the ListView's type-ahead on the first column.
//   - Placement: GL.INI <section> LEFT/TOP/RIGHT/BOTTOM, saved when the
//     window is destroyed (TListWindow::CanClose -> SavePosition). Without a
//     saved position the original went to screen (0,0), 100 pixels high; the
//     port puts it beside the dialog, as tall as the dialog.
//   - Closing the window hides it. The original's IDCANCEL handler destroyed
//     it, after which the dialog's next Search used the deleted window.
//   - ~TListWindow called cb.closeAll(), closing the dialog's own tables
//     underneath it; the port closes only the list's own handle.
//   - TSetQuery (1035) is a plain dialog here: its IDCANCEL was
//     TMainDialog::Cancel, whose cb.closeAll() closed the dialog's tables.
//   - A QUERY that cb4sqlite can't run (outside the Spec #2 design 5.1
//     subset) is reported and the default is used instead.
//   - SetFields(table, tag, n) with more columns than the table has fields
//     (gldept has 2, SetFields asked for 3) showed an empty column; the port
//     shows only the columns the table has.
//
#include "TableList.h"

#include "Accounts.h"   // TrimText
#include "Globals.h"
#include "Ini.h"

#include <algorithm>
#include <exception>
#include <string>
#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr wchar_t kIni[] = L"GL.INI";
constexpr char kDefaultQuery[] = ".NOT. DELETED()";
constexpr int kListId = ID_DISTLIST;
constexpr int kQueryEdit = 101;

bool registerClass(const wchar_t* name)
{
    WNDCLASSEXW existing{};
    existing.cbSize = sizeof existing;
    if (GetClassInfoExW(hInst, name, &existing)) return true;
    WNDCLASSEXW wc = fx::Window::ClassInfo(hInst, name);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    return RegisterClassExW(&wc) != 0;
}

// TSetQuery: edit GL.INI <section> QUERY.
class SetQueryDialog : public fx::Dialog {
public:
    explicit SetQueryDialog(const wchar_t* section) : section_(section) {}

protected:
    bool OnInitDialog() override
    {
        SetDlgItemTextW(hwnd(), kQueryEdit, GetIniString(kIni, section_, L"QUERY").c_str());
        return true;
    }
    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != IDOK) return false;   // IDCANCEL: closes (fx::Dialog)
        WriteIniString(kIni, section_, L"QUERY", fx::GetDlgItemString(hwnd(), kQueryEdit).c_str());
        End(IDOK);
        return true;
    }

private:
    const wchar_t* section_;
};

} // namespace

TableList::~TableList()
{
    if (hwnd()) DestroyWindow(hwnd());
}

int TableList::IniInt(const wchar_t* key) const
{
    return static_cast<int>(GetPrivateProfileIntW(spec_.section, key, 0, IniPath(kIni).c_str()));
}

std::string TableList::CurrentQuery() const
{
    const std::string q = TrimText(fx::WideToUtf8(GetIniString(kIni, spec_.section, L"QUERY")));
    return q.empty() ? kDefaultQuery : q;
}

bool TableList::Create(HWND owner, Spec spec, std::function<void(int row)> onPick, bool hideOnPick, bool show)
{
    owner_ = owner;
    spec_ = std::move(spec);
    onPick_ = std::move(onPick);
    hideOnPick_ = hideOnPick;
    rows_ = std::make_unique<Data4>(&cb, spec_.table);
    if (!registerClass(spec_.section)) return false;

    // FIELD1..3, or the first fields of the table, one per header.
    const int available = rows_->isValid() ? rows_->numFields() : 0;
    const int columns = std::min<int>(static_cast<int>(spec_.headers.size()), std::max(available, 1));
    const int f[3] = {IniInt(L"FIELD1"), IniInt(L"FIELD2"), IniInt(L"FIELD3")};
    const bool fromIni = f[0] != 0 && f[1] != 0 && f[2] != 0;
    for (int c = 0; c < columns; ++c) fields_.push_back(fromIni ? f[c] : c + 1);
    spec_.headers.resize(static_cast<size_t>(columns));
    spec_.stretchColumn = std::min(spec_.stretchColumn, columns);

    RECT dlg{};
    GetWindowRect(owner, &dlg);
    int left = IniInt(L"LEFT"), top = IniInt(L"TOP"), right = IniInt(L"RIGHT"), bottom = IniInt(L"BOTTOM");
    if (left == 0 || top == 0 || right == 0 || bottom == 0) {
        left = dlg.right;
        top = dlg.top;
        right = left + 360;
        bottom = dlg.bottom;
    }
    // WS_OVERLAPPED (no WS_CHILD) with an owner, as the OWL TWindow was.
    if (!fx::Window::Create(WS_EX_TOOLWINDOW, spec_.section, spec_.caption.c_str(),
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN, left, top,
                            right - left, bottom - top, owner, nullptr, hInst))
        return false;
    Redraw();
    if (show) ShowWindow(hwnd(), SW_SHOWNOACTIVATE);
    return true;
}

bool TableList::OnCreate(LPCREATESTRUCTW)
{
    grid_ = fx::Grid(fx::Grid::Create(hwnd(), kListId, 0, 0, 0, 0));
    if (!grid_.IsGrid()) return false;

    // TListWindow::SetFields: widths from the field lengths; the stretch
    // column takes the rest (the owners' WMSize).
    const int digit = grid_.CharWidth();
    std::vector<fx::GridColumn> columns;
    for (size_t c = 0; c < fields_.size(); ++c) {
        Field4 f;
        const int len = rows_ && rows_->isValid() && f.init(*rows_, fields_[c]) == 0 ? static_cast<int>(f.len()) : 10;
        const bool stretch = static_cast<int>(c) + 1 == spec_.stretchColumn;
        columns.push_back({spec_.headers[c], (stretch ? 30 : len + 2) * digit});
    }
    grid_.SetColumns(columns);
    grid_.SetStretchColumn(spec_.stretchColumn);
    return false;   // DefWindowProc too
}

bool TableList::OnSize(UINT, int cx, int cy)
{
    if (!grid_.hwnd()) return false;
    MoveWindow(grid_.hwnd(), 0, 0, cx, cy, TRUE);   // the grid stretches its stretch column
    return true;
}

bool TableList::OnDestroy()
{
    SavePosition();
    grid_ = fx::Grid();
    return false;
}

bool TableList::OnCommand(WORD id, WORD code, HWND)
{
    if (id != kListId) return false;
    switch (code) {
    case GRN_PICK: {   // SSM_DBLCLK, or Enter
        const int row = grid_.ActiveRow();
        if (row >= 1 && onPick_) onPick_(row);
        if (hideOnPick_) ShowWindow(hwnd(), SW_HIDE);
        return true;
    }
    case GRN_RCLICK:   // WM_RBUTTONDOWN -> TListWindow::SetParamenters
        SetQuery();
        return true;
    case GRN_KEYDOWN:
        if (grid_.LastKey() == VK_ESCAPE) Seek({});
        return true;
    default:
        return false;
    }
}

bool TableList::OnMessage(UINT msg, WPARAM, LPARAM, LRESULT& result)
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

void TableList::Redraw()
{
    if (!grid_.IsGrid()) return;
    grid_.SetRedraw(false);
    grid_.Clear();
    if (rows_ && rows_->isValid()) {
        rows_->SetIndex(spec_.tag);
        Relate4set query(rows_.get());
        const std::string text = CurrentQuery();
        try {
            query.querySet(text.c_str());
        } catch (const std::exception&) {
            const std::wstring msg = L"GL.INI [" + std::wstring(spec_.section) + L"] QUERY=" + fx::Utf8ToWide(text) +
                                     L"\n\nThis query isn't supported; .NOT. DELETED() is used instead.";
            MessageBoxW(hwnd(), msg.c_str(), szApp, MB_ICONEXCLAMATION);
            query.querySet(kDefaultQuery);
        }
        std::vector<Field4> fields(fields_.size());
        for (size_t c = 0; c < fields_.size(); ++c) fields[c].init(*rows_, fields_[c]);
        int row = 0;
        for (int rc = query.top(); rc == r4success; rc = query.skip(1)) {
            ++row;
            for (size_t c = 0; c < fields.size(); ++c)
                grid_.SetData(static_cast<int>(c) + 1, row, fx::Utf8ToWide(fields[c].isValid() ? fields[c].str() : ""));
        }
        query.unlock();
        query.free();
    }
    grid_.SetRedraw(true);
}

bool TableList::Seek(const std::string& text)
{
    if (!grid_.IsGrid()) return false;
    const std::string want = TrimText(text);
    const int count = grid_.Rows();
    int found = 0;
    if (!want.empty())
        for (int row = 1; row <= count && found == 0; ++row)
            if (TrimText(RowText(row, 1)).compare(0, want.size(), want) == 0) found = row;
    grid_.SetActiveCell(1, found > 0 ? found : (count > 0 ? 1 : 0));
    ShowWindow(hwnd(), SW_SHOW);
    SetFocus(grid_.list());
    return found > 0;
}

void TableList::SetQuery()
{
    SetQueryDialog dlg(spec_.section);
    if (dlg.DoModal(hInst, 1035, hwnd()) == IDOK) Redraw();
}

std::string TableList::RowText(int row, int column) const
{
    return fx::WideToUtf8(grid_.GetData(column, row));
}

void TableList::SavePosition() const
{
    RECT rc{};
    if (!hwnd() || !GetWindowRect(hwnd(), &rc)) return;
    WriteIniString(kIni, spec_.section, L"LEFT", std::to_wstring(rc.left).c_str());
    WriteIniString(kIni, spec_.section, L"TOP", std::to_wstring(rc.top).c_str());
    WriteIniString(kIni, spec_.section, L"RIGHT", std::to_wstring(rc.right).c_str());
    WriteIniString(kIni, spec_.section, L"BOTTOM", std::to_wstring(rc.bottom).c_str());
}
