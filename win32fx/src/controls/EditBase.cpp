//
// EditBase.cpp - the window procedure shared by the tb* edit superclasses.
//
// Creation (WM_NCCREATE): the template text is kept as the mask and the
// control starts empty; the Toolbox style bits are replaced by EDIT styles
// (ES_3D -> WS_EX_CLIENTEDGE, else WS_BORDER; ES_STATIC -> ES_READONLY;
// numbers ES_RIGHT; ES_UPPERCASE/LOWERCASE for tbPic) before EDIT sees them.
//
// Typing (WM_CHAR) and pasting (WM_PASTE): the text the edit would have
// afterwards is fitted by the behavior's conform(); a rejected character
// beeps. If conform() changed it (a literal inserted, case changed), the text
// is replaced through EM_REPLACESEL so undo and EN_CHANGE still work.
// Control characters (Backspace, Ctrl+C/V/X, ...) go to EDIT unchecked, so
// deleting is always possible.
//
// Everything else, including EN_CHANGE / EN_KILLFOCUS to the parent, is the
// system EDIT control's.
//

#include "ControlsInternal.h"

#include <algorithm>
#include <memory>

namespace win32fx::detail {

namespace {

WNDCLASSEXW g_edit{};          // the system EDIT class, captured on first registration
int g_editExtra = 0;           // offset of our EditState pointer

LRESULT base(HWND h, UINT m, WPARAM w, LPARAM l)
{
    return CallWindowProcW(g_edit.lpfnWndProc, h, m, w, l);
}

bool readOnly(HWND h) noexcept { return (GetWindowLongW(h, GWL_STYLE) & ES_READONLY) != 0; }

void selection(HWND h, size_t textLen, size_t& start, size_t& end) noexcept
{
    DWORD s = 0, e = 0;
    SendMessageW(h, EM_GETSEL, reinterpret_cast<WPARAM>(&s), reinterpret_cast<LPARAM>(&e));
    start = std::min<size_t>(std::min(s, e), textLen);
    end = std::min<size_t>(std::max(s, e), textLen);
}

void replaceAll(HWND h, const std::wstring& text, size_t caret)
{
    SendMessageW(h, EM_SETSEL, 0, -1);
    SendMessageW(h, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(text.c_str()));
    SendMessageW(h, EM_SETSEL, caret, static_cast<LPARAM>(caret));
}

// Inserts `ins` over the selection if the behavior accepts the result.
bool insert(const EditBehavior& b, EditState& st, HWND h, const std::wstring& ins, WPARAM charWp, LPARAM charLp)
{
    const std::wstring text = WindowText(h);
    size_t s = 0, e = 0;
    selection(h, text.size(), s, e);
    const std::wstring candidate = text.substr(0, s) + ins + text.substr(e);
    std::wstring out;
    size_t caret = 0;
    if (!b.conform(st, candidate, s + ins.size(), out, caret)) {
        MessageBeep(MB_OK);
        return false;
    }
    if (out == candidate && charWp)
        base(h, WM_CHAR, charWp, charLp);   // the edit's own typing
    else
        replaceAll(h, out, caret);
    return true;
}

std::wstring clipboardLine(HWND h)
{
    std::wstring clip;
    if (!OpenClipboard(h)) return clip;
    if (HANDLE data = GetClipboardData(CF_UNICODETEXT)) {
        if (auto* p = static_cast<const wchar_t*>(GlobalLock(data))) {
            clip = p;
            GlobalUnlock(data);
        }
    }
    CloseClipboard();
    return clip.substr(0, clip.find_first_of(L"\r\n"));
}

LRESULT onCreate(const EditBehavior& b, HWND h, WPARAM wp, LPARAM lp)
{
    auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
    auto st = std::make_unique<EditState>();
    const DWORD tb = static_cast<DWORD>(cs->style) & 0xFFFF;
    st->tbStyle = tb;
    if (cs->lpszName && !IS_INTRESOURCE(cs->lpszName)) st->mask = cs->lpszName;
    wchar_t cls[64] = {};
    if (GetClassNameW(h, cls, 64) && CompareStringOrdinal(cls, -1, st->mask.c_str(), -1, TRUE) == CSTR_EQUAL)
        st->mask.clear();   // "tbDate", "tbInteger": no mask, just the class name
    b.configure(*st);

    DWORD low = ES_AUTOHSCROLL;
    if (b.alignRight && !(tb & tbES_LEFTALIGN)) low |= ES_RIGHT;
    if (b.caseStyles) low |= tb & (tbES_UPPERCASE | tbES_LOWERCASE);   // same bits as EDIT's
    if (tb & tbES_STATIC) low |= ES_READONLY;
    DWORD style = (static_cast<DWORD>(cs->style) & 0xFFFF0000u) | low;
    DWORD ex = cs->dwExStyle;
    if (tb & tbES_3D) ex |= WS_EX_CLIENTEDGE;
    else style |= WS_BORDER;
    SetWindowLongPtrW(h, GWL_STYLE, static_cast<LONG_PTR>(style));
    SetWindowLongPtrW(h, GWL_EXSTYLE, static_cast<LONG_PTR>(ex));
    cs->style = static_cast<LONG>(style);
    cs->dwExStyle = ex;
    cs->lpszName = L"";

    SetWindowLongPtrW(h, g_editExtra, reinterpret_cast<LONG_PTR>(st.get()));
    const LRESULT r = base(h, WM_NCCREATE, wp, lp);
    if (r) (void)st.release();
    else SetWindowLongPtrW(h, g_editExtra, 0);
    return r;
}

LRESULT dispatch(const EditBehavior& b, HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE) return onCreate(b, h, wp, lp);
    auto* st = reinterpret_cast<EditState*>(GetWindowLongPtrW(h, g_editExtra));
    if (msg == WM_NCDESTROY) {
        const LRESULT r = base(h, msg, wp, lp);
        SetWindowLongPtrW(h, g_editExtra, 0);
        delete st;
        return r;
    }
    if (!st) return base(h, msg, wp, lp);

    switch (msg) {
    case WM_GETDLGCODE:
        return base(h, msg, wp, lp) | DLGC_WANTCHARS | DLGC_WANTARROWS;   // spec 10 Q6

    case WM_CHAR: {
        wchar_t ch = static_cast<wchar_t>(wp);
        if (ch < L' ' || ch == 0x7F || readOnly(h)) break;
        if (st->callback) {
            DWORD s = 0;
            SendMessageW(h, EM_GETSEL, reinterpret_cast<WPARAM>(&s), 0);
            const WORD r = st->callback(h, static_cast<short>(GetDlgCtrlID(h)), static_cast<short>(s), ch);
            if (!r) { MessageBeep(MB_OK); return 0; }
            ch = static_cast<wchar_t>(r);
        }
        insert(b, *st, h, std::wstring(1, ch), ch, lp);
        return 0;
    }

    case WM_PASTE:
        if (readOnly(h)) break;
        insert(b, *st, h, clipboardLine(h), 0, 0);
        return 0;

    case WM_SETTEXT:
        if (b.setText && lp) {
            std::wstring out;
            if (b.setText(*st, reinterpret_cast<const wchar_t*>(lp), out))
                return base(h, msg, wp, reinterpret_cast<LPARAM>(out.c_str()));
        }
        break;

    case WM_KILLFOCUS:
        // Before EDIT sends EN_KILLFOCUS, so the parent reads the final text.
        if (b.reformat) {
            const std::wstring text = WindowText(h);
            std::wstring out;
            if (b.reformat(*st, text, out) && out != text)
                base(h, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(out.c_str()));
        }
        break;

    case EPM_SETMASK:
        st->mask = lp ? reinterpret_cast<const wchar_t*>(lp) : L"";
        b.configure(*st);
        return TRUE;

    case EPM_GETMASK: {
        auto* buf = reinterpret_cast<wchar_t*>(lp);
        if (buf && wp) {
            const size_t n = std::min(static_cast<size_t>(wp) - 1, st->mask.size());
            st->mask.copy(buf, n);
            buf[n] = L'\0';
        }
        return static_cast<LRESULT>(st->mask.size());
    }

    case EPM_SETCALLBACK:
        st->callback = reinterpret_cast<PICCALLBACK>(lp);
        return TRUE;
    }
    return base(h, msg, wp, lp);
}

} // namespace

std::wstring WindowText(HWND hwnd)
{
    std::wstring s(static_cast<size_t>(GetWindowTextLengthW(hwnd)) + 1, L'\0');
    s.resize(static_cast<size_t>(GetWindowTextW(hwnd, s.data(), static_cast<int>(s.size()))));
    return s;
}

LRESULT EditProc(const EditBehavior& b, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // Nothing may propagate into user32; the only throw is std::bad_alloc.
    try {
        return dispatch(b, hwnd, msg, wp, lp);
    } catch (...) {
        return msg == WM_NCCREATE ? FALSE : base(hwnd, msg, wp, lp);
    }
}

bool RegisterEditClass(HINSTANCE inst, const wchar_t* name, WNDPROC proc)
{
    if (!g_edit.lpfnWndProc) {
        WNDCLASSEXW e{};
        e.cbSize = sizeof e;
        if (!GetClassInfoExW(nullptr, L"EDIT", &e)) return false;
        g_edit = e;
        g_editExtra = e.cbWndExtra;
    }
    WNDCLASSEXW existing{};
    existing.cbSize = sizeof existing;
    if (GetClassInfoExW(inst, name, &existing)) return true;

    WNDCLASSEXW wc = g_edit;
    wc.cbSize = sizeof wc;
    wc.style |= CS_GLOBALCLASS | CS_DBLCLKS;
    wc.lpfnWndProc = proc;
    wc.cbWndExtra = g_editExtra + static_cast<int>(sizeof(LONG_PTR));
    wc.hInstance = inst;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = name;
    return RegisterClassExW(&wc) != 0;
}

EditState* GetEditState(HWND hwnd, WNDPROC proc) noexcept
{
    if (!hwnd || reinterpret_cast<WNDPROC>(GetClassLongPtrW(hwnd, GCLP_WNDPROC)) != proc) return nullptr;
    return reinterpret_cast<EditState*>(GetWindowLongPtrW(hwnd, g_editExtra));
}

} // namespace win32fx::detail
