//
// AccountList.cpp - see AccountList.h.
//
// Translation notes (plan per-port checklist):
//   - TListWindow is a tbSpread; the port's grid is fx::Grid (Spec #2 S2-M0).
//     The matched row is selected rather than painted green, and
//     TListWindow::GetKey's hand-built seek buffer (which leaked a malloc per
//     keystroke) is the ListView's type-ahead on the first column.
//   - Placement: GL.INI [GLACCTS] LEFT/TOP/RIGHT/BOTTOM, saved when the
//     window is destroyed (TListWindow::CanClose -> SavePosition). Without a
//     saved position the original went to screen (0,0), 100 pixels high; the
//     port puts it beside the dialog, as tall as the dialog.
//   - Closing the window hides it. The original's IDCANCEL handler destroyed
//     it, after which the dialog's next Search used the deleted window.
//   - TSetQuery (1035) is a plain dialog here: its IDCANCEL was
//     TMainDialog::Cancel, whose cb.closeAll() closed the Chart of Accounts'
//     own tables underneath it.
//   - A QUERY that cb4sqlite can't run (Relate4set supports only
//     ".NOT. DELETED()" in Spec #1, plan risk P4) is reported and the default
//     is used instead.
//
#include "AccountList.h"

#include "Accounts.h"
#include "Globals.h"
#include "Ini.h"

#include <exception>
#include <string>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr wchar_t kClass[] = L"GLACCTS";    // TAccountList::GetClassName: the table's name
constexpr wchar_t kSection[] = L"GLACCTS";  // TListWindow's GL.INI section (szlpAWindow)
constexpr wchar_t kIni[] = L"GL.INI";
constexpr char kDefaultQuery[] = ".NOT. DELETED()";
constexpr int kListId = ID_DISTLIST;
constexpr int kQueryEdit = 101;

int iniInt(const wchar_t* key)
{
    return static_cast<int>(GetPrivateProfileIntW(kSection, key, 0, IniPath(kIni).c_str()));
}

std::string currentQuery()
{
    const std::string q = TrimText(fx::WideToUtf8(GetIniString(kIni, kSection, L"QUERY")));
    return q.empty() ? kDefaultQuery : q;
}

bool registerClass()
{
    WNDCLASSEXW existing{};
    existing.cbSize = sizeof existing;
    if (GetClassInfoExW(hInst, kClass, &existing)) return true;
    WNDCLASSEXW wc = fx::Window::ClassInfo(hInst, kClass);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    return RegisterClassExW(&wc) != 0;
}

// TSetQuery: edit GL.INI [GLACCTS] QUERY.
class SetQueryDialog : public fx::Dialog {
protected:
    bool OnInitDialog() override
    {
        SetDlgItemTextW(hwnd(), kQueryEdit, GetIniString(kIni, kSection, L"QUERY").c_str());
        return true;
    }
    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != IDOK) return false;   // IDCANCEL: closes (fx::Dialog)
        WriteIniString(kIni, kSection, L"QUERY", fx::GetDlgItemString(hwnd(), kQueryEdit).c_str());
        End(IDOK);
        return true;
    }
};

} // namespace

AccountList::~AccountList()
{
    if (hwnd()) DestroyWindow(hwnd());
}

bool AccountList::Create(HWND owner, Tglaccts& gl)
{
    owner_ = owner;
    gl_ = &gl;
    rows_ = std::make_unique<Data4>(&cb, "glaccts");
    if (!registerClass()) return false;

    const int f1 = iniInt(L"FIELD1"), f2 = iniInt(L"FIELD2"), f3 = iniInt(L"FIELD3");
    if (f1 != 0 && f2 != 0 && f3 != 0) {
        fields_[0] = f1;
        fields_[1] = f2;
        fields_[2] = f3;
    }

    RECT dlg{};
    GetWindowRect(owner, &dlg);
    int left = iniInt(L"LEFT"), top = iniInt(L"TOP"), right = iniInt(L"RIGHT"), bottom = iniInt(L"BOTTOM");
    if (left == 0 || top == 0 || right == 0 || bottom == 0) {
        left = dlg.right;
        top = dlg.top;
        right = left + 360;
        bottom = dlg.bottom;
    }
    // WS_OVERLAPPED (no WS_CHILD) with an owner, as the OWL TWindow was.
    const std::wstring title = fx::LoadResString(hInst, 6084);
    if (!fx::Window::Create(WS_EX_TOOLWINDOW, kClass, title.c_str(),
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN, left, top,
                            right - left, bottom - top, owner, nullptr, hInst))
        return false;
    Redraw();
    ShowWindow(hwnd(), SW_SHOWNOACTIVATE);
    return true;
}

bool AccountList::OnCreate(LPCREATESTRUCTW)
{
    grid_ = fx::Grid(fx::Grid::Create(hwnd(), kListId, 0, 0, 0, 0));
    if (!grid_.IsGrid()) return false;

    // TAccountList::SetWndDetail: headers 6068/6054/6044, widths from the
    // field lengths; the description takes the rest (TAccountList::WMSize).
    const UINT headers[] = {6068, 6054, 6044};
    const int digit = grid_.CharWidth();
    std::vector<fx::GridColumn> columns;
    for (int c = 0; c < 3; ++c) {
        Field4 f;
        const int len = rows_ && rows_->isValid() && f.init(*rows_, fields_[c]) == 0 ? static_cast<int>(f.len()) : 10;
        columns.push_back({fx::LoadResString(hInst, headers[c]), (c == 2 ? 30 : len + 2) * digit});
    }
    grid_.SetColumns(columns);
    grid_.SetStretchColumn(3);
    return false;   // DefWindowProc too
}

bool AccountList::OnSize(UINT, int cx, int cy)
{
    if (!grid_.hwnd()) return false;
    MoveWindow(grid_.hwnd(), 0, 0, cx, cy, TRUE);   // the grid stretches the description column
    return true;
}

bool AccountList::OnDestroy()
{
    SavePosition();
    grid_ = fx::Grid();
    return false;
}

bool AccountList::OnCommand(WORD id, WORD code, HWND)
{
    if (id != kListId) return false;
    switch (code) {
    case GRN_PICK:     // SSM_DBLCLK, or Enter
        SelectRow(grid_.ActiveRow());
        return true;
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

bool AccountList::OnMessage(UINT msg, WPARAM, LPARAM, LRESULT& result)
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

void AccountList::Redraw()
{
    if (!grid_.IsGrid()) return;
    grid_.SetRedraw(false);
    grid_.Clear();
    if (rows_ && rows_->isValid()) {
        rows_->SetIndex("accounts");
        Relate4set query(rows_.get());
        const std::string text = currentQuery();
        try {
            query.querySet(text.c_str());
        } catch (const std::exception&) {
            const std::wstring msg = L"GL.INI [GLACCTS] QUERY=" + fx::Utf8ToWide(text) +
                                     L"\n\nThis query isn't supported; .NOT. DELETED() is used instead.";
            MessageBoxW(hwnd(), msg.c_str(), szApp, MB_ICONEXCLAMATION);
            query.querySet(kDefaultQuery);
        }
        Field4 f1(*rows_, fields_[0]), f2(*rows_, fields_[1]), f3(*rows_, fields_[2]);
        int row = 0;
        for (int rc = query.top(); rc == r4success; rc = query.skip(1)) {
            ++row;
            int col = 0;
            for (const Field4* f : {&f1, &f2, &f3}) grid_.SetData(++col, row, fx::Utf8ToWide(f->isValid() ? f->str() : ""));
        }
        query.unlock();
        query.free();
    }
    grid_.SetRedraw(true);
}

bool AccountList::Seek(const std::string& account)
{
    if (!grid_.IsGrid()) return false;
    const std::string want = TrimText(account);
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

void AccountList::SelectRow(int row)
{
    if (row < 1 || !gl_) return;
    if (SeekAccount(*gl_, RowText(row, 1), RowText(row, 2))) {
        gl_->descript->SetField(owner_, ID_JDESCRIPT);
        gl_->accounts->SetField(owner_, ID_ACCOUNTS);
        gl_->dept->SetField(owner_, ID_DEPART);
        SendMessageW(owner_, WM_COMMAND, ID_ACCTLIST, 0);   // TGLAcctsAdd::Update
    }
    ShowWindow(hwnd(), SW_HIDE);
}

void AccountList::SetQuery()
{
    SetQueryDialog dlg;
    if (dlg.DoModal(hInst, 1035, hwnd()) == IDOK) Redraw();
}

std::string AccountList::RowText(int row, int column) const
{
    return fx::WideToUtf8(grid_.GetData(column, row));
}

void AccountList::SavePosition() const
{
    RECT rc{};
    if (!hwnd() || !GetWindowRect(hwnd(), &rc)) return;
    WriteIniString(kIni, kSection, L"LEFT", std::to_wstring(rc.left).c_str());
    WriteIniString(kIni, kSection, L"TOP", std::to_wstring(rc.top).c_str());
    WriteIniString(kIni, kSection, L"RIGHT", std::to_wstring(rc.right).c_str());
    WriteIniString(kIni, kSection, L"BOTTOM", std::to_wstring(rc.bottom).c_str());
}
