//
// MainDialog.h - the original TMainDialog (include/gl_dlg.h; bodies in
// gl/src/ap_dlg.cpp and gl_dlg.cpp), the base of the ported GL screens.
//
// Only the helpers the Spec #1 screens call are kept. Field text crosses the
// database boundary as UTF-8 (std::string) and the Win32 boundary as UTF-16:
//
//   GetField(id)        control text as UTF-8, as typed (no trimming)
//   GetDate(id)         a tbDate's date as YYYYMMDD, "" if blank or invalid
//                       (TMainDialog::GetDate went through Date4 and string 4024)
//   SetField(id, ...)   UTF-8 text, a Field4's buffer, or wide text
//   SetBitmap(id)       SBM_SETPICT with the bitmap resource of the same ID
//   GetString(id)       a string resource
//
// OnInitDialog gives the usual buttons their bitmaps (TMainDialog::SetupWindow
// in ap_dlg.cpp). Cancel() is TMainDialog::Cancel: cb.closeAll(); cb.init();
// close the dialog. A subclass that handles IDCANCEL itself replaces it.
//
#pragma once

#include "win32fx.h"

#include "cb4sqlite.h"

#include <string>

class MainDialog : public fx::Dialog {
public:
    INT_PTR DoModal(UINT templateId, HWND parent);

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;

    virtual void Cancel();

    std::string GetField(int id) const;
    std::string GetDate(int id) const;
    void SetField(int id, const char* utf8);
    void SetField(int id, const std::string& utf8) { SetField(id, utf8.c_str()); }
    void SetField(int id, const Field4& field);
    void SetFieldW(int id, const std::wstring& text);
    void SetBitmap(UINT id);
    std::wstring GetString(UINT id) const;

    // MessageBoxW owned by this dialog with the szApp caption.
    int Message(const std::wstring& text, UINT flags) const;

    bool IsChecked(int id) const;
    void SetCheck(int id, bool on);
};

// "  abc  " -> "abc", on UTF-8 text (the original StrTrim on char*).
std::string StrTrim(const std::string& s);
