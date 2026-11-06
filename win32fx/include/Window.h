//
// Window.h - HWND-owning base class with per-instance dispatch (spec 3.1).
//
// Register a class whose WndProc is Window::WndProc (ClassInfo() fills one
// in), then call Create(). Create() passes `this` as lpParam; on WM_NCCREATE
// the WndProc stores it in GWLP_USERDATA and from then on routes every
// message to this object. GWLP_USERDATA is therefore not available to
// subclasses. MDI children created through MdiChild work the same way (their
// lpParam arrives inside an MDICREATESTRUCT).
//
// Dispatch order for each message:
//   1. ProcessMessageMap (BEGIN_MSG_MAP ... END_MSG_MAP), if it handles it;
//   2. the virtual handler for the message, if it returns true;
//   3. OnMessage, if it returns true;
//   4. DefaultProc (DefWindowProcW; MdiFrame/MdiChild use the MDI versions).
// The whole dispatch runs inside one try/catch (spec 7.2): an exception is
// written to OutputDebugStringW and shown in a MessageBox, and the message
// loop continues. An exception from WM_CREATE fails CreateWindowExW.
//
// Lifetime: the object must outlive its window. After WM_NCDESTROY the HWND
// is detached and OnFinalMessage() runs, the place for `delete this` when a
// window owns itself (the OWL `new TWindow` pattern). Destroying the object
// first destroys the window.
//
#pragma once

#include <Windows.h>

namespace win32fx {

class Window {
public:
    Window() noexcept = default;
    virtual ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // A WNDCLASSEXW for RegisterClassExW with lpfnWndProc = Window::WndProc,
    // an arrow cursor and a COLOR_WINDOW background; adjust before registering.
    static WNDCLASSEXW ClassInfo(HINSTANCE inst, const wchar_t* className) noexcept;

    // CreateWindowExW with this object as the instance. Null on failure.
    HWND Create(DWORD exStyle, const wchar_t* className, const wchar_t* title, DWORD style,
                int x, int y, int cx, int cy, HWND parent, HMENU menuOrId, HINSTANCE inst);

    HWND hwnd() const noexcept { return hwnd_; }
    operator HWND() const noexcept { return hwnd_; }

    // The Window that owns hwnd, or null (not ours, or not created yet).
    static Window* FromHandle(HWND hwnd) noexcept;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

protected:
    // Handlers return true when they handled the message (result 0).
    virtual bool OnCreate(LPCREATESTRUCTW) { return false; }
    virtual bool OnCommand(WORD /*id*/, WORD /*code*/, HWND /*ctl*/) { return false; }
    virtual bool OnSize(UINT /*type*/, int /*cx*/, int /*cy*/) { return false; }
    virtual bool OnDestroy() { return false; }
    // Everything else; set `result` and return true to handle.
    virtual bool OnMessage(UINT /*msg*/, WPARAM, LPARAM, LRESULT& /*result*/) { return false; }
    // Filled in by BEGIN_MSG_MAP / END_MSG_MAP.
    virtual bool ProcessMessageMap(UINT, WPARAM, LPARAM, LRESULT&) { return false; }

    virtual LRESULT DefaultProc(UINT msg, WPARAM wp, LPARAM lp);
    // After WM_NCDESTROY; hwnd() is already null.
    virtual void OnFinalMessage() {}

private:
    LRESULT Dispatch(UINT msg, WPARAM wp, LPARAM lp);

    HWND hwnd_ = nullptr;
};

} // namespace win32fx
