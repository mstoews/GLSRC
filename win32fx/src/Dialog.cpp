//
// Dialog.cpp - see Dialog.h.
//

#include "Dialog.h"
#include "Internal.h"
#include "MessageLoop.h"

#include <exception>

namespace win32fx {

namespace {

// Messages whose result is the DlgProc return value itself.
bool directResult(UINT msg) noexcept
{
    switch (msg) {
    case WM_CTLCOLORMSGBOX: case WM_CTLCOLOREDIT: case WM_CTLCOLORLISTBOX: case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG: case WM_CTLCOLORSCROLLBAR: case WM_CTLCOLORSTATIC:
    case WM_COMPAREITEM: case WM_VKEYTOITEM: case WM_CHARTOITEM: case WM_QUERYDRAGICON:
    case WM_INITDIALOG:
        return true;
    default:
        return false;
    }
}

INT_PTR handledResult(HWND h, UINT msg, LRESULT r) noexcept
{
    if (directResult(msg)) return static_cast<INT_PTR>(r);
    SetWindowLongPtrW(h, DWLP_MSGRESULT, r);
    return TRUE;
}

} // namespace

Dialog::~Dialog()
{
    if (HWND h = hwnd_) {
        SetWindowLongPtrW(h, DWLP_USER, 0);
        hwnd_ = nullptr;
        if (!modal_) {
            RemoveModeless(h);
            DestroyWindow(h);
        }
    }
}

INT_PTR Dialog::DoModal(HINSTANCE inst, UINT templateId, HWND parent)
{
    return DoModal(inst, MAKEINTRESOURCEW(templateId), parent);
}

INT_PTR Dialog::DoModal(HINSTANCE inst, const wchar_t* templateName, HWND parent)
{
    modal_ = true;
    return DialogBoxParamW(inst, templateName, parent, &Dialog::DlgProc, reinterpret_cast<LPARAM>(this));
}

HWND Dialog::CreateModeless(HINSTANCE inst, UINT templateId, HWND parent)
{
    return CreateModeless(inst, MAKEINTRESOURCEW(templateId), parent);
}

HWND Dialog::CreateModeless(HINSTANCE inst, const wchar_t* templateName, HWND parent)
{
    modal_ = false;
    return CreateDialogParamW(inst, templateName, parent, &Dialog::DlgProc, reinterpret_cast<LPARAM>(this));
}

void Dialog::End(INT_PTR result)
{
    if (!hwnd_) return;
    if (modal_) EndDialog(hwnd_, result);
    else DestroyWindow(hwnd_);
}

Dialog* Dialog::FromHandle(HWND h) noexcept
{
    if (!h || reinterpret_cast<DLGPROC>(GetWindowLongPtrW(h, DWLP_DLGPROC)) != &Dialog::DlgProc)
        return nullptr;
    return reinterpret_cast<Dialog*>(GetWindowLongPtrW(h, DWLP_USER));
}

INT_PTR CALLBACK Dialog::DlgProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    Dialog* self = nullptr;
    if (msg == WM_INITDIALOG) {
        self = reinterpret_cast<Dialog*>(lp);
        if (self) {
            self->hwnd_ = h;
            SetWindowLongPtrW(h, DWLP_USER, reinterpret_cast<LONG_PTR>(self));
            if (!self->modal_) AddModeless(h);
        }
    } else {
        self = reinterpret_cast<Dialog*>(GetWindowLongPtrW(h, DWLP_USER));
    }
    if (!self) return FALSE;

    INT_PTR r = FALSE;
    bool failed = false;
    try {
        r = self->Dispatch(msg, wp, lp);
    } catch (const std::exception& e) {
        detail::ReportException(h, e.what());
        failed = true;
    } catch (...) {
        detail::ReportException(h, nullptr);
        failed = true;
    }
    if (failed) {
        r = FALSE;
        if (msg == WM_INITDIALOG) {   // half-initialised: close it
            if (self->modal_) EndDialog(h, IDABORT);
            else DestroyWindow(h);
        }
    }

    if (msg == WM_NCDESTROY && reinterpret_cast<Dialog*>(GetWindowLongPtrW(h, DWLP_USER)) == self) {
        SetWindowLongPtrW(h, DWLP_USER, 0);
        if (!self->modal_) RemoveModeless(h);
        self->hwnd_ = nullptr;
        try {
            self->OnFinalMessage();
        } catch (const std::exception& e) {
            detail::ReportException(nullptr, e.what());
        } catch (...) {
            detail::ReportException(nullptr, nullptr);
        }
    }
    return r;
}

INT_PTR Dialog::Dispatch(UINT msg, WPARAM wp, LPARAM lp)
{
    const HWND h = hwnd_;   // handlers may destroy this object
    LRESULT r = 0;
    if (ProcessMessageMap(msg, wp, lp, r)) return handledResult(h, msg, r);
    switch (msg) {
    case WM_INITDIALOG:
        return OnInitDialog() ? TRUE : FALSE;
    case WM_COMMAND: {
        const WORD id = LOWORD(wp), code = HIWORD(wp);
        if (OnCommand(id, code, reinterpret_cast<HWND>(lp))) return handledResult(h, msg, 0);
        if ((id == IDOK || id == IDCANCEL) && code == BN_CLICKED) {
            End(id);
            return TRUE;
        }
        break;
    }
    case WM_SIZE:
        if (OnSize(static_cast<UINT>(wp), static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp))))
            return handledResult(h, msg, 0);
        break;
    case WM_DESTROY:
        if (OnDestroy()) return handledResult(h, msg, 0);
        break;
    default:
        break;
    }
    r = 0;
    if (OnMessage(msg, wp, lp, r)) return handledResult(h, msg, r);
    return FALSE;
}

} // namespace win32fx
