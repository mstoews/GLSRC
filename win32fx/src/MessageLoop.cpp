//
// MessageLoop.cpp - see MessageLoop.h.
//

#include "MessageLoop.h"

#include <algorithm>
#include <vector>

namespace win32fx {

namespace {
std::vector<HWND>& modeless()
{
    static std::vector<HWND> v;
    return v;
}
} // namespace

void AddModeless(HWND dialog)
{
    auto& v = modeless();
    if (std::find(v.begin(), v.end(), dialog) == v.end()) v.push_back(dialog);
}

void RemoveModeless(HWND dialog)
{
    auto& v = modeless();
    v.erase(std::remove(v.begin(), v.end(), dialog), v.end());
}

bool PreTranslate(MSG& msg, HWND frame, HWND mdiClient, HACCEL accel)
{
    if (mdiClient && TranslateMDISysAccel(mdiClient, &msg)) return true;
    if (accel && frame && TranslateAcceleratorW(frame, accel, &msg)) return true;
    const std::vector<HWND> dialogs = modeless();   // a dialog may close while handling the message
    for (HWND d : dialogs)
        if (IsWindow(d) && IsDialogMessageW(d, &msg)) return true;
    return false;
}

int RunMessageLoop(HWND frame, HWND mdiClient, HACCEL accel)
{
    MSG msg{};
    for (;;) {
        const BOOL got = GetMessageW(&msg, nullptr, 0, 0);
        if (got == 0 || got == -1) break;
        if (PreTranslate(msg, frame, mdiClient, accel)) continue;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

} // namespace win32fx
