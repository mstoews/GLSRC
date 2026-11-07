//
// MainDialog.cpp - see MainDialog.h.
//
#include "MainDialog.h"

#include "Globals.h"

#include "LegacyIds.h"   // last: see LegacyIds.h

INT_PTR MainDialog::DoModal(UINT templateId, HWND parent)
{
    return fx::Dialog::DoModal(hInst, templateId, parent);
}

bool MainDialog::OnInitDialog()
{
    // TMainDialog::SetupWindow (gl/src/ap_dlg.cpp). ID_LOOK is left out: it
    // is defined only in the AP headers, and no GL template has it.
    for (const UINT id : {static_cast<UINT>(IDCANCEL), static_cast<UINT>(IDOK), static_cast<UINT>(ID_NEW),
                          static_cast<UINT>(ID_TOP), static_cast<UINT>(ID_BOTTOM), static_cast<UINT>(ID_NEXT),
                          static_cast<UINT>(ID_PREVIOUS), static_cast<UINT>(ID_DELETE),
                          static_cast<UINT>(ID_SLIPLIST)})
        SetBitmap(id);
    return true;
}

bool MainDialog::OnCommand(WORD id, WORD, HWND)
{
    if (id == IDCANCEL) {
        Cancel();
        return true;
    }
    return false;
}

void MainDialog::Cancel()
{
    cb.closeAll();
    cb.init();
    End(IDCANCEL);
}

std::string MainDialog::GetField(int id) const
{
    return fx::WideToUtf8(fx::GetDlgItemString(hwnd(), id));
}

std::string MainDialog::GetDate(int id) const
{
    return fx::WideToUtf8(fx::DateGetYmd(GetDlgItem(hwnd(), id)));
}

void MainDialog::SetField(int id, const char* utf8)
{
    SetDlgItemTextW(hwnd(), id, fx::Utf8ToWide(utf8 ? utf8 : "").c_str());
}

void MainDialog::SetField(int id, const Field4& field)
{
    field.SetField(hwnd(), static_cast<UINT>(id));
}

void MainDialog::SetFieldW(int id, const std::wstring& text)
{
    SetDlgItemTextW(hwnd(), id, text.c_str());
}

void MainDialog::SetBitmap(UINT id)
{
    SendDlgItemMessageW(hwnd(), static_cast<int>(id), SBM_SETPICT, SUPERBTN_PICT_BITMAP, static_cast<LPARAM>(id));
}

std::wstring MainDialog::GetString(UINT id) const
{
    return fx::LoadResString(hInst, id);
}

int MainDialog::Message(const std::wstring& text, UINT flags) const
{
    return MessageBoxW(hwnd(), text.c_str(), szApp, flags);
}

bool MainDialog::IsChecked(int id) const
{
    return SendDlgItemMessageW(hwnd(), id, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void MainDialog::SetCheck(int id, bool on)
{
    SendDlgItemMessageW(hwnd(), id, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
}

std::string StrTrim(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}
