//
// AccountList.cpp - see AccountList.h.
//
// Translation notes (plan per-port checklist):
//   - TListWindow is a tbSpread; the port's grid is a ListView (LVS_REPORT).
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

#include <CommCtrl.h>

#include <exception>
#include <string>

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
    list_ = CreateWindowExW(0, WC_LISTVIEWW, L"",
                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            0, 0, 0, 0, hwnd(), reinterpret_cast<HMENU>(static_cast<INT_PTR>(kListId)), hInst,
                            nullptr);
    if (!list_) return false;
    ListView_SetExtendedListViewStyle(list_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    // TAccountList::SetWndDetail: headers 6068/6054/6044, widths from the field lengths.
    const UINT headers[] = {6068, 6054, 6044};
    const int digit = ListView_GetStringWidth(list_, L"0");
    for (int c = 0; c < 3; ++c) {
        const std::wstring text = fx::LoadResString(hInst, headers[c]);
        Field4 f;
        const int len = rows_ && rows_->isValid() && f.init(*rows_, fields_[c]) == 0 ? static_cast<int>(f.len()) : 10;
        LVCOLUMNW col{};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = const_cast<wchar_t*>(text.c_str());
        col.cx = (c == 2 ? 30 : len + 2) * digit;
        col.iSubItem = c;
        ListView_InsertColumn(list_, c, &col);
    }
    return false;   // DefWindowProc too
}

bool AccountList::OnSize(UINT, int cx, int cy)
{
    // TAccountList::WMSize: the grid fills the window; the description column takes the rest.
    if (!list_) return false;
    MoveWindow(list_, 0, 0, cx, cy, TRUE);
    const int used = ListView_GetColumnWidth(list_, 0) + ListView_GetColumnWidth(list_, 1);
    const int rest = cx - used - GetSystemMetrics(SM_CXVSCROLL) - 4;
    if (rest > 40) ListView_SetColumnWidth(list_, 2, rest);
    return true;
}

bool AccountList::OnDestroy()
{
    SavePosition();
    list_ = nullptr;
    return false;
}

bool AccountList::OnMessage(UINT msg, WPARAM, LPARAM lp, LRESULT& result)
{
    switch (msg) {
    case WM_CLOSE:
        ShowWindow(hwnd(), SW_HIDE);
        if (owner_) SetFocus(owner_);
        result = 0;
        return true;
    case WM_SETFOCUS:
        if (list_) SetFocus(list_);
        result = 0;
        return true;
    case WM_NOTIFY: {
        const auto* nm = reinterpret_cast<const NMHDR*>(lp);
        if (nm->hwndFrom != list_) return false;
        switch (nm->code) {
        case NM_DBLCLK:   // SSM_DBLCLK
            if (reinterpret_cast<const NMITEMACTIVATE*>(lp)->iItem >= 0)
                SelectRow(reinterpret_cast<const NMITEMACTIVATE*>(lp)->iItem);
            break;
        case NM_RETURN:
            SelectRow(ListView_GetNextItem(list_, -1, LVNI_SELECTED));
            break;
        case NM_RCLICK:   // WM_RBUTTONDOWN -> TListWindow::SetParamenters
            SetQuery();
            break;
        case LVN_KEYDOWN:
            if (reinterpret_cast<const NMLVKEYDOWN*>(lp)->wVKey == VK_ESCAPE) Seek({});
            break;
        default:
            return false;
        }
        result = 0;
        return true;
    }
    default:
        return false;
    }
}

void AccountList::Redraw()
{
    if (!list_) return;
    SendMessageW(list_, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(list_);
    if (rows_ && rows_->isValid()) {
        rows_->SetIndex("accounts");
        Relate4set query(rows_.get());
        const std::string text = currentQuery();
        try {
            query.querySet(text.c_str());
        } catch (const std::exception&) {
            const std::wstring msg = L"GL.INI [GLACCTS] QUERY=" + fx::Utf8ToWide(text) +
                                     L"\n\nSpec #1 can only run the query .NOT. DELETED(), which is used instead.";
            MessageBoxW(hwnd(), msg.c_str(), szApp, MB_ICONEXCLAMATION);
            query.querySet(kDefaultQuery);
        }
        Field4 f1(*rows_, fields_[0]), f2(*rows_, fields_[1]), f3(*rows_, fields_[2]);
        int row = 0;
        for (int rc = query.top(); rc == r4success; rc = query.skip(1)) {
            LVITEMW item{};
            item.mask = LVIF_TEXT;
            item.iItem = row;
            std::wstring t = fx::Utf8ToWide(f1.isValid() ? f1.str() : "");
            item.pszText = t.data();
            ListView_InsertItem(list_, &item);
            for (int c = 1; c < 3; ++c) {
                const Field4& f = c == 1 ? f2 : f3;
                std::wstring s = fx::Utf8ToWide(f.isValid() ? f.str() : "");
                ListView_SetItemText(list_, row, c, s.data());
            }
            ++row;
        }
        query.unlock();
        query.free();
    }
    SendMessageW(list_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(list_, nullptr, TRUE);
}

bool AccountList::Seek(const std::string& account)
{
    if (!list_) return false;
    const std::string want = TrimText(account);
    const int count = ListView_GetItemCount(list_);
    int found = -1;
    if (!want.empty())
        for (int row = 0; row < count && found < 0; ++row)
            if (TrimText(RowText(row, 0)).compare(0, want.size(), want) == 0) found = row;
    const int row = found >= 0 ? found : (count > 0 ? 0 : -1);
    ListView_SetItemState(list_, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
    if (row >= 0) {
        ListView_SetItemState(list_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(list_, row, FALSE);
    }
    ShowWindow(hwnd(), SW_SHOW);
    SetFocus(list_);
    return found >= 0;
}

void AccountList::SelectRow(int row)
{
    if (row < 0 || !gl_) return;
    if (SeekAccount(*gl_, RowText(row, 0), RowText(row, 1))) {
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
    wchar_t buf[256] = {};
    ListView_GetItemText(list_, row, column, buf, 256);
    return fx::WideToUtf8(buf);
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
