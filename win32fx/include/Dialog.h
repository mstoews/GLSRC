//
// Dialog.h - modal and modeless dialogs (spec 3.2).
//
//     class AboutBox : public win32fx::Dialog {
//         bool OnInitDialog() override { ...; return true; }
//     };
//     AboutBox dlg;
//     dlg.DoModal(hInst, 1008, frame);
//
// The DlgProc stores `this` on WM_INITDIALOG (DWLP_USER) and routes later
// messages to the same object, so each instance gets its own messages.
// Messages that arrive before WM_INITDIALOG (WM_SETFONT, ...) get the dialog
// manager's default handling. DWLP_USER is not available to subclasses.
//
// Dispatch order and the exception boundary are as for Window (Window.h). An
// exception from OnInitDialog closes the dialog: EndDialog(IDABORT) for a
// modal one, DestroyWindow for a modeless one.
//
// Result values: a handled message returns TRUE from the DlgProc with
// `result` stored in DWLP_MSGRESULT, except for the messages whose result the
// dialog manager takes from the DlgProc return itself (WM_CTLCOLOR*,
// WM_COMPAREITEM, WM_VKEYTOITEM, WM_CHARTOITEM, WM_QUERYDRAGICON,
// WM_INITDIALOG); for those `result` is returned directly.
//
// IDOK and IDCANCEL that OnCommand doesn't handle close the dialog with that
// ID (EndDialog when modal, DestroyWindow when modeless), as OWL's TDialog
// did. A modeless dialog joins the message loop's IsDialogMessage list
// (MessageLoop.h) while it exists.
//
#pragma once

#include <Windows.h>

namespace win32fx {

class Dialog {
public:
    Dialog() noexcept = default;
    virtual ~Dialog();
    Dialog(const Dialog&) = delete;
    Dialog& operator=(const Dialog&) = delete;

    // DialogBoxParamW. Returns the EndDialog value, or -1 if the template
    // couldn't be loaded.
    INT_PTR DoModal(HINSTANCE inst, UINT templateId, HWND parent);
    INT_PTR DoModal(HINSTANCE inst, const wchar_t* templateName, HWND parent);

    // CreateDialogParamW. Null on failure. The object must outlive the window
    // (see OnFinalMessage).
    HWND CreateModeless(HINSTANCE inst, UINT templateId, HWND parent);
    HWND CreateModeless(HINSTANCE inst, const wchar_t* templateName, HWND parent);

    // Ends a modal dialog with `result`, or destroys a modeless one.
    void End(INT_PTR result);

    HWND hwnd() const noexcept { return hwnd_; }
    operator HWND() const noexcept { return hwnd_; }
    bool isModal() const noexcept { return modal_; }

    static Dialog* FromHandle(HWND hwnd) noexcept;
    static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

protected:
    // The WM_INITDIALOG return: true lets the dialog manager focus the first
    // tab stop; return false after calling SetFocus yourself.
    virtual bool OnInitDialog() { return true; }
    virtual bool OnCommand(WORD /*id*/, WORD /*code*/, HWND /*ctl*/) { return false; }
    virtual bool OnSize(UINT /*type*/, int /*cx*/, int /*cy*/) { return false; }
    virtual bool OnDestroy() { return false; }
    virtual bool OnMessage(UINT /*msg*/, WPARAM, LPARAM, LRESULT& /*result*/) { return false; }
    virtual bool ProcessMessageMap(UINT, WPARAM, LPARAM, LRESULT&) { return false; }
    // After WM_NCDESTROY; hwnd() is already null. `delete this` is allowed here.
    virtual void OnFinalMessage() {}

private:
    INT_PTR Dispatch(UINT msg, WPARAM wp, LPARAM lp);

    HWND hwnd_ = nullptr;
    bool modal_ = false;
};

} // namespace win32fx
