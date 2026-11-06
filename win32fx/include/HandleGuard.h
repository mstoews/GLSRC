//
// HandleGuard.h - RAII ownership of a GDI/USER handle (spec 3.4).
//
//     FontGuard font(CreateFontIndirectW(&lf));
//     SendMessageW(ctl, WM_SETFONT, reinterpret_cast<WPARAM>(font.get()), TRUE);
//
// Move-only. The deleter runs once, when the guard is destroyed or reset.
//
#pragma once

#include <Windows.h>

#include <utility>

namespace win32fx {

template <class H, class Deleter>
class HandleGuard {
public:
    HandleGuard() noexcept = default;
    explicit HandleGuard(H h) noexcept : h_(h) {}
    ~HandleGuard() { reset(); }

    HandleGuard(const HandleGuard&) = delete;
    HandleGuard& operator=(const HandleGuard&) = delete;
    HandleGuard(HandleGuard&& o) noexcept : h_(std::exchange(o.h_, H{})) {}
    HandleGuard& operator=(HandleGuard&& o) noexcept
    {
        if (this != &o) reset(std::exchange(o.h_, H{}));
        return *this;
    }

    H get() const noexcept { return h_; }
    explicit operator bool() const noexcept { return h_ != H{}; }

    // Gives up ownership without deleting.
    [[nodiscard]] H release() noexcept { return std::exchange(h_, H{}); }

    void reset(H h = H{}) noexcept
    {
        if (h_ && h_ != h) Deleter{}(h_);
        h_ = h;
    }

private:
    H h_{};
};

struct GdiObjectDeleter { void operator()(HGDIOBJ h) const noexcept { DeleteObject(h); } };
struct MenuDeleter      { void operator()(HMENU h) const noexcept { DestroyMenu(h); } };
struct IconDeleter      { void operator()(HICON h) const noexcept { DestroyIcon(h); } };
struct DcDeleter        { void operator()(HDC h) const noexcept { DeleteDC(h); } };

using MenuGuard   = HandleGuard<HMENU, MenuDeleter>;
using FontGuard   = HandleGuard<HFONT, GdiObjectDeleter>;
using BitmapGuard = HandleGuard<HBITMAP, GdiObjectDeleter>;
using BrushGuard  = HandleGuard<HBRUSH, GdiObjectDeleter>;
using PenGuard    = HandleGuard<HPEN, GdiObjectDeleter>;
using RgnGuard    = HandleGuard<HRGN, GdiObjectDeleter>;
using IconGuard   = HandleGuard<HICON, IconDeleter>;
// A DC from CreateCompatibleDC / CreateDCW (not GetDC, which needs ReleaseDC).
using MemDcGuard  = HandleGuard<HDC, DcDeleter>;

} // namespace win32fx
